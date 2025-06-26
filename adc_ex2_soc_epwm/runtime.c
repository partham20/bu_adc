#include "runtime.h"
#include "data_structures.h"
#include "can_module.h"

// Global voltage data structures - two instances
VoltageData runtimeVoltageData;     // Runtime collection instance
VoltageData syncedVoltageData;      // Synced complete data instance
bool voltageDataReady = false;
bool voltageFrameReceived = false;
bool allVoltagesReceived = false;

// Frame tracking variables
static uint8_t receivedFramesR = 0;
static uint8_t receivedFramesY = 0;
static uint8_t receivedFramesB = 0;
static uint8_t phaseRMask = 0;
static uint8_t phaseYMask = 0;
static uint8_t phaseBMask = 0;

// Process individual voltage frame
void processVoltageFrame(uint8_t phase, uint8_t frameSequence, uint8_t* frameData)
{
    uint16_t voltageValues[31];
    uint16_t* targetArray;
    uint8_t baseIndex;
    uint8_t frameBit;
    uint8_t* maskPtr;

    // Convert CAN data to 16-bit voltage values
    int i;
    for (i = 0; i < 31; i++)
    {
        voltageValues[i] = (frameData[2 + i * 2] << 8) | frameData[2 + i * 2 + 1];
    }

    // Determine target array and tracking variables based on phase
    switch (phase)
    {
        case 1: // Phase R
            targetArray = runtimeVoltageData.phaseR;
            maskPtr = &phaseRMask;
            break;

        case 2: // Phase Y
            targetArray = runtimeVoltageData.phaseY;
            maskPtr = &phaseYMask;
            break;

        case 3: // Phase B
            targetArray = runtimeVoltageData.phaseB;
            maskPtr = &phaseBMask;
            break;

        default:
            return; // Invalid phase
    }

    // Validate frame sequence (1-7 for each phase)
    if (frameSequence < 1 || frameSequence > 7)
    {
        return;
    }

    frameBit = 1 << (frameSequence - 1);

    // Check if frame already received
    if (*maskPtr & frameBit)
    {
        return; // Frame already processed
    }

    // Calculate base index for this frame
    baseIndex = (frameSequence - 1) * 31;

    // Handle last frame (frame 7) which has only 29 values instead of 31
    uint8_t valuesToCopy = (frameSequence == 7) ? 29 : 31;

    // Copy voltage values to appropriate position in target array
    for (i = 0; i < valuesToCopy && (baseIndex + i) < 200; i++)
    {
        targetArray[baseIndex + i] = voltageValues[i];
    }

    // Mark frame as received
    *maskPtr |= frameBit;

    // Update frame counters
    switch (phase)
    {
        case 1:
            receivedFramesR++;
            break;
        case 2:
            receivedFramesY++;
            break;
        case 3:
            receivedFramesB++;
            break;
    }
}

// Copy and process voltage data from RX FIFO
void copyVoltageRxFIFO(void)
{
    MCAN_RxFIFOStatus rxFIFO0Status;
    rxFIFO0Status.num = MCAN_RX_FIFO_NUM_0;

    // Get RX FIFO 0 status
    MCAN_getRxFIFOStatus(MCANA_DRIVER_BASE, &rxFIFO0Status);

    // Process all messages in RX FIFO 0
    while (rxFIFO0Status.fillLvl)
    {
        uint32_t getIndex = rxFIFO0Status.getIdx;

        // Read message from RX FIFO 0
        MCAN_readMsgRam(MCANA_DRIVER_BASE, MCAN_MEM_TYPE_FIFO, 0U,
                        MCAN_RX_FIFO_NUM_0, &rxFIFO0Msg[getIndex]);

        // Acknowledge reading the FIFO element
        MCAN_writeRxFIFOAck(MCANA_DRIVER_BASE, MCAN_RX_FIFO_NUM_0, getIndex);

        // Extract frame information
        uint8_t phase = rxFIFO0Msg[getIndex].data[0];
        uint8_t frameSequence = rxFIFO0Msg[getIndex].data[1];

        // Process the voltage frame
        processVoltageFrame(phase, frameSequence, rxFIFO0Msg[getIndex].data);

        // Update RX FIFO 0 status
        MCAN_getRxFIFOStatus(MCANA_DRIVER_BASE, &rxFIFO0Status);
    }

    // Check if all voltage data is complete
    if (isVoltageDataComplete())
    {
        voltageDataReady = true;
        allVoltagesReceived = true;

        // Sync runtime data to synced instance
        syncVoltageData();
    }
}

// Check if all voltage data has been received
bool isVoltageDataComplete(void)
{
    // Check if all 7 frames received for each phase (0x7F = 0111 1111)
    return (phaseRMask == 0x7F && phaseYMask == 0x7F && phaseBMask == 0x7F);
}

// Sync runtime voltage data to synced instance
void syncVoltageData(void)
{
    memcpy(&syncedVoltageData, &runtimeVoltageData, sizeof(VoltageData));
}

// Reset voltage collection for new sequence
void resetVoltageCollection(void)
{
    // Clear runtime voltage data only
    memset(&runtimeVoltageData, 0, sizeof(VoltageData));

    // Reset tracking variables
    receivedFramesR = 0;
    receivedFramesY = 0;
    receivedFramesB = 0;
    phaseRMask = 0;
    phaseYMask = 0;
    phaseBMask = 0;
    voltageDataReady = false;
    voltageFrameReceived = false;
    allVoltagesReceived = false;
}
