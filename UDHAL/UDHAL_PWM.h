/*
 * UDHAL_PWM.h
 *
 *  Created on: 10 Jan 2024
 *      Author: User
 */

#ifndef UDHAL_UDHAL_PWM_H_
#define UDHAL_UDHAL_PWM_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
/* Driver Header files */
#include <ti/drivers/PWM.h>

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * FUNCTIONS
 */
extern void UDHAL_PWM_init(void);
extern void UDHAL_PWM1_params_init(void);
extern void UDHAL_PWM2_params_init(void);
static void UDHAL_PWM1_pwmStart( void );
static void UDHAL_PWM2_pwmStart( void );
extern void UDHAL_PWM_pwmClose( void );
extern void UDHAL_PWM1_setDutyAndPeriod(uint8_t pwm1_DutyPercent, uint16_t pwm1_Period);
extern void UDHAL_PWM2_setDutyAndPeriod(uint16_t pwm2_Duty, uint16_t pwm2_Period);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif


#endif /* UDHAL_UDHAL_PWM_H_ */
