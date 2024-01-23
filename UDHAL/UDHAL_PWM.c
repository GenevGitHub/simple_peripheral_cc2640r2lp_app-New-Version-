/*
 * UDHAL_PWM.c
 *
 *  Created on: 10 Jan 2024
 *      Author: User
 */
/*********************************************************************
 * INCLUDES
 */

/* Example/Board Header files */
#include <Board.h>
#include <UDHAL/UDHAL_PWM.h>
#include "application/ledControl.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
static PWM_Handle  UDHAL_PWM1_Handle; // = NULL;
static PWM_Params  UDHAL_PWM1_params;
static PWM_Handle  UDHAL_PWM2_Handle; // = NULL;
static PWM_Params  UDHAL_PWM2_params;

uint8_t     UDHAL_PWM1_pwmDutyPercent = 0;
uint32_t    UDHAL_PWM1_pwmDuty;
uint16_t    UDHAL_PWM1_pwmPeriod = 3200;
uint16_t    UDHAL_PWM2_pwmDuty = 0;
uint16_t    UDHAL_PWM2_pwmPeriod = 1000;

/*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/* ********************************************************************
 * @fn:     UDHAL_PWM_init
 *
 * @brief: It is used to initialize PWM
 *
 * Arguments:   None
 *
 * Return:      None
 ******************************************************************** */
extern void UDHAL_PWM_init()
{
    /* Initialize the PWM driver */
    PWM_init();
}
/*********************************************************************
 * @fn      UDHAL_PWM1_params_init
 *
 * @brief   It is used to initialize the PWM1 parameters - Frequency Mode
 *
 * @param   None
 *
 * @return  None
 */
void UDHAL_PWM1_params_init()
{
    /* Initialize the PWM parameters */
    PWM_Params_init(&UDHAL_PWM1_params);
    UDHAL_PWM1_pwmDuty =  (uint32_t) (((uint64_t) PWM_DUTY_FRACTION_MAX * UDHAL_PWM1_pwmDutyPercent) / 100);
    UDHAL_PWM1_params.idleLevel = PWM_IDLE_LOW;
    UDHAL_PWM1_params.periodUnits = PWM_PERIOD_HZ;
    UDHAL_PWM1_params.dutyUnits = PWM_DUTY_FRACTION;
    UDHAL_PWM1_params.dutyValue = UDHAL_PWM1_pwmDuty;
    UDHAL_PWM1_params.periodValue = UDHAL_PWM1_pwmPeriod;
    /* Dashboard Open the PWM instance */
#ifdef CC2640R2_GENEV_5X5_ID
    //UDHAL_PWM1_Handle = PWM_open(Board_PWM0, &UDHAL_PWM1_params); // for dashboard
#endif
    /* Launchpad Open the PWM instance */
#ifdef CC2640R2_LAUNCHXL
    UDHAL_PWM1_Handle = PWM_open(Board_PWM2, &UDHAL_PWM1_params);   // for launchpad
#endif

    UDHAL_PWM1_pwmStart();
}

/*********************************************************************
 * @fn      UDHAL_PWM2_params_init
 *
 * @brief   It is used to initialize the PWM2 parameters - Microseconds mode
 *
 * @param   None
 *
 * @return  None
 */
void UDHAL_PWM2_params_init()
{
    /* Initialize the PWM parameters */
    PWM_Params_init(&UDHAL_PWM2_params);
    UDHAL_PWM2_params.idleLevel = PWM_IDLE_LOW;
    UDHAL_PWM2_params.periodUnits = PWM_PERIOD_US;
    UDHAL_PWM2_params.dutyUnits = PWM_DUTY_US;
    UDHAL_PWM2_params.dutyValue = UDHAL_PWM2_pwmDuty;
    UDHAL_PWM2_params.periodValue = UDHAL_PWM2_pwmPeriod;
    /* Dashboard Open the PWM instance */
#ifdef CC2640R2_GENEV_5X5_ID
    //UDHAL_PWM2_Handle = PWM_open(Board_PWM1, &UDHAL_PWM2_params); // for dashboard
#endif
    /* Launchpad Open the PWM instance */
#ifdef CC2640R2_LAUNCHXL
    UDHAL_PWM2_Handle = PWM_open(Board_PWM0, &UDHAL_PWM2_params);   // launchpad red led
#endif
    UDHAL_PWM2_pwmStart();
}
/*********************************************************************
 * @fn:     UDHAL_PWM1_pwmStart
 *
 * @brief:  Start PWM instance 1 -> buzzer control
 *
 * Arguments:   None
 *
 * Return:      None
*********************************************************************/
static void UDHAL_PWM1_pwmStart( void )
{
    if (!UDHAL_PWM1_Handle) {
        /* Board_PWM did not open */
        /* error handling */
        /* PWM failure is critical & dangerous to the basic operation as it signifies micro-controller failure
         * Protocol -> shut down system
         * It is likely a hardware failure */
    }
    else {
        PWM_start(UDHAL_PWM1_Handle);         // start PWM with 0% duty cycle
    }
}
/*********************************************************************
 * @fn:     UDHAL_PWM2_pwmStart
 *
 * @brief:  Start PWM instance 2 -> light control
 *
 * Arguments:   None
 *
 * Return:      None
*********************************************************************/
static void UDHAL_PWM2_pwmStart( void )
{
    if (!UDHAL_PWM2_Handle) {   //= if (UDHAL_PWM1_Handle == NULL) {
        /* Board_PWM did not open */
        /* error handling */
        /* PWM failure is critical & dangerous to the basic operation as it signifies micro-controller failure
         * Protocol -> shut down system
         * It is likely a hardware failure */
    }
    else {
        PWM_start(UDHAL_PWM2_Handle);         // start PWM with 0% duty cycle
    }
}
/*********************************************************************
 * @fn:     UDHAL_PWM_pwmClose
 *
 * @brief:  Close PWM instance for shut down purposes
 *
 * Arguments:   PWMHandle
 *
 * Return:      None
*********************************************************************/
void UDHAL_PWM_pwmClose( void )
{
    /* Close the PWM instance */
    if (!UDHAL_PWM1_Handle) {
        /* Do nothing */
    }
    else {
        PWM_stop(UDHAL_PWM1_Handle);
        PWM_close(UDHAL_PWM1_Handle);
    }
    if (!UDHAL_PWM2_Handle) {
        /* Do nothing */
    }
    else {
        PWM_stop(UDHAL_PWM2_Handle);
        PWM_close(UDHAL_PWM2_Handle);
    }
}
/*********************************************************************
 * @fn      UDHAL_PWM1_setDutyAndPeriod
 *
 * @brief   set PWM duty and Period in Frequency mode
 *
 * @param   pwm_DutyPercent, pwm_Period
 *
 * @return  none
 */
void UDHAL_PWM1_setDutyAndPeriod(uint8_t pwm1_DutyPercent, uint16_t pwm1_Period)
{
    /* Calculate pwmDuty % */
    if (!UDHAL_PWM1_Handle) {
        /* What kind of error handling should be activated if PWM open fails? */
        ledControl_ErrorPriority(PWM1_OPEN_NULL);
    }
    else {
        UDHAL_PWM1_pwmDuty = (uint32_t) (((uint64_t) PWM_DUTY_FRACTION_MAX * pwm1_DutyPercent) / 100);
        PWM_setDutyAndPeriod(UDHAL_PWM1_Handle, UDHAL_PWM1_pwmDuty, pwm1_Period);        // set duty cycle to UDHAL_PWM1_pwmDuty % and set Period to pwmPeriod
    }
}
/*********************************************************************
 * @fn      UDHAL_PWM2_setDutyAndPeriod
 *
 * @brief   set PWM duty and Period in US mode
 *
 * @param   pwm_Duty, pwm_Period
 *
 * @return  none
 */
void UDHAL_PWM2_setDutyAndPeriod(uint16_t pwm2_Duty, uint16_t pwm2_Period)
{
    if (!UDHAL_PWM2_Handle) {
        /* Do nothing */
        ledControl_ErrorPriority(PWM2_OPEN_NULL);
    }
    else {
        PWM_setDutyAndPeriod(UDHAL_PWM2_Handle, pwm2_Duty, pwm2_Period);        // set duty cycle to pwm_Duty and set Period to pwmPeriod
    }
}


