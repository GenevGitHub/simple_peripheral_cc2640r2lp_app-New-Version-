/*
 *  @file  UDHAL.c
 *  @brief This file contain the functions about
 *  Created on: 26 Jan 2022
 *      Author: harrysiu
 */
/*********************************************************************
 * INCLUDES
 */
#include "UDHAL/UDHAL.h"
#include "UDHAL/UDHAL_I2C.h"
#include "UDHAL/UDHAL_NVSINT.h"
#include "UDHAL/UDHAL_TIM1.h"
#include "UDHAL/UDHAL_TIM2.h"
#include "UDHAL/UDHAL_TIM3.h"
#include "UDHAL/UDHAL_TIM4.h"
//#include "UDHAL/UDHAL_TIM5.h"
#include "UDHAL/UDHAL_TIM6.h"
#include "UDHAL/UDHAL_ADC1.h"
#include "UDHAL/UDHAL_ADC2.h"
#include "UDHAL/UDHAL_UART.h"
#include "UDHAL/UDHAL_GPIO.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include <ti/devices/cc26x0r2/driverlib/cpu.h>
#include <UDHAL/UDHAL_PWM.h>
/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
/*********************************************************************
 * Marco
 */
/*********************************************************************
 * @fn      UDHAL_init
 *
 * @brief   It is used to initialize all peripherals
 *
 * @param   None
 *
 * @return  None
 */
uint8_t udhalcheck = 0;
void UDHAL_init()
{
    uint32_t bootSource = 0xFF;
    bootSource = SysCtrlResetSourceGet();
//    UDHAL_NVSINT_init();          // nvs internal
    //UDHAL_NVSINT_params_init();   // nvs internal
    udhalcheck = 1;

    UDHAL_GPIO_init();              // GPIO --> single button
    UDHAL_GPIO_params_init();       // GPIO --> single button
    udhalcheck = 2;

    UDHAL_I2C_init();               // i2C
    UDHAL_I2C_params_init();        // i2c
    udhalcheck = 3;

    UDHAL_TIM1_init();              // used for STM32MCP.h flow control retransmission
    UDHAL_TIM1_params_init();       // used for STM32MCP.h flow control retransmission
    udhalcheck = 4;

    UDHAL_TIM2_init();              // STM32MCP/STM32MCP.h to counter the heartbeat duration
    UDHAL_TIM2_params_init();       // STM32MCP/STM32MCP.h to counter the heartbeat duration
    //udhalcheck = 5;

    UDHAL_TIM3_init();              // single button source --> dependency --> GPIO must be initialized
    UDHAL_TIM3_params_init();       // single button header --> dependency --> GPIO must be initialized
    udhalcheck = 6;
/*  TImer 4 can be replaced with a task with task sleep = PERIODIC_COMMUNICATION_HF_SAMPLING_TIME   */
    UDHAL_TIM4_init();              // periodic communication hf
    UDHAL_TIM4_params_init();       // periodic communication hf
    udhalcheck = 7;

//    UDHAL_TIM5_init();              // periodic communication lf
//    UDHAL_TIM5_params_init();       // periodic communication lf
    udhalcheck = 8;

    UDHAL_TIM6_init();              // brake And Throttle
    UDHAL_TIM6_params_init();       // brake And Throttle
    udhalcheck = 9;

    UDHAL_ADC1_init();
    UDHAL_ADC1_params_init();
    udhalcheck = 11;

    UDHAL_ADC2_init();
    UDHAL_ADC2_params_init();
    udhalcheck = 12;

    UDHAL_UART_init();
    UDHAL_UART_params_init();
    udhalcheck = 13;

#ifdef CC2640R2_LAUNCHXL
    UDHAL_PWM_init();
    UDHAL_PWM1_params_init();
    UDHAL_PWM2_params_init();
    udhalcheck = 14;
#endif


}
/*********************************************************************
 * @fn:     UDHAL_I2C_getStatus
 *
 * @brief:  return I2C open status
 *
 * Arguments:   None
 *
 * Return:      None
*********************************************************************/
uint8_t UDHAL_getI2CStatus()
{
    return UDHAL_I2C_getOpenStatus();
}

/*Peripheral De-Initialization*/
void UDHAL_Boot()
{
    /*Initialize the timer for Boot*/
    UDHAL_TIM3_init();              // single button source --> dependency --> GPIO must be initialized
    UDHAL_TIM3_params_init();       // single button header --> dependency --> GPIO must be initialized
}
