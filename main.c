#include "CONFIG.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include "MCAL/GPIO/GPIO.h"
#include "MCAL/DIO/DIO.h"
#include "HAL/LCD/LCD.h"
#include "HAL/KEYPAD/KEYPAD.h"
#include "HAL/BUZZER/BUZZER.h"
#include "HAL/LEDS/LEDS.h"
#include "HAL/SERVO/SERVO.h"
#include "HAL/RELAY/RELAY.h"
#include "HAL/PIR/PIR.h"
#include "HAL/RC522/RC522.h"
#include "HAL/BT/BT.h"
#include "MCAL/SPI/SPI.h"

#define LED_UNLOCKED   LED3
#define LED_ALARM      LED2
#define LED_ARMED      LED4
#define LED_DENIED     LED1

#if USE_RFID
#define UID_ARG uid
#else
#define UID_ARG ((uint8_t *)0)
#endif

typedef enum {
    OUTCOME_NONE,
    OUTCOME_OK,
    OUTCOME_BAD,
    OUTCOME_TIMEOUT
} outcome_t;

#if USE_RFID
static const uint8_t allowed_cards[ACCESS_CARD_COUNT][RC522_UID_SIZE] = ACCESS_CARDS;
static uint8_t rfid_ready = 0;
#endif

static void wait_ms(uint16_t ms)
{
    while (ms--) {
        _delay_ms(1);
    }
}

#if USE_BLUETOOTH
static void bt_put(char c)
{
    while (!BT_writeByte((uint8_t)c)) {
    }
}
#endif

static void notify(const char *text)
{
#if USE_BLUETOOTH
    while (*text) {
        bt_put(*text++);
    }
    bt_put('\r');
    bt_put('\n');
#else
    (void)text;
#endif
}

#if USE_RFID
static void notify_card(const char *label, const uint8_t *uid)
{
#if USE_BLUETOOTH
    const char hex[] = "0123456789ABCDEF";
    uint8_t i;

    while (*label) {
        bt_put(*label++);
    }
    for (i = 0; i < RC522_UID_SIZE; i++) {
        bt_put(' ');
        bt_put(hex[uid[i] >> 4]);
        bt_put(hex[uid[i] & 0x0F]);
    }
    bt_put('\r');
    bt_put('\n');
#else
    (void)label;
    (void)uid;
#endif
}
#endif

static void beep(uint16_t freq, uint16_t ms)
{
    BUZZER_on(freq, ms);
    wait_ms((uint16_t)(ms + 10));
}

static void beep_key(void)
{
    BUZZER_on(2000, 40);
}

static void beep_fail(void)
{
    uint8_t i;

    for (i = 0; i < 3; i++) {
        beep(1000, 150);
        wait_ms(100);
    }
}

static char key_to_char(KEYPAD_switch_t key)
{
    return (char)('0' + (uint8_t)key);
}

static void door_lock(void)
{
#if USE_SERVO
    SERVO_lock();
#endif
#if USE_RELAY
    RELAY_off();
#endif
    LED_Off(LED_UNLOCKED);
    notify("Door locked");
}

static void door_unlock(void)
{
#if USE_SERVO
    SERVO_unlock();
#endif
#if USE_RELAY
    RELAY_on();
#endif
    LED_On(LED_UNLOCKED);
    notify("Door unlocked");
}

static void leds_selftest(void)
{
    LED_On(LED_ARMED);
    wait_ms(150);
    LED_Off(LED_ARMED);
    LED_On(LED_UNLOCKED);
    wait_ms(150);
    LED_Off(LED_UNLOCKED);
    LED_On(LED_ALARM);
    wait_ms(150);
    LED_Off(LED_ALARM);
#if !USE_RELAY
    LED_On(LED_DENIED);
    wait_ms(150);
    LED_Off(LED_DENIED);
#endif
}

static void warmup(void)
{
#if USE_PIR
    uint8_t s;

    if (PIR_WARMUP_S == 0) {
        return;
    }
    LCD_Clear();
    LCD_Send_Str("PIR warming up");
    for (s = PIR_WARMUP_S; s > 0; s--) {
        LCD_Goto(1, 0);
        LCD_Send_Num(s);
        LCD_Send_Str("  ");
        wait_ms(1000);
    }
#endif
}

static uint8_t rfid_active(void)
{
#if USE_RFID
    return rfid_ready;
#else
    return 0;
#endif
}

#if USE_RFID
static uint8_t rfid_poll(uint8_t *uid)
{
    RC522_status_t rc;

    if (!rfid_ready) {
        return 0;
    }
    rc = RC522_readUID(uid);
    if (rc == RC522_OK) {
        return 1;
    }
    if (rc == RC522_NO_DEVICE) {
        rfid_ready = 0;
    }
    return 0;
}

static uint8_t card_allowed(const uint8_t *uid)
{
    uint8_t i;

    for (i = 0; i < ACCESS_CARD_COUNT; i++) {
        if (memcmp(uid, allowed_cards[i], RC522_UID_SIZE) == 0) {
            return 1;
        }
    }
    return 0;
}

static void show_unknown_card(const uint8_t *uid)
{
    uint8_t i;

    LCD_Clear();
    LCD_Send_Str("Unknown card");
    LCD_Goto(1, 0);
    for (i = 0; i < RC522_UID_SIZE; i++) {
        LCD_Send_Hex(uid[i]);
        LCD_Send_Data(' ');
    }
    wait_ms(2500);
}
#endif

static void show_idle(uint8_t card_enabled)
{
    LED_On(LED_ARMED);
#if !USE_RELAY
    LED_Off(LED_DENIED);
#endif
    LCD_Clear();
    LCD_Send_Str("System Armed");
    LCD_Goto(1, 0);
    if (card_enabled) {
        LCD_Send_Str("Key or card");
    } else {
        LCD_Send_Str("Press any key");
    }
}

static int8_t read_pin(char *buffer, uint16_t budget_ms, KEYPAD_switch_t first, uint8_t *uid)
{
    uint8_t i = 0;
    KEYPAD_switch_t key;
#if USE_RFID
    uint8_t poll_ms = 0;
#endif

    LCD_Clear();
    if (uid != 0 && rfid_active()) {
        LCD_Send_Str("PIN or card:");
    } else {
        LCD_Send_Str("Enter PIN:");
    }
    LCD_Goto(1, 0);

    if (first != KEYPAD_NO_KEY) {
        buffer[i++] = key_to_char(first);
        LCD_Send_Data('*');
    }

    while (i < ACCESS_PIN_LENGTH) {
        key = KEYPAD_getKey();
        if (key != KEYPAD_NO_KEY) {
            beep_key();
            buffer[i++] = key_to_char(key);
            LCD_Send_Data('*');
        } else {
            if (budget_ms == 0) {
                return -1;
            }
#if USE_RFID
            if (uid != 0 && rfid_ready && i == 0 && ++poll_ms >= RFID_POLL_MS) {
                poll_ms = 0;
                if (rfid_poll(uid)) {
                    return 1;
                }
            }
#endif
            _delay_ms(1);
            budget_ms--;
        }
    }
    buffer[i] = '\0';
    return 0;
}

static uint8_t pin_matches(const char *buffer)
{
    return (strcmp(buffer, ACCESS_PIN) == 0);
}

static void grant_access(void)
{
    LED_Off(LED_ARMED);
    LCD_Clear();
    LCD_Send_Str("Access Granted");
    beep(2500, 300);
    door_unlock();
    wait_ms(ACCESS_UNLOCK_TIME_MS);
    door_lock();
}

static void deny_access(void)
{
    LED_Off(LED_ARMED);
#if !USE_RELAY
    LED_On(LED_DENIED);
#endif
    LCD_Clear();
    LCD_Send_Str("Access Denied");
    beep_fail();
    wait_ms(500);
}

static void lockout(void)
{
    uint8_t s;

    LCD_Clear();
    LCD_Send_Str("System Locked");
    notify("System locked");
    for (s = ACCESS_LOCKOUT_TIME_S; s > 0; s--) {
        LCD_Goto(1, 0);
        LCD_Send_Num(s);
        LCD_Send_Str("  ");
        beep(1500, 100);
        wait_ms(890);
    }
}

static void show_alarm(void)
{
    LCD_Clear();
    LCD_Send_Str("INTRUDER ALERT");
    LCD_Goto(1, 0);
    LCD_Send_Str("PIN to stop");
}

static uint8_t alarm_loop(void)
{
    char buffer[ACCESS_PIN_LENGTH + 1];
#if USE_RFID
    uint8_t card[RC522_UID_SIZE];
#endif
    uint16_t i;
    uint8_t t;
    KEYPAD_switch_t key;

    LED_Off(LED_ARMED);
    LED_On(LED_ALARM);
    show_alarm();

    for (i = 0; i < (uint16_t)(ACCESS_ALARM_TIME_S * 5U); i++) {
        BUZZER_on((i & 1U) ? 3500 : 2500, 200);
        for (t = 0; t < 200; t++) {
            key = KEYPAD_getKey();
            if (key != KEYPAD_NO_KEY) {
                beep_key();
                if (read_pin(buffer, ACCESS_PIN_TIMEOUT_MS, key, 0) == 0 && pin_matches(buffer)) {
                    BUZZER_off();
                    LED_Off(LED_ALARM);
                    return 1;
                }
                show_alarm();
            }
#if USE_RFID
            if (t == 100 && rfid_ready) {
                if (rfid_poll(card) && card_allowed(card)) {
                    BUZZER_off();
                    LED_Off(LED_ALARM);
                    return 1;
                }
            }
#endif
            _delay_ms(1);
        }
    }

    BUZZER_off();
    LED_Off(LED_ALARM);
    return 0;
}

#if TEST_SPI_LOOPBACK
static void test_spi_loopback(void)
{
    uint8_t sent = 0x55;
    uint8_t got = 0;
    uint16_t i;

    SPI_init_master(SPI_PRESCALER_16);
    LCD_Clear();
    LCD_Send_Str("SPI loopback");
    while (1) {
        for (i = 0; i < 20000; i++) {
            got = SPI_transfer(sent);
        }
        LCD_Goto(1, 0);
        LCD_Send_Str("Sent ");
        LCD_Send_Hex(sent);
        LCD_Send_Str(" Got ");
        LCD_Send_Hex(got);
        LCD_Send_Str(" ");
        sent = (sent == 0x55) ? 0xAA : 0x55;
    }
}
#endif

int main(void)
{
    char pin_buffer[ACCESS_PIN_LENGTH + 1];
    uint8_t attempts = 0;
#if USE_RFID
    uint8_t uid[RC522_UID_SIZE];
#endif

    MCUCSR = (1 << JTD);
    MCUCSR = (1 << JTD);
    LCD_init();
    KEYPAD_init(KEYPAD_INTERNAL_PULLUP);
    BUZZER_init();
#if USE_BLUETOOTH
    BT_init();
#endif
    sei();
#if USE_PIR
    PIR_init();
#endif
    GPIO_Pin_Mode(LED_UNLOCKED, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(LED_ALARM, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(LED_ARMED, GPIO_DIO_OUTPUT);
#if !USE_RELAY
    GPIO_Pin_Mode(LED_DENIED, GPIO_DIO_OUTPUT);
#endif
#if USE_RELAY
    RELAY_init();
#endif
#if USE_SERVO
    SERVO_init();
#endif
#if USE_RFID
    RC522_init();
    rfid_ready = RC522_isPresent();
    if (!rfid_ready) {
        LCD_Clear();
        notify("RFID reader not found");
        LCD_Send_Str("RFID not found");
        LCD_Goto(1, 0);
        LCD_Send_Str("Version ");
        LCD_Send_Hex(RC522_getVersion());
        beep(1000, 300);
        wait_ms(3000);
    }
#endif

    notify("System started");
    leds_selftest();
    door_lock();
#if TEST_SPI_LOOPBACK
    test_spi_loopback();
#endif
    warmup();

    while (1) {
        KEYPAD_switch_t first = KEYPAD_NO_KEY;
        uint8_t triggered = 0;
        uint8_t settle = 0;
        uint16_t budget;
        int8_t r;
        outcome_t outcome = OUTCOME_NONE;
#if USE_RFID
        uint8_t card_present = 0;
        uint8_t poll_ms = 0;
        uint16_t retry_ms = 0;
#endif

        show_idle(rfid_active());

        while (!triggered) {
            first = KEYPAD_getKey();
            if (first != KEYPAD_NO_KEY) {
                break;
            }
#if USE_PIR
            if (PIR_motion()) {
                triggered = 1;
            }
#endif
#if USE_RFID
            if (rfid_ready) {
                if (++poll_ms >= RFID_POLL_MS) {
                    poll_ms = 0;
                    if (rfid_poll(uid)) {
                        card_present = 1;
                        break;
                    }
                    if (!rfid_ready) {
                        notify("RFID reader lost");
                        show_idle(0);
                    }
                }
            } else if (++retry_ms >= RFID_RETRY_MS) {
                retry_ms = 0;
                RC522_init();
                if (RC522_isPresent()) {
                    rfid_ready = 1;
                    notify("RFID reader ready");
                    show_idle(1);
                }
            }
#endif
            _delay_ms(1);
        }

        if (triggered) {
            notify("Motion detected");
        }

        budget = triggered ? (uint16_t)(ACCESS_ENTRY_DELAY_S * 1000UL) : ACCESS_PIN_TIMEOUT_MS;

#if USE_RFID
        if (card_present) {
            r = 1;
        } else
#endif
        {
            if (first != KEYPAD_NO_KEY) {
                beep_key();
            }
            r = read_pin(pin_buffer, budget, first, UID_ARG);
        }

        if (r < 0) {
            outcome = OUTCOME_TIMEOUT;
        } else if (r == 1) {
#if USE_RFID
            outcome = card_allowed(uid) ? OUTCOME_OK : OUTCOME_BAD;
            if (outcome == OUTCOME_OK) {
                notify_card("Access granted - card", uid);
            } else {
                notify_card("Unknown card", uid);
                show_unknown_card(uid);
            }
#endif
        } else {
            outcome = pin_matches(pin_buffer) ? OUTCOME_OK : OUTCOME_BAD;
            notify(outcome == OUTCOME_OK ? "Access granted - PIN" : "Wrong PIN");
        }

        switch (outcome) {
        case OUTCOME_OK:
            attempts = 0;
            grant_access();
            settle = 1;
            break;

        case OUTCOME_BAD:
            attempts++;
            deny_access();
            if (attempts >= ACCESS_MAX_ATTEMPTS) {
                attempts = 0;
                lockout();
            }
            break;

        case OUTCOME_TIMEOUT:
            if (triggered) {
                notify("INTRUDER ALARM");
                if (alarm_loop()) {
                    notify("Alarm stopped");
                    LCD_Clear();
                    LCD_Send_Str("Alarm stopped");
                    wait_ms(1000);
                                } else {
                    notify("Alarm ended");
                }
                settle = 1;
            }
            break;

        default:
            break;
        }

#if USE_PIR
        if (triggered || settle) {
            PIR_settle(10000);
        }
#else
        (void)settle;
#endif
    }

    return 0;
}
