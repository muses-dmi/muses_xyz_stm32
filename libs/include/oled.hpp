/*
 * Name: oled.cpp
 * Author: Benedict R. Gaster
 * Date: 27th Oct 2019
 * Desc:
 *       
 *  OLED class for display incoming ORAC module/page and controls
 *
 * 
 *  All incoming messages take the form:
 *  
 *    kvalue\n
 * 
 *  where k is a single character tag, which determines the y screen postion that incoming message is to 
 *  displayed. The valid tags are defined as follows:
 * 
 *    m for module
 *    p for page
 *    1..8 for each control
 *    
 *  The screen is formated as follows:
 * 
 *       module (name)
 *       Ctrl 1
 *       Ctrl 2
 *       Ctrl 3
 *       Ctrl 4
 *       page (name)
 * 
 *   Only four controls are display at any given time. A method is provided to 
 *   switch between the first and second set of four controls.
 * 
 *   value is the message to be displayed on the screen. 
 *  
 */
#pragma once

#if defined(__ARCH_M4__)
#include "stm32f4xx_hal.h"
#elif defined(__ARCH_M7__)
#include "stm32f7xx_hal.h"
#elif defined(__ARCH_M0__)
#include "stm32f0xx_hal.h"
#else
#error ("__ARCH_XX undefined")
#endif

#include <ssd1306.h>
#include <circular_buffer.hpp>

template <std::size_t mcapacity>
class oled {
private:
    static constexpr uint32_t TEXT_LINE_HEIGHT_ = 10;
    static constexpr uint32_t NUM_CONTROLS_PER_BANK_     = 4;
    static constexpr uint32_t LINE_LENGTH_      = 21;
    static const char blank_line_[LINE_LENGTH_];

    // rx buffer for data comming into oled
    circular_buffer<char, mcapacity>& cbuffer_;

    // buffer to store partially read line
    char line_[LINE_LENGTH_];
    uint32_t current_offset_;

    // values for controls
    char controls_bank1_[NUM_CONTROLS_PER_BANK_][LINE_LENGTH_];
    char controls_bank2_[NUM_CONTROLS_PER_BANK_][LINE_LENGTH_];

    // which control bank to display
    bool display_bank_1_;

    // map key to display line on screen
    uint32_t line_y(char key) {
        switch (key) {
            case 'm': {
                return 0;
            }
            case 'p': {
                return 5;
            }
            case '1': {
                return 1;
            }
            case '2': {
                return 2;
            }
            case '3': {
                return 3;
            }
            case '4': {
                return 4;
            }
            case '5': {
                return 6;
            }
            case '6': {
                return 7;
            }
            case '7': {
                return 8;
            }
            case '8': {
                return 9;
            }
        }

        return 0;
    }

    /// update line, blanking first to avoid leaving any chars about
    void update_line(char * line, uint32_t y) {
        ssd1306_set_cursor(0,y*TEXT_LINE_HEIGHT_);
        ssd1306_write_string((char*)blank_line_, Font_7x10, 1);
        ssd1306_set_cursor(0,y*TEXT_LINE_HEIGHT_);
        ssd1306_write_string(line, Font_7x10, 1);
        ssd1306_update_display();
    }

public:
    /// setup oled class
    /// NOTE: does not initialize OLED itself, init() must be called for this
    oled(circular_buffer<char, mcapacity>& cbuffer) : 
        cbuffer_{cbuffer},
        current_offset_{0},
        display_bank_1_{true} {
    }

    /// intialize OLED display
    /// NOTE: must be called after i2c interface is intialized
    void init() const {
        ssd1306_init_display();
        ssd1306_clear_display();
        ssd1306_set_cursor(0,0);
        ssd1306_write_string("   Muses xyz", Font_7x10, 1);
        ssd1306_update_display();
    }

    void display() {
        while (!cbuffer_.empty()) {
            char c = cbuffer_.pop();
            // update OLED line, if at end of update message
            if (c == '\n') {
                uint32_t y = line_y(line_[0]);
                //ssd1306_set_cursor(0,y);

                line_[current_offset_] = '\0';
                // do we need to update bank
                if (y >= 1 && y <= 4) {
                    // bank 1
                    strcpy(controls_bank1_[y-1], &line_[1]);
                    if (display_bank_1_) {
                        update_line(&line_[1], y);
                    }
                }
                else if (y >= 6 && y <= 9) {
                    // bank 2
                    y = y - 5; // fit in control range for y
                    strcpy(controls_bank2_[y], &line_[1]);
                    if (!display_bank_1_) {
                        update_line(&line_[1], y);
                    }
                }
                else {
                    // module or page
                    update_line(&line_[1], y);
                }

                // reset ready for next message
                current_offset_ = 0;
            }
            // otherwise, just store the next char of current message
            else { 
                line_[current_offset_++] = c;
            }
        }
    }

    void switch_bank() {
        if (display_bank_1_) {
            // update screen with switched bank
            for (int i = 0; i < NUM_CONTROLS_PER_BANK_; i++ ) {
                ssd1306_set_cursor(0,(i+1)*TEXT_LINE_HEIGHT_);
                ssd1306_write_string((char*)blank_line_, Font_7x10, 1);
                ssd1306_set_cursor(0,(i+1)*TEXT_LINE_HEIGHT_);
                ssd1306_write_string(controls_bank2_[i], Font_7x10, 1);
            }
            display_bank_1_ = false;
        }
        else {
            // update screen with switched bank
            for (int i = 0; i < NUM_CONTROLS_PER_BANK_; i++ ) {
                ssd1306_set_cursor(0,(i+1)*TEXT_LINE_HEIGHT_);
                ssd1306_write_string((char*)blank_line_, Font_7x10, 1);
                ssd1306_set_cursor(0,(i+1)*TEXT_LINE_HEIGHT_);
                ssd1306_write_string(controls_bank1_[i], Font_7x10, 1);
            }
            display_bank_1_ = true;
        }
        // now update OLED
        ssd1306_update_display();
    }
};