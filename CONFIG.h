#ifndef CONFIG_H
#define CONFIG_H

#define F_CPU 16000000UL

#define ACCESS_PIN                  "1234"
#define ACCESS_PIN_LENGTH           4
#define ACCESS_MAX_ATTEMPTS         3
#define ACCESS_UNLOCK_TIME_MS       5000
#define ACCESS_LOCKOUT_TIME_S       10
#define ACCESS_PIN_TIMEOUT_MS       8000
#define ACCESS_ENTRY_DELAY_S        3
#define ACCESS_ALARM_TIME_S         20

#define PIR_WARMUP_S                30
#define PIR_ACTIVE_HIGH             1
#define USE_PIR                     1
#define USE_SERVO                   1
#define SERVO_LOCKED_US             1000
#define SERVO_UNLOCKED_US           2000

#define USE_RELAY                   1
#define RELAY_ACTIVE_LOW            1

#define USE_RFID                    1
#define RFID_POLL_MS                150
#define RFID_RETRY_MS               2000
#define ACCESS_CARD_COUNT           1
#define ACCESS_CARDS                {{0x02, 0xAE, 0xC7, 0x4D}}

#define TEST_SPI_LOOPBACK           0
#define USE_BLUETOOTH               1
#endif