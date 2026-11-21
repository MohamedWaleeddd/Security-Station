/**
 * @file BUZZER.h
 * @brief Interface for interrupt-driven, timed buzzer tones on Timer0.
 */
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "../../MCAL/DIO/DIO.h"

/** @brief Buzzer output pin, toggled in software by the Timer0 callback. */
#define BUZZER_PIN GPIO_PB0

/**
 * @brief Configure the buzzer output and Timer0 compare callback.
 * @details Selects CTC mode, prescaler 64, and disconnected hardware OC0.
 * Enables the Timer0 compare interrupt source but does not enable global
 * interrupts; the caller must do so, for example with sei().
 * @pre Timer0 is available exclusively for buzzer timing.
 * @warning The timer and its interrupt are enabled before a tone is requested.
 * OCR0, the pending compare flag, and the output latch are not initialized
 * here, so an initial callback can occur as soon as global interrupts allow it.
 */
void BUZZER_init();
/**
 * @brief Start or update a timed tone and return immediately.
 * @param freq Requested frequency in hertz; zero calls BUZZER_off().
 * @param ms Requested duration in milliseconds, measured by counting compare
 * interrupts. This function does not block for that duration.
 * @pre Call BUZZER_init() and enable global interrupts for tone generation.
 * @details The timer half-period is rounded and clamped to 1..256 timer ticks.
 * At F_CPU = 16 MHz and prescaler 64, the timer's theoretical square-wave
 * range is approximately 488.28 Hz to 125 kHz. Software interrupt overhead
 * limits the achievable rate; this range is not a guaranteed output range.
 * For 2500 Hz, OCR0 is 49 and a nominal 1000 ms tone needs 5000 interrupts.
 * @warning Known limitations of the current implementation:
 * - Zero duration still enables the callback and can generate a brief pulse.
 * - Large freq/ms products can overflow the 32-bit duration calculation.
 * - Duration uses requested frequency, so rounding or clamping changes it.
 * - Shared duration updates are not volatile-qualified or atomic.
 * - Starting another tone does not reset the callback's elapsed counter.
 * - TCNT0 and a pending compare flag are not cleared before enabling the
 *   interrupt, so the first interval may be shortened; changing OCR0 below
 *   the running counter may also delay the next match until wraparound.
 */
void BUZZER_on(uint16_t freq, uint16_t ms);
/**
 * @brief Disable the compare interrupt and drive the buzzer output low.
 * @details Timer0 continues counting. Its pending compare flag, the target
 * count, and the callback's elapsed count are not cleared by this function.
 */
void BUZZER_off();

#endif /* BUZZER_H */
