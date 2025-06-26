#ifndef RUNTIME_H
#define RUNTIME_H

#include "config.h"
#include "data_structures.h"


// Runtime functions
void processVoltageFrame(uint8_t phase, uint8_t frameSequence, uint8_t* frameData);
void copyVoltageRxFIFO(void);
bool isVoltageDataComplete(void);
void resetVoltageCollection(void);
void syncVoltageData(void);

// Global voltage data - two instances
extern VoltageData runtimeVoltageData;    // Runtime collection instance
extern VoltageData syncedVoltageData;     // Synced complete data instance
extern bool voltageDataReady;
extern bool voltageFrameReceived;
extern bool allVoltagesReceived;

#endif // RUNTIME_H
