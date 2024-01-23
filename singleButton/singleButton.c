/******************************************************************************

@file  singleButton.c

@brief This file contains the Code of single button control

@ single button timer is controlled by UDHAL_TIM3.c
*****************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include "singleButton.h"
#include "lightControl.h"
#include "buzzerControl.h"
#include "dataAnalysis.h"

/*********************************************************************
* LOCAL VARIABLES
*/
static uint8_t buttonState = SINGLE_BUTTON_WAITING_STATE; //It's a default waiting state!!!!
uint32_t timerPeriod;
uint8_t risingEdgeCount = 0;    // make this static if not debugging
uint8_t fallingEdgeCount = 0;   // make this static if not debugging
singleButton_timerManager_t *singleButton_timerManager; //singleButton_timerManager

static singleButtonCBs_t *singleButtonCBs;

/*********************************************************************
* LOCAL FUNCTIONS
*/
/*********************************************************************
 * @fn      singleButton_Init
 *
 * @brief   It is used to initialize the library
 *
 * @param   none
 *
 *
 * @return  none
 */
void singleButton_init()
{

}
/*********************************************************************
 * @fn      singleButton_registerClock
 *
 * @brief   It is used to process the button click time when the button is pressed
 *
 * @param   deviceTimer     A set of function pointer contain the timer function of specific device(example, STM32, TI chips)
 *
 *
 * @return  none
 */
void singleButton_registerTimer(singleButton_timerManager_t *singleButtonTimer)
{
    singleButton_timerManager = singleButtonTimer;
}
/*********************************************************************
 * @fn      singleButton_registerCBs
 *
 * @brief   It informs the application if the button is clicked
 *
 * @param   singleButton_CBs    A set of function pointer that informs the application if the button is clicked
 *
 *
 * @return  none
 */
void singleButton_registerCBs(singleButtonCBs_t *singleButtoncb)
{
    singleButtonCBs = singleButtoncb;
}

/*********************************************************************
 * @fn      singleButton_processButtonEvt
 *
 * @brief   It is used to process the data when the button is pressed
 *
 * @param   logicLevel It takes the logic level of the GPIO to decide whether it is a falling edge or rising edge
 *
 *
 * @return  none
 */
bool singleButton_buzzerStatus = 0;

void singleButton_processButtonEvt(uint8_t logicLevel)
{
    if(logicLevel == 0)
    {
        fallingEdgeCount++;
        /* instruct buzzer to make a single beep on every falling edge */
#ifdef CC2640R2_LAUNCHXL
        singleButton_buzzerStatus = 1; // when buzzerStatus == 1, buzzer makes a single beep, 1 == short beep
        buzzerControl_buttonHandler(singleButton_buzzerStatus);
        dataAnalysis_singleButtonBuzzerStatus( singleButton_buzzerStatus );
#endif

    }
    if(fallingEdgeCount == 0)    // Ignores the rising edge after a long press
    {
        risingEdgeCount = 0;
        return;
    }
    if(logicLevel == 1)
    {
        risingEdgeCount++;
#ifdef CC2640R2_LAUNCHXL
        singleButton_buzzerStatus = 0; // single button will always be releases (end with a rising edge) after a press. Reset buzzer status to 0 on every release.
        buzzerControl_buttonHandler(singleButton_buzzerStatus);
        dataAnalysis_singleButtonBuzzerStatus( singleButton_buzzerStatus );
#endif
    }

    switch(buttonState)
    {
    case SINGLE_BUTTON_WAITING_STATE:
        {
            buttonState = SINGLE_BUTTON_EXECUTING_STATE;
            timerPeriod = SINGLE_BUTTON_TIMER_OV_TIME_LONG;
            singleButton_timerManager->timerSetPeriod(timerPeriod);
            singleButton_timerManager->timerStart();
            break;
        }
    case SINGLE_BUTTON_EXECUTING_STATE:
        {
            timerPeriod = SINGLE_BUTTON_TIMER_OV_TIME_SHORT;
            singleButton_timerManager->timerStop();
            singleButton_timerManager->timerSetPeriod(timerPeriod);
            singleButton_timerManager->timerStart();
            break;
        }
    default:
       break;
    }
}

/*********************************************************************
 * @fn      singleButton_processTimerOv
 *
 * @brief   Use this function on the timer overflow interrupt/callback
 *
 * @param   none
 *
 *
 * @return  none
 */
uint8_t buttonEvent = 0x00;

void singleButton_processTimerOv()
{
//    singleButton_timerManager->timerStop();
    // TOGGLE POWER ON/OFF (1 long press)
    if (risingEdgeCount == 0 && fallingEdgeCount == 1)
    {
        buttonEvent = 0x01;
    }
    // Change Light Mode (1 short press)
    else if (risingEdgeCount == 1 && fallingEdgeCount == 1)
    {
        buttonEvent = 0x02;                             //callback -> lightControl_change();
    }
    // TOGGLE BLE Advertising (1 short + 1 long presses)
    else if (risingEdgeCount == 1 && fallingEdgeCount == 2)
    {
        buttonEvent = 0x03;
    }
    // CHANGE SPEED MODE (2 short presses)
    else if (risingEdgeCount == 2 && fallingEdgeCount == 2)
    {
        buttonEvent = 0x04;
    }
    // TOGGLE UNITS METRIC/IMPERIAL (3 short presses)
    else if (risingEdgeCount == 3 && fallingEdgeCount == 3)
    {
        buttonEvent = 0x05;
    }
    // TOGGLE CONTROL LAW (4 short + 1 long presses)
    else if (risingEdgeCount == 4 && fallingEdgeCount == 5)
    {
        buttonEvent = 0x06;
    }
    // DO NOTHING
    else
    {
        buttonEvent = 0x00;
    }

#ifdef CC2640R2_LAUNCHXL
    /* reset buzzer to off here */
    buzzerControl_buttonHandler(0);
    dataAnalysis_singleButtonBuzzerStatus(0);
#endif

    timerPeriod = SINGLE_BUTTON_TIMER_OV_TIME_LONG;     // resets to "SINGLE_BUTTON_TIMER_OV_TIME_LONG" after each overflow
    risingEdgeCount = 0;                                // reset to 0
    fallingEdgeCount = 0;                               // reset to 0
    buttonState = SINGLE_BUTTON_WAITING_STATE;          // reset to 0

    singleButtonCBs -> singleButtonCB_t(buttonEvent);

}
