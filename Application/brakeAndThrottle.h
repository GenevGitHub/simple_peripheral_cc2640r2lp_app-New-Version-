/*
 * brake.h
 *
 *  Created on: 14 Feb 2022
 *      Author: harrysiu
 */

#ifndef APPLICATION_BRAKEANDTHROTTLE_H_
#define APPLICATION_BRAKEANDTHROTTLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <math.h>
#include "motorControl.h"
/*********************************************************************
*  EXTERNAL VARIABLES
*/
/*********************************************************************
 * CONSTANTS
 */
// Speed limit protection selection
// * Direct Law -> speed limit protection is deactivated
// * Normal law -> speed limit protection is activated
#define BRAKE_AND_THROTTLE_NORMALLAW                              1
#define BRAKE_AND_THROTTLE_DIRECTLAW                              0

// The parameters BRAKE_AND_THROTTLE_ADC_SAMPLING_PERIOD & BRAKE_AND_THROTTLE_SAMPLES control the sensitivity of the throttle input to motor output
#define BRAKE_AND_THROTTLE_ADC_SAMPLING_PERIOD                    10   // 80: [time period in mS]
#define BRAKE_AND_THROTTLE_SAMPLES                                3     // 3 samples seem ideal, 5 is okay, 8 is too laggy
#define BRAKE_AND_THROTTLE_QQ                                     2
#define BRAKE_AND_THROTTLE_SPEED_FACTOR                           0.95
#define BRAKE_AND_THROTTLE_IQ_FACTOR                              5

//Speed modes
#define BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE                       0x00
#define BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE                     0x01
#define BRAKE_AND_THROTTLE_SPEED_MODE_SPORTS                      0x02

#define BRAKE_AND_THROTTLE_MAXIMUMN_SPEED                         674       // 674 RPM = 25.4 Km/hr

//Speed mode TORQUEIQ reduction ratio
#define BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_AMBLE       80        //80%   Pout = 240 W
#define BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_LEISURE     90        //90%   Pout = 270 W
#define BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_SPORTS      100       //100%  Pout = 300 W

//Speed mode maximum "powered" speed in RPM
#define BRAKE_AND_THROTTLE_MAXSPEED_AMBLE                         275       // 275 RPM = 10.4 Km/hr
#define BRAKE_AND_THROTTLE_MAXSPEED_LEISURE                       488       // 488 RPM = 18.4 Km/hr
#define BRAKE_AND_THROTTLE_MAXSPEED_SPORTS                        674       // 674 RPM = 25.4 Km/hr
#define BRAKE_AND_THROTTLE_MINSPEED                               80        //  80 RPM = 3 Km/hr

//Speed mode ramp rate (acceleration) in milliseconds
#define BRAKE_AND_THROTTLE_RAMPRATE_AMBLE                         3000      // 4000
#define BRAKE_AND_THROTTLE_RAMPRATE_LEISURE                       2250      // 3000
#define BRAKE_AND_THROTTLE_RAMPRATE_SPORTS                        1500      // 2000

//Speed mode Torque IQ value
#define BRAKE_AND_THROTTLE_TORQUEIQ_MAX                           16000     // IQ 16000 = 14.222 Amp

//Hard braking definition   (What is Hard Braking? why is this necessary?)
#define HARD_BRAKING_THROTTLE_PERCENTAGE                          5
#define HARD_BRAKING_BRAKE_PERCENTAGE                             5
#define BRAKEPERCENTTHRESHOLD                                     5
#define THROTTLEPERCENTREDUCTION                                  0.3
//Throttle calibration values = value range the throttle ADC is conditioned to be within
#define THROTTLE_ADC_CALIBRATE_H                                  2300
#define THROTTLE_ADC_CALIBRATE_L                                  850

//Throttle error thresholds = values that should not be possible under nominal operation
#define THROTTLE_ADC_THRESHOLD_H                                  2700
#define THROTTLE_ADC_THRESHOLD_L                                  500

//Brake calibration values = value range the Brake ADC is conditioned to be within
#define BRAKE_ADC_CALIBRATE_H                                     2300
#define BRAKE_ADC_CALIBRATE_L                                     800

//Brake error thresholds = values that should not be possible under nominal operation
#define BRAKE_ADC_THRESHOLD_H                                     2700
#define BRAKE_ADC_THRESHOLD_L                                     500

//Error message
#define BRAKE_AND_THROTTLE_NORMAL                                 0x00
#define THROTTLE_ERROR                                            0x0C
#define BRAKE_ERROR                                               0x0E
#define HARD_BRAKING_ERROR                                        0x0F

/*********************************************************************
 * MACROS
 */
typedef void (*brakeAndThrottle_timerStart)(void);
typedef void (*brakeAndThrottle_timerStop)(void);
typedef struct
{
    brakeAndThrottle_timerStart timerStart;
    brakeAndThrottle_timerStop  timerStop;
}brakeAndThrottle_timerManager_t;

typedef void (*brakeAndThrottle_ADC_Open)(void);
typedef void (*brakeAndThrottle_ADC_Convert)(uint16_t*);
typedef void (*brakeAndThrottle_ADC_Close)(void);
typedef struct
{
    brakeAndThrottle_ADC_Open       brakeAndThrottle_ADC_Open;
    brakeAndThrottle_ADC_Convert    brakeAndThrottle_ADC_Convert;
    brakeAndThrottle_ADC_Close      brakeAndThrottle_ADC_Close;
}brakeAndThrottle_adcManager_t;

typedef void (*brakeAndThrottle_CB_t)(uint16_t, uint16_t, uint8_t);
typedef struct
{
    brakeAndThrottle_CB_t       brakeAndThrottle_CB;
}brakeAndThrottle_CBs_t;

/*********************************************************************
 * FUNCTIONS
 */
/*
 * Task creation function for the Simple Peripheral.
 */
static void brakeAndThrottle_normalLawControl();
extern void brakeAndThrottle_motorControl(MCUD_t (*ptrMCUD));
extern void brakeAndThrottle_motorControl_rpm(uint16_t *ptrBrakeAndThrottleRPM);
extern void brakeAndThrottle_init();
extern void brakeAndThrottle_start();
extern void brakeAndThrottle_stop();
extern void brakeAndThrottle_toggle();
extern void brakeAndThrottle_setSpeedMode(uint8_t speed_Mode);
extern uint8_t brakeAndThrottle_getSpeedMode();
extern uint8_t brakeAndThrottle_toggleSpeedMode();
extern void brakeAndThrottle_registerCBs(brakeAndThrottle_CBs_t *obj);

extern void brakeAndThrottle_registerTimer(brakeAndThrottle_timerManager_t *obj);
extern void brakeAndThrottle_registerADC1(brakeAndThrottle_adcManager_t *obj);
extern void brakeAndThrottle_registerADC2(brakeAndThrottle_adcManager_t *obj);
extern void brakeAndThrottle_ADC_conversion();
extern uint16_t brakeAndThrottle_getThrottlePercent();
extern uint16_t brakeAndThrottle_getBrakePercent();

extern void brakeAndThrottle_gapRoleChg(uint8_t flag);
extern uint8_t brakeAndThrottle_getControlLaw();
extern void brakeAndThrottle_setControlLaw(uint8_t newControlLaw);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_BRAKEANDTHROTTLE_H_ */
