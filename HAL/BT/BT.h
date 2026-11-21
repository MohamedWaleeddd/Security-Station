#ifndef BT_H
#define BT_H

#include <stdint.h>

#include "../../MCAL/UART/UART.h"

#define BT_DEFAULT_BAUDRATE    9600UL
#define BT_AT_EOL              "\r\n"

#define BT_AT_TEST             "AT"
#define BT_AT_VERSION          "AT+VERSION"
#define BT_AT_STAT             "AT+STAT"
#define BT_AT_SLEEP            "AT+SLEEP"
#define BT_AT_BAUD             "AT+BAUD"
#define BT_AT_NAME             "AT+NAME"
#define BT_AT_NAMB             "AT+NAMB"
#define BT_AT_PIN              "AT+PIN"
#define BT_AT_LADDR            "AT+LADDR"
#define BT_AT_RESET            "AT+RESET"
#define BT_AT_DEFAULT          "AT+DEFAULT"
#define BT_AT_DISC             "AT+DISC"
#define BT_AT_TYPE             "AT+TYPE"
#define BT_AT_ENLOG            "AT+ENLOG"
#define BT_AT_UUIDLEN          "AT+UUIDLEN"
#define BT_AT_SVRUUID          "AT+SVRUUID"
#define BT_AT_CHRUUID          "AT+CHRUUID"
#define BT_AT_CRXUUID          "AT+CRXUUID"
#define BT_AT_UARTMODE         "AT+UARTMODE"
#define BT_AT_STTS             "AT+STTS"

#define BT_AT_SET_BAUD(value)          BT_AT_BAUD value
#define BT_AT_SET_NAME(value)          BT_AT_NAME value
#define BT_AT_SET_BLE_NAME(value)      BT_AT_NAMB value
#define BT_AT_SET_PIN(value)           BT_AT_PIN value
#define BT_AT_SET_ADDRESS(value)       BT_AT_LADDR value
#define BT_AT_SET_TYPE(value)          BT_AT_TYPE value
#define BT_AT_SET_ENLOG(value)         BT_AT_ENLOG value
#define BT_AT_SET_UUIDLEN(value)       BT_AT_UUIDLEN value
#define BT_AT_SET_SERVICE_UUID(value)  BT_AT_SVRUUID value
#define BT_AT_SET_CHAR_UUID(value)     BT_AT_CHRUUID value
#define BT_AT_SET_RX_UUID(value)       BT_AT_CRXUUID value
#define BT_AT_SET_UARTMODE(value)      BT_AT_UARTMODE value
#define BT_AT_SET_STAT_MODE(value)     BT_AT_STTS value

#define BT_BAUD_2400           "2"
#define BT_BAUD_4800           "3"
#define BT_BAUD_9600           "4"
#define BT_BAUD_19200          "5"
#define BT_BAUD_38400          "6"
#define BT_BAUD_57600          "7"
#define BT_BAUD_115200         "8"
#define BT_BAUD_128000         "9"

#define BT_TYPE_NO_PIN         "0"
#define BT_TYPE_WITH_PIN       "1"

#define BT_ENLOG_OFF           "0"
#define BT_ENLOG_ON            "1"

#define BT_UUIDLEN_16_BIT      "0"
#define BT_UUIDLEN_128_BIT     "1"

#define BT_UARTMODE_NO_PARITY  "0,0"
#define BT_UARTMODE_ODD        "0,1"
#define BT_UARTMODE_EVEN       "0,2"

#define BT_STAT_CONNECT_ONLY   "0"
#define BT_STAT_RX_PULSE       "1"

void BT_init(void);
uint8_t BT_available(void);
uint8_t BT_readByte(uint8_t *data);
uint8_t BT_writeByte(uint8_t data);
uint8_t BT_writeString(const char *str);
uint8_t BT_sendCommand(const char *command);

#endif