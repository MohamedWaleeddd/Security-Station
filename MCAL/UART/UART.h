#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/*
 * ============================================
 * ATmega32A Non-Blocking UART Driver
 * ============================================
 *
 * Communication:
 * - Asynchronous
 * - 8 Data Bits
 * - No Parity
 * - 1 Stop Bit
 *
 * Driver Design:
 * - TX Ring Buffer
 * - RX Ring Buffer
 * - RX Complete Interrupt
 * - Data Register Empty Interrupt
 *
 * No blocking loops are used.
 */


/* ============================================
 * Buffer Size
 * ============================================
 *
 * One location is always kept unused
 * to distinguish FULL from EMPTY.
 *
 * With size 64:
 * usable capacity = 63 bytes
 */

#define UART_TX_BUFFER_SIZE     64
#define UART_RX_BUFFER_SIZE     64


/* ============================================
 * Initialization
 * ============================================ */

/**
 * @brief Initialize UART.
 *
 * @param baud_rate Desired baud rate.
 *
 * Example:
 *
 * UART_init(9600);
 */
void UART_init(uint32_t baud_rate);


/* ============================================
 * Transmit Functions
 * ============================================ */

/**
 * @brief Add one byte to TX buffer.
 *
 * Non-blocking.
 *
 * @param data Byte to send.
 *
 * @return
 * 1 -> Byte added successfully
 * 0 -> TX buffer is full
 */
uint8_t UART_sendByte(uint8_t data);


/**
 * @brief Add a string to TX buffer.
 *
 * Non-blocking.
 *
 * Stops if TX buffer becomes full.
 *
 * @param str Null-terminated string.
 *
 * @return Number of characters added.
 */
uint8_t UART_sendString(const char *str);


/**
 * @brief Check whether TX buffer is full.
 *
 * @return
 * 1 -> Full
 * 0 -> Space available
 */
uint8_t UART_isTxBufferFull(void);


/* ============================================
 * Receive Functions
 * ============================================ */

/**
 * @brief Read one byte from RX buffer.
 *
 * Non-blocking.
 *
 * @param data Pointer where received byte
 *             will be stored.
 *
 * @return
 * 1 -> Byte successfully read
 * 0 -> RX buffer empty
 */
uint8_t UART_receiveByte(uint8_t *data);


/**
 * @brief Check if received data is available.
 *
 * @return
 * 1 -> Data available
 * 0 -> RX buffer empty
 */
uint8_t UART_isDataAvailable(void);


/**
 * @brief Check whether RX buffer overflow occurred.
 *
 * @return
 * 1 -> Overflow occurred
 * 0 -> No overflow
 */
uint8_t UART_getRxOverflow(void);


/**
 * @brief Clear RX overflow flag.
 */
void UART_clearRxOverflow(void);


#endif /* UART_H_ */