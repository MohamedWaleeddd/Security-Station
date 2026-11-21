#include "SPI.h"
#include "../GPIO/GPIO.h"
#include "../DIO/DIO.h"
#include "../../TOOLS.h"

void SPI_init_master(SPI_prescaler_t prescaler)
{
    GPIO_Pin_Mode(GPIO_PB4, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GPIO_PB5, GPIO_DIO_OUTPUT);
    GPIO_Pin_Mode(GPIO_PB6, GPIO_DIO_INPUT);
    GPIO_Pin_Mode(GPIO_PB7, GPIO_DIO_OUTPUT);
    DIO_set_pin(GPIO_PB4);

    SPCR = (1 << SPE) | (1 << MSTR) | (uint8_t)prescaler;
}

uint8_t SPI_transfer(uint8_t data)
{
    SPDR = data;
    while (!READ_BIT(SPSR, SPIF)) {
    }
    return SPDR;
}