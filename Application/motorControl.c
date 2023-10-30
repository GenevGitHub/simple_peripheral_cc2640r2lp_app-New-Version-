/******************************************************************************

 @file  motorControl.c

 @brief This file contains the Code of motor control interfacing


 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <ti/drivers/I2C.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/GPIO.h>

#include "UDHAL/UDHAL.h"
#include "STM32MCP/STM32MCP.h"
#include "Controller.h"
#include "Dashboard.h"
#include "Battery.h"
#include "motorControl.h"
#include "util.h"
#include "periodicCommunication.h"
#include "simple_peripheral.h"
#include "brakeAndThrottle.h"
#include "ledControl.h"
#include "lightControl.h"
#include "buzzerControl.h"
#include "powerOnTime.h"
#include "dataAnalysis.h"
#include "singleButton/singleButton.h"
#include "peripheral.h"
#include "TSL2561/TSL2561.h"
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * GLOBAL VARIABLES
 */
/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t motorcontrol_i2cOpenStatus = 0;
static simplePeripheral_bleCBs_t *motorcontrol_bleCBs;
static uint8_t motorControl_getGAPRole_taskCreate_flag = 0;
uint8_t powerOn = 1;            // How does power on and power off work? powerOn Should be 1
/**********************************************************************
 *  Local functions
 */
static void motorcontrol_processGetRegisterFrameMsg(uint8_t *txPayload, uint8_t txPayloadLength, uint8_t *rxPayload, uint8_t rxPayloadLength);

static void motorcontrol_rxMsgCb(uint8_t *rxMsg, STM32MCP_txMsgNode_t *STM32MCP_txMsgNode);
static void motorcontrol_exMsgCb(uint8_t exceptionCode);
static void motorcontrol_erMsgCb(uint8_t errorCode);

static void motorcontrol_brakeAndThrottleCB(uint16_t allowableSpeed, uint16_t IQValue, uint8_t errorMsg);
static void motorcontrol_controllerCB(uint8_t paramID);
static void motorcontrol_dashboardCB(uint8_t paramID);
static void motorcontrol_singleButtonCB(uint8_t messageID);
static void motorcontrol_getGAPROLE(void);

MCUD_t MCUDArray = {30000, 3000, 380, 20, 20, 0};
MCUD_t *ptrMCUDArray = &MCUDArray;

uint16_t brakeAndThrottle_rpm;
uint16_t *ptrbrakeAndThrottle_rpm = &brakeAndThrottle_rpm;

//uint16_t throttle_Percent;

//extern void motorcontrol_setGatt(uint16_t serviceUUID, uint8_t charteristics, uint8_t payloadLength, uint8_t* payload);
/*********************************************************************
 * TYPEDEFS
 */
static STM32MCP_CBs_t STM32MCP_CBs =
{
     motorcontrol_rxMsgCb,
     motorcontrol_exMsgCb,
     motorcontrol_erMsgCb
};

static ControllerCBs_t ControllerCBs =
{
     motorcontrol_controllerCB
};

static DashboardCBs_t DashboardCBs =
{
     motorcontrol_dashboardCB
};

static singleButtonCBs_t singleButtonCBs =
{
     motorcontrol_singleButtonCB
};

static brakeAndThrottle_CBs_t brakeAndThrottle_CBs =
{
     motorcontrol_brakeAndThrottleCB                    // brake and throttle callback -> what happens when brake & throttle interrupts
};

//static lightControl_CBs_t lightControl_CBs =
//{
//     motorcontrol_lightControlCB
//};

//static buzzerControl_CBs_t buzzerControl_CBs =
//{
//     motorcontrol_buzzerControlCB
//};

/*********************************************************************
 * @fn      MC_init
 *
 * @brief   Initialization function for the Motor Control Task.
 *
 * @param   none
 *
 * @return  none
 */
uint8_t mccheck = 0;        // FOR DEBUGGING ONLY
void motorcontrol_init(void)
{
// Activate NVS_internal to Recall the last set of saved data in memory
//    NVS_init();

    UDHAL_init();
    mccheck = 1;        // FOR DEBUGGING ONLY

    Controller_RegisterAppCBs(&ControllerCBs);

    Dashboard_RegisterAppCBs(&DashboardCBs);

    STM32MCP_init();
    STM32MCP_registerCBs(&STM32MCP_CBs);
    STM32MCP_startCommunication();    // Not activated

    periodicCommunication_STM32MCP_getRegisterFrame();
    dataAnalysis_motorControl(ptrMCUDArray);            // passes ptrMCUDArray to dataAnalysis.c
    brakeAndThrottle_motorControl_rpm(ptrbrakeAndThrottle_rpm);    // passes ptrbrakeAndThrottle_rpm to brakeAndThrottle.c
    periodicCommunication_start();

//    dataAnalysis_init();                // Initiate data analytics

    brakeAndThrottle_init();
    brakeAndThrottle_registerCBs(&brakeAndThrottle_CBs);
    brakeAndThrottle_start();

    singleButton_init();
    singleButton_registerCBs(&singleButtonCBs);

    buzzerControl_init();               // Initiate buzzer on dashboard

    motorcontrol_i2cOpenStatus = UDHAL_getI2CStatus();

// ************* when i2c open successfully, motorcontrol_i2cOpenStatus = 1,
// ************* when i2c opens unsuccessfully, motorcontrol_i2cOpenStatus = 0 -> TSL2561 light sensor and Led Display are not initiated or activated
    if (motorcontrol_i2cOpenStatus == 1)
    {
        TSL2561_init();
        ledControl_init();
    }

    lightControl_init( motorcontrol_i2cOpenStatus );                            // UDHAL_I2C must be initiated before lightControl_init();

    dataAnalysis_init();                // Initiate data analytics

    powerOnTime_init();           // merged with lightControl 20230705

// If GAPRole_createTask initiated and called successfully, gapRole_getGAPRole_taskCreate_flag() returns 1
    motorControl_getGAPRole_taskCreate_flag = gapRole_getGAPRole_taskCreate_flag();

    mccheck = 3;        // FOR DEBUGGING ONLY

}

/*********************************************************************
 * @fn      motorcontrol_registerCB
 *
 * @brief   When the motor controller sends the get register frame message
 *
 * @param   STM32MCP_rxMsg_t - The memory message to the received message, the size of the message is the second index rxMsg[1]
 *
 * @return  None.
 */
void motorcontrol_registerCB(simplePeripheral_bleCBs_t *obj)
{
    motorcontrol_bleCBs = obj;
}

/*********************************************************************
 * @fn      motorcontrol_processGetRegisterFrameMsg
 *
 * @brief   When the motor controller sends the get register frame message
 *
 * @param   STM32MCP_rxMsg_t - The memory message to the received message, the size of the message is the second index rxMsg[1]
 *
 * @return  None.
 */
static void motorcontrol_processGetRegisterFrameMsg(uint8_t *txPayload, uint8_t txPayloadLength, uint8_t *rxPayload, uint8_t rxPayloadLength)
{
    uint8_t regID = txPayload[0];
    switch(regID)
    {
    case STM32MCP_BUS_VOLTAGE_REG_ID:
        {
            uint16_t voltage_mV = *((uint16_t*) rxPayload) * 1000; // rxPayload in V, voltage in mV
        // **** store voltage_mV in (*ptrMCUDArray).voltage_mV
            (*ptrMCUDArray).voltage_mV = voltage_mV;

            break;
        }
    // *********  current sensor to be added to MCU.  Reserved case for current measurement
    case STM32MCP_TORQUE_MEASURED_REG_ID:
        {
        //keep current in mV - do not convert it to A
            uint16_t current_mA = *((uint8_t*) rxPayload) * 1000;
        // **** store current_mA in (*ptrMCUDArray).current_mV
            (*ptrMCUDArray).current_mA = 3000;//current_mA;
            break;
        }
    case STM32MCP_HEATSINK_TEMPERATURE_REG_ID:
        {
            int8_t heatSinkTemperature_Celcius = (int8_t) (*((uint8_t*) rxPayload) & 0xFF);     // temperature can be a negative value, unless it is in Kelvin
        // **** store heatSinkTemperature_Celcius in (*ptrMCUDArray).heatSinkTemperature_Celcius
            (*ptrMCUDArray).heatSinkTemperature_Celcius = 15;//heatSinkTemperature_Celcius;
            break;
        }
    case STM32MCP_SPEED_MEASURED_REG_ID:
        {
            uint16_t rpm;
            int32_t rawRPM = *((int32_t*) rxPayload);
            if(rawRPM >= 0)
            {
                rpm = (uint16_t) (rawRPM & 0xFFFF);
            }
            else  // **** what if rawRPM is negative??? e.g. pushing the E-scooter in reverse.  Then rawRPM would be negative!!!
            {
                rpm = (uint16_t) (-rawRPM & 0xFFFF);
            }
            // **** store rpm in (*ptrMCUDArray).speed_rpm
            (*ptrMCUDArray).speed_rpm = rpm;
            brakeAndThrottle_rpm = rpm;
            break;
        }
// ********************    Need to create new REG_IDs
    case STM32MCP_MOTOR_TEMPERATURE_REG_ID:
        {
            int8_t motorTemperature_Celcius = (int8_t) (*((uint8_t*) rxPayload) & 0xFF);     // temperature can be a negative value, unless it is in Kelvin

            (*ptrMCUDArray).motorTemperature_Celcius = 20;//motorTemperature_Celcius;

            break;
        }
    case STM32MCP_CONTROLLER_ERRORCODE_REG_ID:
        {

            //
            break;
        }
    default:
            break;
    }

}

/*********************************************************************
 * @fn      motorcontrol_dataAnalysis_sampling
 *
 * @brief   Pass motor sensor data to data analysis
 *
 * @param   x_hf - counts the number of high frequency periodic communication executions
 *
 * @return  None.
 */
void motorcontrol_dataAnalysis_sampling(uint8_t x_hf)
{
    (*ptrMCUDArray).count_hf = x_hf;
    dataAnalysis_sampling((*ptrMCUDArray).count_hf, (*ptrMCUDArray).voltage_mV, (*ptrMCUDArray).current_mA, (*ptrMCUDArray).speed_rpm,
                          (*ptrMCUDArray).heatSinkTemperature_Celcius, (*ptrMCUDArray).motorTemperature_Celcius);
}

/*********************************************************************
 * @fn      motorcontrol_rxMsgCb
 *
 * @brief   When the motor controller sends the feedback message back, it reaches here
 *
 * @param   rxMsg - The memory message to the received message, the size of the message is the second index rxMsg[1]
 *          motorID - The motor ID described in STM32MCP
 *          frameID - The frame ID described in STM32MCP
 *
 * @return  None.
 */
static void motorcontrol_rxMsgCb(uint8_t *rxMsg, STM32MCP_txMsgNode_t *STM32MCP_txMsgNode)
{
    uint8_t frameID = STM32MCP_txMsgNode->txMsg[0] & 0x1F;

    uint8_t rxPayloadLength = rxMsg[1];
    uint8_t *rxPayload = (uint8_t*)malloc(sizeof(uint8_t)*rxPayloadLength);
    memcpy(rxPayload, rxMsg + 2, rxPayloadLength);

    uint8_t txPayloadLength = STM32MCP_txMsgNode->size;
    uint8_t *txPayload = (uint8_t*)malloc(sizeof(uint8_t)*txPayloadLength);
    memcpy(txPayload, STM32MCP_txMsgNode->txMsg + 2, txPayloadLength);

    switch(frameID)
    {
    case STM32MCP_SET_REGISTER_FRAME_ID:
        break;
    case STM32MCP_GET_REGISTER_FRAME_ID:
        motorcontrol_processGetRegisterFrameMsg(txPayload, txPayloadLength, rxPayload, rxPayloadLength);
        break;
    case STM32MCP_EXECUTE_COMMAND_FRAME_ID:
        break;
    case STM32MCP_GET_BOARD_INFO_FRAME_ID:
        break;
    case STM32MCP_EXEC_RAMP_FRAME_ID:
        break;
    case STM32MCP_GET_REVUP_DATA_FRAME_ID:
        break;
    case STM32MCP_SET_REVUP_DATA_FRAME_ID:
        break;
    case STM32MCP_SET_CURRENT_REFERENCES_FRAME_ID:
        break;
    case STM32MCP_SET_SYSTEM_CONTROL_CONFIG_FRAME_ID:
        break;
    case STM32MCP_SET_DRIVE_MODE_CONFIG_FRAME_ID:
        break;
    case STM32MCP_SET_DYNAMIC_TORQUE_FRAME_ID:
        break;
    default:
        break;
    }
    free(rxPayload);
    free(txPayload);
}

/*********************************************************************
 * @fn      motorcontrol_exMsgCb
 *
 * @brief   When the motor controller sends the exception back, it reaches here
 *
 * @param   exceptionCode - The exception code described in STM32MCP
 *
 * @return  None.
 */
static void motorcontrol_exMsgCb(uint8_t exceptionCode)
{
    switch(exceptionCode)
        {
        case STM32MCP_QUEUE_OVERLOAD:
            break;
        case STM32MCP_EXCEED_MAXIMUM_RETRANSMISSION_ALLOWANCE:
            powerOn = 0;
            break;
        default:
            break;
        }
}

/*********************************************************************
 * @fn      motorcontrol_erMsgCb
 *
 * @brief   When the motor controller sends the error back, it reaches here
 *
 * @param   exceptionCode - The error code described in STM32MCP
 *
 * @return  None.
 */
static void motorcontrol_erMsgCb(uint8_t errorCode)
{
    switch(errorCode)
    {
    case STM32MCP_BAD_FRAME_ID:
        break;
    case STM32MCP_WRITE_ON_READ_ONLY:
        break;
    case STM32MCP_READ_NOT_ALLOWED:
        break;
    case STM32MCP_BAD_TARGET_DRIVE:
        break;
    case STM32MCP_OUT_OF_RANGE:
        break;
    case STM32MCP_BAD_COMMAND_ID:
        break;
    case STM32MCP_OVERRUN_ERROR:
        break;
    case STM32MCP_TIMEOUT_ERROR:
        break;
    case STM32MCP_BAD_CRC:
        break;
    default:
        break;
    }
}

/*********************************************************************
 * @fn      motorcontrol_brakeAndThrottleCB
 *
 * @brief   When the brake and throttle completed the adc conversion, it sends message here to communicate with STM32
 *
 * @param   throttlePercent - how much speed or torque in percentage the escooter should reach
 *          errorMsg - The error Msg
 *
 * @return  None.
 */
uint16_t execute_rpm;
//static void motorcontrol_brakeAndThrottleCB(uint16_t allowableSpeed, uint16_t IQValue, uint8_t errorMsg)
static void motorcontrol_brakeAndThrottleCB(uint16_t allowableSpeed, uint16_t IQValue, uint8_t errorMsg)
{
    if((errorMsg == BRAKE_AND_THROTTLE_NORMAL))
    {
        /*When driver accelerates / decelerates by twisting the throttle, the IQ signal with max. speed will be sent to the motor controller.
         * */
        STM32MCP_setDynamicCurrent(allowableSpeed, IQValue);

    }
    else
    {
        /*In case the brake and throttle are in malfunction, for safety, the E-Scooter stops operation. User
         *has to check the wire connections.
         *You have to ensure the wires are connected properly!
         * */
        //STM32MCP_executeCommandFrame(STM32MCP_MOTOR_1_ID, STM32MCP_STOP_MOTOR_COMMAND_ID);
        STM32MCP_setDynamicCurrent(allowableSpeed, 0);
        /*Sends Error Report to STM32 Motor Controller --> Transition to EMERGENCY STOP state*/
    }

}

/*********************************************************************
 * @fn      motorcontrol_speedModeChgCB
 *
 * @brief   When there is a speed mode change, it sends message here to communicate with STM32
 *
 * @param   speed mode parameters
 *
 *
 * @return  None.
 */
uint8_t motorcontrol_speedModeChgCount = 0;     // for debugging only
void motorcontrol_speedModeChgCB(uint16_t torqueIQ, uint16_t allowableSpeed, uint16_t rampRate)
{
    motorcontrol_speedModeChgCount++;           // for debugging only
    // send speed mode change parameters to motor control
    STM32MCP_setSpeedModeConfiguration(torqueIQ, allowableSpeed, rampRate);

}

/*********************************************************************
 * @fn      motorcontrol_controllerCB
 *
 * @brief   When the motor controller sends the error back, it reaches here
 *
 * @param   exceptionCode - The error code described in STM32MCP
 *
 * @return  None.
 */
static void motorcontrol_controllerCB(uint8_t paramID)
{
    switch(paramID)
    {
        case CONTROLLER_TOTAL_DISTANCE_TRAVELLED:
            {
                //Controller_GetParameter(CONTROLLER_TOTAL_DISTANCE_TRAVELLED, &Total_Distance_Travelled);
                break;
            }
        default:
            break;
    }
}
/*********************************************************************
 * @fn      motorcontrol_dashboardCB
 *
 * @brief   When the client (The mobile app) writes the Dashboard GATT server
 *          It will post the paramID of the changed characteristic, and Set the gatt due to client Callback
 *
 *          G-Link v1 allows the following characteristics to be changed from the client (mobile app)
 *          - light mode (paramID 3)
 *
 * @param   paramID: the paramID of the characteristics
 *
 * @return  TRUE or FALSE
 */
static void motorcontrol_dashboardCB(uint8_t paramID)
{
    switch(paramID)
    {
    case DASHBOARD_LIGHT_MODE: // whenever user tabs the light mode icon on the mobile app, motorcontrol_dashboardCB will toggle the light mode by calling lightControl_change()
        {
            uint8_t light_mode_temp = lightControl_lightModeChange();
            ledControl_setLightMode( light_mode_temp );
            break;
        }
    default:
        break;
    }
}

/*********************************************************************
 * @fn      motorcontrol_singleButtonCB
 * @brief   Set the gatt due to singleButton Callback
 *
 * @param   serviceUUID - the service UUID
 *          characteristics - the characteristics (Not UUID, just the index)
 *          payload - just the payload
 *
 * @return  TRUE or FALSE
 */
//uint8_t advertEnable = FALSE;
uint8_t messageid;              // for debugging only
/*Might be we have to create a state machine to monitor the power status! */
static void motorcontrol_singleButtonCB(uint8_t messageID)
{
    messageid = messageID;  // for debugging only

    switch(messageID)
    {
    case SINGLE_BUTTON_SINGLE_LONG_PRESS_MSG:       // case = 0x01 - toggle Device Power ON / OFF
        {// toggle Power ON/OFF or Enter/Exit Sleep Mode
            // if Powering On -> switch to Power Off
            if(powerOn == 1){
                powerOn = 0;
                //  trigger Power Transition Notification, when Power Stage Transition is triggered, before shutting down, the
                //  system is jumped into powerTransitionNotifyFxn Function to doing the followings if the transition event is PowerCC26XX_ENTERING_SHUTDOWN:
                //  send power off command to motor controller
                //  Peripheral De-Initialization
                //  before enters low power mode.........
                //  1.turn off ADC
                //  2.turn off I2C
                //  3.turn off TSL2561
                //  4.turn off all tasks
                // At very last:
                // call nvs write to write into NVS memory only when power off - will take a few milliseconds to complete
                // 5.nvsControl_nvsWrite();
                // STM32MCP_setSystemControlConfigFrame(STM32MCP_POWER_OFF);
                // pinConfig = PINCC26XX_setWakeup(ExternalWakeUpPin);
                // Power_shutdown(0,0);
            }
            // if Powering Off -> switch to Power On
            else if(powerOn == 0){
                powerOn = 1;
                //  system reset (Program Counter resets)
                //  wait for 1.5 seconds to power on
                //  call nvs read to read NVS memory - will take a few milliseconds to complete
                //  nvsControl_nvsRead();
                //  close NVS
                //  initialize ADC and turn On ADC
                //  initialize I2C
                //  initialize TSL2561
                //  turn ON tasks
                //  checks UART connection
                //  send power on command to motor controller
            }
        // ICallPlatform_pwrNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg)
            break;
        }
    case SINGLE_BUTTON_SINGLE_SHORT_PRESS_MSG:      // case = 0x02 - toggle light modes
        {
            uint8_t light_mode_temp = lightControl_lightModeChange();

            ledControl_setLightMode( light_mode_temp );

            break;
        }
    case SINGLE_BUTTON_SINGLE_SHORT_LONG_PRESS_MSG: // case = 0x03 - ADVERT_ENABLE if BLE in waiting state or waiting after timeout state
        {
            if (motorControl_getGAPRole_taskCreate_flag == 1)
            {
                gaprole_States_t get_gaproleState;
                GAPRole_GetParameter(GAPROLE_STATE, &get_gaproleState);
                if((get_gaproleState == GAPROLE_WAITING) || (get_gaproleState == GAPROLE_WAITING_AFTER_TIMEOUT) || (get_gaproleState == GAPROLE_STARTED))
                {
                    uint8_t advertising = true;
                    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising);
                }
                brakeAndThrottle_gapRoleChg(1);

            }


            break;
        }
    case SINGLE_BUTTON_DOUBLE_SHORT_PRESS_MSG:      // case = 0x04 - toggle speed modes
        {
            uint8_t speedMode_temp = brakeAndThrottle_toggleSpeedMode(); // update speed mode displayed on dash board
            ledControl_setSpeedMode(speedMode_temp);  // update speed mode displayed on dash board

            break;
        }
    case SINGLE_BUTTON_TREBLE_SHORT_PRESS_MSG:      // case = 0x05 - toggle units
        {
            uint8_t UnitSelectDash = dataAnalysis_getUnitSelectDash();
            if (UnitSelectDash == IMP_UNIT) {
                UnitSelectDash = SI_UNIT;
            }
            else if (UnitSelectDash == SI_UNIT) {
                UnitSelectDash = IMP_UNIT;
            }

            dataAnalysis_changeUnitSelectDash(UnitSelectDash);

            // send UnitSelectDash to ledControl.c
            ledControl_setUnitSelectDash(UnitSelectDash);
            break;
        }
    case SINGLE_BUTTON_QUADRUPLE_SHORT_PRESS_MSG:      // case = 0x06 - toggle control law
        {
            // toggles control law
            uint8_t currentControlLaw = brakeAndThrottle_getControlLaw();
            if (currentControlLaw == BRAKE_AND_THROTTLE_NORMALLAW){
                currentControlLaw = BRAKE_AND_THROTTLE_DIRECTLAW;
            }
            else if (currentControlLaw == BRAKE_AND_THROTTLE_DIRECTLAW){
                currentControlLaw = BRAKE_AND_THROTTLE_NORMALLAW;
            }
            // send currentControlLaw to brakeAndThrotte.c
            brakeAndThrottle_setControlLaw(currentControlLaw);
            break;
        }
    default:                                        // case 0x00 and all other cases
        break;
    }
}
/*********************************************************************
 * @fn      motorcontrol_setGatt
 * @brief   Set the gatt
 *
 * @param   serviceUUID - the service UUID
 *          characteristics - the characteristics (Not UUID, just the index)
 *          payloadLength - the length of the payload
 *          payload - just the payload
 *
 * @return  TRUE or FALSE <--- Nil (void)???
 */
//static void motorcontrol_setGatt(uint16_t serviceUUID, uint8_t characteristics, uint8_t payloadLength, uint8_t* payload)
void motorcontrol_setGatt(uint16_t serviceUUID, uint8_t charteristics, uint8_t payloadLength, uint8_t* payload)
{
    uint8_t *theMessage = ICall_malloc(sizeof(uint8_t)*(payloadLength + 4));
    if (theMessage)
    {
        theMessage[0] = (serviceUUID >> 0) & 0xFF;
        theMessage[1] = (serviceUUID >> 8) & 0xFF;
        theMessage[2] = charteristics;
        theMessage[3] = payloadLength;
        memcpy(theMessage + 4, payload, payloadLength);
        motorcontrol_bleCBs->enqueueMsg(SBP_MC_GATT_EVT, 0, theMessage);
    }
}
