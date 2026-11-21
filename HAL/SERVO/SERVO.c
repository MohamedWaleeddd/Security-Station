#include "../../CONFIG.h"
#include "SERVO.h"
#include "../../MCAL/TIMER/TIMER.h"

#define SERVO_PERIOD_TICKS   ((F_CPU / 8UL / 50UL) - 1UL)
#define SERVO_TICKS_PER_US   (F_CPU / 8000000UL)

void SERVO_init(void)
{
    GPIO_Pin_Mode(SERVO_PIN, GPIO_DIO_OUTPUT);
    TIMER1_setICR1((uint16_t)SERVO_PERIOD_TICKS);
    TIMER1_setOCR1A((uint16_t)((uint32_t)SERVO_LOCKED_US * SERVO_TICKS_PER_US));
    TIMER1_init(TIMER1_FAST_PWM_ICR1,
                TIMER1_OCR1A_FAST_PWM_NON_INVERTING,
                TIMER1_OCR1B_DISCONNECTED,
                TIMER1_PRESCALER_8);
}

void SERVO_set_pulse_us(uint16_t us)
{
    TIMER1_setOCR1A((uint16_t)((uint32_t)us * SERVO_TICKS_PER_US));
}

void SERVO_lock(void)
{
    SERVO_set_pulse_us(SERVO_LOCKED_US);
}

void SERVO_unlock(void)
{
    SERVO_set_pulse_us(SERVO_UNLOCKED_US);
}