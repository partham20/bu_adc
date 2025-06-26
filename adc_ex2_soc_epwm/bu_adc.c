#include "bu_adc.h"
#include "CPU1_FLASH/syscfg/board.h"
#include <math.h>

#define SAMPLE_COUNT_THRESHOLD  100

// Globals
DWW_CHANNEL_PARAMETER Branch;
int m = 0;

// Private Globals
static uint32_t myADCResults[NUM_CHANNELS];
static unsigned long dwSumPhaseVolt[NUM_CHANNELS] = {0};
static unsigned int sampleCount = 0;
static float calib = 5265.0f;


void initEPWM(void)
{
    // Stop the timer first for configuration
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);

    // Disable ADC trigger during configuration
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

    // Configure the time base
    // Set period for a 20kHz sampling frequency (150MHz / (4*4) / 20kHz = 468.75)
    // Let's use a round number. For 50kHz -> 150MHz / (4*4) / 50kHz = 187.5
    // Let's stick to the original 2999 for now.
    EPWM_setTimeBasePeriod(EPWM1_BASE, 2999);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 1500);
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_4, EPWM_HSCLOCK_DIVIDER_4);

    // Configure ADC trigger
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);

    // Reset the counter to 0
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0);

    // Enable ADC trigger
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

    // Start the timer
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);
}

__interrupt void adcA1ISR(void)
{
    m++;
    int i;
    float avgSquare, rms;
    unsigned long square;

    // Read ADC conversion results for all configured SOCs
    myADCResults[0]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC0);
    myADCResults[1]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC1);
    myADCResults[2]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC2);
    myADCResults[3]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC3);
    myADCResults[4]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC4);
    myADCResults[5]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC5);
    myADCResults[6]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC6);
    myADCResults[7]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC7);
    myADCResults[8]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC8);
    myADCResults[9]  = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC9);
    myADCResults[10] = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC10);
    myADCResults[11] = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC11);
    myADCResults[12] = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC12);
    myADCResults[13] = ADC_readResult(myADC0_RESULT_BASE, myADC0_SOC13);

    myADCResults[14] = ADC_readResult(myADC1_RESULT_BASE, myADC1_SOC0);
    myADCResults[15] = ADC_readResult(myADC1_RESULT_BASE, myADC1_SOC1);
    myADCResults[16] = ADC_readResult(myADC1_RESULT_BASE, myADC1_SOC2);
    myADCResults[17] = ADC_readResult(myADC1_RESULT_BASE, myADC1_SOC3);

    // Process results
    for(i = 0; i < NUM_CHANNELS; i++)
    {
        // Subtract offset (assuming 12-bit ADC with 2048 offset)
        square = (unsigned long)((int32_t)myADCResults[i] - 2048) * ((int32_t)myADCResults[i] - 2048);
        dwSumPhaseVolt[i] += square;
    }

    sampleCount++;

    if(sampleCount >= SAMPLE_COUNT_THRESHOLD)
    {
        for(i = 0; i < NUM_CHANNELS; i++)
        {
            avgSquare = (float)dwSumPhaseVolt[i] / (float)SAMPLE_COUNT_THRESHOLD;
            rms = sqrtf(avgSquare) * calib / 10000.0f;
            Branch.dww_channel[i].RMS = (unsigned int)rms;

            // Reset accumulator for next set of samples
            dwSumPhaseVolt[i] = 0;
        }
        sampleCount = 0;
    }

    // Clear the ADC interrupt flags
    ADC_clearInterruptStatus(myADC0_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(myADC1_BASE, ADC_INT_NUMBER1);

    // Check and clear overflow flags if necessary
    if(true == ADC_getInterruptOverflowStatus(myADC0_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(myADC0_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(myADC0_BASE, ADC_INT_NUMBER1);
    }
    if(true == ADC_getInterruptOverflowStatus(myADC1_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(myADC1_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(myADC1_BASE, ADC_INT_NUMBER1);
    }

    // Acknowledge the interrupt to the PIE
    Interrupt_clearACKGroup(INT_myADC0_1_INTERRUPT_ACK_GROUP);
}
