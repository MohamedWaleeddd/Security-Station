#ifndef RC522_H
#define RC522_H

#include <stdint.h>
#include "../../MCAL/GPIO/GPIO.h"

#define RC522_SS_PIN   GPIO_PB4
#define RC522_UID_SIZE 4

typedef enum {
    RC522_OK,
    RC522_NO_CARD,
    RC522_ERROR,
    RC522_NO_DEVICE
} RC522_status_t;

void RC522_init(void);
uint8_t RC522_getVersion(void);
uint8_t RC522_isPresent(void);
RC522_status_t RC522_readUID(uint8_t *uid);

#endif