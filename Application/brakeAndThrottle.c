/******************************************************************************

 @file  brakeAndThrottle.c

 @brief This file contains the Code of brake and throttle

 *****************************************************************************/
/*********************************************************************
 * INCLUDES
 */
#include "brakeAndThrottle.h"
#include "Dashboard.h"
#include "motorControl.h"
#include "ledControl.h"
#include "dataAnalysis.h"
#include "lightControl.h"
#include "STM32MCP/STM32MCP.h"
#include <stdint.h>
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t     gapRoleChanged = 0;
uint8_t     speedMode;
uint16_t    adc2Result;             // adc2Result is a holder of the throttle ADC reading
uint16_t    throttlePercent;        // Actual throttle applied in percentage
uint16_t    throttlePercent0;
uint16_t    IQValue;                // Iq value command sent to STM32 / motor Controller
uint16_t    brakePercent;           // Actual brake applied in percentage
uint16_t    brakeStatus = 0;
uint16_t    brakeADCAvg;            // declared as global variable for debugging only
uint16_t    throttleADCAvg;         // declared as global variable for debugging only

/********************************************************************
 *  when brakeAndThrottle_errorMsg is true (=1),
 *  it generally means either (1) brake signal is not connected and/or (2) throttle signal is not connect
 *  IQValue is set to 0 = zero throttle in default
 */
uint8_t brakeAndThrottle_errorMsg = BRAKE_AND_THROTTLE_NORMAL;
uint8_t throttle_errorStatus = 0;
uint8_t brake_errorStatus = 0;

// A Flag: SpeedModeChgFlag becomes 1 when speed mode has been changed but instruction is not yet sent to Motor Controller due to Unreleased Throttle Condition
//uint8_t speedModeChgFlag = 0;

/*********************************************************************
 *
 * LOCAL VARIABLES
 */
static brakeAndThrottle_timerManager_t  *brake_timerManager;
static brakeAndThrottle_adcManager_t    *brake_adc1Manager;
static brakeAndThrottle_adcManager_t    *brake_adc2Manager;
static brakeAndThrottle_CBs_t           *brakeAndThrottle_CBs;

static uint8_t  state = 0;
static uint8_t  brakeAndThrottleIndex = 0;
//static uint8_t  brakeIndex = 0;
static uint16_t brakeADCValues[BRAKE_AND_THROTTLE_SAMPLES];
//static uint8_t  throttleIndex = 0;
static uint16_t throttleADCValues[BRAKE_AND_THROTTLE_SAMPLES];


/**********************************************************************
 *  Local functions
 */
static void brakeAndThrottle_getSpeedModeParams();

/*********************************************************************
 * @fn      brake_init
 *
 * @brief   Start the brake ADC and timer
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void brakeAndThrottle_init()
{
    speedMode = dataAnalysis_getInitSpeedMode(); //BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE; // load and Read NVSinternal and get the last speed mode
    brakeAndThrottle_getSpeedModeParams();
    uint8_t ii;
    for (ii = 0; ii < BRAKE_AND_THROTTLE_SAMPLES; ii++)
    {
        brakeADCValues[ii] = BRAKE_ADC_CALIBRATE_L;
        throttleADCValues[ii] = THROTTLE_ADC_CALIBRATE_L;
    }
    ledControl_setSpeedMode(speedMode);  // update speed mode displayed on dash board

}
/*********************************************************************
 * @fn      brake_start
 *
 * @brief   Start the brake ADC and timer.  This occurs at Power On
 *
 * @param   none
 *
 * @return  none
 */
uint8_t testpoint = 0;
void brakeAndThrottle_start()
{
    testpoint = 1;
    brake_timerManager->timerStart();
    brake_adc1Manager->brakeAndThrottle_ADC_Open();
    brake_adc2Manager->brakeAndThrottle_ADC_Open();
}
/*********************************************************************
 * @fn      brake_stop
 *
 * @brief   Stop the brake ADC and timer.  This only occurs at Power Off
 *
 * @param   none
 *
 * @return  none
 */
void brakeAndThrottle_stop()
{
    brake_timerManager -> timerStop();
    brake_adc1Manager -> brakeAndThrottle_ADC_Close();
    brake_adc2Manager -> brakeAndThrottle_ADC_Close();
}
/***************************************************************************************************
 * @fn      brakeAndThrottle_StartAndStopToggle
 *
 * @brief   Initialization/start timer and stop timer for the Brake and Throttle periodic sampling.
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
void brakeAndThrottle_toggle()
{
    if(state == 0)                                                  // State = 0 initially.  Must be called at Power On to start timer
    {
        brake_timerManager -> timerStart();
        brake_adc1Manager -> brakeAndThrottle_ADC_Open();             // this is repeated and duplicated in timerStart()
        brake_adc2Manager -> brakeAndThrottle_ADC_Open();             // this is repeated and duplicated in timerStart()
        state = 1;
    }
    else if(state == 1)
    {
        brake_timerManager -> timerStop();
        brake_adc1Manager -> brakeAndThrottle_ADC_Close();             // this is repeated and duplicated in timerStart()
        brake_adc2Manager -> brakeAndThrottle_ADC_Close();             // this is repeated and duplicated in timerStart()
        state = 0;
    }
}
/*********************************************************************
 * @fn      brakeAndThrottle_setSpeedMode
 *
 * @brief   To set the speed mode of the escooter
 *
 * @param   speedMode - the speed mode of the escooter
 *
 * @return  none
 */
void brakeAndThrottle_setSpeedMode(uint8_t speed_Mode)
{
    speedMode = speed_Mode;
}
/*********************************************************************
 * @fn      brakeAndThrottle_getSpeedMode
 *
 * @brief   To get the speed mode of the escotter
 *
 * @param   none
 *
 * @return  the speedmode of the escootter
 */
uint8_t brakeAndThrottle_getSpeedMode()
{
    return speedMode;
}
/*********************************************************************
 * @fn      brakeAndThrottle_getThrottlePercent
 *
 * @brief   To get the throttle percentage of the escooter
 *
 * @param   none
 *
 * @return  the throttle percentage of the escooter
 */
uint16_t brakeAndThrottle_getThrottlePercent()
{
    return throttlePercent;
}
/*********************************************************************
 * @fn      brakeAndThrottle_getBrakePercent
 *
 * @brief   To get the brake percentage of the escooter
 *
 * @param   none
 *
 * @return  the brake percentage of the escooter
 */
uint16_t brakeAndThrottle_getBrakePercent()
{
    return brakePercent;
}

/*********************************************************************
 * @fn      brakeAndThrottle_getSpeedModeParams
 *
 * @brief   Get speed Mode parameters
 *
 * @param   speedMode
 *
 * @return  none
 */
static uint16_t speedModeIQmax;
static uint8_t reductionRatio;
static uint16_t rampRate;
static uint16_t allowableSpeed;
void brakeAndThrottle_getSpeedModeParams()
{
    switch(speedMode)
    {
    case BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE:                       // Amble mode
        {
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE;
            reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_AMBLE;
            speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
            rampRate = BRAKE_AND_THROTTLE_RAMPRATE_AMBLE;
            allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_AMBLE;
            break;
        }
    case BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE:                 // Leisure mode
        {
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE;
            reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_LEISURE;
            speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
            rampRate = BRAKE_AND_THROTTLE_RAMPRATE_LEISURE;
            allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_LEISURE;
            break;
        }
    case BRAKE_AND_THROTTLE_SPEED_MODE_SPORTS:                  // Sports mode
        {
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_SPORTS;
            reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_SPORTS;
            speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
            rampRate = BRAKE_AND_THROTTLE_RAMPRATE_SPORTS;
            allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_SPORTS;
            break;
        }
    default:
        break;
    }
}
/*********************************************************************
 * @fn      brakeAndThrottle_toggleSpeedMode
 *
 * @brief   To change / toggle the speed Mode of the e-scooter
 *
 * @param   none
 *
 * @return  none
 */
uint8_t brakeAndThrottle_toggleSpeedMode()
{
//    speedModeChgFlag = 1;
    if (brake_errorStatus == 0)
    {
        if (adc2Result <= THROTTLE_ADC_CALIBRATE_L)                                    // Only allow speed mode change when no throttle is applied - will by-pass if throttle is applied
        {
            if(speedMode == BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE)                       // Amble mode to Leisure mode
            {
                speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE;
                reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_LEISURE;
                speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
                rampRate = BRAKE_AND_THROTTLE_RAMPRATE_LEISURE;
                allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_LEISURE;
            }
            else if(speedMode == BRAKE_AND_THROTTLE_SPEED_MODE_LEISURE)                 // Leisure mode to Sports mode
            {
                speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_SPORTS;
                reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_SPORTS;
                speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
                rampRate = BRAKE_AND_THROTTLE_RAMPRATE_SPORTS;
                allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_SPORTS;
            }
            else if(speedMode == BRAKE_AND_THROTTLE_SPEED_MODE_SPORTS)                  // Sports mode back to Amble mode
            {
                speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE;
                reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_AMBLE;
                speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
                rampRate = BRAKE_AND_THROTTLE_RAMPRATE_AMBLE;
                allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_AMBLE;
            }
            // Send updated speed mode parameters to motor control unit
            motorcontrol_speedModeChgCB(speedModeIQmax, allowableSpeed, rampRate);
            //Save the current setting
        }
    }
    else
    {
        if(speedMode != BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE)        // This condition prevents unnecessary repetitive changes that does nothing
        {
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE;
            reductionRatio = BRAKE_AND_THROTTLE_SPEED_MODE_REDUCTION_RATIO_AMBLE;
            speedModeIQmax = reductionRatio * BRAKE_AND_THROTTLE_TORQUEIQ_MAX / 100;
            rampRate = BRAKE_AND_THROTTLE_RAMPRATE_AMBLE;
            allowableSpeed = BRAKE_AND_THROTTLE_MAXSPEED_AMBLE;
            // Send updated speed mode parameters to motor control unit
            motorcontrol_speedModeChgCB(speedModeIQmax, allowableSpeed, rampRate);
        }
    }
    // Send updated speed mode parameters to App
    motorcontrol_setGatt(DASHBOARD_SERV_UUID, DASHBOARD_SPEED_MODE, DASHBOARD_SPEED_MODE_LEN, (uint8_t *) &speedMode);  //update speed mode on client (App)
    return speedMode;
}

/*********************************************************************
 * @fn      brakeAndThrottle_registerCBs
 *
 * @brief   When the ADC completed the conversion, it makes a callback to the main function
 *
 * @param   none
 *
 * @return  none
 */
void brakeAndThrottle_registerCBs(brakeAndThrottle_CBs_t *obj)
{
    brakeAndThrottle_CBs = obj;
}
/*********************************************************************
 * @fn      brake_registerTimer
 *
 * @brief   Initialization timer for the brake ADC sampling.
 *
 * @param   none
 *
 * @return  none
 */
void brakeAndThrottle_registerTimer(brakeAndThrottle_timerManager_t *obj)
{
    brake_timerManager = obj;
}
/*********************************************************************
 * @fn      brake_registerADC1
 *
 * @brief   Initialization timer for the brake ADC sampling.
 *
 * @param   none
 *
 * @return  none
 */
void brakeAndThrottle_registerADC1(brakeAndThrottle_adcManager_t *obj)
{
    brake_adc1Manager = obj;
}
/*********************************************************************
 * @fn      brake_registerADC2
 *
 * @brief   Initialization timer for the brake ADC sampling.
 *
 * @param   none
 *
 * @return  none
 */
void brakeAndThrottle_registerADC2(brakeAndThrottle_adcManager_t *obj)
{
    brake_adc2Manager = obj;
}
/*********************************************************************
 * @fn      brake_registerADC2
 *
 * @brief   Initialization timer for the brake ADC sampling.
 *
 * @param   none
 *
 * @return  none
 ********************************************************************/
void brakeAndThrottle_convertion(brakeAndThrottle_adcManager_t *obj)
{
    brake_adc2Manager = obj;
}
/*********************************************************************
 * @fn      brakeAndThrottle_ADC_conversion
 *
 * @brief   This function perform ADC conversion
 *          This function is called when timer6 overflows
 *
 * @param
 */
//uint8_t brakeAndThrottle_errorMsg_Old = 0x00;
//uint8_t speedModeChgCheck = 0;
uint8_t brake_errorLog = 0;

void brakeAndThrottle_ADC_conversion()
{
    STM32MCP_setSystemControlConfigFrame(STM32MCP_HEARTBEAT);
    /*******************************************************************************************************************************
     *      get brake ADC measurement
     *      get throttle ADC measurement
     *      Stores ADC measurement in arrays brakeADCValues & throttleADCValues
     *******************************************************************************************************************************/
    uint16_t adc1Result;                           // adc1Result is a holder of the brake ADC reading
    brake_adc1Manager -> brakeAndThrottle_ADC_Convert( &adc1Result );
    brakeADCValues[ brakeAndThrottleIndex ] = adc1Result;

    brake_adc2Manager -> brakeAndThrottle_ADC_Convert( &adc2Result );
    throttleADCValues[ brakeAndThrottleIndex ] = adc2Result;

    brakeAndThrottleIndex = brakeAndThrottleIndex++;
    if (brakeAndThrottleIndex >= BRAKE_AND_THROTTLE_SAMPLES)
    {
        brakeAndThrottleIndex = 0;
    }

    /*******************************************************************************************************************************
     *      the sampling interval is defined by "BRAKE_AND_THROTTLE_ADC_SAMPLING_PERIOD"
     *      the number of samples is defined by "BRAKE_AND_THROTTLE_SAMPLES"
     *      Sum the most recent "BRAKE_AND_THROTTLE_SAMPLES" number of data points, and
     *      calculate moving average brake and throttle ADC values
     *******************************************************************************************************************************/
    uint16_t    brakeADCTotal = 0;
    uint16_t    throttleADCTotal = 0;
    for (uint8_t index = 0; index < BRAKE_AND_THROTTLE_SAMPLES; index++)
    {
        brakeADCTotal += brakeADCValues[index];
        throttleADCTotal += throttleADCValues[index];
    }
    //uint16_t
    brakeADCAvg = brakeADCTotal/BRAKE_AND_THROTTLE_SAMPLES;             // declared as global variable for debugging only
    //uint16_t
    throttleADCAvg = throttleADCTotal/BRAKE_AND_THROTTLE_SAMPLES;       // declared as global variable for debugging only

    /*******************************************************************************************************************************
     *      Error Checking
     *      Check whether throttle ADC reading is logical, if illogical, brakeAndThrottle_errorMsg = error (!=0)
     *      These Conditions occur when throttle or brake signals/power are not connected, or incorrect supply voltage
     *      Once this condition occurs (brakeAndThrottle_errorMsg != 0), check throttle connections, hall sensor fault,
     *      Reset (Power off and power on again) is require to reset brakeAndThrottle_errorMsg.
     *******************************************************************************************************************************/
    if (throttleADCAvg < THROTTLE_ADC_THRESHOLD_L)
    {
        throttle_errorStatus = 1;
        // if throttle errorStatus = 1 -> disable throttle input and zero Iq command to Motor Controller Unit
        ledControl_getError(11);

    }
    if (throttleADCAvg > THROTTLE_ADC_THRESHOLD_H)
    {
        throttle_errorStatus = 1;
        // if throttle errorStatus = 1 -> disable throttle input and zero Iq command to Motor Controller Unit
        ledControl_getError(11);
    }
    /*******************************************************************************************************************************
     *      Throttle Signal Calibration
     *      Truncates the average throttle ADC signals to within THROTTLE_ADC_CALIBRATE_L and THROTTLE_ADC_CALIBRATE_H
     *******************************************************************************************************************************/
    if((throttleADCAvg > THROTTLE_ADC_CALIBRATE_H) && (throttleADCAvg <= THROTTLE_ADC_THRESHOLD_H))
    {
        throttleADCAvg = THROTTLE_ADC_CALIBRATE_H;
    }
    if((throttleADCAvg < THROTTLE_ADC_CALIBRATE_L) && (throttleADCAvg >= THROTTLE_ADC_THRESHOLD_L))
    {
        throttleADCAvg = THROTTLE_ADC_CALIBRATE_L;
    }
    /*******************************************************************************************************************************
     *      Error Checking
     *      Check whether brake ADC reading is logical, if illogical, brakeAndThrottle_errorMsg = error (!=0)
     *      These Conditions occur when brake signals/power are not connected, or incorrect supply voltage
     *      Once this condition occurs (brakeAndThrottle_errorMsg != 0), check brake connections, hall sensor fault,
     *      Reset (Power off and power on again) is require to reset brakeAndThrottle_errorMsg.
     *******************************************************************************************************************************/
    if ((brakeADCAvg < BRAKE_ADC_THRESHOLD_L) || (brake_errorLog == 1))
    {
        brake_errorLog = 1;
        if ((brake_errorStatus == 0) && (adc2Result <= THROTTLE_ADC_CALIBRATE_L))       // For safety reason, will change only when throttle value is less than THROTTLE_ADC_CALIBRATE_L
        {
            brakeAndThrottle_errorMsg = BRAKE_ERROR;
            brake_errorStatus = 1;
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE;
            brakeAndThrottle_toggleSpeedMode();
            ledControl_setSpeedMode(speedMode);
            // if brake errorStatus = 1 -> limited to Amble mode Only
            brake_errorLog = 0;
            /*Send error code to the Motor Controller*/
        }
        ledControl_getError(10);
        /*Send error code to the Motor Controller*/
    }

    if ((brakeADCAvg < BRAKE_ADC_THRESHOLD_L) || (brake_errorLog == 1))
    {
        brake_errorLog = 1;
        if ((brake_errorStatus == 0) && (adc2Result <= THROTTLE_ADC_CALIBRATE_L))       // For safety reason, will change only when throttle value is less than THROTTLE_ADC_CALIBRATE_L
        {
            brakeAndThrottle_errorMsg = BRAKE_ERROR;
            brake_errorStatus = 1;
            speedMode = BRAKE_AND_THROTTLE_SPEED_MODE_AMBLE;
            brakeAndThrottle_toggleSpeedMode();
            ledControl_setSpeedMode(speedMode);
            // if brake errorStatus = 1 -> limited to Amble mode Only
            brake_errorLog = 0;
            /*Send error code to the Motor Controller*/
        }
        ledControl_getError(10);
        /*Send error code to the Motor Controller*/
    }

    /*******************************************************************************************************************************
     *      Brake Signal Calibration
     *      Truncates the average brake ADC signals to within BRAKE_ADC_CALIBRATE_L and BRAKE_ADC_CALIBRATE_H
     *******************************************************************************************************************************/
    if((brakeADCAvg > BRAKE_ADC_CALIBRATE_H) && (brakeADCAvg <= BRAKE_ADC_THRESHOLD_H))
    {
        brakeADCAvg = BRAKE_ADC_CALIBRATE_H;
    }
    if((brakeADCAvg < BRAKE_ADC_CALIBRATE_L) && (brakeADCAvg >= BRAKE_ADC_THRESHOLD_L))
    {
        brakeADCAvg = BRAKE_ADC_CALIBRATE_L;
    }

    /********************************************************************************************************************************
     *  brakePercent is in percentage - has value between 0 - 100 %
     ********************************************************************************************************************************/
    //uint16_t
    brakePercent = (uint16_t) ((brakeADCAvg - BRAKE_ADC_CALIBRATE_L) * 100 / (BRAKE_ADC_CALIBRATE_H - BRAKE_ADC_CALIBRATE_L));

    /********************** Brake Power Off Protect State Machine  *******************************************************************************
     *              if brake is pressed, i.e. brakePercent is greater than a certain value (15%), for safety purposes,
     *              dashboard will instruct motor controller to cut power to motor.
     *              Once power to motor is cut, both the brake & throttle must be released before power delivery can be resumed
    **********************************************************************************************************************************************/
    if (brake_errorStatus == 0)         // under normal condition
    {
        if ((brakeStatus == 1) && (throttlePercent >= throttlePercent0 * THROTTLEPERCENTREDUCTION))
        {                           // condition when rider has not release the throttle
            if ((throttlePercent0 == 0) && (brakePercent <= BRAKEPERCENTTHRESHOLD))                                             // This condition resets brakeStatus to zero
            {
                brakeStatus = 0;
            }
//            else {                                                                                                            // This condition is not necessary
//                brakeStatus = 1;                                                                                              // Since brakeStatus is initially = 1
//            }
        }
        else if ((brakeStatus == 0) && (brakePercent > BRAKEPERCENTTHRESHOLD)) {                                                // condition when brake is not initially pressed and rider pulls on the brake
            brakeStatus = 1;
            throttlePercent0 = throttlePercent;
        }
        else if ((throttlePercent < throttlePercent0 * THROTTLEPERCENTREDUCTION) && (brakePercent <= BRAKEPERCENTTHRESHOLD)) {  // condition when rider releases the throttle && brake is released
            brakeStatus = 0;
        }
    }
    else {                              // when brake error is present
        brakeStatus = 0;
    }

    if (brakeStatus == 1) //Brake Pressed
    {
        STM32MCP_setEscooterControlDebugFrame(STM32MCP_ESCOOTER_BRAKE_PRESS);
        // send instruction to STM32 to activate brake light
        // for regen-brake, send brakePercent to STM32
    }
    else //Brake Released
    {
        //STM32MCP_setEscooterControlDebugFrame(STM32MCP_ESCOOTER_BRAKE_RELEASE);
        // brake light off
        // regen-brake off
    }
    /********************************************************************************************************************************
     *  throttlePercent is in percentage - has value between 0 - 100 %
     ********************************************************************************************************************************/
    //uint16_t
    throttlePercent = (uint16_t) ((throttleADCAvg - THROTTLE_ADC_CALIBRATE_L) * 100 / (THROTTLE_ADC_CALIBRATE_H - THROTTLE_ADC_CALIBRATE_L));

    // Throttle Error Safety Protocol -> when throttle error detected, IQValue is set to zero
    if (throttle_errorStatus == 0)
    {
        if (brakeStatus == 1)
        {
            IQValue = 0;
        }
        else
        {
            IQValue = BRAKE_AND_THROTTLE_TORQUEIQ_MAX * reductionRatio * throttlePercent / 10000;
        }
    }
    else
    {
        IQValue = 0;
    }

    /********************************************************************************************************************************
     * Send the throttle signal to STM32 Motor Controller
     ********************************************************************************************************************************/
    // in "brakeAndThrottle_CB(allowableSpeed, IQValue, brakeAndThrottle_errorMsg)", brakeAndThrottle_errorMsg is sent to the motor control unit for error handling if necessary.
    brakeAndThrottle_CBs -> brakeAndThrottle_CB(allowableSpeed, IQValue, brakeAndThrottle_errorMsg);
    /********************************************************************************************************************************
     *      The following is a safety critical routine/condition
     *      Firmware only allows speed mode change when throttle is not pressed concurrently/fully released
     *      If speed mode is changed && throttle is not pressed or is released,
     *      firmware will then send instructions to STM32 and assigns speed mode parameters
     ********************************************************************************************************************************/
//    if ((speedModeChgFlag == 1) && (adc2Result <= THROTTLE_ADC_CALIBRATE_L))
//    {
////        motorcontrol_speedModeChgCB(speedModeIQmax, allowableSpeed, rampRate);
//        speedModeChgFlag = 0;
//        speedModeChgCheck++;
//    }

    //Sends brake signal to the controller for tail light toggling
    //Do it as you like !!!

    //Send brake percentage for Regen Brake....
    //Do it as you like

    if (gapRoleChanged == 1)
    {
        motorcontrol_setGatt(DASHBOARD_SERV_UUID, DASHBOARD_SPEED_MODE, DASHBOARD_SPEED_MODE_LEN, (uint8_t *) &speedMode);  //update speed mode on client (App)
        lightControl_gapRoleChg();
        gapRoleChanged = 0;
    }
}


void brakeAndThrottle_gapRoleChg(uint8_t flag)
{
    gapRoleChanged = flag;
}
