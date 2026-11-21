#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "../../MCAL/GPIO/GPIO.h"

#define SERVO_PIN GPIO_PD5

void SERVO_init(void);
void SERVO_set_pulse_us(uint16_t us);
void SERVO_lock(void);
void SERVO_unlock(void);

#endif