/*
 * ledControl.c
 *
 *  Created on: 30 Nov 2022
 *      Author: User
 */
/******************************************************************************
@file  ledControl.c
*
@brief This file contains the Code of led display control
*
*
*/
/*********************************************************************
* INCLUDES
*/
#include "ledControl.h"
#include "IS31FL3236A/IS31FL3236A.h"
#include "Controller.h"
#include "Dashboard.h"
#include "Battery.h"
#include "peripheral.h"

/*********************************************************************
* LOCAL VARIABLES
*/
uint8_t ledSetpower;
uint8_t ledSpeed;
uint8_t ledBatteryStatus;
uint8_t ledSpeedMode;
uint8_t ledUnitSelectDash;
uint8_t ledBLEStatus;
uint8_t ledErrorCode;
uint8_t ledLightmode;
uint8_t ledLightstatus;
uint8_t ledControl_i2cTransferStatus;
uint8_t ledLightSelect_old;
uint8_t ledBLESelect_old;
uint8_t ledUnitSelect_old;
uint8_t ledSMSelect_old;
uint8_t ledSpeed_old;
uint8_t ledBattery_old;
uint8_t ledError_old;
uint8_t ledLightstatus_old;
uint8_t ledPower_old;
uint8_t ledBrightness;
uint8_t ledBrightness_old;
/*********************************************************************
*
* LOCAL FUNCTIONS
*/
static ledControl_ledDisplayManager_t *ledControl_ledDisplayManager;


//LED basic functions
void reset_led_driver()
{
    uint8_t writeBuf_reset[2] = {IS31FL3236A_RESET_REG, 0x00};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_reset, sizeof(writeBuf_reset), NULL, 0);
}


void turn_on_led_driver()
{
    uint8_t writeBuf_shutdown[2] = {IS31FL3236A_ON_REG, ZERO_ONE_CUSTOM};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_shutdown, sizeof(writeBuf_shutdown), NULL, 0);
}

void turn_off_led_driver()
{
    uint8_t writeBuf_shutdown[2] = {IS31FL3236A_ON_REG, ZERO_ZERO_CUSTOM};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_shutdown, sizeof(writeBuf_shutdown), NULL, 0);
}

void disable_channels()
{
    uint8_t writeBuf_disable[2] = {IS31FL3236A_ENABLE_REG, 0x01};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_disable, sizeof(writeBuf_disable), NULL, 0);
}

void enable_channels()
{
    uint8_t writeBuf_enable[2] = {IS31FL3236A_ENABLE_REG, 0x00};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_enable, sizeof(writeBuf_enable), NULL, 0);
}


void update_bit()
{
    uint8_t writeBuf_update[2] = {IS31FL3236A_UPDATE_REG, 0x00};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_update, sizeof(writeBuf_update), NULL, 0);
}

void PWM_Frequency_22k()
{
    uint8_t writeBuf_22k[2] = {IS31FL3236A_PWM_FREQUENCY, 0x01};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_22k, sizeof(writeBuf_22k), NULL, 0);
}
void PWM_Frequency_3k()
{
    uint8_t writeBuf_3k[2] = {IS31FL3236A_PWM_FREQUENCY, 0x00};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_3k, sizeof(writeBuf_3k), NULL, 0);
}

void IS31FL3236A_Sports_Mode_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED43_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED43_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_Leisure_Mode_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED35_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED35_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_Amble_Mode_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED34_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED34_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_Sports_Mode(uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Sports_Mode_pin(status_buf, brightness_buf);
    IS31FL3236A_Leisure_Mode_pin(status_buf, PWM_ZERO);
    IS31FL3236A_Amble_Mode_pin(status_buf, PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Leisure_Mode(uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Sports_Mode_pin(status_buf, PWM_ZERO);
    IS31FL3236A_Leisure_Mode_pin(status_buf, brightness_buf);
    IS31FL3236A_Amble_Mode_pin(status_buf, PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Amble_Mode(uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Sports_Mode_pin(status_buf, PWM_ZERO);
    IS31FL3236A_Leisure_Mode_pin(status_buf, PWM_ZERO);
    IS31FL3236A_Amble_Mode_pin(status_buf, brightness_buf);
    update_bit();
}

void IS31FL3236A_Bluetooth_LED(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED19_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED19_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
    update_bit();
}

void IS31FL3236A_Light_On(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED20_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED20_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
    update_bit();
}

void IS31FL3236A_Auto_Mode_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED21_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED21_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
    update_bit();
}

void IS31FL3236A_Auto_Mode( uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Light_On(status_buf, brightness_buf);
    IS31FL3236A_Auto_Mode_pin(status_buf, brightness_buf);
}

void IS31FL3236A_Attention(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED22_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED22_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
    update_bit();
}

void IS31FL3236A_1_Bar(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED1_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED1_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_2_Bar(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED2_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED2_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_3_Bar(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED6_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED6_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_4_Bar(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED10_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED10_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_5_Bar(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED11_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED11_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_kmph_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED4_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED4_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
}

void IS31FL3236A_mph_pin(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED7_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED7_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
}
void IS31FL3236A_kmph(uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_kmph_pin(status_buf, brightness_buf);        // mph Off
    IS31FL3236A_mph_pin(status_buf, PWM_ZERO);               // kmph On
    update_bit();
}
void IS31FL3236A_mph(uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_kmph_pin(status_buf, PWM_ZERO);              // kmph Off
    IS31FL3236A_mph_pin(status_buf, brightness_buf);         // mph On
    update_bit();
}

void IS31FL3236A_Digit_1_PIN_44(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED44_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED44_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_3(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED3_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED3_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_5(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED5_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED5_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_30(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED30_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED30_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_31(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED31_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED31_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_32(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED32_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED32_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_1_PIN_33(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED33_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED33_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}

void IS31FL3236A_Digit_2_PIN_8(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED8_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED8_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_9(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED9_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED9_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_12(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED12_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED12_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_27(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED27_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED27_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_28(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED28_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED28_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_29(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED29_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED29_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);

}
void IS31FL3236A_Digit_2_PIN_13(uint8_t status_buf, uint8_t brightness_buf)
{
    uint8_t writeBuf_pwm[2] = {IS31FL3236A_LED13_PWM_ADDR, brightness_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_pwm, sizeof(writeBuf_pwm), NULL, 0);
    uint8_t writeBuf_led[2] = {IS31FL3236A_LED13_ADDR, status_buf};
    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &writeBuf_led, sizeof(writeBuf_led), NULL, 0);
    update_bit();
}

void IS31FL3236A_All_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,brightness_buf);
    IS31FL3236A_2_Bar(status_buf,brightness_buf);
    IS31FL3236A_3_Bar(status_buf,brightness_buf);
    IS31FL3236A_4_Bar(status_buf,brightness_buf);
    IS31FL3236A_5_Bar(status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_4_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,brightness_buf);
    IS31FL3236A_2_Bar(status_buf,brightness_buf);
    IS31FL3236A_3_Bar(status_buf,brightness_buf);
    IS31FL3236A_4_Bar(status_buf,brightness_buf);
    IS31FL3236A_5_Bar(status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_3_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,brightness_buf);
    IS31FL3236A_2_Bar(status_buf,brightness_buf);
    IS31FL3236A_3_Bar(status_buf,brightness_buf);
    IS31FL3236A_4_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_5_Bar(status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_2_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,brightness_buf);
    IS31FL3236A_2_Bar(status_buf,brightness_buf);
    IS31FL3236A_3_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_4_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_5_Bar(status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_1_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,brightness_buf);
    IS31FL3236A_2_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_3_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_4_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_5_Bar(status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_0_Bars (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_1_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_2_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_3_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_4_Bar(status_buf,PWM_ZERO);
    IS31FL3236A_5_Bar(status_buf,PWM_ZERO);
    update_bit();
}


void IS31FL3236A_Digit_1_Number_0 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32(status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33(status_buf,PWM_ZERO);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_1 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);        // PIN_44 off
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);         // PIN_3 off
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);   // PIN_5 on
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);  // PIN_30 on
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);        // PIN_31 off
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);        // PIN_32 off
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);        // PIN_33 off
    update_bit();
}

void IS31FL3236A_Digit_1_Number_2 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_3 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_4 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_5 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_6 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_7 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_8 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_9 (uint8_t status_buf, uint8_t brightness_buf){

    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}

void IS31FL3236A_Digit_2_Number_0 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_1 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_2 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_3 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_4 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_5 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_6 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_7 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_8 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void IS31FL3236A_Digit_2_Number_9 (uint8_t status_buf, uint8_t brightness_buf){

        IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
        IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
        update_bit();
}

void Brightness (uint8_t status_buf, uint8_t brightness_buf)
{

        IS31FL3236A_Sports_Mode_pin(status_buf,brightness_buf);
        IS31FL3236A_Leisure_Mode_pin(status_buf,brightness_buf);
        IS31FL3236A_Amble_Mode_pin(status_buf,brightness_buf);
        IS31FL3236A_Bluetooth_LED(status_buf,brightness_buf);
        IS31FL3236A_Light_On(status_buf,brightness_buf);
        IS31FL3236A_Auto_Mode_pin(status_buf,brightness_buf);
        IS31FL3236A_Attention(status_buf,brightness_buf);
        IS31FL3236A_1_Bar(status_buf,brightness_buf);
        IS31FL3236A_2_Bar(status_buf,brightness_buf);
        IS31FL3236A_3_Bar(status_buf,brightness_buf);
        IS31FL3236A_4_Bar(status_buf,brightness_buf);
        IS31FL3236A_5_Bar(status_buf,brightness_buf);
        IS31FL3236A_kmph_pin(status_buf,brightness_buf);
        IS31FL3236A_mph_pin(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_44(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_3(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_5(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_30(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_31(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_32(status_buf,brightness_buf);
        IS31FL3236A_Digit_1_PIN_33(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_8(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_9(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_12(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_27(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_28(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_29(status_buf,brightness_buf);
        IS31FL3236A_Digit_2_PIN_13(status_buf,brightness_buf);
}



void IS31FL3236A_Digit_1_Number_A (uint8_t status_buf, uint8_t brightness_buf){
       IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
       update_bit();
}
void IS31FL3236A_Digit_2_Number_A (uint8_t status_buf, uint8_t brightness_buf){
       IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
       IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
       IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
       IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
       IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
       IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
       update_bit();
}
void IS31FL3236A_Digit_1_Number_C (uint8_t status_buf, uint8_t brightness_buf){
       IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
       IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
       update_bit();
}
void IS31FL3236A_Digit_2_Number_C (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
}
void IS31FL3236A_Digit_1_Number_E (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_E (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();

}
void IS31FL3236A_Digit_1_Number_F (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_F (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_H (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_H (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_J (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_J (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_L (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_L (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_P (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_P (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_U (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_U (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_u (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,PWM_ZERO);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_u (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,PWM_ZERO);
    update_bit();
}

void IS31FL3236A_Digit_1_Number_n (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_n (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_b (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_b (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_c (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_c (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_h (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_h (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_d (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_32( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_d (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_29( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_q (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_3( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_5( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_30( status_buf,brightness_buf);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_q (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_9( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_12( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_27( status_buf,brightness_buf);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_1_Number_ (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_1_PIN_44( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_3( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_5( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_30( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_31( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_32( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_1_PIN_33( status_buf,brightness_buf);
    update_bit();
}
void IS31FL3236A_Digit_2_Number_ (uint8_t status_buf, uint8_t brightness_buf){
    IS31FL3236A_Digit_2_PIN_8( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_9( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_12( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_27( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_28( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_29( status_buf,PWM_ZERO);
    IS31FL3236A_Digit_2_PIN_13( status_buf,brightness_buf);
    update_bit();
}



void Flashing (uint8_t status_buf, uint8_t brightness_buf, uint8_t index){
    if(brightness_buf==PWM_ZERO){
    functionTable[index]( status_buf, brightness_buf);
    functionTable[index]( status_buf, PWM_ZERO);
    };
    if(brightness_buf==brightness_buf){
    functionTable[index]( status_buf, PWM_ZERO);
    functionTable[index]( status_buf, brightness_buf);
    };

}

void LED_Turn_OFF_ALL(){
       IS31FL3236A_Sports_Mode_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_Leisure_Mode_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_Amble_Mode_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_Bluetooth_LED(I_OUT,PWM_ZERO);
       IS31FL3236A_Light_On(I_OUT,PWM_ZERO);
       IS31FL3236A_Auto_Mode_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_Attention(I_OUT,PWM_ZERO);
       IS31FL3236A_1_Bar(I_OUT,PWM_ZERO);
       IS31FL3236A_2_Bar(I_OUT,PWM_ZERO);
       IS31FL3236A_3_Bar(I_OUT,PWM_ZERO);
       IS31FL3236A_4_Bar(I_OUT,PWM_ZERO);
       IS31FL3236A_5_Bar(I_OUT,PWM_ZERO);
       IS31FL3236A_kmph_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_mph_pin(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_44(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_3(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_5(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_30(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_31(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_32(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_1_PIN_33(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_8(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_9(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_12(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_27(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_28(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_29(I_OUT,PWM_ZERO);
       IS31FL3236A_Digit_2_PIN_13(I_OUT,PWM_ZERO);
       update_bit();
}

void LED_Turn_ON_ALL(){
       IS31FL3236A_Sports_Mode_pin(I_OUT,ledBrightness);
       IS31FL3236A_Leisure_Mode_pin(I_OUT,ledBrightness);
       IS31FL3236A_Amble_Mode_pin(I_OUT,ledBrightness);
       IS31FL3236A_Bluetooth_LED(I_OUT,ledBrightness);
       IS31FL3236A_Light_On(I_OUT,ledBrightness);
       IS31FL3236A_Auto_Mode_pin(I_OUT,ledBrightness);
       IS31FL3236A_Attention(I_OUT,ledBrightness);
       IS31FL3236A_1_Bar(I_OUT,ledBrightness);
       IS31FL3236A_2_Bar(I_OUT,ledBrightness);
       IS31FL3236A_3_Bar(I_OUT,ledBrightness);
       IS31FL3236A_4_Bar(I_OUT,ledBrightness);
       IS31FL3236A_5_Bar(I_OUT,ledBrightness);
       IS31FL3236A_kmph_pin(I_OUT,ledBrightness);
       IS31FL3236A_mph_pin(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_44(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_3(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_5(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_30(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_31(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_32(I_OUT,ledBrightness);
       IS31FL3236A_Digit_1_PIN_33(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_8(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_9(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_12(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_27(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_28(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_29(I_OUT,ledBrightness);
       IS31FL3236A_Digit_2_PIN_13(I_OUT,ledBrightness);
       update_bit();
}
// Function tables

IS31FL3236A_Function functionTable[FUNCTION_COUNT] = {
    Brightness,                         //0
    IS31FL3236A_Sports_Mode,
    IS31FL3236A_Leisure_Mode,
    IS31FL3236A_Amble_Mode,
    IS31FL3236A_Bluetooth_LED,
    IS31FL3236A_Light_On,
    IS31FL3236A_Auto_Mode_pin,
    IS31FL3236A_Attention,
    IS31FL3236A_1_Bar,
    IS31FL3236A_2_Bar,
    IS31FL3236A_3_Bar,                  //10
    IS31FL3236A_4_Bar,
    IS31FL3236A_5_Bar,
    IS31FL3236A_kmph,
    IS31FL3236A_mph,
    IS31FL3236A_Digit_1_PIN_44,
    IS31FL3236A_Digit_1_PIN_3,
    IS31FL3236A_Digit_1_PIN_5,
    IS31FL3236A_Digit_1_PIN_30,
    IS31FL3236A_Digit_1_PIN_31,
    IS31FL3236A_Digit_1_PIN_32,         //20
    IS31FL3236A_Digit_1_PIN_33,
    IS31FL3236A_Digit_2_PIN_8,
    IS31FL3236A_Digit_2_PIN_9,
    IS31FL3236A_Digit_2_PIN_12,
    IS31FL3236A_Digit_2_PIN_27,
    IS31FL3236A_Digit_2_PIN_28,
    IS31FL3236A_Digit_2_PIN_29,
    IS31FL3236A_Digit_2_PIN_13,
    IS31FL3236A_All_Bars,               //29
    IS31FL3236A_4_Bars,                 //30
    IS31FL3236A_3_Bars,                 //31
    IS31FL3236A_2_Bars,                 //32
    IS31FL3236A_1_Bars,                 //33
    IS31FL3236A_0_Bars,                 //34
    IS31FL3236A_Digit_1_Number_0,
    IS31FL3236A_Digit_1_Number_1,
    IS31FL3236A_Digit_1_Number_2,
    IS31FL3236A_Digit_1_Number_3,
    IS31FL3236A_Digit_1_Number_4,
    IS31FL3236A_Digit_1_Number_5,
    IS31FL3236A_Digit_1_Number_6,
    IS31FL3236A_Digit_1_Number_7,
    IS31FL3236A_Digit_1_Number_8,
    IS31FL3236A_Digit_1_Number_9,
    IS31FL3236A_Digit_2_Number_0,
    IS31FL3236A_Digit_2_Number_1,
    IS31FL3236A_Digit_2_Number_2,
    IS31FL3236A_Digit_2_Number_3,
    IS31FL3236A_Digit_2_Number_4,
    IS31FL3236A_Digit_2_Number_5,
    IS31FL3236A_Digit_2_Number_6,
    IS31FL3236A_Digit_2_Number_7,
    IS31FL3236A_Digit_2_Number_8,
    IS31FL3236A_Digit_2_Number_9,
    IS31FL3236A_Digit_1_Number_A,
    IS31FL3236A_Digit_2_Number_A,
    IS31FL3236A_Digit_1_Number_C,
    IS31FL3236A_Digit_2_Number_C,
    IS31FL3236A_Digit_1_Number_E,
    IS31FL3236A_Digit_2_Number_E,
    IS31FL3236A_Digit_1_Number_F,
    IS31FL3236A_Digit_2_Number_F,
    IS31FL3236A_Digit_1_Number_H,
    IS31FL3236A_Digit_2_Number_H,
    IS31FL3236A_Digit_1_Number_J,
    IS31FL3236A_Digit_2_Number_J,
    IS31FL3236A_Digit_1_Number_L,
    IS31FL3236A_Digit_2_Number_L,
    IS31FL3236A_Digit_1_Number_P,
    IS31FL3236A_Digit_2_Number_P,
    IS31FL3236A_Digit_1_Number_U,
    IS31FL3236A_Digit_2_Number_U,
    IS31FL3236A_Digit_1_Number_u,
    IS31FL3236A_Digit_2_Number_u,
    IS31FL3236A_Digit_1_Number_n,
    IS31FL3236A_Digit_2_Number_n,
    IS31FL3236A_Digit_1_Number_b,
    IS31FL3236A_Digit_2_Number_b,
    IS31FL3236A_Digit_1_Number_c,
    IS31FL3236A_Digit_2_Number_c,
    IS31FL3236A_Digit_1_Number_h,
    IS31FL3236A_Digit_2_Number_h,
    IS31FL3236A_Digit_1_Number_d,
    IS31FL3236A_Digit_2_Number_d,
    IS31FL3236A_Digit_1_Number_q,
    IS31FL3236A_Digit_2_Number_q,
    IS31FL3236A_Digit_1_Number_,
    IS31FL3236A_Digit_2_Number_,

};

//Steps to call
//IS31FL3236A_FunctionIndex index = LIGHT_ON;
//functionTable[index]( status_buf, brightness_buf);


//Brightness 32 Gamma Steps
const uint8_t brightness_32_ori[]={
                               0x00, 0x01, 0x02, 0x04, 0x06, 0x0A, 0x0D, 0x12,
                               0x16, 0x1C, 0x21, 0x27, 0x2E, 0x35, 0x3D, 0x45,
                               0x4E, 0x56, 0x60, 0x6A, 0x74, 0x7E, 0x8A, 0x95,
                               0xA1, 0xAD, 0xBA, 0xC7, 0xD4, 0xE2, 0xF0, 0xFF
                                };
const uint8_t brightness_32[]={
                               0x00, 0x01, 0x02, 0x04, 0x06, 0x0A, 0x0D, 0x12,
                               0x16, 0x1C, 0x21, 0x27, 0x2E, 0x35, 0x3D, 0x45,
                               0x4E, 0x56, 0x60, 0x6A, 0x74, 0x7E, 0x8A, 0x95,
                               0xA1, 0xAD, 0xBA, 0xC7, 0xD4, 0xE2, 0xF0, 0xFF
                                };

//Brightness 64 Gamma Steps
const uint8_t brightness_64_ori[]={
                               0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16,
                               0x18, 0x1A, 0x1D, 0x20, 0x23, 0x26, 0x29, 0x2C,
                               0x2F, 0x32, 0x35, 0x39, 0x3D, 0x41, 0x45, 0x49,
                               0x4D, 0x51, 0x55, 0x59, 0x5E, 0x63, 0x68, 0x6D,
                               0x72, 0x77, 0x7C, 0x81, 0x86, 0x8C, 0x92, 0x98,
                               0x9E, 0xA4, 0xAA, 0xB0, 0xB6, 0xBC, 0xC3, 0xCA,
                               0xD1, 0xD8, 0xDF, 0xE6, 0xED, 0xF4, 0xFB, 0xFF
                                };
const uint8_t brightness_64[]={
                               0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16,
                               0x18, 0x1A, 0x1D, 0x20, 0x23, 0x26, 0x29, 0x2C,
                               0x2F, 0x32, 0x35, 0x39, 0x3D, 0x41, 0x45, 0x49,
                               0x4D, 0x51, 0x55, 0x59, 0x5E, 0x63, 0x68, 0x6D,
                               0x72, 0x77, 0x7C, 0x81, 0x86, 0x8C, 0x92, 0x98,
                               0x9E, 0xA4, 0xAA, 0xB0, 0xB6, 0xBC, 0xC3, 0xCA,
                               0xD1, 0xD8, 0xDF, 0xE6, 0xED, 0xF4, 0xFB, 0xFF
                                };
// LED pin array
const uint8_t led_array[][2] = {
    {IS31FL3236A_LED43_ADDR, IS31FL3236A_LED43_PWM_ADDR},
    {IS31FL3236A_LED44_ADDR, IS31FL3236A_LED44_PWM_ADDR},
    {IS31FL3236A_LED1_ADDR,  IS31FL3236A_LED1_PWM_ADDR},
    {IS31FL3236A_LED2_ADDR,  IS31FL3236A_LED2_PWM_ADDR},
    {IS31FL3236A_LED3_ADDR,  IS31FL3236A_LED3_PWM_ADDR},
    {IS31FL3236A_LED4_ADDR,  IS31FL3236A_LED4_PWM_ADDR},
    {IS31FL3236A_LED5_ADDR,  IS31FL3236A_LED5_PWM_ADDR},
    {IS31FL3236A_LED6_ADDR,  IS31FL3236A_LED6_PWM_ADDR},
    {IS31FL3236A_LED7_ADDR,  IS31FL3236A_LED7_PWM_ADDR},
    {IS31FL3236A_LED8_ADDR,  IS31FL3236A_LED8_PWM_ADDR},
    {IS31FL3236A_LED9_ADDR,  IS31FL3236A_LED9_PWM_ADDR},
    {IS31FL3236A_LED10_ADDR, IS31FL3236A_LED10_PWM_ADDR},
    {IS31FL3236A_LED11_ADDR, IS31FL3236A_LED11_PWM_ADDR},
    {IS31FL3236A_LED12_ADDR, IS31FL3236A_LED12_PWM_ADDR},
    {IS31FL3236A_LED13_ADDR, IS31FL3236A_LED13_PWM_ADDR},
    {IS31FL3236A_LED19_ADDR, IS31FL3236A_LED19_PWM_ADDR},
    {IS31FL3236A_LED20_ADDR, IS31FL3236A_LED20_PWM_ADDR},
    {IS31FL3236A_LED21_ADDR, IS31FL3236A_LED21_PWM_ADDR},
    {IS31FL3236A_LED22_ADDR, IS31FL3236A_LED22_PWM_ADDR},
    {IS31FL3236A_LED27_ADDR, IS31FL3236A_LED27_PWM_ADDR},
    {IS31FL3236A_LED28_ADDR, IS31FL3236A_LED28_PWM_ADDR},
    {IS31FL3236A_LED29_ADDR, IS31FL3236A_LED29_PWM_ADDR},
    {IS31FL3236A_LED30_ADDR, IS31FL3236A_LED30_PWM_ADDR},
    {IS31FL3236A_LED31_ADDR, IS31FL3236A_LED31_PWM_ADDR},
    {IS31FL3236A_LED32_ADDR, IS31FL3236A_LED32_PWM_ADDR},
    {IS31FL3236A_LED33_ADDR, IS31FL3236A_LED33_PWM_ADDR},
    {IS31FL3236A_LED34_ADDR, IS31FL3236A_LED34_PWM_ADDR},
    {IS31FL3236A_LED35_ADDR, IS31FL3236A_LED35_PWM_ADDR},
};






/*********************************************************************
 * @fn      ledcontrol_setLEDPower
 *
 * @brief   call this function to set brightness (power) of the LED display
 *
 * @param   light_status
 *
 * @return  None
 *********************************************************************/
void ledControl_setLEDPower(uint8_t ledPower){
    // I2C command to set LED Power
    ledSetpower = ledPower;
}
void ledControl_changeLEDPower(){
    if(ledPower_old!=ledSetpower){
        if(ledSetpower==60){
            ledBrightness=PWM_LOW;
        }
        else{
            ledBrightness=PWM_CUSTOM;
        }
        ledPower_old = ledSetpower;
    }
}

/*********************************************************************
 * @fn      ledControl_Init
 *
 * @brief   It is used to initialize the library
 *
 * @param   none
 *
 * @return  none
 */
void ledControl_init()
{
    // At Start-Up/initiation, lit all LED on Dashboard. Then, turn off ones that are not required to be on.
    // initialize ledPower
    // set all led lights on at Power On ->
    reset_led_driver();
    turn_on_led_driver();
    enable_channels();
    ledLightSelect_old=10;
    ledBLESelect_old=10;
    ledUnitSelect_old=10;
    ledSMSelect_old=10;
    ledSpeed_old=10;
    ledPower_old=10;
    ledBrightness=PWM_CUSTOM;

}

/*******************      Basic Operation       **********************
 *
 *
 *********************************************************************/
void ledControl_i2c2Display( uint8_t Param, uint8_t *ParamValue )
{
    uint8_t ledControl_readBuffer = NULL;                       // local variables are not stored in memory. This is equivalent to reset each time this function is called.
    uint8_t ledControl_writeBuffer[2] = {0, 0};
    size_t  readBufferSize  = 0;
    size_t  writeBufferSize = 2;

    ledControl_i2cTransferStatus = ledControl_ledDisplayManager -> ledControl_transfer(IS31FL3236A_ADDR, &ledControl_writeBuffer, writeBufferSize, &ledControl_readBuffer, readBufferSize);

}

/*********************************************************************
 * @fn      ledControl_setAllOn
 *
 * @brief   call this function to lit all LED on display
 *
 * @param   Nil
 *
 * @return  Nil
 */
void ledControl_setAllOn(){
    // I2C to command lit all LED on Dashboard
    ledControl_i2c2Display( 0, NULL );
//    LED_Turn_ON_ALL();
}
/*********************************************************************
 * @fn      ledControl_setAllOff
 *
 * @brief   call this function to turn off all LED on display
 *
 * @param   Nil
 *
 * @return  Nil
 */
void ledControl_setAllOff(){
    // I2C to command turn off all LED on Dashboard
    LED_Turn_OFF_ALL();

}
/*********************************************************************
 * @fn      ledControl_setDashSpeed
 *
 * @brief   call this function to set Speed indicator on LED display
 *
 * @param   dashSpeed
 *
 * @return  none
 */

void ledControl_setDashSpeed(uint8_t dashSpeed){
    // I2C command to set Speed Indicator
    ledSpeed = dashSpeed;
}

/*********************************************************************
 * @fn      ledControl_changeDashSpeed
 *
 * @brief   Updates the dashboard Speed on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeDashSpeed(){
    if(ledSpeed_old != ledSpeed || ledBrightness!=ledBrightness_old){
        int dashspeed_unit;
        int dashspeed_ten;
        dashspeed_unit = ledSpeed % 10;
        dashspeed_ten = ledSpeed / 10;
        functionTable[35+dashspeed_ten](I_OUT,ledBrightness);
        functionTable[45+dashspeed_unit](I_OUT,ledBrightness);
        ledSpeed_old=ledSpeed;
    }
}
/*********************************************************************
 * @fn      ledControl_setBatteryStatus
 *
 * @brief   call this function to set battery bar on LED display
 *
 * @param   batteryStatus
 *
 * @return  none
 */
void ledControl_setBatteryStatus(uint8_t batteryStatus)
{
    // I2C command to set Battery Status
    ledBatteryStatus = batteryStatus;
}

/*********************************************************************
 * @fn      ledControl_changeBatteryStatus
 *
 * @brief   Updates the  battery status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeBatteryStatus()
{
    if(ledBattery_old != ledBatteryStatus || ledBrightness!=ledBrightness_old){

        if(ledBatteryStatus == 0){
            functionTable[34](I_OUT,ledBrightness);
        }
        else if(ledBatteryStatus == 1){
            functionTable[33](I_OUT,ledBrightness);
        }
        else if(ledBatteryStatus == 2){
            functionTable[32](I_OUT,ledBrightness);
        }
        else if(ledBatteryStatus == 3){
            functionTable[31](I_OUT,ledBrightness);
        }
        else if(ledBatteryStatus == 4){
            functionTable[30](I_OUT,ledBrightness);
        }
        else if(ledBatteryStatus == 5){
            functionTable[29](I_OUT,ledBrightness);
        }

        ledBattery_old = ledBatteryStatus;
    }
}
/*********************************************************************
 * @fn      ledControl_setSpeedMode
 *
 * @brief   call this function to set speed mode on LED display
 *
 * @param   speedMode
 *
 * @return  none
 */
void ledControl_setSpeedMode(uint8_t speedMode)
{
    // I2C command to set Speed Mode Status
    ledSpeedMode = speedMode;
}

/*********************************************************************
 * @fn      ledControl_changeSpeedMode
 *
 * @brief   Updates the Speed Mode status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeSpeedMode()
{
    if ( ledSMSelect_old != ledSpeedMode || ledBrightness!=ledBrightness_old )
    {
        // change speed mode
        if(ledSpeedMode == 0){
            functionTable[3](I_OUT,ledBrightness);
        }
        else if(ledSpeedMode == 1){
            functionTable[2](I_OUT,ledBrightness);
        }
        else{
            functionTable[1](I_OUT,ledBrightness);
        }

        ledSMSelect_old = ledSpeedMode;

    }
}
/*********************************************************************
 * @fn      ledControl_setUnitSelectDash
 *
 * @brief   call this function to set Dashboard Display Unit on LED display
 *
 * @param   UnitSelectDash
 *
 * @return  none
 */
void ledControl_setUnitSelectDash(uint8_t UnitSelectDash)
{
    // I2C command to set Unit Light
    ledUnitSelectDash = UnitSelectDash;
}

/*********************************************************************
 * @fn      ledControl_changeUnit
 *
 * @brief   Updates the Unit selection on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeUnit()
{
    if ( ledUnitSelect_old != ledUnitSelectDash || ledBrightness!=ledBrightness_old )
    {
        // change unit
        if(ledUnitSelectDash==0){
            functionTable[14](I_OUT,ledBrightness);
        }
        else{
            functionTable[13](I_OUT,ledBrightness);
        }

        ledUnitSelect_old = ledUnitSelectDash;

    }
}
/*********************************************************************
 * @fn      ledControl_setBLEStatus
 *
 * @brief   call this function to activate Warning Light on LED display
 *
 * @param   BLEStatus
 *
 * @return  none
 */
void ledControl_setBLEStatus(uint8_t BLEStatus){
    // I2C command to set BLE Light Status
    ledBLEStatus = BLEStatus;
}

/*********************************************************************
 * @fn      ledControl_changeBLE
 *
 * @brief   Updates the BLE status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeBLE()
{
    gaprole_States_t get_gaproleState;
    GAPRole_GetParameter(GAPROLE_STATE, &get_gaproleState);
    if (get_gaproleState == GAPROLE_ADVERTISING  || ledBLEStatus == GAPROLE_CONNECTED || ledBLEStatus == GAPROLE_CONNECTED_ADV)
    {
        ledBLEStatus = 1;
    }
    else
    {
        ledBLEStatus = 0;
    }

    if ( ledBLESelect_old != ledBLEStatus || ledBrightness!=ledBrightness_old )
    {
        // change BLE
        if(ledBLEStatus == 1)
        {
            functionTable[4](I_OUT,ledBrightness);
        }
        else
        {
            functionTable[4](I_OUT,PWM_ZERO);
        }

        ledBLESelect_old = ledBLEStatus;

    }
}
/*********************************************************************
 * @fn      ledControl_setWarningMode
 *
 * @brief   call this function to activate Warning Light on LED display
 *
 * @param   WarningStatus
 *
 * @return  none
 */
void ledControl_setErrorCodeWarning(uint8_t errorCode)
{
    // I2C command to set Warning Light Status
    ledErrorCode = 1; //errorCode;

}

/*********************************************************************
 * @fn      ledControl_changeError
 *
 * @brief   Updates the Error status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeError(){
    if ( ledError_old != ledErrorCode || ledBrightness!=ledBrightness_old )
    {
        // change Error code
        if(ledErrorCode==0){
            functionTable[7](I_OUT,PWM_ZERO);
        }
        else{
            functionTable[7](I_OUT,ledBrightness);
        }

        ledError_old = ledErrorCode;

    }
}
/*********************************************************************
 * @fn      ledControl_setLightMode
 *
 * @brief   call this function to set light mode on LED display
 *
 * @param   light_mode
 *
 * @return  None
 *********************************************************************/
void ledControl_setLightMode(uint8_t LED_light_mode)
{
    // I2C command to set Light Mode
    ledLightmode = LED_light_mode;
}

/*********************************************************************
 * @fn      ledControl_changeLightMode
 *
 * @brief   Updates the light mode status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeLightMode(){
    if ( ledLightSelect_old != ledLightmode || ledBrightness!=ledBrightness_old )
    {
        // change Light mode
        if(ledLightmode==0){
            functionTable[5](I_OUT,PWM_ZERO);
            functionTable[6](I_OUT,PWM_ZERO);
        }
        else if(ledLightmode==1){
            functionTable[5](I_OUT,ledBrightness);
//            functionTable[6](I_OUT,PWM_ZERO);
        }
        else{
            functionTable[5](I_OUT,PWM_ZERO);
            functionTable[6](I_OUT,ledBrightness);
        }

        ledLightSelect_old = ledLightmode;

    }
}
/*********************************************************************
 * @fn      ledcontrol_setLightStatus
 *
 * @brief   call this function to set light status on LED display
 *
 * @param   light_status
 *
 * @return  None
 *********************************************************************/
void ledControl_setLightStatus(uint8_t light_status)
{
    // I2C command to set Light Status
    ledLightstatus = light_status;
}

/*********************************************************************
 * @fn      ledControl_changeLightStatus
 *
 * @brief   Updates the light status on LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_changeLightStatus(){
    if ( ledLightstatus_old != ledLightstatus  || ledBrightness!=ledBrightness_old)
    {
        // change Light status
        if(ledLightstatus==0){
            functionTable[5](I_OUT,PWM_ZERO);
        }
        else{
            functionTable[5](I_OUT,ledBrightness);
        }

        ledLightstatus_old = ledLightstatus;
        ledBrightness_old = ledBrightness;
    }
}

/*********************************************************************
 * @fn      ledControl_registerLedDisplay
 *
 * @brief   Register for LED display
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void ledControl_registerLedDisplay( ledControl_ledDisplayManager_t *ledDisplayI2C )
{
    ledControl_ledDisplayManager = ledDisplayI2C;
}
