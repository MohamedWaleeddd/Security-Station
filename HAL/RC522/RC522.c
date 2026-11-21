#include "../../CONFIG.h"
#include "RC522.h"
#include "../../MCAL/SPI/SPI.h"
#include "../../MCAL/DIO/DIO.h"
#include <util/delay.h>

#define RC522_REG_COMMAND      0x01
#define RC522_REG_COM_IRQ      0x04
#define RC522_REG_ERROR        0x06
#define RC522_REG_FIFO_DATA    0x09
#define RC522_REG_FIFO_LEVEL   0x0A
#define RC522_REG_BIT_FRAMING  0x0D
#define RC522_REG_COLL         0x0E
#define RC522_REG_MODE         0x11
#define RC522_REG_TX_CONTROL   0x14
#define RC522_REG_TX_ASK       0x15
#define RC522_REG_T_MODE       0x2A
#define RC522_REG_T_PRESCALER  0x2B
#define RC522_REG_T_RELOAD_H   0x2C
#define RC522_REG_T_RELOAD_L   0x2D
#define RC522_REG_VERSION      0x37

#define RC522_CMD_IDLE         0x00
#define RC522_CMD_TRANSCEIVE   0x0C
#define RC522_CMD_SOFT_RESET   0x0F

#define PICC_CMD_REQA          0x26
#define PICC_CMD_ANTICOLL_CL1  0x93

static void RC522_write(uint8_t reg, uint8_t value)
{
    DIO_clear_pin(RC522_SS_PIN);
    SPI_transfer((uint8_t)((reg << 1) & 0x7E));
    SPI_transfer(value);
    DIO_set_pin(RC522_SS_PIN);
}

static uint8_t RC522_read(uint8_t reg)
{
    uint8_t value;

    DIO_clear_pin(RC522_SS_PIN);
    SPI_transfer((uint8_t)(((reg << 1) & 0x7E) | 0x80));
    value = SPI_transfer(0x00);
    DIO_set_pin(RC522_SS_PIN);
    return value;
}

static void RC522_setMask(uint8_t reg, uint8_t mask)
{
    RC522_write(reg, (uint8_t)(RC522_read(reg) | mask));
}

static void RC522_clearMask(uint8_t reg, uint8_t mask)
{
    RC522_write(reg, (uint8_t)(RC522_read(reg) & (uint8_t)~mask));
}

static RC522_status_t RC522_transceive(const uint8_t *send,
                                       uint8_t send_len,
                                       uint8_t *back,
                                       uint8_t *back_len,
                                       uint8_t tx_last_bits)
{
    uint16_t guard = 30000U;
    uint8_t irq;
    uint8_t n;
    uint8_t i;

    RC522_write(RC522_REG_COMMAND, RC522_CMD_IDLE);
    RC522_write(RC522_REG_COM_IRQ, 0x7F);
    RC522_write(RC522_REG_FIFO_LEVEL, 0x80);

    for (i = 0; i < send_len; i++) {
        RC522_write(RC522_REG_FIFO_DATA, send[i]);
    }

    RC522_write(RC522_REG_BIT_FRAMING, tx_last_bits);
    RC522_write(RC522_REG_COMMAND, RC522_CMD_TRANSCEIVE);
    RC522_setMask(RC522_REG_BIT_FRAMING, 0x80);

    do {
        irq = RC522_read(RC522_REG_COM_IRQ);
        guard--;
    } while (guard && !(irq & 0x31));

    RC522_clearMask(RC522_REG_BIT_FRAMING, 0x80);

    if (guard == 0 || !(irq & 0x30)) {
        return RC522_NO_CARD;
    }
    if (RC522_read(RC522_REG_ERROR) & 0x13) {
        return RC522_ERROR;
    }

    n = RC522_read(RC522_REG_FIFO_LEVEL);
    if (n > *back_len) {
        return RC522_ERROR;
    }
    for (i = 0; i < n; i++) {
        back[i] = RC522_read(RC522_REG_FIFO_DATA);
    }
    *back_len = n;
    return RC522_OK;
}

static RC522_status_t RC522_request(void)
{
    uint8_t cmd = PICC_CMD_REQA;
    uint8_t atqa[2];
    uint8_t len = 2;
    RC522_status_t status;

    RC522_clearMask(RC522_REG_COLL, 0x80);
    status = RC522_transceive(&cmd, 1, atqa, &len, 0x07);
    if (status == RC522_OK && len != 2) {
        status = RC522_ERROR;
    }
    return status;
}

static RC522_status_t RC522_anticollision(uint8_t *uid)
{
    uint8_t cmd[2] = {PICC_CMD_ANTICOLL_CL1, 0x20};
    uint8_t buf[5];
    uint8_t len = 5;
    uint8_t bcc = 0;
    uint8_t i;
    RC522_status_t status;

    RC522_clearMask(RC522_REG_COLL, 0x80);
    status = RC522_transceive(cmd, 2, buf, &len, 0x00);
    if (status != RC522_OK) {
        return status;
    }
    if (len != 5) {
        return RC522_ERROR;
    }
    for (i = 0; i < 4; i++) {
        bcc ^= buf[i];
        uid[i] = buf[i];
    }
    if (bcc != buf[4]) {
        return RC522_ERROR;
    }
    return RC522_OK;
}

void RC522_init(void)
{
    SPI_init_master(SPI_PRESCALER_16);
    GPIO_Pin_Mode(RC522_SS_PIN, GPIO_DIO_OUTPUT);
    DIO_set_pin(RC522_SS_PIN);

    RC522_write(RC522_REG_COMMAND, RC522_CMD_SOFT_RESET);
    _delay_ms(50);

    RC522_write(RC522_REG_T_MODE, 0x80);
    RC522_write(RC522_REG_T_PRESCALER, 0xA9);
    RC522_write(RC522_REG_T_RELOAD_H, 0x03);
    RC522_write(RC522_REG_T_RELOAD_L, 0xE8);
    RC522_write(RC522_REG_TX_ASK, 0x40);
    RC522_write(RC522_REG_MODE, 0x3D);
    RC522_setMask(RC522_REG_TX_CONTROL, 0x03);
}

uint8_t RC522_getVersion(void)
{
    return RC522_read(RC522_REG_VERSION);
}

uint8_t RC522_isPresent(void)
{
    uint8_t version = RC522_getVersion();
    return (version != 0x00 && version != 0xFF);
}

static uint8_t RC522_antennaOn(void)
{
    return ((RC522_read(RC522_REG_TX_CONTROL) & 0x03) == 0x03);
}

RC522_status_t RC522_readUID(uint8_t *uid)
{
    if (!RC522_antennaOn()) {
        RC522_init();
        if (!RC522_antennaOn()) {
            return RC522_NO_DEVICE;
        }
    }
    if (RC522_request() != RC522_OK) {
        return RC522_NO_CARD;
    }
    return RC522_anticollision(uid);
}