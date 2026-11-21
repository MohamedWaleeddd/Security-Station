#include "BT.h"

#include <string.h>

void BT_init(void)
{
    UART_init(BT_DEFAULT_BAUDRATE);
}

uint8_t BT_available(void)
{
    return UART_isDataAvailable();
}

uint8_t BT_readByte(uint8_t *data)
{
    return UART_receiveByte(data);
}

uint8_t BT_writeByte(uint8_t data)
{
    return UART_sendByte(data);
}

uint8_t BT_writeString(const char *str)
{
    return UART_sendString(str);
}

uint8_t BT_sendCommand(const char *command)
{
    uint8_t command_length = BT_writeString(command);

    if (command_length != strlen(command))
    {
        return 0;
    }

    return (BT_writeString(BT_AT_EOL) == sizeof(BT_AT_EOL) - 1);
}