/**
 * @file KEYPAD.h
 * @brief Debounced input driver for direct buttons or a 3x3 matrix keypad.
 *
 * Define KEYPAD_DIRECT_MODE before including this header to use three direct
 * active-low buttons. Otherwise, the default 3x3 matrix scanner is built.
 * @note Define the mode consistently when compiling KEYPAD.c as well as
 * application files; a define in main.c alone does not configure this driver.
 */
#ifndef KEYPAD_H
#define KEYPAD_H

#include "../../MCAL/GPIO/GPIO.h"

/*
 * Define KEYPAD_DIRECT_MODE before including this file to use three direct
 * buttons. If it is not defined, the driver uses the 3x3 matrix keypad.
 */

#ifdef KEYPAD_DIRECT_MODE

/** @name Direct-button hardware mapping
 * @{ */
#define KEYPAD_BUTTON_1_DDR            DDRD
#define KEYPAD_BUTTON_1_PORT           PORTD
#define KEYPAD_BUTTON_1_PIN            PIND
#define KEYPAD_BUTTON_1_PIN_NO         PIN2

#define KEYPAD_BUTTON_2_DDR            DDRD
#define KEYPAD_BUTTON_2_PORT           PORTD
#define KEYPAD_BUTTON_2_PIN            PIND
#define KEYPAD_BUTTON_2_PIN_NO         PIN3

#define KEYPAD_BUTTON_3_DDR            DDRD
#define KEYPAD_BUTTON_3_PORT           PORTD
#define KEYPAD_BUTTON_3_PIN            PIND
#define KEYPAD_BUTTON_3_PIN_NO         PIN7
/** @} */

#else

/** @name Matrix-row hardware mapping
 * @{ */
#define KEYPAD_ROW_1_DDR               DDRA
#define KEYPAD_ROW_1_PORT              PORTA
#define KEYPAD_ROW_1_PIN               PINA
#define KEYPAD_ROW_1_PIN_NO            PIN1

#define KEYPAD_ROW_2_DDR               DDRA
#define KEYPAD_ROW_2_PORT              PORTA
#define KEYPAD_ROW_2_PIN               PINA
#define KEYPAD_ROW_2_PIN_NO            PIN2

#define KEYPAD_ROW_3_DDR               DDRA
#define KEYPAD_ROW_3_PORT              PORTA
#define KEYPAD_ROW_3_PIN               PINA
#define KEYPAD_ROW_3_PIN_NO            PIN3
/** @} */

/** @name Matrix-column hardware mapping
 * @{ */
#define KEYPAD_COL_1_DDR               DDRD
#define KEYPAD_COL_1_PORT              PORTD
#define KEYPAD_COL_1_PIN               PIND
#define KEYPAD_COL_1_PIN_NO            PIN2

#define KEYPAD_COL_2_DDR               DDRD
#define KEYPAD_COL_2_PORT              PORTD
#define KEYPAD_COL_2_PIN               PIND
#define KEYPAD_COL_2_PIN_NO            PIN3

#define KEYPAD_COL_3_DDR               DDRD
#define KEYPAD_COL_3_PORT              PORTD
#define KEYPAD_COL_3_PIN               PIND
#define KEYPAD_COL_3_PIN_NO            PIN7
/** @} */

#endif

/** @brief Number of equal samples required before accepting a state change. */
#define KEYPAD_DEBOUNCE_COUNT          5
/** @brief Raw electrical state of a released key when pull-ups are used. */
#define KEYPAD_RELEASED                GPIO_HIGH
/** @brief Raw electrical state of a pressed key when pull-ups are used. */
#define KEYPAD_PRESSED                 GPIO_LOW

/** @brief Key identifiers returned to the application. */
typedef enum {
    KEYPAD_NO_KEY, /**< No new debounced key press is available. */
    KEYPAD_SW1 = 1,
    KEYPAD_SW2,
    KEYPAD_SW3,
    KEYPAD_SW4,
    KEYPAD_SW5,
    KEYPAD_SW6,
    KEYPAD_SW7,
    KEYPAD_SW8,
    KEYPAD_SW9
} KEYPAD_switch_t;

/** @brief Selects whether the driver enables AVR internal input pull-ups. */
typedef enum {
    KEYPAD_INTERNAL_PULLUP,
    KEYPAD_EXTERNAL_PULLUP
} KEYPAD_Pull_t;

/**
 * @brief Configure keypad GPIO directions, idle levels, and pull-ups.
 * @param pullup Internal or external pull-up configuration.
 * @note External mode leaves existing PORT pull-up bits unchanged; provide
 * external pull-ups and clear previously enabled internal ones if required.
 * @note Reinitializing GPIO does not reset the static debounce state.
 */
void KEYPAD_init(KEYPAD_Pull_t pullup);
/**
 * @brief Scan and debounce the keypad once.
 * @return A key identifier on a new press, otherwise KEYPAD_NO_KEY.
 * @note Call periodically; debounce counts samples, not milliseconds.
 * A changed sample resets the counter, five matching calls increment it,
 * and the following matching call accepts the state at the default setting.
 * @details Only new press transitions are returned; a held key does not
 * repeat. A debounced release is needed before another press is reported.
 * @warning Static state is not reentrant. Matrix scanning gives priority to
 * the first active column/row and is intended for single-key operation;
 * other keys can miss state updates while an earlier key remains held.
 */
KEYPAD_switch_t KEYPAD_getKey(void);

#endif /* KEYPAD_H */
