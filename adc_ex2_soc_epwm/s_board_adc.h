//
// DWW_FIRMWARE.h - Power Monitoring System Header File
//

#ifndef DWW_FIRMWARE_H
#define DWW_FIRMWARE_H

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "dww_parameter.h"

//
// Defines
//
#define NUM_CHANNELS            18
#define SAMPLE_COUNT_THRESHOLD  100

#define DEVICE_GPIO_PIN_LED1        29U             // GPIO number for LED1
#define DEVICE_GPIO_CFG_LED1        GPIO_29_GPIO29  // "pinConfig" for LED1

//
// Globals
//
extern int k;
extern  signed int myADC0Results[NUM_CHANNELS];   // Buffer for raw ADC results

// Removed unused index and bufferFull variables

// Global accumulator for each channels sum of squares
extern unsigned long dwSumPhaseVolt[NUM_CHANNELS];

// Sample counter for RMS computation
extern unsigned int sampleCount ;

// Global variable for calibration factor (adjust as needed)
extern float calib;

//
// Global Declaration for Demand and other parameters (unchanged)
//
extern DWW_CHANNEL_PARAMETER Branch;
extern unsigned int struct_size;
extern unsigned long dwwMathBuff;
extern unsigned int max_curr;
extern  int i;
extern unsigned int demand_chk_1sec, demand_chk_1hr, demand_chk_24hr;
extern unsigned int wDemandSumCntr[4];
extern unsigned long dw_CurrDemandSum[NUM_CHANNELS];
extern dww_structflag *sFlag;

// Other global variables remain as originally defined
extern unsigned int adcAResult[NUM_CHANNELS];
extern unsigned int dww_RMS_Current;
extern unsigned int count;

extern int root;
extern int pinstate;

extern volatile bool syncSignalReceived ;  // Flag to indicate sync signal

//
// Function Prototypes
//
void initEPWM(void);
void dww_Loadper_Calc(void);
void dww_max_min_Calc(void);
void dww_timer(void);
void dww_demand_Calc(void);


void adc_start_process ();
void adc_init();
__interrupt void adcA1ISR(void);

extern uint16_t pinState;




#endif // DWW_FIRMWARE_H
