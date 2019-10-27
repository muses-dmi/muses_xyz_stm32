/*
 * ssd1306.h
 *
 *  Created on: 3 May 2019
 *      Author: frazer
 */

#ifndef INC_SSD1306_H_
#define INC_SSD1306_H_

#include "font.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_I2C_ADDR	0x3C

#define SSD1306_HEIGHT		64
#define SSD1306_WIDTH		128

typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} SSD1306_t;

void ssd1306_write_command(uint8_t byte);
void ssd1306_write_data(uint8_t* buffer, size_t buff_size);
void ssd1306_init_display();
void ssd1306_clear_display();
void ssd1306_update_display();
void ssd1306_write_pixel(uint8_t x, uint8_t y, uint8_t value);
char ssd1306_write_char(char ch, FontDef font, uint8_t value);
char ssd1306_write_string(char* str, FontDef font, uint8_t value);
void ssd1306_set_cursor(uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif

#endif /* INC_SSD1306_H_ */
