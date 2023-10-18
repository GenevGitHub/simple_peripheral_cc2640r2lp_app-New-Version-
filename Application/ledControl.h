/*
 * ledControl.h
 *
 *  Created on: 30 Nov 2022
 *      Author: User
 */

#ifndef APPLICATION_LEDCONTROL_H_
#define APPLICATION_LEDCONTROL_H_
//
#ifdef __cplusplus
extern "C"
{
#endif
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
* CONSTANTS
*/
#define LED_POWER_LIGHT_ON                              60
#define LED_POWER_LIGHT_OFF                             100
//#define LEDCONTROL_INIT_TIME                            1500
//#define LEDCONTROL_REFRESH_TIME                         120

/*********************************************************************
 * @Structure ledControl_LedDisplayManager_t
 *
 * @brief     It defines a set of function pointer that the the library can access and control the device peripheral to manipulate the ALS
 *
 * @data      ledControl_open: Called when the application wants to open the ALS
 *            ledControl_close: Called when the application wants to close the ALS
 *            ledControl_transfer: Called when the application wants to transfer data to the ALS

 */

typedef void (*ledControl_open)(void);
typedef uint8_t (*ledControl_transfer)(uint_least8_t slave_address, void *writeBuffer, size_t writeSize, void *readBuffer, size_t readSize);
typedef void (*ledControl_close)(void);

typedef struct
{
    ledControl_open            ledControl_open;
    ledControl_transfer        ledControl_transfer;
    ledControl_close           ledControl;
}ledControl_ledDisplayManager_t;

/*********************************************************************
 * API FUNCTIONS
 */
extern void ledControl_registerLedDisplay( ledControl_ledDisplayManager_t *ledDisplayI2C );

/*********************************************************************
 *  Global Function declaration
 *
*/
extern void ledControl_init( void );

static void ledControl_taskFxn(UArg a0, UArg a1);

extern void ledControl_setAllOn( void );                                // Led All On
extern void ledControl_setAllOff( void );                               // Led All Off
extern void ledControl_setDashSpeed( uint8_t dashSpeed );               // Set Speed Digit 1 and Digit 2
extern void ledControl_changeDashSpeed();
extern void ledControl_setBatteryStatus( uint8_t batteryStatus );       // Set battery level
extern void ledControl_changeBatteryStatus(uint16_t gpt_taskCounter);
extern void ledControl_setSpeedMode( uint8_t speedMode );               // Set speed mode
extern void ledControl_changeSpeedMode();
extern void ledControl_setUnitSelectDash( uint8_t UnitSelectDash );     // Set Unit
extern void ledControl_changeUnit();
//extern void ledControl_setBLEStatus( uint8_t BLEStatus );               // Set BLE status
extern void ledControl_changeBLE(uint16_t gpt_taskCounter);
extern void ledControl_getError(uint8_t error_code);
extern void ledControl_ErrorDisplay();
extern void ledControl_setLightMode( uint8_t light_mode );           // Set light mode
extern void ledControl_changeLightMode();
extern void ledControl_setLightStatus( uint8_t light_status );          // Set Light Status
extern void ledControl_changeLightStatus();
extern void ledControl_setLEDPower( uint8_t ledPower );                 // Set LED Power Level / Brightness
extern void ledControl_changeLEDPower();

typedef void (*IS31FL3236A_Function)(uint8_t status_buf, uint8_t brightness_buf);
typedef enum {
    BRIGHTNESS,
    SPORTS_MODE, LEISURE_MODE, AMBLE_MODE, BLUETOOTH_LED, LIGHT_ON, AUTO_MODE, ATTENTION,
    BAR_1, BAR_2, BAR_3, BAR_4, BAR_5, KMPH, MPH,
    DIGIT_1_PIN_44, DIGIT_1_PIN_3, DIGIT_1_PIN_5, DIGIT_1_PIN_30, DIGIT_1_PIN_31, DIGIT_1_PIN_32, DIGIT_1_PIN_33,
    DIGIT_2_PIN_8, DIGIT_2_PIN_9,DIGIT_2_PIN_12, DIGIT_2_PIN_27, DIGIT_2_PIN_28,  DIGIT_2_PIN_29,DIGIT_2_PIN_13,
    ALL_BARS, BARS_4_ON, BARS_3_ON, BARS_2_ON, BARS_1_ON, BARS_0_ON,
    DIGIT_1_NO_0, DIGIT_1_NO_1, DIGIT_1_NO_2, DIGIT_1_NO_3, DIGIT_1_NO_4,
    DIGIT_1_NO_5, DIGIT_1_NO_6, DIGIT_1_NO_7, DIGIT_1_NO_8, DIGIT_1_NO_9,
    DIGIT_2_NO_0, DIGIT_2_NO_1, DIGIT_2_NO_2, DIGIT_2_NO_3, DIGIT_2_NO_4,
    DIGIT_2_NO_5, DIGIT_2_NO_6, DIGIT_2_NO_7, DIGIT_2_NO_8, DIGIT_2_NO_9,
    DIGIT_1_A, DIGIT_2_A, DIGIT_1_C, DIGIT_2_C, DIGIT_1_E, DIGIT_2_E,
    DIGIT_1_F, DIGIT_2_F, DIGIT_1_H, DIGIT_2_H, DIGIT_1_J, DIGIT_2_J,
    DIGIT_1_L, DIGIT_2_L, DIGIT_1_P, DIGIT_2_P, DIGIT_1_U, DIGIT_2_U,
    DIGIT_1_u, DIGIT_2_u, DIGIT_1_n, DIGIT_2_n, DIGIT_1_b, DIGIT_2_b,
    DIGIT_1_c, DIGIT_2_c, DIGIT_1_h, DIGIT_2_h, DIGIT_1_d, DIGIT_2_d,
    DIGIT_1_q, DIGIT_2_q, DIGIT_1_, DIGIT_2_,
    FUNCTION_COUNT
} IS31FL3236A_FunctionIndex;

extern IS31FL3236A_Function functionTable[FUNCTION_COUNT];


//
#ifdef __cplusplus
}
#endif
//
#endif /* APPLICATION_LEDCONTROL_H_ */
