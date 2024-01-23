/*
 * generalPurposeTimer.c
 *
 *  Created on: 12 Jul 2023
 *      Author: User
 */
/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <Board.h>
#include "periodicCommunication.h"
#include "UDHAL/UDHAL.h"
#include "lightControl.h"
#include "powerOnTime.h"
#include "ledControl.h"
#include "motorControl.h"
#include "Dashboard.h"
#include "generalPurposeTimer.h"
#include "UDHAL/UDHAL_I2C.h"
#include "IS31FL3236A/IS31FL3236A.h"
#include "TSL2561/TSL2561.h"
#include "STM32MCP/STM32MCP.h"
#include "buzzerControl.h"
#include "singleButton/singleButton.h"
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
/*Hardware Driver*/
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include <ti/devices/cc26x0r2/driverlib/cpu.h>
#include <UDHAL/UDHAL_PWM.h>

// Application Power_NotifyFxn function prototype
//Power Manager Notification
//Power_NotifyObj powerNotifyObj;
//static int powerTransitionNotifyFxn(unsigned int eventType, uintptr_t eventArg,uintptr_t clientArg);


//For GENEV DASHBOARD

#ifdef CC2640R2_GENEV_5X5_ID
PIN_Config ExternalWakeUpPin[] = {
      CC2640R2_GENEV_5X5_ID_DIO5 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
      PIN_TERMINATE
};
#endif


//For Launchpad
#ifdef CC2640R2_LAUNCHXL
PIN_Config ExternalWakeUpPin[] = {
      CC2640R2_LAUNCHXL_PIN_BTN1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
      PIN_TERMINATE
};
#endif

Semaphore_Struct bootSem;
/*********************************************************************
 * @fn      lightControl_initialization
 *
 * @brief   Initialization and initial set up at each Power On.  AUTO mode at every POWER ON
 *
 * @param   None
 *
 * @return  None
 *********************************************************************/
uint8_t gpTimer_i2cOpenStatus = 0;
uint8_t gpcheck = 0;
uint8_t PWR_STATUS = 0x01;
void gpTimer_init()
{

}


void PWR_CTL(uint8_t PWR)
{
    PWR_STATUS = PWR;
}

bool PWR_OFF()
{
    if(PWR_STATUS == 0x00)
    {
        return true;
    }
    else if(PWR_STATUS == 0x01)
    {
        return false;
    }
    return false;

}

/*********************************************************************
* @fn      GPtimer_createTask
*
* @brief   Task creation function for the General Purpose Task Timer.
*
* @param   None.
*
* @return  None.
*********************************************************************/
// Task configuration
Task_Struct     gptTask;
Char            gptTaskStack[GPT_TASK_STACK_SIZE];

void GPtimer_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = gptTaskStack;
    taskParams.stackSize = GPT_TASK_STACK_SIZE;
    taskParams.priority = GPT_TASK_PRIORITY;

    Task_construct(&gptTask, GPtimer_taskFxn, &taskParams, NULL);     //

}

/*********************************************************************
* @fn      lightControl_taskFxn
*
* @brief   Task creation function for the light Control.
*
* @param   None.
*
* @return  None.
**********************************************************************/
uint16_t    utime_Interval = PERIODIC_COMMUNICATION_HF_SAMPLING_TIME;
uint16_t    GPT_taskCounter = 0;
uint32_t    powerOnTimeMS = 0;                          // power on time in milli-seconds
uint8_t     pinConfig = 0xFF;
uint8_t     GPT_buttonStatus = 0;
uint8_t     GPT_errorStatus = 0xFF;
uint8_t     GPT_ii = 0;

static void GPtimer_taskFxn(UArg a0, UArg a1)
{
    for (; ;)               // infinite for loop, starting at 1 and without exit condition,
    {
        if (( GPT_taskCounter % NUM_PERIODIC_COMMUNICATION_HF_2_ALS ) == 0)
        {
            lightControl_taskFxn();
        }

        powerOnTimeMS += utime_Interval;
        powerOnTime_cal(powerOnTimeMS);

        /* led display commands */
        ledControl_changeLEDPower();
        GPT_errorStatus = ledControl_ErrorDisplay();
        if (GPT_errorStatus < BATTERY_TEMP_ERROR_PRIORITY){
        /* Critical system error has occurred -> put system in while loop */
            // 0: activate error event for error handling
            // 1: error handling shall allow led display to display the error code
            // 2: in error mode, system shall be put into the designate error handling protocol
            // 3: in error mode, single button shall allow user to Power OFF and ON to reset the firmware and restart the system
        }
        ledControl_changeLightMode();
        ledControl_changeLightStatus();
        ledControl_changeSpeedMode();
        ledControl_changeUnit();
        ledControl_changeBLE(GPT_taskCounter);
        ledControl_changeBatteryStatus(GPT_taskCounter);
        ledControl_changeDashSpeed();

        //STM32MCP_setSystemControlConfigFrame(STM32MCP_HEARTBEAT);

        /*  Task timing & delay */
        Task_sleep(utime_Interval * 1000 / Clock_tickPeriod);
        GPT_taskCounter++;

        if(PWR_OFF() == true)
        {
//
//            //Task_sleep(1000 * 1000 / Clock_tickPeriod); // delay for 1 second for buzzer before breaking out of for loop
//            //UDHAL_PWM1_setDutyAndPeriod(0, BUZZER_SINGLEBUTTON_FREQUENCY);
//
//            /* add save data to NVS here */
           break;              // break out of for loop
        }
    }
    //U could send the sleep command

    //Empty Queue. Stop sending the heart beat, Stop the communication, to ensure POWER_OFF CMD is sent to the motor controller
    STM32MCP_toggleCommunication();
    Task_sleep(500 * 1000 / Clock_tickPeriod); /*Wait for a while before sending shutdown command!*/
    STM32MCP_EscooterShutdown(STM32MCP_POWER_OFF);
    ledControl_deinit(); /*turns off led display*/
    pinConfig = PINCC26XX_setWakeup(ExternalWakeUpPin); /*The system resets (REBOOTS) automatically*/
    Power_shutdown(0, 0); /*System enters Shut Down Mode*/
    while(1)
    {
    }

}
