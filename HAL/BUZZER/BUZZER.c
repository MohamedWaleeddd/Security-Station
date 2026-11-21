/**
 * @file BUZZER.c
 * @brief Nonblocking buzzer tone generation using Timer0 compare interrupts.
 *
 * Timer0 runs in CTC mode with a prescaler of 64. Its hardware output is
 * disconnected; Buzzer_Toggle() toggles BUZZER_PIN in software on each
 * compare interrupt. Two interrupts therefore produce one complete cycle.
 * The callback also counts interrupts to stop a timed tone.
 *
 * @note Timer0 is dedicated to this driver while it is in use. Other code
 * changing its mode, prescaler, OCR0, or callback changes buzzer behavior.
 */
#include "../../CONFIG.h"
#include "BUZZER.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../../MCAL/TIMER/TIMER.h"
#include "../../MCAL/DIO/DIO.h"


/**
 * @brief Target number of compare interrupts for the requested tone.
 * @warning Shared between foreground code and the interrupt callback, but
 * currently neither volatile-qualified nor protected against interrupted
 * multi-byte updates. On this 8-bit CPU, a 32-bit write is not atomic.
 */
uint32_t BUZZER_delay_ticks = 0 ;
static void Buzzer_Toggle(void);

/** @copydoc BUZZER_init */
void BUZZER_init(){
    /** Configure CTC counting; the callback, not OC0 hardware, drives the pin. */
    TIMER0_init(TIMER0_CTC_MODE, TIMER0_OCR0_DISCONNECTED,TIMER0_PRESCALER_64);
    GPIO_Pin_Mode(BUZZER_PIN,GPIO_DIO_OUTPUT);
    /** Register the callback before enabling its compare interrupt source. */
    TIMER0_OCR0_interrupt_init(Buzzer_Toggle);
    TIMER0_OCR0_interrupt_EN();
}
/** @copydoc BUZZER_on */
void BUZZER_on(uint16_t freq, uint16_t ms){
    /** Timer clock ticks per half-period; distinct from the ISR's counter. */
    uint32_t ticks;
    /**
     * Requested interrupt count: two toggles per cycle, with ms / 1000
     * converting milliseconds to seconds. Integer division truncates.
     * The cast makes multiplication 32-bit, fixing the original 16-bit
     * overflow for ordinary tones such as 2500 Hz for 1000 ms.
     * @warning Multiplying by 2 can still overflow uint32_t for large input
     * pairs. This count also uses requested rather than realized frequency.
     */
     BUZZER_delay_ticks = (uint32_t)freq * ms * 2UL / 1000UL;
    /** Treat zero frequency as stop, and avoid division by zero below. */
    if (freq == 0U) {
        BUZZER_off();
        return;
    }

    /**
     * @details CTC counts from zero through OCR0, inclusive:
     * @f[ f_{actual} = \frac{F_{CPU}}{2 N (OCR0 + 1)} @f]
     * Thus ticks = OCR0 + 1 is approximately F_CPU / (2 * N * freq).
     * Adding half the denominator rounds to the nearest whole timer tick.
     * The UL constants keep intermediate arithmetic unsigned and 32-bit.
     * Keep 64UL consistent with TIMER0_PRESCALER_64 in BUZZER_init.
     */
    ticks = (F_CPU + (2UL * 64UL * freq) / 2UL) / (2UL * 64UL * freq);

    /** Clamp the half-period to 1..256 ticks so OCR0 fits in eight bits. */
    if (ticks < 1UL) {
        ticks = 1UL;
    } else if (ticks > 256UL) {
        ticks = 256UL;
    }

    /** Convert the tick count to the inclusive TOP value and enable sound. */
    TIMER0_setOCR0((uint8_t)(ticks - 1UL));
    TIMER0_OCR0_interrupt_EN();
}
/** @copydoc BUZZER_off */
void BUZZER_off(){
      TIMER0_OCR0_interrupt_disable();
      DIO_clear_pin(BUZZER_PIN);
}

/**
 * @brief Toggle the buzzer and stop when the interrupt count reaches its target.
 * @details Invoked by the Timer0 compare ISR through the registered callback.
 * Each invocation represents one nominal half-period. Interrupt latency and
 * missed compare events can affect waveform timing, especially at high rates.
 * @warning The elapsed counter resets only on automatic completion. Manual
 * stopping or replacing a tone retains the elapsed count for the next tone.
 * @warning The pin is toggled before testing the target. A zero target still
 * produces a brief high pulse when the pin was initially low.
 */
static void Buzzer_Toggle(void)
{
    /** Elapsed compare interrupts, retained between callback invocations. */
    static uint32_t ticks = 0;
    ticks ++;
    DIO_toggle_pin(BUZZER_PIN);
    /** Automatic completion clears both counts and leaves the output low. */
    if(ticks >= BUZZER_delay_ticks){
        ticks = 0;
        BUZZER_delay_ticks = 0;
        BUZZER_off();
    }
}
