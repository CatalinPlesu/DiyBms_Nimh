#ifdef ByteLinkEnable

#ifndef BYTE_LINK_H_
#define BYTE_LINK_H_

#include <stdint.h>

// Enums
typedef enum {
    INT8 = 0,
    INT16,
    INT32,
    UINT8,
    UINT16,
    UINT32,
    FLOAT
} TE_DataType;


// Structures 
typedef struct {
    TE_DataType type;
    void* data;
    char name[ByteLinkCharLength];
} TS_SharedData;

// Macros
#define BYTE_LINK_REGISTER_VARIABLE(variable, type) (byte_link_register_variable((void *)&(variable), type, #variable))

// Prototypes
void byte_link_register_variable(void* pvVariable, TE_DataType eDataType, char name[ByteLinkCharLength]);

uint16_t byte_link_get_variable_count();
void byte_link_get_variable_array(TS_SharedData** array);

uint64_t byte_link_read_variable(uint8_t index);
void byte_link_write_variable(uint8_t index, uint64_t var);
void byte_link_send_message(uint8_t message[32]);
uint8_t* byte_link_read_message();

#endif
#endif  