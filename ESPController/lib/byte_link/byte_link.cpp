#ifdef ByteLinkEnable

#include "stddef.h"
#include "byte_link.h"

TS_SharedData asSharedData[ByteLinkLength];
uint8_t u8Index = 0;
uint16_t message_buffer[16];

uint16_t byte_link_get_variable_count()
{
    return (uint16_t)u8Index;
}

void byte_link_register_variable(void* pvVariable, TE_DataType eDataType, char name[ByteLinkCharLength])
{
    if (u8Index + 1 < ByteLinkLength) // This variable can be stored in the buffer
    {
        asSharedData[u8Index].data = pvVariable;
        asSharedData[u8Index].type = eDataType;
        for(uint8_t i = 0; i < ByteLinkCharLength; i++)
        {
            asSharedData[u8Index].name[i] = name[i];
        }
    }
}

uint64_t byte_link_read_variable(uint8_t index)
{
    return (*((uint16_t*)&asSharedData[index].data) << 32) | *((uint16_t*)&asSharedData[index].data+1);
}

void byte_link_write_variable(uint8_t index, uint64_t var)
{
    if (index <= u8Index)
    {
        switch (asSharedData[index].type)
        {
            case INT8:
            case UINT8:
                *((uint8_t*)asSharedData[index].data) = var;
                break;

            case INT16:
            case UINT16:
                *((uint16_t*)asSharedData[index].data) = var;
                break;

            case INT32:
            case UINT32:
            case FLOAT:
                *((uint16_t*)asSharedData[index].data) = var;
                *((uint16_t*)asSharedData[index].data + 1) = var>>16;
                break;

            default:
                break;
        }
    }
    else
    {
    }
}

void byte_link_send_message(uint8_t message[32])
{
    for (int i = 0; i < 16; i++)
    {
        message_buffer[i] = ((uint16_t)message[i * 2] << 8) | message[i * 2 + 1];
        if(message[i * 2] == '\n' | message[i * 2 + 1] == '\n')
        {
            return;
        }
    }
    message_buffer[15] |= '\n';
}

void byte_link_get_variable_array(TS_SharedData** array)
{
    *array = asSharedData;
}

uint8_t* byte_link_read_message()
{
    return (uint8_t*)&message_buffer[0];
}

#endif
