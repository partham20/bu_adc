#include "bu_runtime.h"
#include "data_structures.h"
#include "can_module.h"

// Global branch runtime data for all 18 CTs
BranchRuntimeData branchData[NumberOfBranches];

// Initialize branch runtime data with dummy values
void initBranchRuntime(void)
{
    int i;
    for (i = 0; i < NumberOfBranches; i++)
    {
        // Initialize with dummy data - replace with actual measurements
        branchData[i].branchCurrent = 1250 + i * 50;              // 12.50A + increment
        branchData[i].branchMaxCurrent = 1500 + i * 50;           // 15.00A + increment
        branchData[i].branchMinCurrent = 500 + i * 25;            // 5.00A + increment
        branchData[i].branchCurrentTHD = 350 + i * 10;            // 3.5% + increment
        branchData[i].branchVoltageTHD = 250 + i * 5;             // 2.5% + increment
        branchData[i].branchKW = 2800 + i * 100;                  // 2800W + increment
        branchData[i].branchKVA = 3000 + i * 100;                 // 3000VA + increment
        branchData[i].branchKVAR = 800 + i * 50;                  // 800VAR + increment
        branchData[i].branchAvgKW = 2700 + i * 90;                // 2700W + increment
        branchData[i].branchPowerFactor = 950 + i * 2;            // 0.950 + increment
        branchData[i].branchCurrentDemandHourly = 1300 + i * 45;  // 13.00A + increment
        branchData[i].branchMaxCurrentDemand24Hour = 1600 + i * 55; // 16.00A + increment
        branchData[i].branchKWDemandHourly = 2900 + i * 110;      // 2900W + increment
        branchData[i].branchMaxKWDemand24Hour = 3200 + i * 120;   // 3200W + increment
        branchData[i].status = 0x0001 | (i & 0x000F);             // Status with CT index
    }
}

// Prepare CAN frame data for specific CT
void prepareBranchFrame(uint8_t ctNumber, uint8_t* frameData)
{
    // Clear frame data
    memset(frameData, 0, 64);

    // Validate CT number
    if (ctNumber >= NumberOfBranches)
        return;

    // Frame header (Bytes 0-3)
    frameData[0] = 0x01;                // Version
    frameData[1] = 0x01;                // S-Board number
    frameData[2] = address;             // BU Board number
    frameData[3] = ctNumber;            // CT number

    // Branch parameter data (Bytes 4-35)
    BranchRuntimeData* data = &branchData[ctNumber];

    // Bytes 4-5: Branch Current
    frameData[4] = data->branchCurrent & 0xFF;
    frameData[5] = (data->branchCurrent >> 8) & 0xFF;

    // Bytes 6-7: Branch Max Current
    frameData[6] = data->branchMaxCurrent & 0xFF;
    frameData[7] = (data->branchMaxCurrent >> 8) & 0xFF;

    // Bytes 8-9: Branch Min Current
    frameData[8] = data->branchMinCurrent & 0xFF;
    frameData[9] = (data->branchMinCurrent >> 8) & 0xFF;

    // Bytes 10-11: SKIP (gap in frame format)

    // Bytes 12-13: Branch Current THD
    frameData[12] = data->branchCurrentTHD & 0xFF;
    frameData[13] = (data->branchCurrentTHD >> 8) & 0xFF;

    // Bytes 14-15: Branch Voltage THD
    frameData[14] = data->branchVoltageTHD & 0xFF;
    frameData[15] = (data->branchVoltageTHD >> 8) & 0xFF;

    // Bytes 16-17: Branch KW
    frameData[16] = data->branchKW & 0xFF;
    frameData[17] = (data->branchKW >> 8) & 0xFF;

    // Bytes 18-19: Branch KVA
    frameData[18] = data->branchKVA & 0xFF;
    frameData[19] = (data->branchKVA >> 8) & 0xFF;

    // Bytes 20-21: Branch KVAR
    frameData[20] = data->branchKVAR & 0xFF;
    frameData[21] = (data->branchKVAR >> 8) & 0xFF;

    // Bytes 22-23: Branch Average KW
    frameData[22] = data->branchAvgKW & 0xFF;
    frameData[23] = (data->branchAvgKW >> 8) & 0xFF;

    // Bytes 24-25: Branch Power Factor
    frameData[24] = data->branchPowerFactor & 0xFF;
    frameData[25] = (data->branchPowerFactor >> 8) & 0xFF;

    // Bytes 26-27: Branch Current Demand Hourly
    frameData[26] = data->branchCurrentDemandHourly & 0xFF;
    frameData[27] = (data->branchCurrentDemandHourly >> 8) & 0xFF;

    // Bytes 28-29: Branch Max Current Demand 24Hour
    frameData[28] = data->branchMaxCurrentDemand24Hour & 0xFF;
    frameData[29] = (data->branchMaxCurrentDemand24Hour >> 8) & 0xFF;

    // Bytes 30-31: Branch KW Demand Hourly
    frameData[30] = data->branchKWDemandHourly & 0xFF;
    frameData[31] = (data->branchKWDemandHourly >> 8) & 0xFF;

    // Bytes 32-33: Branch Max KW Demand 24Hour
    frameData[32] = data->branchMaxKWDemand24Hour & 0xFF;
    frameData[33] = (data->branchMaxKWDemand24Hour >> 8) & 0xFF;

    // Bytes 34-35: Status
    frameData[34] = data->status & 0xFF;
    frameData[35] = (data->status >> 8) & 0xFF;

    // Bytes 36-63: Already zeroed by memset
}

// Transmit all 18 branch parameter frames
void transmitBranchParameters(void)
{
    MCAN_TxBufElement TxMsg;
    uint8_t frameData[64];
    int i;

    // Send frame for each CT (0-17)
    for (i = 0; i < NumberOfBranches; i++)
    {
        // Prepare frame data for this CT
        prepareBranchFrame(i, frameData);

        // Setup CAN message
        memset(&TxMsg, 0, sizeof(MCAN_TxBufElement));
        TxMsg.id = ((uint32_t)(5 + address)) << 18U;  // Use existing ID scheme
        TxMsg.brs = 0x1;
        TxMsg.dlc = 15;  // 64 bytes
        TxMsg.rtr = 0;
        TxMsg.xtd = 0;
        TxMsg.fdf = 0x1;
        TxMsg.esi = 0U;
        TxMsg.efc = 1U;
        TxMsg.mm = 0xAAU;

        // Copy frame data to CAN message
        memcpy(TxMsg.data, frameData, 64);

        // Write and transmit
        MCAN_writeMsgRam(MCANA_DRIVER_BASE, MCAN_MEM_TYPE_BUF, 0, &TxMsg);
        MCAN_txBufAddReq(MCANA_DRIVER_BASE, 0U);

        // Wait for transmission to complete
        while (MCAN_getTxBufReqPend(MCANA_DRIVER_BASE))
        {
        }

        // Small delay between frames (adjust as needed)
        DEVICE_DELAY_US(10000);  // 10ms delay
    }
}

// Start branch transmission - call this periodically
void startBranchTransmission(void)
{
    // Update dummy data (increment values to show change)
    static uint16_t updateCounter = 0;
    updateCounter++;

    int i;
    for (i = 0; i < NumberOfBranches; i++)
    {
        // Simulate changing measurements
        branchData[i].branchCurrent += (updateCounter % 10);
        branchData[i].branchKW += (updateCounter % 50);
        // Add more updates as needed
    }

    // Transmit all branch parameters
    transmitBranchParameters();
}
