/*
 * Name: sensel.cpp
 * Author: Benedict R. Gaster
 * Date: 9th Oct 2019
 * Desc:
 *       
 *
 */

#include <string.h>
#include <stdlib.h>

#include "usbd_cdc_if.h"

#include "semihost.hpp"

#include <sensel.hpp>
#include <sensel_reg_map.hpp>

//-----------------------------------------------------------------------------------

// uncomment the following define to enable debug info generation
#define SenselDebugSerial 1

//-----------------------------------------------------------------------------------

unsigned long _convertBytesToU32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
  return ((((unsigned long)b3) & 0xff) << 24) | ((((unsigned long)b2) & 0xff) << 16) |
         ((((unsigned long)b1) & 0xff) << 8) | (((unsigned long)b0) & 0xff);
}

// convert 2 uint8_t s to an unsigned int
unsigned int _convertBytesToU16(uint8_t b0, uint8_t  b1)
{
  return ((((unsigned int)b1) & 0xff) << 8) | (((unsigned int)b0) & 0xff);
}

// convert 2 uint8_t s to a signed int
int _convertBytesToS16(uint8_t  b0, uint8_t  b1)
{
  return ((((int)b1)) << 8) | (((int)b0) & 0xff);
}

//Flush the SenselSerial of all data
void _senselFlush()
{
  // TODO.
}

//-----------------------------------------------------------------------------------

// Buffer used to store incomming data
uint8_t Sensel::rx_buf_[SENSEL_RX_BUFFER_SIZE];

// set frame content for Sensel device, supports SENSEL_REG_CONTACTS_FLAG
void Sensel::setFrameContent(uint8_t content) {
    writeReg(SENSEL_REG_FRAME_CONTENT_CONTROL, 1, content);
}

// start scanning on Sensel device
void Sensel::startScanning() {
    writeReg(SENSEL_REG_SCAN_ENABLED, 1, 1);
}

// stop scanning on a Sensel device
void Sensel::stopScanning() {
  writeReg(SENSEL_REG_SCAN_ENABLED, 1, 0);
}

// write byte to register on Sensel device
void Sensel::writeReg(uint8_t addr, uint8_t sizeVar, uint8_t data) {
    uint8_t buffer[5] = {
        0x01,
        addr, 
        sizeVar,
        data,
        data
    };
    HAL_UART_Transmit(&huart_, buffer, sizeof(buffer), HAL_MAX_DELAY);
    //pcaudio::write(2, "2\n", 2);
    HAL_UART_Receive(&huart_, rx_buf_, 2, HAL_MAX_DELAY);
    //pcaudio::write(2, "3\n", 2);
    if (rx_buf_[0] != SENSEL_PT_WRITE_ACK) {
#ifdef SenselDebugSerial
        // semihosting needs to be supported for this to work
        pcaudio::write(2, "FAILED TO RECEIVE ACK ON WRITE\n", 31);
#endif
    }
}

//Read byte array from register on Sensel device
void Sensel::readReg(uint8_t addr, uint8_t sizeVar, uint8_t* buf)
{
    uint8_t checksum = 0;
    uint8_t buffer[3] = {
        0x81,
        addr, 
        sizeVar
    };
    HAL_UART_Transmit(&huart_, buffer, sizeof(buffer), HAL_MAX_DELAY);
    HAL_UART_Receive(&huart_, rx_buf_, 4, HAL_MAX_DELAY);
    
    if (rx_buf_[0] != SENSEL_PT_READ_ACK) {
#ifdef SenselDebugSerial
        // semihosting needs to be supported for this to work
        pcaudio::write(2, "FAILED TO RECEIVE ACK ON READ\n", 30);
#endif
        _senselFlush();
        return;
    }
    unsigned int resp_size = _convertBytesToU16(rx_buf_[2], rx_buf_[3]);
    HAL_UART_Receive(&huart_, buf, resp_size, HAL_MAX_DELAY);
    HAL_UART_Receive(&huart_, &checksum, 1, HAL_MAX_DELAY);
}

// read contact frame data from SenselSerial.
void Sensel::getFrame(SenselFrame& frame)
{
  uint8_t buffer[3] = {
      0x81,
      SENSEL_REG_SCAN_READ_FRAME, 
      0x00
  };
  HAL_UART_Transmit(&huart_, buffer, sizeof(buffer), HAL_MAX_DELAY);
  // reader header for frame packet
  HAL_UART_Receive(&huart_, rx_buf_, 6, HAL_MAX_DELAY); 
  if(rx_buf_[0] != SENSEL_PT_RVS_ACK) {
    _senselFlush();
    return;
  }
  
  frame.n_contacts = 0;
  int contact_size = 16;
  unsigned int resp_size = _convertBytesToU16(rx_buf_[3], rx_buf_[4]);

  HAL_UART_Receive(&huart_, &rx_buf_[6], resp_size, HAL_MAX_DELAY); 
  frame.n_contacts = rx_buf_[12];
  if(rx_buf_[5] == SENSEL_REG_CONTACTS_FLAG && (unsigned int)(frame.n_contacts*contact_size) == resp_size-8) {
    for(int i = 0; i < frame.n_contacts; i++) {
      int offset = 13+i*contact_size;
      frame.contacts[i].id = rx_buf_[offset+0];
      frame.contacts[i].type = rx_buf_[offset+1];
      frame.contacts[i].x_pos = _convertBytesToU16(rx_buf_[offset+2],rx_buf_[offset+3])/256.0f;
      frame.contacts[i].y_pos = _convertBytesToU16(rx_buf_[offset+4],rx_buf_[offset+5])/256.0f;
      frame.contacts[i].total_force = _convertBytesToU16(rx_buf_[offset+6],rx_buf_[offset+7])/8.0f;
      frame.contacts[i].area = _convertBytesToU16(rx_buf_[offset+8],rx_buf_[offset+9])/1.0f;
      frame.contacts[i].orientation = _convertBytesToS16(rx_buf_[offset+10],rx_buf_[offset+11])/16.0f;
      frame.contacts[i].major_axis = _convertBytesToU16(rx_buf_[offset+12],rx_buf_[offset+13])/256.0f;
      frame.contacts[i].minor_axis = _convertBytesToU16(rx_buf_[offset+14],rx_buf_[offset+15])/256.0f;
    }
  }
  else{
    _senselFlush();
  }
}

// read contact frame data from SenselSerial.
void Sensel::getSendFrame()
{
  uint8_t buffer[3] = {
      0x81,
      SENSEL_REG_SCAN_READ_FRAME, 
      0x00
  };
  HAL_UART_Transmit(&huart_, buffer, sizeof(buffer), HAL_MAX_DELAY);
  // reader header for frame packet
  HAL_UART_Receive(&huart_, rx_buf_, 6, HAL_MAX_DELAY); 
  if(rx_buf_[0] != SENSEL_PT_RVS_ACK) {
    _senselFlush();
    return;
  }
  
  int contact_size = 16;
  unsigned int resp_size = _convertBytesToU16(rx_buf_[3], rx_buf_[4]);

  HAL_UART_Receive(&huart_, &rx_buf_[6], resp_size, HAL_MAX_DELAY); 
  int n_contacts = rx_buf_[12];
  if(rx_buf_[5] == SENSEL_REG_CONTACTS_FLAG && (unsigned int)(n_contacts*contact_size) == resp_size-8) {
    for(int i = 0; i < n_contacts; i++) {
      // packet header
      u_int8_t buffer[] = "/s";
      CDC_Transmit_FS(buffer, 2);

      int offset = 13+i*contact_size;
      CDC_Transmit_FS(&rx_buf_[offset], contact_size);

      u_int8_t buffer1[] = "\r\n";
      CDC_Transmit_FS(buffer1, 2);

      // frame.contacts[i].id = rx_buf_[offset+0];
      // frame.contacts[i].type = rx_buf_[offset+1];
      // frame.contacts[i].x_pos = _convertBytesToU16(rx_buf_[offset+2],rx_buf_[offset+3])/256.0f;
      // frame.contacts[i].y_pos = _convertBytesToU16(rx_buf_[offset+4],rx_buf_[offset+5])/256.0f;
      // frame.contacts[i].total_force = _convertBytesToU16(rx_buf_[offset+6],rx_buf_[offset+7])/8.0f;
      // frame.contacts[i].area = _convertBytesToU16(rx_buf_[offset+8],rx_buf_[offset+9])/1.0f;
      // frame.contacts[i].orientation = _convertBytesToS16(rx_buf_[offset+10],rx_buf_[offset+11])/16.0f;
      // frame.contacts[i].major_axis = _convertBytesToU16(rx_buf_[offset+12],rx_buf_[offset+13])/256.0f;
      // frame.contacts[i].minor_axis = _convertBytesToU16(rx_buf_[offset+14],rx_buf_[offset+15])/256.0f;
    }
  }
  else{
    _senselFlush();
  }
}

// output frame details to VCO
// intended for debug
// void Sensel::outputFrame(char *buffer, SenselFrame& frame) {
// #ifdef SenselDebugSerial
//   sprintf(buffer, "Num Contacts: %d\r\n\0", frame.n_contacts);
//   CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
//   for(int i = 0; i < frame.n_contacts; i++){
//     sprintf(
//       buffer, 
//       "Contact: %d, %d: x_pos %d  y_pos %d  total_force %d\r\n\0", 
//       frame.contacts[i].id,
//       frame.contacts[i].type, 
//       (int)frame.contacts[i].x_pos,
//       (int)frame.contacts[i].y_pos,
//       (int)frame.contacts[i].total_force);
//     CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
//   }
// #endif // SenselDebugSerial 
// }

void Sensel::outputFrame(char *buffer, SenselFrame& frame) {
  //CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
  for(int i = 0; i < frame.n_contacts; i++){
    sprintf(
      buffer, 
      "/s %d %d %d %d %d\n\0", 
      frame.contacts[i].id,
      frame.contacts[i].type, 
      (int)frame.contacts[i].x_pos,
      (int)frame.contacts[i].y_pos,
      (int)frame.contacts[i].total_force);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
  }

}

