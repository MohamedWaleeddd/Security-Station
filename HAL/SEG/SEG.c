/**
 * @file SEG.c
 * @brief Implementation of the multiplexed seven-segment display driver.
 */
#include "SEG.h"

/** @name Segment bit masks used to assemble display glyphs
 * @{ */
#define SEG_PATTERN_A  (1 << SEG_A_PIN_NO)
#define SEG_PATTERN_B  (1 << SEG_B_PIN_NO)
#define SEG_PATTERN_C  (1 << SEG_C_PIN_NO)
#define SEG_PATTERN_D  (1 << SEG_D_PIN_NO)
#define SEG_PATTERN_E  (1 << SEG_E_PIN_NO)
#define SEG_PATTERN_F  (1 << SEG_F_PIN_NO)
#define SEG_PATTERN_G  (1 << SEG_G_PIN_NO)
#define SEG_PATTERN_DP (1 << SEG_DP_PIN_NO)
#define SEG_PATTERN_MASK (SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP)
/** @} */

/** @brief Mask containing both digit-select output bits. */
#define SEG_DIGITS_MASK ((1 << SEG_DIGIT1_PIN_NO) | (1 << SEG_DIGIT2_PIN_NO))
/** @brief Electrical level that enables a digit. */
#define SEG_DIGIT_ON    GPIO_HIGH
/** @brief Electrical level that disables a digit. */
#define SEG_DIGIT_OFF   GPIO_LOW


/** @copydoc SEG_All_Off */
void SEG_All_Off ();
/** @copydoc SEG_On */
void SEG_On (SEG_t SEG);
/** @brief Convert a public glyph identifier into its segment bit pattern. */
static uint8_t SEG_Get_Pattern(SEG_Display_t Digit);

/** @copydoc SEG_init */
void SEG_init (){
    /* Begin dark to avoid a visible flash while direction registers change. */
    SEG_All_Off();
    SEG_Digits_Off();

    GPIO_Pin_Direction(&SEG_A_DDR , SEG_A_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_B_DDR , SEG_B_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_C_DDR , SEG_C_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_D_DDR , SEG_D_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_E_DDR , SEG_E_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_F_DDR , SEG_F_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_G_DDR , SEG_G_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_DP_DDR , SEG_DP_PIN_NO , GPIO_OUTPUT);  

    GPIO_Pin_Direction(&SEG_DIGIT1_DDR , SEG_DIGIT1_PIN_NO , GPIO_OUTPUT);
    GPIO_Pin_Direction(&SEG_DIGIT2_DDR , SEG_DIGIT2_PIN_NO , GPIO_OUTPUT);  
}
/** @copydoc SEG_On */
void SEG_On (SEG_t SEG){
    switch ( SEG ){
        case SEG_A:
         GPIO_Pin_Write(&SEG_A_PORT, SEG_A_PIN_NO , GPIO_HIGH);
         break;
        case SEG_B:
         GPIO_Pin_Write(&SEG_B_PORT, SEG_B_PIN_NO , GPIO_HIGH);
         break;
        case SEG_C:
         GPIO_Pin_Write(&SEG_C_PORT, SEG_C_PIN_NO , GPIO_HIGH);
         break;
        case SEG_D:
         GPIO_Pin_Write(&SEG_D_PORT, SEG_D_PIN_NO , GPIO_HIGH);
         break;
        case SEG_E:
         GPIO_Pin_Write(&SEG_E_PORT, SEG_E_PIN_NO , GPIO_HIGH);
         break;
        case SEG_F:
         GPIO_Pin_Write(&SEG_F_PORT, SEG_F_PIN_NO , GPIO_HIGH);
         break;
        case SEG_G:
         GPIO_Pin_Write(&SEG_G_PORT, SEG_G_PIN_NO , GPIO_HIGH);
         break;
        case SEG_DP:
         GPIO_Pin_Write(&SEG_DP_PORT, SEG_DP_PIN_NO , GPIO_HIGH);
         break;
    }
}
/** @copydoc SEG_All_Off */
void SEG_All_Off (){
    SEG_A_PORT &= (uint8_t)(~SEG_PATTERN_MASK);
}

/** @copydoc SEG_Write */
void SEG_Write (SEG_Display_t Digit){
    SEG_A_PORT = (SEG_A_PORT & (uint8_t)(~SEG_PATTERN_MASK)) | SEG_Get_Pattern(Digit);
}
/** @copydoc SEG_Digit_Set */
void SEG_Digit_Set (SEG_Digit_t Digit){
    /* Prevent ghosting by blanking both digits before selecting the next one. */
    SEG_Digits_Off();

    switch(Digit){
        case SEG_DIGIT1:
            GPIO_Pin_Write(&SEG_DIGIT1_PORT, SEG_DIGIT1_PIN_NO , SEG_DIGIT_ON);
            break;
        case SEG_DIGIT2:
            GPIO_Pin_Write(&SEG_DIGIT2_PORT, SEG_DIGIT2_PIN_NO , SEG_DIGIT_ON);
            break;
    }
}

/** @copydoc SEG_Digits_Off */
void SEG_Digits_Off (){
    if (SEG_DIGIT_OFF == GPIO_HIGH)
    {
        SEG_DIGIT1_PORT |= SEG_DIGITS_MASK;
    }
    else
    {
        SEG_DIGIT1_PORT &= (uint8_t)(~SEG_DIGITS_MASK);
    }
}

/**
 * @brief Build the port pattern for one display glyph.
 * @param Digit Glyph requested by the application.
 * @return Bit mask for segments A through DP, or zero for an invalid value.
 */
static uint8_t SEG_Get_Pattern(SEG_Display_t Digit)
{
    switch(Digit){
        /* ---------------- NUMBERS ---------------- */
        case SEG_DISPLAY_0: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F;
        case SEG_DISPLAY_1: return SEG_PATTERN_B | SEG_PATTERN_C;
        case SEG_DISPLAY_2: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_3: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_G;
        case SEG_DISPLAY_4: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_5: return SEG_PATTERN_A | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_6: return SEG_PATTERN_A | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_7: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C;
        case SEG_DISPLAY_8: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_9: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_F | SEG_PATTERN_G;

        /* ---------------- LETTERS ---------------- */
        case SEG_DISPLAY_A: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_b: return SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_C: return SEG_PATTERN_A | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F;
        case SEG_DISPLAY_c: return SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_d: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_E: return SEG_PATTERN_A | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_e: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_F: return SEG_PATTERN_A | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_H: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_h: return SEG_PATTERN_C | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_i: return SEG_PATTERN_C;
        case SEG_DISPLAY_J: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E;
        case SEG_DISPLAY_L: return SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F;
        case SEG_DISPLAY_n: return SEG_PATTERN_C | SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_o: return SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_p: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G;
        case SEG_DISPLAY_r: return SEG_PATTERN_E | SEG_PATTERN_G;
        case SEG_DISPLAY_u: return SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E;
        case SEG_DISPLAY_y: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_F | SEG_PATTERN_G;

        /* -------------- NUMBERS (WITH DOT) -------------- */
        case SEG_DISPLAY_0_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_DP;
        case SEG_DISPLAY_1_DOT: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_DP;
        case SEG_DISPLAY_2_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_3_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_4_DOT: return SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_5_DOT: return SEG_PATTERN_A | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_6_DOT: return SEG_PATTERN_A | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_7_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_DP;
        case SEG_DISPLAY_8_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_E | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP;
        case SEG_DISPLAY_9_DOT: return SEG_PATTERN_A | SEG_PATTERN_B | SEG_PATTERN_C | SEG_PATTERN_D | SEG_PATTERN_F | SEG_PATTERN_G | SEG_PATTERN_DP;

        /* -------------- LETTERS (WITH DOT) -------------- */
        case SEG_DISPLAY_A_DOT: return SEG_Get_Pattern(SEG_DISPLAY_A) | SEG_PATTERN_DP;
        case SEG_DISPLAY_b_DOT: return SEG_Get_Pattern(SEG_DISPLAY_b) | SEG_PATTERN_DP;
        case SEG_DISPLAY_C_DOT: return SEG_Get_Pattern(SEG_DISPLAY_C) | SEG_PATTERN_DP;
        case SEG_DISPLAY_c_DOT: return SEG_Get_Pattern(SEG_DISPLAY_c) | SEG_PATTERN_DP;
        case SEG_DISPLAY_d_DOT: return SEG_Get_Pattern(SEG_DISPLAY_d) | SEG_PATTERN_DP;
        case SEG_DISPLAY_E_DOT: return SEG_Get_Pattern(SEG_DISPLAY_E) | SEG_PATTERN_DP;
        case SEG_DISPLAY_e_DOT: return SEG_Get_Pattern(SEG_DISPLAY_e) | SEG_PATTERN_DP;
        case SEG_DISPLAY_F_DOT: return SEG_Get_Pattern(SEG_DISPLAY_F) | SEG_PATTERN_DP;
        case SEG_DISPLAY_H_DOT: return SEG_Get_Pattern(SEG_DISPLAY_H) | SEG_PATTERN_DP;
        case SEG_DISPLAY_h_DOT: return SEG_Get_Pattern(SEG_DISPLAY_h) | SEG_PATTERN_DP;
        case SEG_DISPLAY_i_DOT: return SEG_Get_Pattern(SEG_DISPLAY_i) | SEG_PATTERN_DP;
        case SEG_DISPLAY_J_DOT: return SEG_Get_Pattern(SEG_DISPLAY_J) | SEG_PATTERN_DP;
        case SEG_DISPLAY_L_DOT: return SEG_Get_Pattern(SEG_DISPLAY_L) | SEG_PATTERN_DP;
        case SEG_DISPLAY_n_DOT: return SEG_Get_Pattern(SEG_DISPLAY_n) | SEG_PATTERN_DP;
        case SEG_DISPLAY_o_DOT: return SEG_Get_Pattern(SEG_DISPLAY_o) | SEG_PATTERN_DP;
        case SEG_DISPLAY_p_DOT: return SEG_Get_Pattern(SEG_DISPLAY_p) | SEG_PATTERN_DP;
        case SEG_DISPLAY_r_DOT: return SEG_Get_Pattern(SEG_DISPLAY_r) | SEG_PATTERN_DP;
        case SEG_DISPLAY_u_DOT: return SEG_Get_Pattern(SEG_DISPLAY_u) | SEG_PATTERN_DP;
        case SEG_DISPLAY_y_DOT: return SEG_Get_Pattern(SEG_DISPLAY_y) | SEG_PATTERN_DP;
        default: return 0;
    }
}
