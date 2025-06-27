

#ifndef DWW_POWER_H_
#define DWW_POWER_H_

//
// Included Files
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dww_parameter.h"

//
// Defines
//
#define BRANCH_VOLT_DEFAULT     230     // Default branch voltage in Volts
#define MAX_CHANNELS            18      // Maximum number of channels
#define POWER_SCALE_FACTOR      1000    // Scaling factor for power calculations

//
// Existing DWW Parameter Structures (from dww_parameter.h)
//
#ifndef MAX_CT_LENGTH
#define MAX_CT_LENGTH 18
#endif

// Flag structure for timing control
typedef struct {
    unsigned int demand_chk_1sec :1;
    unsigned int demand_chk_1hr :1;
    unsigned int demand_chk_24hr :1;
} dww_structflag;

// Individual channel parameter structure
typedef struct {
    unsigned int RMS;
    unsigned int Loadper;
    unsigned int Mincurrent;
    unsigned int Maxcurrent;
    unsigned int CTCurrDemand_1hr;
    unsigned int CTMaxCurrDemand_1hr;
    unsigned int CTMaxCurrDemand_24hr;
} DWW_BRANCH_PARAMETER;

// Main channel parameter structure
typedef struct {
    DWW_BRANCH_PARAMETER dww_channel[MAX_CT_LENGTH];
} DWW_CHANNEL_PARAMETER;

//
// Type Definitions - Power Extension Structures
//
// Extended power calculation structure to work with existing DWW_BRANCH_PARAMETER
typedef struct {
    unsigned int voltage;               // Branch voltage in Volts
    unsigned int active_power;         // Active power (kW * 1000)
    unsigned int apparent_power;       // Apparent power (kVA * 1000)
    unsigned int reactive_power;       // Reactive power (kVAR * 1000)
    unsigned int power_factor;         // Power factor * 1000
} dww_power_extension_t;

// Power data structure that extends the existing channel structure
typedef struct {
    DWW_CHANNEL_PARAMETER *channel_params;  // Pointer to existing channel data
    dww_power_extension_t power_ext[MAX_CHANNELS]; // Power calculations
    unsigned int channel_voltages[MAX_CHANNELS];   // Individual channel voltages
    unsigned int total_active_power;    // Total kW * 1000
    unsigned int total_apparent_power;  // Total kVA * 1000
    unsigned int total_reactive_power;  // Total kVAR * 1000
    unsigned int overall_power_factor;  // Overall PF * 1000
} dww_power_data_t;

// Legacy structure compatibility (commented out sections from dww_parameter.h)
#if 0
typedef struct {
    unsigned int dww_RMS_Current[18];
    unsigned int dww_branch_Volt[18];
    unsigned int dww_max_curr[18];
    unsigned int dww_min_curr[18];
    unsigned int dww_branch_kW[18];
    unsigned int dww_branch_kVA[18];
    unsigned int dww_branch_kVAR[18];
    unsigned int dww_Loadper[18];
    unsigned int dww_CTCurrDemand_1hr[18];
    unsigned int dww_CTCurrDemand_24hr[18];
} dww_branch_parameter;

typedef struct {
    unsigned int Channel_1;
    unsigned int Channel_2;
    unsigned int Channel_3;
    unsigned int Channel_4;
    unsigned int Channel_5;
    unsigned int Channel_6;
    unsigned int Channel_7;
    unsigned int Channel_8;
    unsigned int Channel_9;
    unsigned int Channel_10;
    unsigned int Channel_11;
    unsigned int Channel_12;
    unsigned int Channel_13;
    unsigned int Channel_14;
    unsigned int Channel_15;
    unsigned int Channel_16;
    unsigned int Channel_17;
    unsigned int Channel_18;
} DWW_BRANCH_PARAMETER_LEGACY;

typedef union {
    unsigned int array[18];
    DWW_BRANCH_PARAMETER_LEGACY word;
} DWW_BRANCH_UNION;
#endif

//
// Global Variables (External Declarations)
//
extern dww_power_data_t power_data;
extern unsigned int dwMathBuff;
extern unsigned int kw_gain;
extern int i;

//
// Function Prototypes
//

/**
 * @brief Calculate active power (kW) for all channels using existing channel structure
 * @param channel_params Pointer to DWW_CHANNEL_PARAMETER structure
 * @param power_data Pointer to power data structure
 * @return None
 */
void dww_kW_Calc(DWW_CHANNEL_PARAMETER *channel_params, dww_power_data_t *power_data);

/**
 * @brief Calculate apparent power (kVA) for all channels using existing channel structure
 * @param channel_params Pointer to DWW_CHANNEL_PARAMETER structure
 * @param power_data Pointer to power data structure
 * @return None
 */
void dww_kVA_Calc(DWW_CHANNEL_PARAMETER *channel_params, dww_power_data_t *power_data);

/**
 * @brief Calculate reactive power (kVAR) for all channels using existing channel structure
 * @param channel_params Pointer to DWW_CHANNEL_PARAMETER structure
 * @param power_data Pointer to power data structure
 * @return None
 */
void dww_kVAR_Calc(DWW_CHANNEL_PARAMETER *channel_params, dww_power_data_t *power_data);

/**
 * @brief Calculate power factor for all channels
 * @param power_data Pointer to power data structure
 * @return None
 */
void dww_PowerFactor_Calc(dww_power_data_t *power_data);

/**
 * @brief Calculate total power parameters (sum of all channels)
 * @param power_data Pointer to main power data structure
 * @return None
 */
void dww_TotalPower_Calc(dww_power_data_t *power_data);

/**
 * @brief Initialize power calculation module with existing channel structure
 * @param power_data Pointer to main power data structure
 * @param channel_params Pointer to existing DWW_CHANNEL_PARAMETER structure
 * @param default_voltage Default voltage to use for all channels
 * @return None
 */
void dww_Power_Init(dww_power_data_t *power_data,
                   DWW_CHANNEL_PARAMETER *channel_params,
                   unsigned int default_voltage);

/**
 * @brief Set voltage for a specific channel
 * @param power_data Pointer to power data structure
 * @param channel_index Channel index (0-17)
 * @param voltage Voltage value in Volts
 * @return 0 on success, -1 on error
 */
int dww_SetChannelVoltage(dww_power_data_t *power_data,
                         unsigned int channel_index,
                         unsigned int voltage);

/**
 * @brief Get power values for a specific channel
 * @param power_data Pointer to power data structure
 * @param channel_index Channel index (0-17)
 * @param kw Pointer to store kW value
 * @param kva Pointer to store kVA value
 * @param kvar Pointer to store kVAR value
 * @param pf Pointer to store power factor
 * @return 0 on success, -1 on error
 */
int dww_GetChannelPower(dww_power_data_t *power_data,
                       unsigned int channel_index,
                       unsigned int *kw,
                       unsigned int *kva,
                       unsigned int *kvar,
                       unsigned int *pf);

/**
 * @brief Update power calculations for all channels (main calculation function)
 * @param channel_params Pointer to DWW_CHANNEL_PARAMETER structure (contains RMS values)
 * @param power_data Pointer to power data structure
 * @return None
 */
void dww_UpdateAllPowerCalculations(DWW_CHANNEL_PARAMETER *channel_params,
                                   dww_power_data_t *power_data);

//
// Inline utility functions
//

/**
 * @brief Convert power value to floating point (divide by 1000)
 * @param power_value Power value * 1000
 * @return Floating point power value
 */
static inline float dww_PowerToFloat(unsigned int power_value)
{
    return (float)power_value / 1000.0f;
}

/**
 * @brief Convert floating point power to integer (multiply by 1000)
 * @param power_float Floating point power value
 * @return Power value * 1000
 */
static inline unsigned int dww_FloatToPower(float power_float)
{
    return (unsigned int)(power_float * 1000.0f);
}

/**
 * @brief Check if channel index is valid
 * @param channel_index Channel index to check
 * @return 1 if valid, 0 if invalid
 */
static inline int dww_IsValidChannel(unsigned int channel_index)
{
    return (channel_index < MAX_CHANNELS) ? 1 : 0;
}

#endif /* DWW_POWER_H_ */
