
//
// Included Files
//
#include "s_board_adc.h"

int k;

signed int myADC0Results[NUM_CHANNELS];   // Buffer for raw ADC results

// Removed unused index and bufferFull variables

// Global accumulator for each channels sum of squares
unsigned long dwSumPhaseVolt[NUM_CHANNELS] = {0};

// Sample counter for RMS computation
unsigned int sampleCount = 0;

// Global variable for calibration factor (adjust as needed)
float calib = 5265;

//
// Global Declaration for Demand and other parameters (unchanged)
//
DWW_CHANNEL_PARAMETER Branch;
unsigned int struct_size;
unsigned long dwwMathBuff;
unsigned int max_curr;
static int i;
unsigned int demand_chk_1sec, demand_chk_1hr, demand_chk_24hr;
unsigned int wDemandSumCntr[4];
unsigned long dw_CurrDemandSum[NUM_CHANNELS];
dww_structflag *sFlag;

// Other global variables remain as originally defined
unsigned int adcAResult[NUM_CHANNELS];
unsigned int dww_RMS_Current;
unsigned int count = 0;

int root;
int pinstate;

volatile bool syncSignalReceived = false;  // Flag to indicate sync signal

//
// Function Prototypes
//
void initEPWM(void);
void dww_Loadper_Calc(void);
void dww_max_min_Calc(void);
void dww_timer(void);
void dww_demand_Calc(void);

uint16_t pinState;

int a=0;


//
// Main

signed long buf[200], buf_v[200], buf_c[200];
uint8_t countSample;



/*
 * configured GPIO28 for the sync signal
 */
void configureGPIO28(void)
{
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP | GPIO_PIN_TYPE_INVERT);
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(28, GPIO_QUAL_SYNC);

}
//
// Function to configure ePWM1 to generate the SOC.
//
void initEPWM(void)
{
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);
    //EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 2048);
    //EPWM_setTimeBasePeriod(EPWM1_BASE, 4095);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 1500);
    EPWM_setTimeBasePeriod(EPWM1_BASE, 2999);
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_4, EPWM_HSCLOCK_DIVIDER_4);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);

    /*
     * Added logic to calculate the frequency
     */
    EPWM_setPeriodLoadMode(EPWM1_BASE,EPWM_PERIOD_DIRECT_LOAD);
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0);

}

//
// adcA1ISR - ADC A Interrupt 1 ISR
//
__interrupt void adcA1ISR(void)
{k++;

    int i;
    float avgSquare, rms;
    unsigned long square;


  //  GPIO_writePin(28, 0);

    /*
     * ADC Conversion for the Input, Output and Ground Current
     */
myADC0Results[0]  = (signed int)(ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0) - 2010);
   myADC0Results[1]  = (unsigned int)ADC_readResult(ADCERESULT_BASE, ADC_SOC_NUMBER2) - 2048;
   myADC0Results[2]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1) - 2048;
   myADC0Results[3]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2) - 2048;
   myADC0Results[4]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3) - 2048;
   myADC0Results[5]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4) - 2048;
   myADC0Results[6]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5) - 2048;
   myADC0Results[7]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER6) - 2048;
   myADC0Results[8]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER7) - 2048;
   myADC0Results[9]  = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER8) - 2048;
   myADC0Results[10] = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER9) - 2048;
   myADC0Results[11] = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER10) - 2048;
   myADC0Results[12] = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER11) - 2048;
   myADC0Results[13] = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER12) - 2048;
   myADC0Results[14] = (signed int)ADC_readResult(ADCERESULT_BASE, ADC_SOC_NUMBER3) - 2048;
   myADC0Results[15] = (signed int)ADC_readResult(ADCERESULT_BASE, ADC_SOC_NUMBER0) - 2048;
   myADC0Results[16] = (signed int)ADC_readResult(ADCERESULT_BASE, ADC_SOC_NUMBER1) - 2048;
   myADC0Results[17] = (signed int)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER13) - 2048;


   // For each channel, square the result and accumulate it.
      //
      for(i = 0; i < NUM_CHANNELS; i++)
      {
          square = (unsigned long)myADC0Results[i] * (unsigned long)myADC0Results[i];
          dwSumPhaseVolt[i] += square;
      }

      //
      // Increment the sample counter.
      //
      sampleCount++;

      //
      // Once 100 samples have been collected, compute the RMS for each channel.
      //
      if(sampleCount >= SAMPLE_COUNT_THRESHOLD)
      {
          for(i = 0; i < NUM_CHANNELS; i++)
          {
              avgSquare = (float)dwSumPhaseVolt[i] / (float)SAMPLE_COUNT_THRESHOLD;
              rms = sqrtf(avgSquare) * calib/10000;
              Branch.dww_channel[i].RMS = (unsigned int)rms;

              // Reset accumulator for next set of samples
              dwSumPhaseVolt[i] = 0;
          }
          sampleCount = 0;
      }






    ADC_clearInterruptStatus(myADC0_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(myADC1_BASE, ADC_INT_NUMBER1);

    if(ADC_getInterruptOverflowStatus(myADC0_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(myADC0_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(myADC0_BASE, ADC_INT_NUMBER1);
    }

    if(ADC_getInterruptOverflowStatus(myADC1_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(myADC1_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(myADC1_BASE, ADC_INT_NUMBER1);
    }

    Interrupt_clearACKGroup(INT_myADC0_1_INTERRUPT_ACK_GROUP);
}



void adc_init()

{

  //  struct_size = sizeof(primary);
    Board_init();

    configureGPIO28();

       /*
        * Enabling the watchdog after the reset
        */
       //SysCtl_enableWatchdog();

       //initEPWM();

       sampleCount = 0;

//       for(i = 0; i < PHASE_CURR; i++)
//       {
//           dwSumPhaseCurr[i] = 0;
//       }
//

       for(i = 0; i < 3; i++)
       {
           dwSumPhaseVolt[i] = 0;
       }

       /*
        * Added for the kw calculation
        */

//       for(i = 0; i < 5; i++)
//       {
//           dwSumPhaseWatt[i] = 0;
//       }

       EINT;
       ERTM;
       ADC_setVREF(ADCARESULT_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
       EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
       EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);
}


void adc_start_process ()
{
    //ADC functions
            a++;

             dww_Loadper_Calc();
             dww_max_min_Calc();
             dww_timer();
             dww_demand_Calc();




                    // Additional function can be placed here.

}




//
// Logic for Load Percentage Calculation (unchanged)
//
void dww_Loadper_Calc(void)
{
    for (i = 0; i < MAX_CT_LENGTH; i++)
    {
        dwwMathBuff = (unsigned long)(Branch.dww_channel[i].RMS * 1000);
        if (max_curr != 0)
        {
            Branch.dww_channel[i].Loadper = dwwMathBuff / max_curr;
        }
    }
}

//
// Logic for Maximum and Minimum Parameter Calculation (with added parentheses for clarity)
//
void dww_max_min_Calc(void)
{
    unsigned int i;
    for (i = 0; i < MAX_CT_LENGTH; i++)
    {
        if (((Branch.dww_channel[i].RMS != 0xFFFF) &&
             ((Branch.dww_channel[i].Maxcurrent < Branch.dww_channel[i].RMS) ||
              (Branch.dww_channel[i].Maxcurrent == 0xFFFF))))
        {
            Branch.dww_channel[i].Maxcurrent = Branch.dww_channel[i].RMS;
        }

        if (((Branch.dww_channel[i].RMS != 0x0000) &&
             ((Branch.dww_channel[i].Mincurrent > Branch.dww_channel[i].RMS) ||
              (Branch.dww_channel[i].Mincurrent == 0x0000))))
        {
            Branch.dww_channel[i].Mincurrent = Branch.dww_channel[i].RMS;
        }

        if (((Branch.dww_channel[i].CTCurrDemand_1hr != 0xFFFF) &&
             ((Branch.dww_channel[i].CTMaxCurrDemand_1hr < Branch.dww_channel[i].CTCurrDemand_1hr) ||
              (Branch.dww_channel[i].CTMaxCurrDemand_1hr == 0xFFFF))))
        {
            Branch.dww_channel[i].CTMaxCurrDemand_1hr = Branch.dww_channel[i].CTCurrDemand_1hr;
        }
    }
}

//
// Timer function (unchanged for now; adjust as per your timing requirements)
//
void dww_timer(void)
{
    if (demand_chk_1sec >= 270) // for 1 sec
    {
        sFlag->demand_chk_1sec = 1;
        demand_chk_1sec = 0;
        demand_chk_1hr++;
    }
    else
    {
        demand_chk_1sec++;
    }

    if(demand_chk_1hr >= 60) // Changed from 3600 to 60 for testing
    {
        sFlag->demand_chk_1hr = 1;
        demand_chk_1hr = 0;
        demand_chk_24hr++;
    }

    if(demand_chk_24hr >= 5) // Changed from 24 to 5 for testing
    {
        sFlag->demand_chk_24hr = 1;
        demand_chk_24hr = 0;
    }
}

//
// Demand Calculation function (unchanged)
//
void dww_demand_Calc(void)
{
    int i;
    if (sFlag->demand_chk_1sec == 1)
    {
        for (i = 0; i < MAX_CT_LENGTH; i++)
        {
            dw_CurrDemandSum[i] += Branch.dww_channel[i].RMS;
        }
        sFlag->demand_chk_1sec = 0;
        wDemandSumCntr[0]++;
    }

    if (sFlag->demand_chk_1hr == 1)
    {
        for (i = 0; i < MAX_CT_LENGTH; i++)
        {
            Branch.dww_channel[i].CTCurrDemand_1hr = (unsigned int)(dw_CurrDemandSum[i] / wDemandSumCntr[0]);
            dw_CurrDemandSum[i] = 0;
        }
        sFlag->demand_chk_1hr = 0;
        wDemandSumCntr[0] = 0;
    }

    if (sFlag->demand_chk_24hr == 1)
    {
        for (i = 0; i < MAX_CT_LENGTH; i++)
        {
            Branch.dww_channel[i].CTMaxCurrDemand_24hr = Branch.dww_channel[i].CTMaxCurrDemand_1hr;
            Branch.dww_channel[i].CTMaxCurrDemand_1hr = 0;
        }
        sFlag->demand_chk_24hr = 0;
    }
}

