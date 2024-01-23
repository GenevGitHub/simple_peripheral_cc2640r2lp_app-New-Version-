/*********************************************************************************************
 * buzzerControl.h
 *
 * Description:
 *
 *
 *  Created on: 14 Nov 2022
 *      Author: Chee
 *
 *********************************************************************************************/

#ifndef APPLICATION_BUZZERCONTROL_H_
#define APPLICATION_BUZZERCONTROL_H_

#ifdef _cplusplus
extern "C"
{
#endif
//
/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stddef.h>
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
* CONSTANTS
*/
// constants for buzzerStatus
#define BUZZER_NO_BEEP                              0x00
#define BUZZER_SINGLE_SHORT_BEEP                    0x01
#define BUZZER_SINGLE_LONG_BEEP                     0x02
#define BUZZER_WARNING_BEEP                         0x03

// Warning beeps starts after the following duration (in milliseconds)
#define BUZZER_WARNING_FREQUENCY                    3200   //  BUZZER frequency
#define BUZZER_WARNING_DUTYPERCENT                  5
#define BUZZER_WARNING_BEEP_PERIOD                  60000  //  BUZZER repeat beeps at the following period (in milliseconds) - Values should be in multiples of PERIODIC_COMMUNICATION_HF_SAMPLING_TIME
// singleButton beeps
#define BUZZER_SINGLEBUTTON_FREQUENCY               4200
#define BUZZER_SINGLEBUTTON_DUTYPERCENT             5

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */
extern void buzzerControl_init();
extern void buzzerControl_buttonHandler(uint8_t buttonStatus);
extern void buzzerControl_errorHandle(uint8_t warningDutyPercent, uint16_t warningPeriod);

#ifdef _cplusplus
}
#endif

#endif /* APPLICATION_BUZZERCONTROL_H_ */
