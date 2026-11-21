#include "UART.h"

#include "../../CONFIG.h"
#include <avr/io.h>
#include <avr/interrupt.h>



/* ============================================
 * Buffers
 * ============================================ */

static volatile uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
static volatile uint8_t rx_buffer[UART_RX_BUFFER_SIZE];


/* ============================================
 * Buffer Indexes
 * ============================================ */

static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;

static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;


/* ============================================
 * Error Flags
 * ============================================ */

static volatile uint8_t rx_overflow = 0;


/* ============================================
 * UART Initialization
 * ============================================ */

void UART_init(uint32_t baud_rate)
{
    uint16_t ubrr_value;


    /*
     * Asynchronous Normal Mode
     *
     *              F_CPU
     * UBRR = ------------------ - 1
     *         16 x Baud Rate
     */

    ubrr_value =
        (uint16_t)((F_CPU / (16UL * baud_rate)) - 1UL);


    /* Normal asynchronous speed */

    UCSRA &= ~(1 << U2X);


    /* Set Baud Rate */

    UBRRH = (uint8_t)(ubrr_value >> 8);
    UBRRL = (uint8_t)ubrr_value;


    /*
     * Enable:
     *
     * RX
     * TX
     * RX Complete Interrupt
     */

    UCSRB =
        (1 << RXEN) |
        (1 << TXEN) |
        (1 << RXCIE);


    /*
     * 8 Data
     * No Parity
     * 1 Stop
     */

    UCSRC =
        (1 << URSEL) |
        (1 << UCSZ1) |
        (1 << UCSZ0);


    /* Reset Buffers */

    tx_head = 0;
    tx_tail = 0;

    rx_head = 0;
    rx_tail = 0;

    rx_overflow = 0;
}


/* ============================================
 * Send One Byte
 * ============================================ */

uint8_t UART_sendByte(uint8_t data)
{
    uint8_t next_head;


    next_head = tx_head + 1;


    if (next_head >= UART_TX_BUFFER_SIZE)
    {
        next_head = 0;
    }


    /*
     * Buffer Full
     */

    if (next_head == tx_tail)
    {
        return 0;
    }


    /*
     * Store Data
     */

    tx_buffer[tx_head] = data;


    /*
     * Move Head
     */

    tx_head = next_head;

    /*
     * Start / Continue Transmission
     */
    UCSRB |= (1 << UDRIE);


    return 1;
}


/* ============================================
 * Send String
 * ============================================ */

uint8_t UART_sendString(const char *str)
{
    uint8_t count = 0;


    while (*str != '\0')
    {
        if (UART_sendByte((uint8_t)*str) == 0)
        {
            break;
        }


        str++;

        count++;
    }


    return count;
}


/* ============================================
 * Receive One Byte
 * ============================================ */

uint8_t UART_receiveByte(uint8_t *data)
{
    /*
     * No Data
     */

    if (rx_head == rx_tail)
    {
        return 0;
    }


    /*
     * Read Data
     */

    *data = rx_buffer[rx_tail];


    /*
     * Move Tail
     */

    rx_tail++;


    if (rx_tail >= UART_RX_BUFFER_SIZE)
    {
        rx_tail = 0;
    }


    return 1;
}


/* ============================================
 * Is RX Data Available?
 * ============================================ */

uint8_t UART_isDataAvailable(void)
{
    return (rx_head != rx_tail);
}


/* ============================================
 * Is TX Buffer Full?
 * ============================================ */

uint8_t UART_isTxBufferFull(void)
{
    uint8_t next_head;


    next_head = tx_head + 1;


    if (next_head >= UART_TX_BUFFER_SIZE)
    {
        next_head = 0;
    }


    return (next_head == tx_tail);
}


/* ============================================
 * RX Overflow
 * ============================================ */

uint8_t UART_getRxOverflow(void)
{
    return rx_overflow;
}


void UART_clearRxOverflow(void)
{
    rx_overflow = 0;
}


/* ============================================
 * RX Complete Interrupt
 * ============================================ */

ISR(USART_RXC_vect)
{
    uint8_t data;
    uint8_t next_head;


    /*
     * Read hardware receive register.
     */

    data = UDR;


    /*
     * Calculate next position.
     */

    next_head = rx_head + 1;


    if (next_head >= UART_RX_BUFFER_SIZE)
    {
        next_head = 0;
    }


    /*
     * Buffer Full
     */

    if (next_head == rx_tail)
    {
        rx_overflow = 1;

        return;
    }


    /*
     * Store received byte.
     */

    rx_buffer[rx_head] = data;


    /*
     * Move head.
     */

    rx_head = next_head;
}


/* ============================================
 * Data Register Empty Interrupt
 * ============================================ */
ISR(USART_UDRE_vect)
{
    /*
     * Data available?
     */
    if (tx_head != tx_tail)
    {
        /*
         * Send next byte.
         */
        UDR = tx_buffer[tx_tail];
        /*
         * Move tail.
         */
        tx_tail++;
        if (tx_tail >= UART_TX_BUFFER_SIZE)
        {
            tx_tail = 0;
        }
    }
    /*
     * If buffer becomes empty,
     * disable UDRE interrupt.
     */

    if (tx_head == tx_tail)
    {
        UCSRB &= ~(1 << UDRIE);
    }
}