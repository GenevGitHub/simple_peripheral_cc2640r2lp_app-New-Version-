#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "simple_peripheral.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <icall.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/GPIO.h>
#include <ti/sysbios/knl/Task.h>
/*********************************************************************
*  EXTERNAL VARIABLES
*/
/*********************************************************************
 * CONSTANTS
 */
#define SBP_MC_GATT_EVT                        0x0020
#define SBP_MC_ADV_EVT                         0x0040
#define PWR_MGNT_STACK_SIZE                    128
#define PWR_MGNT_PRIORITY                      5
//typedef
// This set of data is stored in ram. It is used to store variables from Motor Controller Unit
typedef struct mcuData{
        uint16_t voltage_mV;
        uint16_t current_mA;
        uint16_t speed_rpm;
        int8_t heatSinkTemperature_Celcius;
        int8_t motorTemperature_Celcius;
        uint8_t count_hf;
}MCUD_t;

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * FUNCTIONS
 */
extern uint8_t Boot();
extern void motorcontrol_init(void);
extern void motorcontrol_registerCB(simplePeripheral_bleCBs_t *obj);
extern void motorcontrol_setGatt(uint16_t serviceUUID, uint8_t charteristics, uint8_t payloadLength, uint8_t* payload);
extern void motorcontrol_speedModeChgCB(uint16_t torqueIQ, uint16_t allowableSpeed, uint16_t rampRate);
extern void motorcontrol_dataAnalysis_sampling(uint8_t x_hf);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEPERIPHERAL_H */
