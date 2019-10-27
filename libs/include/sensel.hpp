/*
 * Name: sensel.cpp
 * Author: Benedict R. Gaster
 * Date: 9th Oct 2019
 * Desc:
 *       
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

#define SENSEL_RX_BUFFER_SIZE 512

// struct for Sensel contact information
struct __attribute__((__packed__)) SenselContact {
    // contact id
    uint8_t id;
    // CONTACT_INVALID = 0 , CONTACT_START = 1 , CONTACT_MOVE = 2 , CONTACT_END = 3
    uint8_t type;
    float x_pos;
    float y_pos;
    float total_force;
    float area;
    float orientation;
    float major_axis;
    float minor_axis;
};

// struct for Sensel frame, only contains contacts
struct __attribute__((__packed__)) SenselFrame {
  uint8_t n_contacts;
  SenselContact contacts[16];
};

// flag for enabling contact scanning
const uint8_t SENSEL_REG_CONTACTS_FLAG = 0x04;

// ack for read register
const uint8_t SENSEL_PT_READ_ACK = 1;

// ack for read variable size register
const uint8_t SENSEL_PT_RVS_ACK = 3;

// ack for write register
const uint8_t SENSEL_PT_WRITE_ACK = 5;

const uint8_t SENSEL_CONTACT_TYPE_INVALID = 0;
const uint8_t SENSEL_CONTACT_TYPE_START = 1;
const uint8_t SENSEL_CONTACT_TYPE_MOVE = 2;
const uint8_t SENSEL_CONTACT_TYPE_END = 3;

//--------------------------------------------------------------

class Sensel {
private:
    // RX buffer for Sensel serial
    static uint8_t rx_buf_[SENSEL_RX_BUFFER_SIZE];

    // counter for RX buffer
    unsigned int counter_;

    UART_HandleTypeDef& huart_;
public:
    Sensel(UART_HandleTypeDef& huart) : 
        counter_(0),
        huart_(huart) {

    }

    void setFrameContent(uint8_t content);
    void startScanning();
    void stopScanning();
    void writeReg(uint8_t addr, uint8_t sizeVar, uint8_t data);
    void readReg(uint8_t addr, uint8_t sizeVar, uint8_t* buf);
    void getFrame(SenselFrame& frame);
    // read frame and send over VCOM
    void getSendFrame();

    // output frame data to virtual com port
    void outputFrame(char * buffer, SenselFrame& frame);
};