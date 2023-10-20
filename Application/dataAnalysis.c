/*
 * dataAnalysis.c
 *
 *  Created on: 13 Oct 2022
 *      Author: Chee
 */
/******************************************************************************
 * @file  dataAnalysis.c
 * @brief This file contains the functions for calculating:
 *     (1) distance travelled
 *     (2) power consumed
 *     (3) Average Speed
 *     (4) Average Battery Voltage
 *     (5) battery percentage
 *     (6) battery status
 *     (7) Instantaneous economy
 *     (8) Economy
 *     (9) Range
 *     (10) CO2 Saved
 *
 *     distance travelled and power consumed are stored in UDBuffer[]
 *     UDBuffer[] is stored in NVS when device shout downs.
 *
 *     The above performance data are packaged and sent to mobile app, to LED display,
 *     & to Cloud storage and saved in flash memory
 *
 *****************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include "dataAnalysis.h"
#include "buzzerControl.h"
#include "ledControl.h"
#include "Controller.h"
#include "Dashboard.h"
#include "Battery.h"
#include "motorControl.h"
#include "periodicCommunication.h"
#include "brakeAndThrottle.h"
#include "lightControl.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <math.h>

/*********************************************************************
* LOCAL VARIABLES
*/
static uint16_t DATA_ANALYSIS_SAMPLING_TIME = PERIODIC_COMMUNICATION_HF_SAMPLING_TIME;

// Simpson's 1/3 rule coefficients
static uint8_t coefficient_array[DATA_ANALYSIS_POINTS] = {0};           // same size as DATA_ANALYSIS_POINTS
static uint8_t (*ptrc)[DATA_ANALYSIS_POINTS] = &coefficient_array;

static uint8_t batteryStatus;
//Default Unit Settings
static uint8_t UnitSelectDash = SI_UNIT;                   // Keep the last units selected by user in memory, the same units is used on restart
static uint8_t UnitSelectApp = SI_UNIT;                    // Mobile app allow user to select the desired display unit - the App units and Dash units are NOT linked
//
static uint16_t rpm[DATA_ANALYSIS_POINTS] = {0};                        //revolutions per minute data collected every time interval
static uint16_t speed_cmps[DATA_ANALYSIS_POINTS] = {0};                 //rpm is converted to cm per second (cm/s)
static uint16_t batteryCurrent_mA[DATA_ANALYSIS_POINTS] = {0};          //battery current data collected at every hf communication interval
static uint16_t batteryVoltage_mV[DATA_ANALYSIS_POINTS] = {0};          //battery voltage data collected at every hf communication interval
static int8_t heatSinkTemperature_C[DATA_ANALYSIS_POINTS] = {0};        // temperature can be negative
static int8_t motorTemperature_C[DATA_ANALYSIS_POINTS] = {0};           // temperature can be negative
static uint16_t (*ptrRpm)[DATA_ANALYSIS_POINTS] = &rpm;
static uint16_t (*ptrSpeed)[DATA_ANALYSIS_POINTS] = &speed_cmps;
static uint16_t (*ptrBatteryVoltage)[DATA_ANALYSIS_POINTS] = &batteryVoltage_mV;
static uint16_t (*ptrBatteryCurrent)[DATA_ANALYSIS_POINTS] = &batteryCurrent_mA;
static int8_t (*ptrhst)[DATA_ANALYSIS_POINTS] = &heatSinkTemperature_C;
static int8_t (*ptrmt)[DATA_ANALYSIS_POINTS] = &motorTemperature_C;
//
static uint16_t dA_Count = 1;
static uint32_t UDDataCounter = 0;                  // At new, UDDataCounter = 0
static uint16_t UDIndex;                            // the last UDIndex saved
static uint16_t UDIndexPrev;

static AD_t ADArray = {0};                            //Since this data set is temporary, array struct is not necesary.  ADArray data that are displayed on the mobile app.
static AD_t (*ptrADArray) = &ADArray;                 // Provides a pointer option

static float lenConvFactorDash;

static uint32_t avgBatteryVoltage_mV = 0;
static int16_t  avgBatteryLevel = 0;
static uint8_t batteryPercentage = BATTERY_PERCENTAGE_INITIAL;
static uint8_t batteryLow = 0;
static uint8_t speed_mode_init;

static uint8_t UDTriggerCounter = 0;
static uint32_t ADDataCounter = 0;
static uint32_t sumDeltaMileage_dm;                 // unit in decimeters.  This is the previous data on the total distance travelled
static uint32_t sumDeltaPowerConsumed_mWh;          // unit in milli-W-hr.  This is the previous data on the total power consumption
static uint32_t totalMileagePrev_dm;                // unit in decimeters.  This is the previous data on the total distance travelled
static uint32_t totalPowerConsumedPrev_mWh;         // unit in milli-W-hr.  This is the previous data on the total power consumption
static uint32_t totalMileage0_dm;                   // unit in decimeters.  This is the oldest data on total distance travelled stored in storage array
static uint32_t totalPowerConsumed0_mWh;            // unit in milli-W-hr.  This is the oldest data on total power consumed stored in storage array

static uint32_t UDBuffer[NVS_BUFFER_SIZE];          //
//
static dataAnalysis_NVS_Manager_t *dataAnalysis_NVSManager;

/******************************************************************************************************
*
* LOCAL FUNCTIONS
*/
/*
 * @fn      coefficient_array_init
 *
 * @brief   Used to initialize the coefficient_array used in Simpson's 1/3 rule
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/
static uint8_t coefficient_array_init(){ //change number of data points if necessary
    for(uint8_t xx = 0; xx < DATA_ANALYSIS_POINTS; xx++){
        if( xx == 0 || xx == DATA_ANALYSIS_POINTS-1 )
        {
            *((*ptrc) + xx) = 1;
        }
        else
        {
            if(xx%2==1)
            {
                *((*ptrc) + xx)=4;
            }
            else
            {
                *((*ptrc) + xx)=2;
            }
        }
    }
    return 0;
}
/******************************************************************************************************
 * @fn      dataAnalysis_timerInterruptHandler
 *
 * @brief   It is used to initialize the callback functions such that the library can notify the application when there are updates
 *
 * @param   None
 *
 * @return  None
 ******************************************************************************************************/
extern void dataAnalysis_timerInterruptHandler()
{
        // motorControl.c pass data from MCU at every DATA_ANALYSIS_SAMPLING_TIME interval to dataSim(uint32_t jj)
}

/****
  * @fun    dataAnalysis_motorControl
 *
 * @brief   Receiving the pointer for strut array MCUDArray
 *
 * @param   ptrMCUD
 *
 * @return  Nil
 */
MCUD_t (*ptr_MCUDArray);
extern void dataAnalysis_motorControl(MCUD_t (*ptrMCUD))
{
    ptr_MCUDArray = ptrMCUD;
}
/******************************************************************************************************
 * @fun      dataAnalysis_Init
 *
 * @brief   Initialization and determines the starting conditions from saved data
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/
extern void dataAnalysis_init()
{
    /* ***************************************************
     * initialize Simpsons 1/3 rule coefficient_array
     *****************************************************/
    coefficient_array_init();

    // ***************************************************
    uint16_t batteryVoltageStartUp_mV;
    uint16_t batteryCurrentStartUp_mA;


    // At the instant of POWER ON, we need to obtain BATTERY status for LED display
    // dashboard will instruct motor controller to obtain a battery voltage and current measurement
    batteryVoltageStartUp_mV = (*ptr_MCUDArray).voltage_mV; //36000;           // -> STM32MCP_getRegisterFrame(STM32MCP_MOTOR_1_ID,STM32MCP_BUS_VOLTAGE_REG_ID);
    batteryCurrentStartUp_mA = (*ptr_MCUDArray).current_mA; // 3000;            // -> STM32MCP_getRegisterFrame(STM32MCP_MOTOR_1_ID,STM32MCP_BUS_CURRENT_REG_ID);
    //avgBatteryVoltage_mV = batteryVoltageStartUp_mV;
    int8_t mTStartUp = (*ptr_MCUDArray).motorTemperature_Celcius; //15;                      // -> STM32MCP_getRegisterFrame(STM32MCP_MOTOR_1_ID,STM32MCP_BUS_MOTORTEMPERATURE_REG_ID);
    int8_t hSTStartUp = (*ptr_MCUDArray).heatSinkTemperature_Celcius; //15;                     // -> STM32MCP_getRegisterFrame(STM32MCP_MOTOR_1_ID,STM32MCP_BUS_HEATSINKTEMPERATURE_REG_ID);

    dA_Count = DATA_ANALYSIS_POINTS;
    // Initialize the following arrays:  RPM, Speed, Battery voltage, Battery Current, Heat sink Temperature, Motor Temperature
    for (uint8_t xx = 0; xx < dA_Count; xx++)
    {
        *((*ptrRpm) + xx) = 0;         // unit in rpm = get rpm
        *((*ptrSpeed) + xx) = round((*((*ptrRpm))) * 2 * (float) M_PI / 60 * WHEELRADIUS);              // Unit in cm / sec
        *((*ptrBatteryCurrent) + xx) = batteryCurrentStartUp_mA;                                                 // unit in mA = get battery current in mA
        *((*ptrBatteryVoltage) + xx) = batteryVoltageStartUp_mV;                                                 // unit in mV = get battery voltage in mV
        *((*ptrhst) + xx) = hSTStartUp;
        *((*ptrmt) + xx) = mTStartUp;
    }
    // Reset dA_Count to 1 after initialization
    dA_Count = 1;

    /***************************************************
     *      Read data stored in NVS Internal
     ***************************************************/
    dummyUDArray();     // read UDArray data that are stored in memory (from nvsinternal)
    //dataAnalysis_NVS_read( void );
    get_UDArrayData();
    /*********************************************************************************************
     * Initializing data
     * defining ADArray variables at initialization allow connectivity with Mobile App instantly.
     *********************************************************************************************/
    ADArray.accumPowerConsumption_mWh = totalPowerConsumedPrev_mWh;         // ADArray = App data strut
    ADArray.accumMileage_dm = totalMileagePrev_dm;                          // ADArray = App data strut
    ADArray.avgSpeed_kph = 0;                                               // ADArray = App data strut
    ADArray.avgHeatSinkTemperature_C = computeAvgHeatSinkTemperature();     // ADArray = App data strut
    ADArray.avgBatteryVoltage_mV = computeAvgBatteryVoltage();                    // ADArray = App data strut
    ADArray.errorCode = 0;                                                  // ADArray = App data strut

    ADArray.batteryPercentage = computeBatteryPercentage();                 // ADArray = App data strut
    ADArray.batteryStatus = determineBatteryStatus();                       // ADArray = App data strut
    ADArray.instantEconomy_100Whpk = computeInstantEconomy(0, 0);           // ADArray = App data strut
    ADArray.economy_100Whpk = computeEconomy();                             // ADArray = App data strut
    ADArray.range_m = computeRange();                                       // ADArray = App data strut
    ptrADArray->co2Saved_g = computeCO2Saved();                             // ADArray = App data strut
    ADArray.motorTemperature_C = computeMotorTemperature();                 // ADArray = App data strut

    dataAnalysis_changeUnitSelectDash(UnitSelectDash);      // Send Unit Select to LED display
    ledControl_setSpeedMode( speed_mode_init );
    brakeAndThrottle_setSpeedMode( speed_mode_init );

    if ((batteryLow == 0) && (batteryPercentage < BATTERY_PERCENTAGE_LL)){
        batteryLow = 1;
        //buzzerControl_Start();
    }
    if ((batteryLow == 1) && (batteryPercentage > BATTERY_PERCENTAGE_LH)){
        batteryLow = 0;
        //buzzerControl_Stop();
    }

}


extern uint8_t dataAnalysis_getInitSpeedMode(void)
{
    return speed_mode_init;
}

/***********************************************************************************************************
 * @fn      dataAnalysis_NVS_read
 *
 * @brief   Read data from NVS memory at start-up
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/

void dataAnalysis_NVSRead( void )
{

}

/***********************************************************************************************************
 * @fn      dataAnalysis_NVS_write
 *
 * @brief   Write data to NVS memory at power-off
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/

void dataAnalysis_NVSWrite( void )
{

}

/***********************************************************************************************************
 * @fn      get_UDArrayData
 *
 * @brief   Set and update data for data analysis
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/
static void get_UDArrayData()
{
    UDDataCounter = UDBuffer[1];

    UDIndexPrev = UDDataCounter % UDARRAYSIZE;      // UDIndexPrev is the remainder of UDDataCounter / UDARRAYSIZE
    if (UDIndexPrev >= (UDARRAYSIZE - 1)) {
            UDIndex = 0;
            }
    else {
            UDIndex = UDIndexPrev + 1;
            }
    //nvsBuffer option
    // SETSIZE = 2
        totalMileage0_dm = UDBuffer[2 + UDIndex * SETSIZE + 0];
        totalPowerConsumed0_mWh = UDBuffer[2 + UDIndex * SETSIZE + 1];
        totalMileagePrev_dm = UDBuffer[2 + UDIndexPrev * SETSIZE + 0];
        totalPowerConsumedPrev_mWh = UDBuffer[2 + UDIndexPrev * SETSIZE + 1];
        ADDataCounter = UDBuffer[0];

     speed_mode_init = UDBuffer[28];
     UnitSelectDash = UDBuffer[29];
     //uint8_t light_mode_init = UDBuffer[30];      // this is not necessary since light mode will always be resets to auto mode at power on
//    */
}

/*************************************************************************************************************
 * @fn      dataAnalysis_LEDSpeed
 *
 * @brief   calculate speeds, send RPM & speed to client (App) and set speed display on dashboard
 *
 * @param   dA_Count
 *
 * @return  Nil
******************************************************************************************************/
uint8_t dashSpeed;  // for debugging only
uint16_t rawSpeed_100kph;
extern void dataAnalysis_LEDSpeed(uint16_t xCounter)
{
    //uint16_t
    rawSpeed_100kph = ((float) speed_cmps[xCounter] * 3.6 );           // 100*km/hr
    dashSpeed = ((float) speed_cmps[xCounter] * 0.036 * lenConvFactorDash);     // km/hr or mph
    // Send rpm and speed to client (App)
    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_MOTOR_RPM, CONTROLLER_MOTOR_RPM_LEN, (uint8_t *) &rpm[dA_Count]);
    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_MOTOR_SPEED, CONTROLLER_MOTOR_SPEED_LEN, (uint8_t *) &rawSpeed_100kph);
    // Send dashSpeed to LED display
    ledControl_setDashSpeed(dashSpeed);
}

/***************************************************************************************************
 * @fn      dataAnalyt
 *
 * @brief   Data Evaluation function:  Call by dataSim() and when Power-OFF
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/
uint32_t deltaPowerConsumption_mWh, deltaMileage_dm;
static void dataAnalyt()
{
//    uint32_t deltaPowerConsumption_mWh, deltaMileage_dm;
    ADDataCounter++;
    ADArray.ADCounter = ADDataCounter;                // Why not ADDataCounter + 1?              // totalDataCount is total count of all computed datasets
    deltaPowerConsumption_mWh = computePowerConsumption();
    deltaMileage_dm = computeDistanceTravelled();
    sumDeltaPowerConsumed_mWh += deltaPowerConsumption_mWh;
    sumDeltaMileage_dm += deltaMileage_dm;
    ADArray.accumPowerConsumption_mWh = totalPowerConsumedPrev_mWh + sumDeltaPowerConsumed_mWh;
    ADArray.accumMileage_dm = totalMileagePrev_dm + sumDeltaMileage_dm;
    ADArray.avgSpeed_kph = computeAvgSpeed(deltaMileage_dm);
    ADArray.avgHeatSinkTemperature_C = computeAvgHeatSinkTemperature();
    ADArray.errorCode = 0;                                                  // error code algorithm to be defined
    ADArray.avgBatteryVoltage_mV = computeAvgBatteryVoltage();
    ADArray.batteryStatus = determineBatteryStatus();
    ADArray.batteryPercentage = computeBatteryPercentage();
    ADArray.instantEconomy_100Whpk = computeInstantEconomy(deltaPowerConsumption_mWh, deltaMileage_dm);
    ADArray.economy_100Whpk = computeEconomy();
    ADArray.range_m = computeRange();
    ADArray.co2Saved_g = computeCO2Saved();
    ADArray.motorTemperature_C = computeMotorTemperature();

    re_Initialize();    // Re-initialize rpm, speed, batteryVoltage and batteryCurrent arrays after data analysis

    UDTriggerCounter++;     // When UDTriggerCounter = UDTrigger, UDArray is saved to flash memory

}

/******************************************************************************************************
 * @fn      data2UDArray
 *
 * @brief   Save data to UDArray or UDBuffer and save UDArray to nvsinternal:  Called by dataAnalyt() and when Power-OFF
 *
 * @param   Nil
 *
 * @return  Nil
******************************************************************************************************/
void data2UDArray()
{
    // UDIndex is tracked to ensure the data is placed at the correct array location
    UDDataCounter+=1;

/******************************************************************************************************
 * Stored data to UDBuffer
 ******************************************************************************************************/
    UDBuffer[0] = ADArray.ADCounter;
    UDBuffer[1] = UDDataCounter;

    // SETSIZE = 2
        UDBuffer[2 + SETSIZE * UDIndex + 0] = ADArray.accumMileage_dm;
        UDBuffer[2 + SETSIZE * UDIndex + 1] = ADArray.accumPowerConsumption_mWh;
    //

        uint8_t speed_mode = brakeAndThrottle_getSpeedMode();
        UDBuffer[28] = speed_mode;
        UDBuffer[29] = UnitSelectDash;
        uint8_t light_mode = lightControl_getLightMode();
        UDBuffer[30] = light_mode;
/*
 *  UDBuffer is erased-write to nvsinternal when device shuts down
 */

/******************************************************************************************************
 * Resets UDDataCounter if it reaches 4292400000 counts (For memory and data management purposes only)
 ******************************************************************************************************/
    if (UDDataCounter >= 4292400000){
            UDDataCounter = 0;
    }

    get_UDArrayData();

/******************************************************************************************************
 * Whenever data2UDArray() is triggered, reset UDTriggerCounter to zero
 * reset sumDeltaPowerConsumed_mWh and sumDeltaMileage_dm to zero
 ******************************************************************************************************/
    UDTriggerCounter = 0;
    sumDeltaPowerConsumed_mWh = 0;
    sumDeltaMileage_dm = 0;

}

/******************************************************************************************************
 * @fn      dataAnalysis_sampling
 *
 * @brief   Simulates data obtaining from MCU
 *
 * @param   jj
 *
 * @return  Nil
******************************************************************************************************/
extern void dataAnalysis_sampling(uint8_t x_hf, uint16_t STM32MCP_batteryVoltage, uint16_t STM32MCP_batteryCurrent,
                                  uint16_t STM32MCP_rpm, int8_t STM32MCP_heatsinkTemp, int8_t STM32MCP_motorTemp)
{
    dA_Count = x_hf;
    // Sim RPM
    *((*ptrRpm)+ dA_Count) = STM32MCP_rpm;                                                    // get RPM from MCU:  unit in rpm,  188 rpm @ r = 0.1016m => 200 cm/sec = 7 km/hr
    // Cal Speed from RPM
    *((*ptrSpeed)+ dA_Count) = round(rpm[dA_Count] * 2 * (float) M_PI / 60 * WHEELRADIUS);      // Unit in cm / sec
    // Sim Battery Current
    *((*ptrBatteryCurrent)+ dA_Count) = STM32MCP_batteryCurrent;                              // get battery current from MCU:  unit in mA
    // Sim Battery Voltage
    *((*ptrBatteryVoltage)+ dA_Count) = STM32MCP_batteryVoltage;                              // get battery voltage from MCU:  unit in mV
    // Sim Heatsink Temp
    *((*ptrhst)+ dA_Count) = STM32MCP_heatsinkTemp;                                           // get heat sink temperature from MCU: unit in degrees Celsius
    // Sim Motor Temp
    *((*ptrmt)+ dA_Count) = STM32MCP_motorTemp;

    dataAnalysis_LEDSpeed(dA_Count);       // covert speed to the selected dashboard unit (dashSpeed) then sent to led display

    dataAnalysis_Main();
}

/*********************************************************************
 * @fn      dataAnalysis_Main
 *
 * @brief   call this function to simulate voltage, current, RPM and temperature measurement for controller
 *
 * @param   None
 *
 * @return  None
 ******************************************************************************************************/
static void dataAnalysis_Main()
{

    if (dA_Count >= (DATA_ANALYSIS_POINTS - 1)) // and also when Power OFF // Caution of the case where dA_Count >= DATA_ANALYSIS_POINTS & POWER OFF
    {
        // dataAnalyt() carries out all the data analytics
        dataAnalyt();
        ledControl_setBatteryStatus(ADArray.batteryStatus);                     // Send battery status and errorCode to led display
        avgBatteryVoltage_mV = ADArray.avgBatteryVoltage_mV;                     // for debugging only
        batteryPercentage = ADArray.batteryPercentage;                             // for debugging only

        // send analytics data to App
        dataAnalysis_motorcontrol_setGatt();
        //batteryPercentage
        if ((batteryLow == 0) && (batteryPercentage < BATTERY_PERCENTAGE_LL))
        {
            batteryLow = 1;
            //if (timer9 is not "started") { buzzerControl_Start();}    // there are several conditions that may demand the buzzer timer to start and repeat
            //Conditions include: (1) battery level low, (2) Power On/OFF, (3) Present of any error code
        }
        if ((batteryLow == 1) && (batteryPercentage > BATTERY_PERCENTAGE_LH))
        {
            batteryLow = 0;
            //if (no other conditions that demand timer9 start) { buzzerControl_Stop();}
            //Conditions include: (1) battery level low, (2) Power On/OFF, (3) Present of any error code
        }
    }
    // The UDArray is used to store the history of total Power Consumed and total distance travelled data in memory.
    // To minimize the SRAM occupied by UDArray, it is stored only once every (DATA_ANALYSIS_SAMPLING_TIME x (DATA_ANALYSIS_POINTS -1) x UDTRIGGER / 1000) seconds.
    // e.g. ( 300 ms x (31 -1) x 40 / 1000) = 360 seconds = 6 minutes
    if (UDTriggerCounter >= UDTRIGGER)       // and also when Power_OFF  // Caution of the case where dA_Count >= DATA_ANALYSIS_POINTS & POWER OFF
    {
        data2UDArray(); // data2UDBuffer, reset UDTriggerCounter
    }

}

/***************************************************************************************************
 * @fn      dataAnalysis_motorcontrol_setGatt
 *
 * @brief   This function setGatt
 *
 * @param   Nil
 *
 * @return  nil
******************************************************************************************************/
static uint8_t heatSinkOVTempState = 0;
static uint8_t motorOVTempState = 0;
static uint8_t dummyBatteryErrorCode = 0;
static void dataAnalysis_motorcontrol_setGatt()
{
    /******  Dashboard services
    *************************************/
    motorcontrol_setGatt(DASHBOARD_SERV_UUID, DASHBOARD_ADCOUNTER, DASHBOARD_ADCOUNTER_LEN, (uint8_t *) &ADArray.ADCounter);

    /******  Controller services
     *************************************/
//    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_VOLTAGE, CONTROLLER_VOLTAGE_LEN, (uint8_t *) &ADArray.voltage);
//    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_CURRENT, CONTROLLER_CURRENT_LEN, (uint8_t *) &ADArray.current);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_HEAT_SINK_TEMPERATURE, CONTROLLER_HEAT_SINK_TEMPERATURE_LEN, (uint8_t *) &ADArray.avgHeatSinkTemperature_C);
    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_TOTAL_DISTANCE_TRAVELLED, CONTROLLER_TOTAL_DISTANCE_TRAVELLED_LEN, (uint8_t *) &ADArray.accumMileage_dm);
    motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_TOTAL_ENERGY_CONSUMPTION, CONTROLLER_TOTAL_ENERGY_CONSUMPTION_LEN, (uint8_t *) &ADArray.accumPowerConsumption_mWh);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_INSTANT_ECONOMY, CONTROLLER_INSTANT_ECONOMY_LEN, (uint8_t *) &ADArray.instantEconomy_100Whpk);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_TOTAL_ENERGY_EFFICIENCY, CONTROLLER_TOTAL_ENERGY_EFFICIENCY_LEN, (uint8_t *) &ADArray.economy_100Whpk);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_RANGE, CONTROLLER_RANGE_LEN, (uint8_t *) &ADArray.range_m);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_CO2SAVED, CONTROLLER_CO2SAVED_LEN, (uint8_t *) &ADArray.co2Saved_g);
    //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_MOTOR_TEMPERATURE, CONTROLLER_MOTOR_TEMPERATURE_LEN, (uint8_t *) &ADArray.motorTemperature_C);

    if ((heatSinkOVTempState == HEATSINK_TEMPERATURE_ABNORMAL) && (motorOVTempState == MOTOR_TEMPERATURE_ABNORMAL))
    {
        //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_ERROR_CODE, CONTROLLER_ERROR_CODE_LEN, (uint8_t *) &heatSinkOVTempState);
    }
    else if ((heatSinkOVTempState == HEATSINK_TEMPERATURE_ABNORMAL) && (motorOVTempState != MOTOR_TEMPERATURE_ABNORMAL))
    {
        //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_ERROR_CODE, CONTROLLER_ERROR_CODE_LEN, (uint8_t *) &heatSinkOVTempState);
        }
    else if ((heatSinkOVTempState != HEATSINK_TEMPERATURE_ABNORMAL) && (motorOVTempState == MOTOR_TEMPERATURE_ABNORMAL))
    {
        //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_ERROR_CODE, CONTROLLER_ERROR_CODE_LEN, (uint8_t *) &motorOVTempState);
    }
    else
    {
        //motorcontrol_setGatt(CONTROLLER_SERV_UUID, CONTROLLER_ERROR_CODE, CONTROLLER_ERROR_CODE_LEN, (uint8_t *) &heatSinkOVTempState);
    }
    /*******  Battery services
    *************************************/
    //motorcontrol_setGatt(BATTERY_SERV_UUID, BATTERY_BATTERY_VOLTAGE, BATTERY_BATTERY_VOLTAGE_LEN, (uint8_t *) &ADArray.avgBatteryVoltage_mV);
    //motorcontrol_setGatt(BATTERY_SERV_UUID, BATTERY_BATTERY_LEVEL, BATTERY_BATTERY_LEVEL_LEN, (uint8_t *) &ADArray.batteryPercentage);
    //motorcontrol_setGatt(BATTERY_SERV_UUID, BATTERY_BATTERY_STATUS, BATTERY_BATTERY_STATUS_LEN, (uint8_t *) &ADArray.batteryStatus);

    //motorcontrol_setGatt(BATTERY_SERV_UUID, BATTERY_BATTERY_TEMPERATURE, BATTERY_BATTERY_TEMPERATURE_LEN, (uint8_t *) &ADArray.avgHeatSinkTemperature_C); // currently not available

    //motorcontrol_setGatt(BATTERY_SERV_UUID, BATTERY_BATTERY_ERROR_CODE, BATTERY_BATTERY_ERROR_CODE_LEN, (uint8_t *) &dummyBatteryErrorCode);
    dummyBatteryErrorCode++;
}

/***************************************************************************************************
 * @fn      computePowerConsumption
 *
 * @brief   This function calculates the change in power consumption of the e_scooter
 *          over the time interval using Simpson's 1/3 Rule
 *
 * @param   AccumPowerConsumed
 *
 * @return  energy consumption value (unit milli W-hr) in type: uint32_t
******************************************************************************************************/
uint32_t computePowerConsumption()
{
    uint32_t deltaPowerConsumption_mWh = 0;
    for(uint8_t ii = 0; ii < DATA_ANALYSIS_POINTS; ii++)
    {
        uint32_t tempholder = (batteryVoltage_mV[ii] * batteryCurrent_mA[ii]) * 0.0001;         // required to avoid possible byte size limitation issue
        deltaPowerConsumption_mWh += (*((*ptrc)+ ii)) * tempholder;
    }
    deltaPowerConsumption_mWh = round((float) deltaPowerConsumption_mWh / 3000 * DATA_ANALYSIS_SAMPLING_TIME / 3600);       // output in milli-W-hr
    return deltaPowerConsumption_mWh;   //  -> convert to the desired unit before displaying on App
}

/***************************************************************************************************
 * @fn      computeDistanceTravelled
 *
 * @brief   This function calculates the change of distance_travelled of the e_scooter
 *          over a period using Simpson's Rule.  The distance is in decimeter.
 *
 * @param   AccumMileage
 *
 * @return  distanceTravelled (unit in decimeters, i.e.  0.1 meters) in type: uint32_t
******************************************************************************************************/
uint32_t computeDistanceTravelled()
{
    uint32_t deltaDistanceTravelled_dm = 0;
    for(uint8_t ii = 0; ii < DATA_ANALYSIS_POINTS; ii++)
    {
         deltaDistanceTravelled_dm += ((*((*ptrc) + ii)) * speed_cmps[ii]);            // for computational accuracy reasons, calculations are performed in centimeter/second
    }
    deltaDistanceTravelled_dm = round((float) deltaDistanceTravelled_dm * DATA_ANALYSIS_SAMPLING_TIME / 30000);// (1/3) x (1/1000) x (1/10) = 1/30000: output is then converted to decimeter
    return deltaDistanceTravelled_dm; // -> convert to the desired unit before displaying on App
}

/***************************************************************************************************
 * @fn      computeAverageSpeed
 *
 * @brief   This function calculates the average speed over the given time interval in km/hr.
 *
 * @param   DeltaDistanceTravelled
 *
  * @return  avgSpeed in km/hr
******************************************************************************************************/
uint8_t computeAvgSpeed(uint32_t deltaMileage_dm)
{
    static uint8_t avgSpeed_kph = 0;    // output in km/hr
    if (dA_Count < 1)                   // dA_Count is always greater than or equal 1, hence it is not possible to get here -> just to guarantee division by 0 is never possible.
    {
        avgSpeed_kph = 0;
        return avgSpeed_kph;
    }                                                   // Safeguard from stack overflow due to division by 0
    avgSpeed_kph = round((float)(deltaMileage_dm)/ (DATA_ANALYSIS_SAMPLING_TIME * (dA_Count - 1)) * 360); // output in km/hr
    return avgSpeed_kph;                                // output rounded off to nearest km/hr
}

/**********************************************************************************************************
 * @fn      computeAverageHST
 *
 * @brief   This function calculates the average heat sink temperature over the given time interval in degrees celsius
 *
 * @param   Nil
 *
  * @return  avghst in degree celsius
******************************************************************************************************/
int8_t computeAvgHeatSinkTemperature()
{
    int16_t sumHeatSinkTemperatuer_C = 0; // output in degree celsius
    int8_t avgHeatSinkTemperature_C = 0;
    for(uint8_t ii = 0; ii < dA_Count; ii++)
    {
        sumHeatSinkTemperatuer_C += heatSinkTemperature_C[ii];                             // sum all measurements
    }

    if (dA_Count < 1)           // dA_Count is always greater than or equal 1, hence it is not possible to get here -> just to guarantee division by 0 is never possible.
    {
        avgHeatSinkTemperature_C = 0;
        // errorCode => will be a System Error in this case
        return avgHeatSinkTemperature_C; // output in degree celsius
    }

    avgHeatSinkTemperature_C = round((sumHeatSinkTemperatuer_C)/ dA_Count);
    if (avgHeatSinkTemperature_C > CRIT_HEATSINKTEMPERATURE_C)
    {
        heatSinkOVTempState = HEATSINK_TEMPERATURE_ABNORMAL;  // errorCode = HeatSink_OVTEMP_WARNING_CODE;
        // Action: Restrict speed mode to AMBLE mode or restrict IQ to 0 until temperature drop below the safe threshold
    }
    else
    {
        heatSinkOVTempState = 0;
    }
    return avgHeatSinkTemperature_C; // output in degree celsius

}

/**********************************************************************************************************
 * @fn      computeMotorTemperature
 *
 * @brief   This function calculates the average Motor temperature over the given time interval in degrees celsius
 *
 * @param   Nil
 *
  * @return  avgMotorTemperature in degree celsius
******************************************************************************************************/
int8_t computeMotorTemperature()
{
    int16_t sumMotorTemperature_C = 0; // output in degree celsius
    int8_t avgMotorTemperature_C = 0;

    for(uint8_t ii = 0; ii  < dA_Count; ii++)
    {
         sumMotorTemperature_C += motorTemperature_C[ii];                             // sum all measurements
    }

    if (dA_Count < 1)           // dA_Count is always greater than or equal 1, hence it is not possible to get here -> just to guarantee division by 0 is never possible.
    {
        avgMotorTemperature_C = 0;
        // errorCode = system error in this case;
        return avgMotorTemperature_C; // output in degree celsius
    }

    avgMotorTemperature_C = round((sumMotorTemperature_C) / dA_Count);

    if (avgMotorTemperature_C > CRIT_MOTORTEMPERATURE_C)
    {
        motorOVTempState = MOTOR_TEMPERATURE_ABNORMAL;
        // Action: Restrict speed mode to AMBLE mode or restrict IQ to 0 until temperature drop below the safe threshold
        ledControl_getError(8);
    }
    else
    {
        motorOVTempState = 0;
    }

    return avgMotorTemperature_C; // output in degree celsius
}

/***************************************************************************************************
 * @fn      computeAvgBatteryVoltage
 *
 * @brief   This function calculates the average of the battery voltage over the given time interval in mV.
 *
 * @param   batteryVoltage_mV in mV
 *
 * @return  AvgBatteryVoltage in milli Volt
******************************************************************************************************/
uint32_t computeAvgBatteryVoltage()
{
    avgBatteryVoltage_mV = 0;
    uint32_t    sumBatteryVoltage_mV = 0;

    for(uint8_t ii = 0; ii < dA_Count; ii++)
    {
        sumBatteryVoltage_mV += batteryVoltage_mV[ii];                               // Average is in mV
    }
    if (dA_Count < 1)    // dA_Count is always greater than or equal 1, hence it is not possible to get here -> just to guarantee division by 0 is never possible.
    {
        avgBatteryVoltage_mV = 0;
        //errorCode = system error in this case;
        return avgBatteryVoltage_mV;   // Unit in mV
    }

    avgBatteryVoltage_mV = round((float) sumBatteryVoltage_mV / dA_Count);            // output in mV

    if (avgBatteryVoltage_mV < BATTERY_CRITICALLY_LOW)
    {
        // battery level critically low -> power shut down
    }

    return avgBatteryVoltage_mV;   // Unit in mV
}

/***************************************************************************************************
 * @fn      computeBatteryPercentage
 *
 * @brief   This function computes the battery percentage based on the average battery voltage.
 *
 * @param   avgBatteryVoltage
 *
 * @return  batteryPercentage = battery_battery_level in battery.h
******************************************************************************************************/
uint8_t computeBatteryPercentage()
{
    avgBatteryLevel = 0;
    int16_t     instantBatteryLevel = 0;
    int16_t     sumBatteryLevel = 0;

    for(uint8_t ii = 0; ii < dA_Count; ii++)
    {
        instantBatteryLevel = ( batteryVoltage_mV[ii] - BATTERY_MIN_VOLTAGE) * 100 /((BATTERY_MAX_VOLTAGE - batteryCurrent_mA[ii] * VOLTAGE_DROP_COEFFICIENT) - BATTERY_MIN_VOLTAGE);
        if(instantBatteryLevel > 100)
        {
            instantBatteryLevel = 100;              // battery % cannot be greater than 100%
        }
        sumBatteryLevel += instantBatteryLevel;
    }

    if (dA_Count < 1)   // dA_Count is always greater than or equal 1, hence it is not possible to get here -> just to guarantee division by 0 is never possible.
    {
        avgBatteryLevel = 0;        // if dA_Count < 0, make avgBatteryLevel = 0.
        //errorCode = system error;
        return avgBatteryLevel;   // Unit in mV
    }

    avgBatteryLevel = round((float) sumBatteryLevel / dA_Count);                      // output in %

    return avgBatteryLevel;              //batteryPercentage_mV;
}

/***************************************************************************************************
 * @fn      determineBatteryStatus
 *
 * @brief   This function returns the battery status based on the average battery voltage.
 *
 * @param   avgBatteryVoltage
 *
 * @return  batteryStatus = battery_battery_status in battery.h
******************************************************************************************************/
uint8_t determineBatteryStatus()
{
    if (avgBatteryLevel > LEVEL45PERCENT) {batteryStatus = GLOWING_AQUA;}
    else if (avgBatteryLevel <= LEVEL45PERCENT && avgBatteryLevel > LEVEL34PERCENT) {batteryStatus = GLOWING_GREEN;}
    else if (avgBatteryLevel <= LEVEL34PERCENT && avgBatteryLevel > LEVEL23PERCENT) {batteryStatus = YELLOW;}
    else if (avgBatteryLevel <= LEVEL23PERCENT && avgBatteryLevel > LEVEL12PERCENT) {batteryStatus = ORANGE;}
    else if (avgBatteryLevel <= LEVEL12PERCENT && avgBatteryLevel > LEVEL01PERCENT) {batteryStatus = RED;}
    else {batteryStatus = FLASHING_RED;}
    ledControl_setBatteryStatus(batteryStatus);
    return batteryStatus;
}
/***************************************************************************************************
 * @fn      computeInstantEconomy
 *
 * @brief   Calculate the instantaneous instantEconomy - Not used for now.
 *
 * @param   None
 *
 * @return  instantEconomy (in W-hr/km)
***************************************************************************************************/
uint16_t computeInstantEconomy(uint32_t deltaPowerConsumption_mWh, uint32_t deltaMileage_dm)
{
    uint16_t instantEconomy_100Whpk_holder = 0;                    // unit in W-hr / km x 100
    if (deltaMileage_dm <= 0)
    {
        instantEconomy_100Whpk_holder = 50000;
        return instantEconomy_100Whpk_holder;
    }                                                       //******** Safeguard from stack overflow due to division by 0

    instantEconomy_100Whpk_holder = (float)(deltaPowerConsumption_mWh * 1000) / deltaMileage_dm;               // unit in W-hr / km x 100

    if (instantEconomy_100Whpk_holder > 50000)
    {              // Safeguard from data truncation in case economy is greater than declared variable size
        instantEconomy_100Whpk_holder = 50000;
        return instantEconomy_100Whpk_holder;
    }
    return instantEconomy_100Whpk_holder;                          // Unit in W-hr / km x 100    -> convert to the desired unit before displaying on App
}

/***************************************************************************************************
 * @fn      computeEconomy
 *
 * @brief   This function calculates the economy (i.e.moving average of Whr/km x 100) over the most recent 1.5 hours (1.5 = DATA_ANALYSIS_SAMPLING_TIME x DATA_EVALUATING_POINTS x NUMINDEX / 1000 / 3600 x 100).
 *
 * @param   None
 *
 * @return  economy (in W-hr/km x 100)
***************************************************************************************************/
uint32_t computeEconomy()
{
    uint32_t overall_economy_100Whpk = 0;                            // unit in W-hr / km x 100
    if ((ADArray.accumMileage_dm - totalMileage0_dm) <= 0)
    { // Safeguard from stack overflow due to division by 0
        overall_economy_100Whpk = 50000;
        return overall_economy_100Whpk;
    }

    overall_economy_100Whpk = (float)(ADArray.accumPowerConsumption_mWh - totalPowerConsumed0_mWh) * 1000 / (ADArray.accumMileage_dm - totalMileage0_dm);     // Unit in W-hr / km x 100

    if (overall_economy_100Whpk > 50000)
    {                                   // Safeguard from data truncation in case economy is greater than declared variable size
        overall_economy_100Whpk = 50000;
        return overall_economy_100Whpk;
    }
    return overall_economy_100Whpk;                                 // Unit in W-hr / km x 100    -> convert to the desired unit before displaying on App
}

/***************************************************************************************************
 * @fn      computeRange
 *
 * @brief   This function calculates the range remaining in meters
 *
 * @param   batteryCap, avgWhr (in Whr/km)
 *
 * @return  Range
***************************************************************************************************/
uint32_t computeRange(){
    uint32_t range_m = 0;
    if (ADArray.economy_100Whpk <= 0) {                     // Safeguard from stack overflow due to division by 0
        range_m = 0;
        return range_m;                                     // output in meters  -> convert to the desired unit before displaying on App
    }                                                       // output in meters
    range_m = ((float) ADArray.batteryPercentage * BATTERY_MAX_CAPACITY * BCF / ADArray.economy_100Whpk );
    return range_m;                                         // output in metres  -> convert to the desired unit before displaying on App
}
/******************************************************************************************************
 * @fn      computeCO2Saved
 *
 * @brief   This function calculates the hypothetical CO2Saved when compared to driving an average car
 *
 * @param   AccumPowerConsumed, AccumMileage
 *
 * @return  co2Saved
******************************************************************************************************/
uint32_t co2Saved_g;                                        // in grams
uint32_t computeCO2Saved()
{
    co2Saved_g = 0;                                         // in grams
    if (ADArray.accumMileage_dm <= 0)
    {
        co2Saved_g = 0;                                     // Safeguard from stack overflow due to division by 0
        return co2Saved_g;                                  // in grams -> convert to the desired unit before displaying on App
    }
    co2Saved_g = ((float) ADArray.accumMileage_dm * 0.10) * (COEFF01 - ((float) (ADArray.accumPowerConsumption_mWh) / (float) (ADArray.accumMileage_dm) * 0.10) * COEFF02);  // in grams
    return co2Saved_g;                                      // in grams -> convert to the desired unit before displaying on App
}
/***************************************************************************************************
 * @fn      re_Initialize
 *
 * @brief   This function re-initialize the data arrays after each data analysis loop.
 *
 * @param   none
 *
 * @return  none
******************************************************************************************************/
static void re_Initialize()
{
    //  Re-initialize arrays after completing each computation loop
    for(uint8_t kk = 0; kk < DATA_ANALYSIS_POINTS; kk++)
    {           // carry over the last data set to the 1st position [0] of the new dataset and reset all other to zero
        if(kk == 0)
        {
            *(*(ptrRpm)) = *(*(ptrRpm) + DATA_ANALYSIS_POINTS - 1);
            *(*(ptrSpeed)) = *(*(ptrSpeed) + DATA_ANALYSIS_POINTS - 1);
            *(*(ptrBatteryCurrent)) = *(*(ptrBatteryCurrent) + DATA_ANALYSIS_POINTS - 1);
            *(*(ptrBatteryVoltage)) = *(*(ptrBatteryVoltage) + DATA_ANALYSIS_POINTS - 1);
            *(*(ptrhst)) = *(*(ptrhst) + DATA_ANALYSIS_POINTS - 1);
            *(*(ptrmt)) = *(*(ptrmt) + DATA_ANALYSIS_POINTS - 1);
        }
        else
        {
            *(*(ptrRpm) + kk) = 0;
            *(*(ptrSpeed) + kk) = 0;
            *(*(ptrBatteryCurrent) + kk) = 0;
            *(*(ptrBatteryVoltage) + kk) = 0;
            *(*(ptrhst) + kk) = 0;
            *(*(ptrmt) + kk) = 0;
        }
    }
}

/*********************************************************************
 * @fn      dataAnalysis_getUnitSelectDash
 *
 * @brief   call this function to retrieve the current Unit setting
 *
 * @param   None
 *
 * @return  UnitSelectDash
 *********************************************************************/
extern uint8_t dataAnalysis_getUnitSelectDash(){
    return UnitSelectDash;
}

/*********************************************************************
 * @fn      dataAnalysis_changeUnitSelectDash
 *
 * @brief   call this function to change/toggle Unit and set led display
 *
 * @param   None
 *
 * @return  None
 *********************************************************************/
extern void dataAnalysis_changeUnitSelectDash(uint8_t unit){
    UnitSelectDash = unit;
    switch(UnitSelectDash)
    {
    case SI_UNIT:
        {
            lenConvFactorDash = 1;
            break;
        }
    case IMP_UNIT:
        {
            lenConvFactorDash = KM2MILE;
            break;
        }
    default:
        break;
    }
    // send UnitSelectDash to ledControl.c
    ledControl_setUnitSelectDash(UnitSelectDash);
}



extern void dataAnalysis_setError( uint8_t errorCode, uint8_t errorStatus )
{
    if(errorCode != 0)
    {

    }
}

// ********************************************************************************************************
// ****************** Dummy data simulates data stored and to be retrieved from flashing memory ***********
// Description: Usage data at every 6 minutes intervals over the last 2 hours [20 sets] are stored on flash memory
//              Economy calculations require past data on distance travelled and power consumption to be known, which
//              will require data to be retrieved from memory.
//              Below is a set of dummy data that simulates some data assigned and stored in memory
// *********************************************************************************************************
// Should UDArray be store in the HEAP or STATIC/GLOBAL ????
// rename to nvsControl_getNVSData();

static void dummyUDArray()
{
// ************* dummy data for testing assigning data to array storage **************
// **** NOTE:  NSV_read at every power on -> at first time initialization, the nvs should be zeros.
// Action: read nvs
// NVS_getNVSData();

// Test case 00
//        UDBuffer[0] = 0;

        // Test case 02 (SETSIZE = 2)
                UDBuffer[0] = 1439;      // ADDataCounter = number of integration completed (each integration contains N = data_analysis_points)
                UDBuffer[1] = 39;        // UDCounter = number of UDTrigger.  UDCounter plus 1 when ADDataCounter = multiples of UDTrigger
                UDBuffer[2] = 21221;     //1 Distance traveled
                UDBuffer[3] = 29589;     //1 Energy Consumed
                UDBuffer[4] = 23705;
                UDBuffer[5] = 33246;     //2
                UDBuffer[6] = 25602;
                UDBuffer[7] = 36127;     //3
                UDBuffer[8] = 27584;
                UDBuffer[9] = 38703;     //4
                UDBuffer[10] = 28530;
                UDBuffer[11] = 40640;    //5
                UDBuffer[12] = 30549;
                UDBuffer[13] = 42959;    //6
                UDBuffer[14] = 32503;
                UDBuffer[15] = 46034;    //7
                UDBuffer[16] = 34485;
                UDBuffer[17] = 48962;    //8
                UDBuffer[18] = 36366;
                UDBuffer[19] = 51871;    //9
                UDBuffer[20] = 38138;
                UDBuffer[21] = 54542;    //10
                UDBuffer[22] = 0;        // Free
                UDBuffer[23] = 0;        // Free
                UDBuffer[24] = 0;        // Free
                UDBuffer[25] = 0;        // Free
                UDBuffer[26] = 0;        // Free
                UDBuffer[27] = 0;        // Free
                UDBuffer[28] = 0;        // speed mode {0 = Amble, 1 = Leisure, 2 = Sports}
                UDBuffer[29] = 0;        // dashboard unit {0 = kmph, 1 = mph}
                UDBuffer[30] = 2;        // light mode {0 = Off, 1 = On, 2 = Auto}
                UDBuffer[31] = 0;        // Free

}

/*********************************************************************
 * @fn      dataAnalysis_registerNVSINT
 *
 * @brief   It is used to register the NVS internal function
 *
 * @param   None
 *
 * @return  None
 *********************************************************************/
void dataAnalysis_registerNVSINT( dataAnalysis_NVS_Manager_t *nvsManager )
{
    dataAnalysis_NVSManager = nvsManager;
}
