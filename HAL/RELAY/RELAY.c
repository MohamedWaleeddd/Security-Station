#include "../../CONFIG.h"
#include "RELAY.h"
#include "../../MCAL/DIO/DIO.h"

void RELAY_on(void)
{
#if RELAY_ACTIVE_LOW
    DIO_clear_pin(RELAY_PIN);
#else
    DIO_set_pin(RELAY_PIN);
#endif
}

void RELAY_off(void)
{
#if RELAY_ACTIVE_LOW
    DIO_set_pin(RELAY_PIN);
#else
    DIO_clear_pin(RELAY_PIN);
#endif
}

void RELAY_init(void)
{
    RELAY_off();
    GPIO_Pin_Mode(RELAY_PIN, GPIO_DIO_OUTPUT);
}