#ifdef ByteLinkEnable

#include "stddef.h"
#include "byte_link.h"


TS_SharedData asSharedData[ByteLinkLength];
uint8_t u8Index = 0;
uint8_t u8SharedIndex = 0;
uint16_t *buffer;

uint16_t message_buffer[16];

uint16_t byte_link_get_variable_count()
{
    return (uint16_t)u8Index;
}

void byte_link_init(uint16_t* buff){
    buffer = buff;
}

// byte 0 - type
// byte 1-15 for variable name
void byte_link_get_one()
{
    if (u8SharedIndex + 1 < ByteLinkLength)
    {
        buffer[0] = (uint16_t)(asSharedData[u8SharedIndex].type);
        int min_length;
        if(ByteLinkCharLength < 30)
        {
            min_length = ByteLinkCharLength / 2;
        }
        else
        {
            min_length = 15;
        }
        for (int i = 0; i < min_length; i++)
        {
            buffer[i + 1] = (uint16_t)((asSharedData[u8SharedIndex].name[i * 2] << 8) | asSharedData[u8SharedIndex].name[i * 2 + 1]);
        }
    }
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

// will extract byte 1 from index 0 of the buffer
// will write 2 uint16 values in index 1,2
void byte_link_read_variable()
{
    uint8_t index = (uint8_t)buffer[0];
    if (index <= u8Index)
    {
        switch (asSharedData[index].type)
        {
            case INT8:
            case UINT8:
                buffer[1] = *((uint8_t*)asSharedData[index].data);
                buffer[2] = NULL;
                break;

            case INT16:
            case UINT16:
                buffer[1] = *((uint16_t*)asSharedData[index].data);
                buffer[2] = NULL;
                break;

            case INT32:
            case UINT32:
            case FLOAT:
                buffer[1] = *(uint16_t*)&asSharedData[index];
                buffer[2] = *((uint16_t*)&asSharedData[index].data + 1);
                break;

            default:
                buffer[1] = NULL;
                buffer[2] = NULL;
                break;
        }
    }
    else
    {
        buffer[1] = NULL;
        buffer[2] = NULL;
    }
}
// will extract byte 1 from index 0 of the buffer
// will read 2 uint16 values from index 1,2 and modify them locally
void byte_link_write_variable()
{
    uint8_t index = (uint8_t)buffer[0];
    if (index <= u8Index)
    {
        switch (asSharedData[index].type)
        {
            case INT8:
            case UINT8:
                *((uint8_t*)asSharedData[index].data) = (uint8_t)buffer[1];
                break;

            case INT16:
            case UINT16:
                *((uint16_t*)asSharedData[index].data) = buffer[1];
                break;

            case INT32:
            case UINT32:
            case FLOAT:
                *((uint16_t*)asSharedData[index].data) = buffer[1];
                *((uint16_t*)asSharedData[index].data + 1) = buffer[2];
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

void byte_link_read_message()
{
    for (int i = 0; i < 16; i++)
    {
        buffer[i] = message_buffer[i];
    }
}

#endif
