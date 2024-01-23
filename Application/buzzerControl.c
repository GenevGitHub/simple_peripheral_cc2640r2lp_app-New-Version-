/******************************************************************************

@file  buzzerControl.c

@brief This file contains the Code of buzzer control


*****************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include <stdlib.h>
/* Driver Header files */
#include <ti/drivers/PWM.h>
/* Example/Board Header files */
#include <Board.h>
#include <UDHAL/UDHAL_PWM.h>
#include "ledControl.h"
#include "dataAnalysis.h"
#include "buzzerControl.h"
#include "singleButton/singleButton.h"
/*********************************************************************
* LOCAL VARIABLES
*/
//static buzzerControl_timerManager_t *buzzerControl_timerManager;
//static buzzerControlCBs_t *buzzer_CBs;
/*********************************************************************
* LOCAL FUNCTIONS
*/
/*********************************************************************
 * @fn      buzzerControl_Init
 *
 * @brief   It is used to initialize the library
 *
 * @param   none
 *
 *
 * @return  none
 */
//uint8_t   buzzerControl_pwmOpenStatus = 0;

void buzzerControl_init()
{

}

/*********************************************************************
 * @fn      buzzerControl_buttonHandler
 *
 * @brief   buzzer execution when single button is pressed
 *
 * @param   buttonStatus
 *
 * @return  none
 */
uint8_t buzzerStart = 0;
uint8_t buzzerControl_buttonStatus = 0;
void buzzerControl_buttonHandler(uint8_t buttonStatus)
{
    /* Start buzzer if Buzzer is OFF */
    buzzerControl_buttonStatus = buttonStatus; // if buzzerControl_buttonStatus is not 0, sound the buzzer
    if ((buzzerControl_buttonStatus != 0) && (buzzerStart == 0)){
        UDHAL_PWM1_setDutyAndPeriod(BUZZER_SINGLEBUTTON_DUTYPERCENT, BUZZER_SINGLEBUTTON_FREQUENCY);
        buzzerStart = 2;                   // buzzerStart = 2 means buzzer is activated due to falling edge of single button press
    }
    /* Stop buzzer if buzzer is ON */
    if (buzzerControl_buttonStatus == 0){
        if (buzzerStart >= 2){
            UDHAL_PWM1_setDutyAndPeriod(0, BUZZER_SINGLEBUTTON_FREQUENCY);   // set duty to 0
            buzzerStart = 0;
        }
    }

}

/*********************************************************************
 * @fn      buzzerControl_errorHandler
 *
 * @brief   buzzer execution when system error is present
 *
 * @param   warningDutyPercent, warningPeriod
 *
 * @return  none
 */
void buzzerControl_errorHandle(uint8_t warningDutyPercent, uint16_t warningPeriod)
{
    UDHAL_PWM1_setDutyAndPeriod(warningDutyPercent, warningPeriod);
}
