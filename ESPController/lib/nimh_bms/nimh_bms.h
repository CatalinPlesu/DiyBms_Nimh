#ifndef NIHM_BMS_H_
#define NIHM_BMS_H_

#include <stdint.h>
#include <stddef.h>

#define RELAY_CHARGER_PIN 0
#define RELAY_LOAD_PIN 1

enum BMS_STATE {
    BMS_STATE_CRITICAL = 0,
    BMS_STATE_INITIALIZED,
    BMS_STATE_CHARGING,
    BMS_STATE_CHARGED,
    BMS_STATE_DISCHARGING,

};

enum BMS_ERROR_STATE {
    BMS_ERROR_STATE_NONE = 0,
    BMS_ERROR_STATE_OVERVOLTAGE,
    BMS_ERROR_STATE_UNDERVOLTAGE,
    BMS_ERROR_STATE_TEMPERATURE_HIGH,
    BMS_ERROR_STATE_TEMPERATURE_LOW,
    BMS_ERROR_STATE_TEMPERATURE_CUT_OFF,
};

#define MAX_MODULE_COUNT 100
typedef struct nimh_bms{
    uint16_t states[MAX_MODULE_COUNT];
    uint8_t module_count;
    uint8_t disable_charger;
    uint8_t disable_load;
}nimh_bms;

void nimh_bms_init(uint16_t (*get_module_count_Ptr)(),
void (*relay_on)(uint8_t pin),
void (*relay_off)(uint8_t pin)
);
void nimh_bms_check_state(uint16_t states, uint8_t current_module);
nimh_bms nih_bms_get_struct();

#endif