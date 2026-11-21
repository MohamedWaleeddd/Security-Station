#include "../../CONFIG.h"
#include "PIR.h"
#include "../../MCAL/DIO/DIO.h"
#include <util/delay.h>

void PIR_init(void)
{
    GPIO_Pin_Mode(PIR_PIN, GPIO_DIO_INPUT);
}

uint8_t PIR_motion(void)
{
#if PIR_ACTIVE_HIGH
    return (DIO_read_pin(PIR_PIN) == DIO_HIGH);
#else
    return (DIO_read_pin(PIR_PIN) == DIO_LOW);
#endif
}

void PIR_settle(uint16_t max_ms)
{
    uint16_t quiet = 0;

    while (max_ms--) {
        if (PIR_motion()) {
            quiet = 0;
        } else if (++quiet >= 2000U) {
            return;
        }
        _delay_ms(1);
    }
}