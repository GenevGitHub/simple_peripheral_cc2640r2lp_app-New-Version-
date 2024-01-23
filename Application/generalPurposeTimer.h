/*
 * generalPurposeTimer.h
 *
 *  Created on: 12 Jul 2023
 *      Author: User
 */

#ifndef APPLICATION_GENERALPURPOSETIMER_H_
#define APPLICATION_GENERALPURPOSETIMER_H_
//
#ifdef __cplusplus
extern "C"
{
#endif
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************/
//#define TIMER_INTERVAL              200  //

// Task configuration
#define GPT_TASK_PRIORITY           4

#ifndef GPT_TASK_STACK_SIZE
#define GPT_TASK_STACK_SIZE         416  //400 //360// Stack size must be multiples of 8
#endif

/* ********************************************************************
 * TYPEDEFS
*/


/* ********************************************************************
* MACROS
*/

/* ********************************************************************
 * API FUNCTIONS
 */
void Power(int16_t status);

void PWR_CTL(uint8_t PWR);

bool PWR_OFF();

extern void GPTimer_init();

static void GPtimer_taskFxn(UArg a0, UArg a1);
/* Task creation function for the general purpose timer */
extern void GPtimer_createTask(void);

#ifdef __cplusplus
}
#endif
//
#endif /* APPLICATION_GENERALPURPOSETIMER_H_ */
