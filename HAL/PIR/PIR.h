#ifndef PIR_H
#define PIR_H

#include <stdint.h>
#include "../../MCAL/GPIO/GPIO.h"

#define PIR_PIN GPIO_PD4

void PIR_init(void);
uint8_t PIR_motion(void);
void PIR_settle(uint16_t max_ms);

#endif