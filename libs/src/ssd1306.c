/*
 * ssd1306.c
 *
 *  Created on: 3 May 2019
 *      Author: frazer
 */

//#include "i2c.h"
#include "ssd1306.h"

static uint8_t display_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static SSD1306_t SSD1306;

// Send a byte to the command register
void ssd1306_write_command(uint8_t byte) {
	HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR<<1, 0x00, 1, &byte, 1, 50);
}

// Send data
void ssd1306_write_data(uint8_t* buffer, size_t buff_size) {
	HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR<<1, 0x40, 1, buffer, buff_size, 50);
}

void ssd1306_init_display() {
	  ssd1306_write_command(0xAE); //display off
	  ssd1306_write_command(0x20); //Set Memory Addressing Mode
	  ssd1306_write_command(0x10); // 00,Horizontal Addressing Mode; 01,Vertical Addressing Mode;
	                              // 10,Page Addressing Mode (RESET); 11,Invalid

	  ssd1306_write_command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

	  ssd1306_write_command(0xA8); //--set multiplex ratio(1 to 64) - CHECK
	  ssd1306_write_command(0x3F); //

	  ssd1306_write_command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	  ssd1306_write_command(0xD3); //-set display offset - CHECK
	  ssd1306_write_command(0x00); //-not offset

	  ssd1306_write_command(0xD5); //--set display clock divide ratio/oscillator frequency
	  ssd1306_write_command(0xF0); //--set divide ratio

	  ssd1306_write_command(0xD9); //--set pre-charge period
	  ssd1306_write_command(0x22); //

	  ssd1306_write_command(0xDA); //--set com pins hardware configuration - CHECK
	  ssd1306_write_command(0x12);

	  ssd1306_write_command(0xDB); //--set vcomh
	  ssd1306_write_command(0x20); //0x20,0.77xVcc

	  ssd1306_write_command(0x8D); //--set DC-DC enable
	  ssd1306_write_command(0x14); //
	  ssd1306_write_command(0xAF); //--turn on SSD1306 panel
}

void ssd1306_clear_display() {

    uint32_t i;

    for(i = 0; i < sizeof(display_buffer); i++) {
    	display_buffer[i] = 0x00;
    }

    ssd1306_set_cursor(0, 0);
}

// Write the screenbuffer with changed to the screen
void ssd1306_update_display() {

    uint8_t i;

    for(i = 0; i < 8; i++) {

    	ssd1306_write_command(0xB0 + i);
    	ssd1306_write_command(0x00);
    	ssd1306_write_command(0x10);
    	ssd1306_write_data(&display_buffer[SSD1306_WIDTH*i], SSD1306_WIDTH);
    }
}

void ssd1306_write_pixel(uint8_t x, uint8_t y, uint8_t value) {

    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Draw in the right color
    if(value) {

    	display_buffer[x+(y/8)*SSD1306_WIDTH] |= 1 << (y%8);

    } else {

    	display_buffer[x+(y/8)*SSD1306_WIDTH] &= ~(1 << (y%8));

    }
}

char ssd1306_write_char(char ch, FontDef font, uint8_t value) {

    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + font.FontWidth) || SSD1306_HEIGHT <= (SSD1306.CurrentY + font.FontHeight)) {
        return 0;
    }

    // Use the font to write
    for(i = 0; i < font.FontHeight; i++) {

        b = font.data[(ch-32) * font.FontHeight + i];

        for(j = 0; j < font.FontWidth; j++) {

            if((b << j) & 0x8000)  {
            	ssd1306_write_pixel((SSD1306.CurrentX + j), (SSD1306.CurrentY + i), 1);
            } else {
            	ssd1306_write_pixel((SSD1306.CurrentX + j), (SSD1306.CurrentY + i), 0);
            }
        }

    }

    // The current space is now taken
    SSD1306.CurrentX += font.FontWidth;

    // Return written char for validation
    return ch;
}

char ssd1306_write_string(char* str, FontDef font, uint8_t value) {

    // Write until null-byte
    while (*str) {

        if (ssd1306_write_char(*str, font, value) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

void ssd1306_set_cursor(uint8_t x, uint8_t y) {

    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;

}
