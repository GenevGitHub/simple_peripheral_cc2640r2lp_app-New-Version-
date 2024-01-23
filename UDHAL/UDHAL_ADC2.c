/*
 *  UDHAL_ADC2.c
 *  ADC2 is assigned for reading throttle ADC signals
 *
 *  Created on: 14 Feb 2022
 *      Author: harrysiu
 */

/*********************************************************************
 * INCLUDES
 */
#include <ti/drivers/ADC.h>
#include <stdint.h>
#include "UDHAL/UDHAL_ADC2.h"
#include "brakeAndThrottle.h"
#include "Application/ledControl.h"
#include <Board.h>
/*********************************************************************
 * LOCAL VARIABLES
 */
static ADC_Handle adc2Handle;
static ADC_Params adc2Params;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void UDHAL_ADC2_Open();
static void UDHAL_ADC2_Convert(uint16_t *result);
static void UDHAL_ADC2_Close();
/*********************************************************************
 * Marco
 */
static brakeAndThrottle_adcManager_t brake_adcManager =
{
     UDHAL_ADC2_Open,
     UDHAL_ADC2_Convert,
     UDHAL_ADC2_Close
};
/*********************************************************************
 *
 * @fn      UDHAL_ADC2_init
 *
 * @brief   To initialize the timer and uart tx function
 *
 * @param   None.
 *
 * @return  None.
 */
void UDHAL_ADC2_init()
{
    ADC_init();
    brakeAndThrottle_registerADC2(&brake_adcManager);
}
/*********************************************************************
 *
 * @fn      UDHAL_ADC2_params_init
 *
 * @brief   To initialize the timer and uart tx function
 *
 * @param   None.
 *
 * @return  None.
 */
void UDHAL_ADC2_params_init()
{
    ADC_Params_init(&adc2Params);
    adc2Params.isProtected = true;
}
/*********************************************************************
 * @fn      UDHAL_ADC2_Open
 *
 * @brief
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC2_Open()
{
    adc2Handle = ADC_open(Board_ADC1, &adc2Params);  //ADC1 = IOID24
    if (!adc2Handle) {
        /* error handling */
        /* ADC failure is critical & dangerous to the basic operation as it signifies micro-controller failure
         * Protocol -> shut down system
         * It is likely a hardware failure */
    }
}
/*********************************************************************
 * @fn      UDHAL_ADC2_Convert
 *
 * @brief
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC2_Convert(uint16_t *result)
{
    if (!adc2Handle) {
        ledControl_ErrorPriority(ADC2_OPEN_NULL);
    }
    else {
        ADC_convert(adc2Handle, result);
    }
}
/*********************************************************************
 * @fn      UDHAL_ADC2_Close
 *
 * @brief
 *
 * @param   None.
 *
 * @return  None.
 */
static void UDHAL_ADC2_Close()
{
    if (!adc2Handle) {

    }
    else {
        ADC_close(adc2Handle);
    }
}
