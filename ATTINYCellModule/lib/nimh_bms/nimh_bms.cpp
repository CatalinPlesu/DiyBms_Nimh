#include "nimh_bms.h"


static uint8_t nimh_bms_get_voltage_state();
static void nimh_bms_shift_samples();

static inline uint8_t nimh_bms_dT_dt();
static inline uint8_t is_temperature_increasing();
static inline uint8_t is_voltage_increasing();

nimh_bms bms = {};

void nimh_bms_init(uint16_t upper_voltage_limit)
{
    bms.state = BMS_STATE_INITIALIZED;
    bms.error_state_volt = BMS_ERROR_STATE_NONE;
    bms.upper_voltage_limit = upper_voltage_limit;
    for (uint8_t u = 0; u < 2; u++)
    {
        bms.max_temp[u] = 0;
        bms.min_temp[u] = 0;
        bms.max_voltage[u] = 0;
        bms.min_voltage[u] = 0;
    }
    bms.timer = 0;
}

void nimh_bms_read_temperature(uint16_t temp)
{
    bms.timer += 1;
    int16_t temperature = 0;
    if (0x8000 & temp)
    {
        temperature = -(temp & 0x7fffU);
    }
    else
    {
        temperature = temp;
    }

    if (bms.timer % SAMPLE_RANGE == 1)
    {
        bms.max_temp[CURREN] = temperature;
        bms.min_temp[CURREN] = temperature;
    }

    if (temperature > bms.max_temp[CURREN])
    {
        bms.max_temp[CURREN] = temperature;
    }
    if (temperature < bms.min_temp[CURREN])
    {
        bms.min_temp[CURREN] = temperature;
    }
}

void nimh_bms_read_voltage(uint16_t voltage)
{
    if (bms.timer % SAMPLE_RANGE == 1)
    {
        bms.max_voltage[CURREN] = voltage;
        bms.min_voltage[CURREN] = voltage;
    }

    if(voltage > bms.max_voltage[CURREN]){
        bms.max_voltage[CURREN] = voltage;
    }
    if(voltage < bms.min_voltage[CURREN]){
        bms.min_voltage[CURREN] = voltage;
    }
}

void nimh_bms_sample_range_tick()
{
    if (bms.timer % SAMPLE_RANGE)
    { 
        // will enter the function every Sample_range seconds
        return;
    }

    bms.error_state_volt = nimh_bms_get_voltage_state();
    switch (bms.state)
    {
    case BMS_STATE_INITIALIZED:
        if(is_temperature_increasing() || is_voltage_increasing()){
            bms.state = BMS_STATE_CHARGING;
        }else{
            bms.state = BMS_STATE_DISCHARGING;
        }
    break;

    case BMS_STATE_CHARGING:
        if(is_temperature_increasing() || is_voltage_increasing()){
           bms.state = BMS_STATE_CHARGED;
        } 
        else
        {
            bms.state = BMS_STATE_DISCHARGING;
        }
    break;

    case BMS_STATE_CHARGED:
        if(!is_temperature_increasing() && !is_voltage_increasing()){
            bms.state = BMS_STATE_DISCHARGING;
        }
    break;

    case BMS_STATE_DISCHARGING:
        if(is_temperature_increasing()){
            bms.state = BMS_STATE_CHARGING;
        }
    break;
    
    default:
        break;
    }

    nimh_bms_shift_samples();
}

uint16_t nimh_bms_check_state()
{
    uint16_t tmp = bms.state & 0xf;
    tmp |= ((bms.error_state_volt & 0xf) << 12);
    return tmp;
}

int16_t nimh_bms_check_temperature_state()
{
    return (
        (((uint8_t)(bms.max_temp[CURREN] - 150)) << 8)|
        ((uint8_t)(bms.min_temp[CURREN] - 150))
    );
}

uint8_t nimh_bms_check_bypass(){
    return ((bms.state == BMS_STATE_CHARGED) || (bms.error_state_volt == BMS_ERROR_STATE_OVERVOLTAGE));
}

static uint8_t nimh_bms_get_voltage_state()
{
    if (bms.min_voltage[CURREN] > bms.upper_voltage_limit)
    {
        return BMS_ERROR_STATE_OVERVOLTAGE;
    }

    return BMS_ERROR_STATE_NONE;
}

static void nimh_bms_shift_samples()
{
    bms.max_temp[PREVIOUS] = bms.max_temp[CURREN];
    bms.min_temp[PREVIOUS] = bms.min_temp[CURREN];
    bms.max_voltage[PREVIOUS] = bms.max_voltage[CURREN];
    bms.min_voltage[PREVIOUS] = bms.min_voltage[CURREN];
}

static inline uint8_t nimh_bms_dT_dt(){
    return ((bms.max_temp[CURREN]-bms.min_temp[CURREN]) * 10 > (bms.max_temp[PREVIOUS] - bms.min_temp[PREVIOUS]) * 15) && ((bms.max_temp[PREVIOUS] - bms.min_temp[PREVIOUS]) > 0);
}

static inline uint8_t is_temperature_increasing(){
    return (bms.max_temp[CURREN] >= bms.max_temp[PREVIOUS]) || (bms.min_temp[CURREN] >= bms.min_temp[PREVIOUS]);
}

static inline uint8_t is_voltage_increasing(){
    return (bms.max_voltage[CURREN] >= bms.max_voltage[PREVIOUS]) || (bms.max_voltage[CURREN] >= bms.max_voltage[PREVIOUS]);
}