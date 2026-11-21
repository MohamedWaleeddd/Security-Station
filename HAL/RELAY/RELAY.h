#ifndef RELAY_H
#define RELAY_H

#include "../../MCAL/GPIO/GPIO.h"

#define RELAY_PIN GPIO_PB3

void RELAY_init(void);
void RELAY_on(void);
void RELAY_off(void);

#endif