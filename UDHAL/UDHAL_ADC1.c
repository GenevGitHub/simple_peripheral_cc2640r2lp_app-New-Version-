/*
 *  UDHAL_ADC1.c
 *  ADC1 is assigned for reading brake ADC signals
 *
 *  Created on: 14 Feb 2022
 *      Author: harrysiu
 */

/*********************************************************************
 * INCLUDES
 */

#include <ti/drivers/ADC.h>
#include <stdint.h>
#include "UDHAL/UDHAL_ADC1.h"
#include "brakeAndThrottle.h"
#include "Application/ledControl.h"
#include <Board.h>
/*********************************************************************
 * LOCAL VARIABLES
 */
static ADC_Handle adc1Handle;
static ADC_Params adc1Params;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void UDHAL_ADC1_Open();
static void UDHAL_ADC1_Convert(uint16_t *result);
static void UDHAL_ADC1_Close();
/*********************************************************************
 * Marco
 */
static brakeAndThrottle_adcManager_t brake_adcManager =
{
     UDHAL_ADC1_Open,
     UDHAL_ADC1_Convert,
     UDHAL_ADC1_Close
};
/*********************************************************************
 *
 * @fn      UDHAL_ADC1_init
 *
 * @brief   To initialize the timer and uart tx function
 *
 * @param   None.
 *
 * @return  None.
 */
void UDHAL_ADC1_init()
{
    ADC_init();
    brakeAndThrottle_registerADC1(&brake_adcManager);
}
/*********************************************************************
 *
 * @fn      UDHAL_ADC1_params_init
 *
 * @brief   To initialize the timer and uart tx function
 *
 * @param   None.
 *
 * @return  None.
 */
void UDHAL_ADC1_params_init()
{
    ADC_Params_init(&adc1Params);
    adc1Params.isProtected = true;
}
/*********************************************************************
 * @fn      UDHAL_ADC1_Open
 *
 * @brief
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC1_Open()
{
    adc1Handle = ADC_open(Board_ADC0, &adc1Params);  //ADC0 = IOID23
    if (!adc1Handle) {
        /* error handling */
        /* ADC failure is critical & dangerous to the basic operation as it signifies micro-controller failure
         * Protocol -> shut down system
         * It is likely a hardware failure */
    }

}
/*********************************************************************
 * @fn      UDHAL_ADC1_Convert
 *
 * @brief   To start the timer for timeout.
 *          This function will be used by STM32MCP flow retransmission.
 *          Put it into the STM32MCP_params_t structure to register as callback function
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC1_Convert(uint16_t *result)
{
    if (!adc1Handle) {
        ledControl_ErrorPriority(ADC1_OPEN_NULL);
    }
    else {
        ADC_convert(adc1Handle, result);
    }
}
/*********************************************************************
 * @fn      UDHAL_ADC1_Close
 *
 * @brief   To stop the timer for flow control timeout.
 *          This function will be used by STM32MCP flow retransmission.
 *          Put it into the STM32MCP_params_t structure to register as callback function.
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC1_Close()
{
    if (!adc1Handle) {

    }
    else {
        ADC_close(adc1Handle);
    }
}
