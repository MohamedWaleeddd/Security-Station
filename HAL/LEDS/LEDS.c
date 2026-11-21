/**
 * @file LEDS.c
 * @brief Implementation of the board indicator LED driver.
 */
#include "LEDS.h"

/** @copydoc LED_init */
void LED_init (){
    /** Preserve existing latch levels while enabling output drivers. */
    GPIO_Pin_Mode (LED1,GPIO_DIO_OUTPUT );
    GPIO_Pin_Mode (LED2,GPIO_DIO_OUTPUT );
    GPIO_Pin_Mode (LED3,GPIO_DIO_OUTPUT );
    GPIO_Pin_Mode (LED4,GPIO_DIO_OUTPUT );
}
/** @copydoc LED_On */
void LED_On (LEDS_t led){
    DIO_set_pin(led);
}
/** @copydoc LED_Off */
void LED_Off (LEDS_t led){
    DIO_clear_pin(led);
}
/** @copydoc LED_Toggle */
void LED_Toggle (LEDS_t led){
    DIO_toggle_pin(led);
}

