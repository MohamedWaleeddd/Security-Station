/**
 * @file LEDS.h
 * @brief Board-level interface for the four indicator LEDs.
 *
 * The pin mapping below is the single source of truth for LED connections.
 * All LEDs are treated as active-high outputs.
 */
#ifndef LEDS_H
#define LEDS_H

#include "../../MCAL/DIO/DIO.h"
#include "../../MCAL/GPIO/GPIO.h"


/** @brief Logical identifiers for the board's indicator LEDs. */
typedef enum {
    LED1 = GPIO_PB3,
    LED2 = GPIO_PC2,
    LED3 = GPIO_PC7,
    LED4 = GPIO_PD6
}LEDS_t;

/**
 * @brief Invert the active-high LED output.
 * @param led One of LED1, LED2, LED3, or LED4.
 * @pre LED_init() has configured the pins as outputs.
 * @note The identifier is passed directly to DIO; invalid values are unchecked.
 */
void LED_Toggle (LEDS_t led);
/**
 * @brief Clear the active-high LED output.
 * @param led One of LED1, LED2, LED3, or LED4.
 * @pre LED_init() has configured the pins as outputs.
 * @note The identifier is passed directly to DIO; invalid values are unchecked.
 */
void LED_Off (LEDS_t led);
/**
 * @brief Set the active-high LED output.
 * @param led One of LED1, LED2, LED3, or LED4.
 * @pre LED_init() has configured the pins as outputs.
 * @note The identifier is passed directly to DIO; invalid values are unchecked.
 */
void LED_On (LEDS_t led);
/**
 * @brief Configure all four LED pins as outputs.
 * @details Uses the existing PORT latch values; does not force the LEDs off.
 * @note LED pins must not be owned by another peripheral.
 */
void LED_init ();

#endif /* LEDS_H */
