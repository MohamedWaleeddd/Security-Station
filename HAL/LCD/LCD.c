/**
 * @file LCD.c
 * @brief GPIO transfers and initialization for a four-bit character LCD.
 * @details Commands and characters are sent high nibble first, then low nibble.
 * Fixed delays replace busy-flag polling; calls block until those delays finish.
 */
#include "LCD.h"
#include "../../MCAL/DIO/DIO.h"
#include "../../CONFIG.h"

/**
 * @copydoc LCD_Enable_Pulse
 */
void LCD_Enable_Pulse () {
    DIO_set_pin(LCD_EN);
    _delay_us(1);
    DIO_clear_pin(LCD_EN);
    _delay_us(50);
}

/** @note A nibble such as 0x0A drives D7..D4 as 1010. */
/**
 * @copydoc LCD_Write4Bits
 */
void LCD_Write4Bits(uint8_t data){
    DIO_write_pin(LCD_D4,READ_BIT(data ,0));
    DIO_write_pin(LCD_D5,READ_BIT(data ,1));
    DIO_write_pin(LCD_D6,READ_BIT(data ,2));
    DIO_write_pin(LCD_D7,READ_BIT(data ,3));
    LCD_Enable_Pulse ();
}

/**
 * @copydoc LCD_SendCommand
 */
void LCD_SendCommand(uint8_t command){
    DIO_clear_pin(LCD_RS);
    LCD_Write4Bits(command >> 4);
    LCD_Write4Bits(command &0x0F);
    _delay_ms(2);
}
/**
 * @copydoc LCD_Send_Data
 */
void LCD_Send_Data(uint8_t command){
    DIO_set_pin(LCD_RS);
    LCD_Write4Bits(command >> 4);
    LCD_Write4Bits(command &0x0F);
    _delay_ms(2);
}
/**
 * @copydoc LCD_Send_Str
 */
void LCD_Send_Str (char * str){
    while ( *str != '\0'){
        LCD_Send_Data(*str);
        str++;
    }
}
/**
 * @copydoc LCD_init
 */
void LCD_init(){
    GPIO_Pin_Mode (LCD_D4 , GPIO_DIO_OUTPUT  );
    GPIO_Pin_Mode (LCD_D5 , GPIO_DIO_OUTPUT  );
    GPIO_Pin_Mode (LCD_D6 , GPIO_DIO_OUTPUT  );
    GPIO_Pin_Mode (LCD_D7 , GPIO_DIO_OUTPUT  );
    GPIO_Pin_Mode (LCD_RS , GPIO_DIO_OUTPUT  );
    GPIO_Pin_Mode (LCD_EN , GPIO_DIO_OUTPUT  );

    
    DIO_clear_pin(LCD_RS);
    DIO_clear_pin(LCD_EN);
    _delay_ms(20);

    /*
     * Force LCD into known state.
     * At power-up the LCD assumes 8-bit mode.
     */

    LCD_Write4Bits(0x03);
    _delay_ms(5);

    LCD_Write4Bits(0x03);
    _delay_us(150);

    LCD_Write4Bits(0x03);

    /*
     * Switch to 4-bit mode
     */
    LCD_Write4Bits(0x02);




    /*
     * 0x28
     *
     * 4-bit mode
     * 2 lines
     * 5x8 font
     */
    LCD_SendCommand(0x28);


    /*
     * 0x0C
     *
     * Display ON
     * Cursor OFF
     * Blink OFF
     */
    LCD_SendCommand(0x0C);


    /*
     * 0x06
     *
     * Cursor moves right
     * No display shift
     */
    LCD_SendCommand(0x06);
    /*
     * Clear display
     */
    LCD_SendCommand(0x01);
    _delay_ms(2);
}

void LCD_Clear(void){
    LCD_SendCommand(LCD_CLEAR_DISPLAY);
    _delay_ms(2);
}

void LCD_Goto(uint8_t row, uint8_t col){
    LCD_SendCommand((uint8_t)((row ? LCD_LINE2 : LCD_LINE1) + col));
}

void LCD_Send_Num(uint16_t number){
    char digits[5];
    uint8_t count = 0;

    if (number == 0){
        LCD_Send_Data('0');
        return;
    }
    while (number > 0){
        digits[count++] = (char)('0' + (number % 10));
        number /= 10;
    }
    while (count > 0){
        LCD_Send_Data((uint8_t)digits[--count]);
    }
}

void LCD_Send_Hex(uint8_t value){
    const char hex[] = "0123456789ABCDEF";
    LCD_Send_Data((uint8_t)hex[value >> 4]);
    LCD_Send_Data((uint8_t)hex[value & 0x0F]);
}
