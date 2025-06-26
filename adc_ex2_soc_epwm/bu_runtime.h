#ifndef BRANCH_RUNTIME_H
#define BRANCH_RUNTIME_H

#include "config.h"

// Branch runtime parameters structure
typedef struct {
    uint16_t branchCurrent;              // Current (A * 100)
    uint16_t branchMaxCurrent;           // Max Current (A * 100)
    uint16_t branchMinCurrent;           // Min Current (A * 100)
    uint16_t branchCurrentTHD;           // Current THD (% * 100)
    uint16_t branchVoltageTHD;           // Voltage THD (% * 100)
    uint16_t branchKW;                   // Power (W)
    uint16_t branchKVA;                  // Apparent Power (VA)
    uint16_t branchKVAR;                 // Reactive Power (VAR)
    uint16_t branchAvgKW;                // Average Power (W)
    uint16_t branchPowerFactor;          // Power Factor (* 1000)
    uint16_t branchCurrentDemandHourly;  // Current Demand Hourly (A * 100)
    uint16_t branchMaxCurrentDemand24Hour; // Max Current Demand 24H (A * 100)
    uint16_t branchKWDemandHourly;       // KW Demand Hourly (W)
    uint16_t branchMaxKWDemand24Hour;    // Max KW Demand 24H (W)
    uint16_t status;                     // Status flags
} BranchRuntimeData;

// Functions
void initBranchRuntime(void);
void transmitBranchParameters(void);
void prepareBranchFrame(uint8_t ctNumber, uint8_t* frameData);
void startBranchTransmission(void);

// Global data
extern BranchRuntimeData branchData[NumberOfBranches];

#endif // BRANCH_RUNTIME_H
