/**
 * @file LCD.h
 * @brief Blocking four-bit character LCD interface.
 * @details The driver writes D4..D7, RS, and EN; there is no busy-flag read.
 * @pre Connect the LCD R/W input low. Configure F_CPU to the actual CPU clock
 * and enable compiler optimization for the avr-libc delay routines.
 * @warning LCD pins overlap the seven-segment display mapping. Do not drive
 * both peripherals on these pins concurrently. Eight-bit command constants
 * are listed for reference; this implementation only transfers four-bit data.
 */
#ifndef LCD_H
#define LCD_H


#include "../../CONFIG.h"
#include <avr/io.h>
#include <util/delay.h>
#include "../../MCAL/GPIO/GPIO.h"

/**
 * @brief Encoded GPIO connections for the LCD control and data bus.
 */
typedef enum {
LCD_RS  = GPIO_PB1,
LCD_EN = GPIO_PB2,
LCD_D4 = GPIO_PA4,
LCD_D5 = GPIO_PA5,
LCD_D6 = GPIO_PA6,
LCD_D7 = GPIO_PA7,
}LCD_pin_t;

// Core Commands
#define LCD_CLEAR_DISPLAY           0x01
#define LCD_RETURN_HOME             0x02

// Entry Mode Set (Cursor Move Direction)
#define LCD_ENTRY_MODE_DEC          0x04 // Cursor moves left
#define LCD_ENTRY_MODE_DEC_SHIFT    0x05 // Cursor moves left, display shifts
#define LCD_ENTRY_MODE_INC          0x06 // Cursor moves right (Default)
#define LCD_ENTRY_MODE_INC_SHIFT    0x07 // Cursor moves right, display shifts

// Display On/Off Control
#define LCD_DISPLAY_OFF             0x08 
#define LCD_DISPLAY_ON              0x0C // Display ON, Cursor OFF, Blink OFF
#define LCD_DISPLAY_ON_CURSOR       0x0E // Display ON, Cursor ON, Blink OFF
#define LCD_DISPLAY_ON_BLINK        0x0F // Display ON, Cursor ON, Blink ON

// Cursor / Display Shift
#define LCD_MOVE_CURSOR_LEFT        0x10 // Shift cursor left
#define LCD_MOVE_CURSOR_RIGHT       0x14 // Shift cursor right
#define LCD_SHIFT_DISPLAY_LEFT      0x18 // Shift entire display left
#define LCD_SHIFT_DISPLAY_RIGHT     0x1C // Shift entire display right

// Function Set
#define LCD_4BIT_1LINE_5x8          0x20
#define LCD_4BIT_2LINE_5x8          0x28 // Standard 4-bit mode for 16x2
#define LCD_8BIT_1LINE_5x8          0x30
#define LCD_8BIT_2LINE_5x8          0x38 // Standard 8-bit mode for 16x2

// Memory Addresses
#define LCD_CGRAM_START             0x40 // Base address for Custom Characters
#define LCD_DDRAM_START             0x80 // Base address for Display RAM (Line 1)
#define LCD_LINE1                   0x80 // Force cursor to Line 1, Col 1
#define LCD_LINE2                   0xC0 // Force cursor to Line 2, Col 1



/**
 * @brief Latch one nibble with an enable pulse.
 * @details Drives EN high for a requested 1 us, then low for 50 us.
 * @pre Data and RS must already be stable; EN must be configured as output.
 */
void LCD_Enable_Pulse ();
/**
 * @brief Send the low nibble on D4..D7 and pulse enable.
 * @param data Bits 0..3 to transmit; bits 4..7 are ignored.
 * @note Does not change RS; the caller chooses command or character mode.
 */
void LCD_Write4Bits(uint8_t data);
/**
 * @brief Send an instruction byte with RS low.
 * @param command LCD instruction or address-setting byte.
 * @details Sends two nibbles and waits an additional 2 ms.
 * @pre LCD_init() has established four-bit operation.
 */
void LCD_SendCommand(uint8_t command);
/**
 * @brief Write one character code with RS high.
 * @param command Character data byte, despite the parameter name.
 * @details Writes to the current LCD address and waits an additional 2 ms.
 * @pre LCD_init() has completed.
 */
void LCD_Send_Data(uint8_t command);
/**
 * @brief Write a null-terminated string at the current LCD address.
 * @param str Non-null pointer to readable, null-terminated character data.
 * @details Sends each byte through LCD_Send_Data(); the buffer is not modified.
 * @note No newline translation, length limit, or screen-boundary check is made.
 */
void LCD_Send_Str (char * str);
/**
 * @brief Initialize the pins and LCD for four-bit, two-line operation.
 * @details Starts with RS and EN low, waits 20 ms, sends the reset nibbles,
 * then selects four-bit transfers, a 5x8 font, display on, cursor/blink off,
 * incrementing addresses, and a cleared display. All waits are blocking.
 */
void LCD_init();
void LCD_Clear(void);
void LCD_Goto(uint8_t row, uint8_t col);
void LCD_Send_Num(uint16_t number);
void LCD_Send_Hex(uint8_t value);
#endif