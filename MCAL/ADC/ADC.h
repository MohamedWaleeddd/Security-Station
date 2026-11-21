/**
 * @file ADC.h
 * @brief Analog-to-digital converter interface for the AVR peripheral.
 *
 * Supports single-ended and differential inputs, selectable references,
 * result alignment, clock prescaling, auto triggering, and interrupts.
 * @note Configure analog pins separately; ADC_init() only writes ADC registers.
 * Reads return raw unsigned codes and do not convert to voltage or sign-extend
 * differential results. Reference voltage and gain belong to the application.
 */
#ifndef ADC_H
#define ADC_H
#include <avr/io.h>
/**
 * @details For a single-ended 10-bit conversion, an ideal code represents
 * approximately Vref / 1024 volts per step. Select AREF, AVCC, or the internal
 * reference as appropriate for the board. The historical identifier
 * ADC_2056mV_REFRENCE selects the internal nominal 2.56 V reference.
 */
/** @brief Selects the voltage used as the ADC full-scale reference. */
typedef enum {
ADC_AREF_REFRENCE =0b00000000,
ADC_AVCC_REFRENCE =0b01000000,
ADC_2056mV_REFRENCE = 0b11000000,

} ADC_refrence_t;

/** @brief Selects an ADC input channel or differential input pair and gain. */
typedef enum {
    ADC_SINGLE_ENDED_ADC0 = 0b0000,
    ADC_SINGLE_ENDED_ADC1 = 0b0001,
    ADC_SINGLE_ENDED_ADC2 = 0b0010,
    ADC_SINGLE_ENDED_ADC3 = 0b0011,
    ADC_SINGLE_ENDED_ADC4 = 0b0100,
    ADC_SINGLE_ENDED_ADC5 = 0b0101,
    ADC_SINGLE_ENDED_ADC6 = 0b0110,
    ADC_SINGLE_ENDED_ADC7 = 0b0111,
    ADC_DIF_ADC0_ADC0_10X =0b01000,
    ADC_DIF_ADC1_ADC0_10X =0b01001,
    ADC_DIF_ADC0_ADC0_200X =0b01010,
    ADC_DIF_ADC1_ADC0_200X =0b01011,
    ADC_DIF_ADC2_ADC2_10X =0b01100,
    ADC_DIF_ADC3_ADC2_10X =0b01101,
    ADC_DIF_ADC2_ADC2_200X =0b01110,
    ADC_DIF_ADC3_ADC2_200X =0b01111,
    ADC_DIF_ADC0_ADC1_1X =0b10000,
    ADC_DIF_ADC1_ADC1_1X =0b10001,
    ADC_DIF_ADC2_ADC1_1X =0b10010,
    ADC_DIF_ADC3_ADC1_1X =0b10011,
    ADC_DIF_ADC4_ADC1_1X =0b10100,
    ADC_DIF_ADC5_ADC1_1X =0b10101,
    ADC_DIF_ADC6_ADC1_1X =0b10110,
    ADC_DIF_ADC7_ADC1_1X =0b10111,
    ADC_DIF_ADC0_ADC2_1X =0b11000,
    ADC_DIF_ADC1_ADC2_1X =0b11001,
    ADC_DIF_ADC2_ADC2_1X =0b11010,
    ADC_DIF_ADC3_ADC2_1X =0b11011,
    ADC_DIF_ADC4_ADC2_1X =0b11100,
    ADC_DIF_ADC5_ADC2_1X =0b11101,
    ADC_SINGLE_ENDED_1220mV = 0b11110,
    ADC_SINGLE_ENDED_GND = 0b11111,
    
}ADC_INPUT_channel_t;

/** @brief Selects the alignment of the conversion result in ADC registers. */
typedef enum {
    ADC_RESULT_RIGHT_ADJUST =0b000000,
    ADC_RESULT_LEFT_ADJUST= 0b100000
}ADC_result_adj_t;
/**
 * 16MHZ
 * ADC 50KHZ - 200KHZ
 * PRESCALER
 * 2      - 4     - 8    - 16   - 32      - 64      - 128
 * 8MHZ   - 4MHZ  - 2MHZ - 1MHZ - 500KHZ  - 250KHZ  - 125KHZ
 */
/** @brief Selects the division ratio between the CPU clock and ADC clock. */
typedef enum {
ADC_PRESCALER_2 = 0b001,
ADC_PRESCALER_4 = 0b010,
ADC_PRESCALER_8 ,
ADC_PRESCALER_16 ,
ADC_PRESCALER_32 ,
ADC_PRESCALER_64 ,
ADC_PRESCALER_128 
}ADC_prescaler_t;
/*
0 0 0 Free Running mode
0 0 1 Analog Comparator
0 1 0 External Interrupt Request 0
0 1 1 Timer/Counter0 Compare Match
1 0 0 Timer/Counter0 Overflow
1 0 1 Timer/Counter1 Compare Match B
1 1 0 Timer/Counter1 Overflow
1 1 1 Timer/Counter1 Capture Event
*/
/** @brief Selects the hardware event used for automatic conversion triggering. */
typedef enum {
ADC_FREE_RUNNING_TRIGGER = 0b00000000,
ADC_ANALOG_COMPARATOR_TRIGGER = 0b00100000,
ADC_EXTERNAL_INTERRUPT_TRIGGER = 0b01000000,
ADC_TIMER0_COMPARE_MATCH_TRIGGER = 0b01100000,
ADC_TIMER0_OVERFLOW_TRIGGER = 0b10000000,
ADC_TIMER1_COMPARE_MATCH_B_TRIGGER = 0b10100000,
ADC_TIMER1_OVERFLOW_TRIGGER = 0b11000000,
ADC_TIMER1_CAPTURE_EVENT_TRIGGER = 0b11100000,
ADC_NO_AUTO_TRIGGER = -1
}ADC_auto_trigger_t;

/**
 * @brief Configure and enable the ADC peripheral without starting a conversion.
 * @param refrence Encoded reference selection for ADMUX.
 * @param input Encoded single-ended or differential MUX selection.
 * @param adjust Left or right result alignment.
 * @param prescale ADC clock divider; ADC clock equals CPU clock / divider.
 * @param tigger Automatic trigger source, or ADC_NO_AUTO_TRIGGER.
 * @details Overwrites ADMUX and ADCSRA, including the interrupt-enable state.
 * For auto triggering, enables ADATE and updates SFIOR trigger bits.
 * @note Call ADC_start_conversion() to start/arm conversion as required.
 * Reconfiguration does not preserve an in-progress conversion.
 */
void ADC_init(ADC_refrence_t refrence ,
              ADC_INPUT_channel_t input ,
              ADC_result_adj_t adjust ,
              ADC_prescaler_t prescale ,
              ADC_auto_trigger_t tigger
            );
/**
 * @brief Replace the ADMUX channel field, preserving reference and alignment.
 * @param input Valid encoded channel or differential pair.
 * @pre Coordinate changes with conversion timing; this function does not wait.
 * @note Does not configure pin direction or start a conversion.
 */
void ADC_set_Channel (ADC_INPUT_channel_t input);
/**
 * @brief Set ADSC to request conversion or arm configured automatic operation.
 * @pre ADC_init() has enabled and configured the ADC.
 * @note Returns immediately; it does not wait for completion.
 */
void ADC_start_conversion ();
/** @brief Callback type used by the conversion-complete interrupt. */
typedef void (*ADC_Callback_t)(void);

/**
 * @brief Wait for ADSC to clear and return the upper eight conversion bits.
 * @return Raw unsigned sample in the range 0..255.
 * @details Supports left alignment via ADCH and right alignment by combining
 * ADCL and ADCH and discarding the two least significant result bits.
 * @pre A conversion must have been requested; this function does not start one.
 * @warning Polling has no timeout. In free-running mode ADSC can remain set,
 * so this wait is not a reliable per-conversion completion mechanism.
 * @warning The right-adjusted expression does not sequence the ADCL and ADCH
 * reads explicitly; correct hardware access requires low-byte-first ordering.
 */
uint8_t ADC_read_8bit();
/**
 * @brief Wait for ADSC to clear and assemble a raw ADC result.
 * @return Intended 10-bit unsigned conversion code.
 * @pre A conversion has been requested; this function does not start one.
 * @warning The left-adjusted branch currently masks ADCL bits 1:0 rather
 * than extracting bits 7:6, so it loses the two low result bits.
 * @warning Expressions do not explicitly sequence low-byte-first reads.
 * @warning The ADSC wait has no timeout and can remain blocked in free-running
 * mode. Differential results are not sign-extended by this interface.
 */
uint16_t ADC_read_10bit();
/**
 * @brief Register the conversion-complete callback.
 * @param callback Function invoked in ISR context, or null to skip dispatch.
 * @pre Keep ADC interrupts disabled while replacing this multi-byte pointer.
 * @warning The current pointer is not volatile-qualified and assignment is
 * not protected from interrupts. Keep callbacks short and nonblocking.
 */
void ADC_interrupt_init (ADC_Callback_t callback);
/**
 * @brief Enable the ADC conversion-complete interrupt source.
 * @note Does not change global interrupt enable, start a conversion, or
 * explicitly clear a pending completion flag.
 */
void ADC_interrupt_enble();
/**
 * @brief Disable the ADC conversion-complete interrupt source.
 * @note Does not change global interrupt enable, start a conversion, or
 * explicitly clear a pending completion flag.
 */
void ADC_interrupt_Disable();

#endif /* ADC_H */
