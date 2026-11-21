#ifndef SPI_H
#define SPI_H

#include <stdint.h>

typedef enum {
    SPI_PRESCALER_4,
    SPI_PRESCALER_16,
    SPI_PRESCALER_64,
    SPI_PRESCALER_128
} SPI_prescaler_t;

void SPI_init_master(SPI_prescaler_t prescaler);
uint8_t SPI_transfer(uint8_t data);

#endif