
/**
 * @file SEG.h
 * @brief Two-digit multiplexed seven-segment display interface.
 *
 * Segment lines share PORTA and active-high digit-select lines choose the
 * physical digit. The application must refresh both digits periodically.
 * @warning PORTA and PB1/PB2 overlap LCD and matrix-keypad connections.
 * Allocate the shared pins to only one active peripheral at a time.
 */
#ifndef SEG_H
#define SEG_H

#include "../../MCAL/GPIO/GPIO.h"

/** @name Segment A connection
 * @{ */
#define SEG_A_PORT   PORTA
#define SEG_A_DDR    DDRA
#define SEG_A_PIN_NO GPIO_PIN0
/** @} */

#define SEG_B_PORT   PORTA
#define SEG_B_DDR    DDRA
#define SEG_B_PIN_NO GPIO_PIN1

#define SEG_C_PORT   PORTA
#define SEG_C_DDR    DDRA
#define SEG_C_PIN_NO GPIO_PIN2

#define SEG_D_PORT   PORTA
#define SEG_D_DDR    DDRA
#define SEG_D_PIN_NO GPIO_PIN3

#define SEG_E_PORT   PORTA
#define SEG_E_DDR    DDRA
#define SEG_E_PIN_NO GPIO_PIN4

#define SEG_F_PORT   PORTA
#define SEG_F_DDR    DDRA
#define SEG_F_PIN_NO GPIO_PIN5

#define SEG_G_PORT   PORTA
#define SEG_G_DDR    DDRA
#define SEG_G_PIN_NO GPIO_PIN6

#define SEG_DP_PORT   PORTA
#define SEG_DP_DDR    DDRA
#define SEG_DP_PIN_NO GPIO_PIN7



/** @name Digit-select connections
 * @{ */
#define SEG_DIGIT1_PORT   PORTB
#define SEG_DIGIT1_DDR    DDRB
#define SEG_DIGIT1_PIN_NO GPIO_PIN1

#define SEG_DIGIT2_PORT   PORTB
#define SEG_DIGIT2_DDR    DDRB
#define SEG_DIGIT2_PIN_NO GPIO_PIN2
/** @} */


/** @brief Identifies one LED segment, including the decimal point. */
typedef enum {
    SEG_A,
    SEG_B,
    SEG_C,
    SEG_D,
    SEG_E,
    SEG_F,
    SEG_G,
    SEG_DP
}SEG_t;





/**
 * @brief Glyph patterns supported by the display driver.
 * @details Every base number or letter has a matching decimal-point variant.
 */
typedef enum {
    SEG_DISPLAY_0,
    SEG_DISPLAY_1,
    SEG_DISPLAY_2,
    SEG_DISPLAY_3,
    SEG_DISPLAY_4,
    SEG_DISPLAY_5,
    SEG_DISPLAY_6,
    SEG_DISPLAY_7,
    SEG_DISPLAY_8,
    SEG_DISPLAY_9,
    SEG_DISPLAY_A,
    SEG_DISPLAY_b,
    SEG_DISPLAY_C,
    SEG_DISPLAY_c,
    SEG_DISPLAY_d,
    SEG_DISPLAY_E,
    SEG_DISPLAY_e,
    SEG_DISPLAY_F,
    SEG_DISPLAY_H,
    SEG_DISPLAY_h,
    SEG_DISPLAY_i,
    SEG_DISPLAY_J,
    SEG_DISPLAY_L,
    SEG_DISPLAY_n,
    SEG_DISPLAY_o,
    SEG_DISPLAY_p,
    SEG_DISPLAY_r,
    SEG_DISPLAY_u,
    SEG_DISPLAY_y,
    SEG_DISPLAY_0_DOT,
    SEG_DISPLAY_1_DOT,
    SEG_DISPLAY_2_DOT,
    SEG_DISPLAY_3_DOT,
    SEG_DISPLAY_4_DOT,
    SEG_DISPLAY_5_DOT,
    SEG_DISPLAY_6_DOT,
    SEG_DISPLAY_7_DOT,
    SEG_DISPLAY_8_DOT,
    SEG_DISPLAY_9_DOT,
    SEG_DISPLAY_A_DOT,
    SEG_DISPLAY_b_DOT,
    SEG_DISPLAY_C_DOT,
    SEG_DISPLAY_c_DOT,
    SEG_DISPLAY_d_DOT,
    SEG_DISPLAY_E_DOT,
    SEG_DISPLAY_e_DOT,
    SEG_DISPLAY_F_DOT,
    SEG_DISPLAY_H_DOT,
    SEG_DISPLAY_h_DOT,
    SEG_DISPLAY_i_DOT,
    SEG_DISPLAY_J_DOT,
    SEG_DISPLAY_L_DOT,
    SEG_DISPLAY_n_DOT,
    SEG_DISPLAY_o_DOT,
    SEG_DISPLAY_p_DOT,
    SEG_DISPLAY_r_DOT,
    SEG_DISPLAY_u_DOT,
    SEG_DISPLAY_y_DOT,
}SEG_Display_t;




/** @brief Identifies one of the two multiplexed physical digits. */
typedef enum {
    SEG_DIGIT1,
    SEG_DIGIT2
}SEG_Digit_t;



/**
 * @brief Blank the display and configure segment/digit pins as outputs.
 * @details Clears segment latches and disables both digits before changing DDR.
 * @note Does not start a refresh timer; multiplexing belongs to the application.
 */
void SEG_init ();
/**
 * @brief Clear all segment outputs without changing digit selection.
 * @note Uses one combined mask on SEG_A_PORT; all segment pins must share it.
 */
void SEG_All_Off ();
/**
 * @brief Set one segment high without altering the other segments.
 * @param SEG Segment A..G or decimal point; unknown values do nothing.
 * @pre SEG_init() has configured the pins.
 */
void SEG_On (SEG_t SEG);
/**
 * @brief Replace the segment bus with a complete glyph pattern.
 * @param Digit Supported glyph identifier; invalid values blank the segments.
 * @note Does not select a digit. All segments must share SEG_A_PORT.
 * For multiplexing, disable digits, write the pattern, then select a digit.
 */
void SEG_Write (SEG_Display_t Digit);

/**
 * @brief Disable both digits, then enable the selected digit.
 * @param Digit SEG_DIGIT1 or SEG_DIGIT2; invalid values leave both disabled.
 */
void SEG_Digit_Set (SEG_Digit_t Digit);
/**
 * @brief Disable both digit-select outputs, preserving the segment pattern.
 * @note The combined mask assumes both digit-select pins share SEG_DIGIT1_PORT.
 */
void SEG_Digits_Off ();

#endif /* SEG_H */
