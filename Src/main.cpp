/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <string.h>
#include <stdlib.h>

//#if defined(__SEMIHOSTING__)
#include "semihost.hpp"
//#endif

//#define __SEMIHOSTING__ 1

#include <sensel.hpp>
#include <circular_buffer.hpp>

#include <ssd1306.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

const uint16_t ENCODER_4_A                    = GPIO_PIN_1;
const uint16_t ENCODER_4_B                    = GPIO_PIN_0;
GPIO_TypeDef* ENCODER_4_BANK            = GPIOA;
GPIO_TypeDef* ENCODER_4_BUTTON_BANK     = GPIOC;
const uint16_t ENCODER_4_BUTTON               = GPIO_PIN_2;

const uint16_t ENCODER_3_A                    = GPIO_PIN_3;
const uint16_t ENCODER_3_B                    = GPIO_PIN_2;
GPIO_TypeDef* ENCODER_3_BUTTON_BANK = GPIOA;
const uint ENCODER_3_BUTTON               = GPIO_PIN_4;

const uint16_t ENCODER_2_A                    = GPIO_PIN_5;
const uint16_t ENCODER_2_B                    = GPIO_PIN_4;
GPIO_TypeDef* ENCODER_2_BUTTON_BANK = GPIOB;
const uint ENCODER_2_BUTTON               = GPIO_PIN_10;

const uint16_t ENCODER_1_A                    = GPIO_PIN_12;
const uint16_t ENCODER_1_B                    = GPIO_PIN_11;
GPIO_TypeDef* ENCODER_1_BUTTON_BANK = GPIOC;
const uint16_t ENCODER_1_BUTTON               = GPIO_PIN_6;

volatile uint8_t idx;
volatile int32_t pos_count;

// OLED controls
bool oled_values_changed = true; 
uint32_t ctrl1 = 0;
uint32_t ctrl2 = 0;
uint32_t ctrl3 = 0;
uint32_t ctrl4 = 0;


//int8_t add_subt[16] = { 0, 1 , -1 , 0, -1 , 0 , 1, -1 , 0 , 0, -1, 0 , -1, 1, 0 };

int8_t add_subt[20] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0,  1,  0,  1, -1, 0, 0, 1, 0};
//                     0  1   2  3  4  5  6  7   8   9  10  11  12  13  14 15 16 17

int8_t indexs[128] = {0};
int8_t pins[128]  = {0};
int8_t as[128] = {0};
int8_t bs[128] = {0};
int32_t pcounts[128] = {0};

int32_t counter = 0;

SenselFrame frame;


const uint32_t OLED_LINE_LENGTH = 21;
char blankLine[OLED_LINE_LENGTH] = "                  ";
char oled_line[OLED_LINE_LENGTH] = "";
uint32_t oled_current_offset = 0;

// allocate buffer used for incomming vcom messages, used also for 
circular_buffer<char, 1024> cbuffer;
void * cbufferC = (void*)&cbuffer;

// char buffer3[100];
// char buffer4[100];
// char buffer5[100];


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static constexpr int ENCODER_MSG_P1_LENGTH  = 7;
static constexpr int ENCODER_MSG_M1_LENGTH  = 8;

struct Encoder {
  
  // bank A
  GPIO_TypeDef* bankA_;
  // encoder pin A
  uint16_t pinA_;
  // bank B
  GPIO_TypeDef* bankB_;
  // encoder pin B
  uint16_t pinB_;
  // current index into action array
  uint8_t idx_;
  int32_t position_;
  // has state of encoder changed
  bool changed_;
  // OSC message to be sent when encoder state changes
  char msgP1_[ENCODER_MSG_P1_LENGTH+1];
  char msgM1_[ENCODER_MSG_M1_LENGTH+1];

  // return the current movement 0, +1, -1 
  int32_t position() {
    return add_subt[idx_];
  }
};

// to avoid any conditionals in the interreupt handler we use an array to map GPIO pins (A/B) to an 
// encoder index 
constexpr uint32_t ENCODER_1 = 0;
constexpr uint32_t ENCODER_2 = 1;
constexpr uint32_t ENCODER_3 = 2;
constexpr uint32_t ENCODER_4 = 3;
static constexpr uint32_t indexes[] = {
  0,  ENCODER_4, ENCODER_4, 
      ENCODER_3, ENCODER_3, 
      ENCODER_2, ENCODER_2, 
  0, 0, 0, 0, 0, 0, 
      ENCODER_1, ENCODER_1 };

constexpr uint16_t ENCODER_1_PIN_A = GPIO_PIN_12;
constexpr uint16_t ENCODER_1_PIN_B = GPIO_PIN_11;

constexpr uint16_t ENCODER_2_PIN_A = GPIO_PIN_5;
constexpr uint16_t ENCODER_2_PIN_B = GPIO_PIN_4;

constexpr uint16_t ENCODER_3_PIN_A = GPIO_PIN_3;
constexpr uint16_t ENCODER_3_PIN_B = GPIO_PIN_2;

constexpr uint16_t ENCODER_4_PIN_A = GPIO_PIN_1;
constexpr uint16_t ENCODER_4_PIN_B = GPIO_PIN_0;


Encoder encoders[] = {
    // not working
    { GPIOB, ENCODER_1_PIN_A, GPIOB, ENCODER_1_PIN_B, 0, 0, false, "/e/1", "/e/1 -1\n" },
    { GPIOC, ENCODER_2_PIN_A, GPIOC, ENCODER_2_PIN_B, 0, 0, false, "/e/2", "/e/2 -1\n" },
    // working
    { GPIOA, ENCODER_3_PIN_A, GPIOA, ENCODER_3_PIN_B, 0, 0, false, "/e/3", "/e/3 -1\n" },
    { GPIOA, ENCODER_4_PIN_A, GPIOA, ENCODER_4_PIN_B, 0, 0, false, "/e/4", "/e/4 -1\n" },
};

static constexpr int NUMBER_ENCODERS = sizeof(encoders)/sizeof(*encoders);

struct Button {
  // bank
  GPIO_TypeDef* bank_;
  // button pin
  uint16_t pin_;
  // current state of button
  uint8_t state_;
  // button pressed
  uint8_t pressed_;
  // OSC message when button pressed
  char msgOn_[15];
  uint32_t msgOnLen_;
  // OSC message when button released
  char msgOff_[15];
  uint32_t msgOffLen_;
};

Button buttons[] = {
  // big buttons
  { GPIOC, GPIO_PIN_7, 1, 0, "/b/0 127\n", 9, "/b/0 0\n", 7 }, 
  { GPIOC, GPIO_PIN_8, 1, 0, "/b/1 127\n", 9, "/b/1 0\n", 7 }, 
  { GPIOC, GPIO_PIN_9, 1, 0, "/b/2 127\n", 9, "/b/2 0\n", 7}, 
  { GPIOA, GPIO_PIN_8, 1, 0, "/b/3 127\n", 9, "/b/3 0\n", 7 }, 
  // encoder buttons, which are used to select ORAC module/page
  { GPIOC, GPIO_PIN_6,  1, 0, "/ModulePrev 1\n", 14, NULL, 0 }, 
  { GPIOB, GPIO_PIN_10, 1, 0, "/ModuleNext 1\n", 14, NULL, 0 }, 
  { GPIOA, GPIO_PIN_4,  1, 0, "/PagePrev 1\n",   12, NULL, 0 }, 
  { GPIOC, GPIO_PIN_2,  1, 0, "/PageNext 1\n",   12, NULL, 0 }, 
};

static constexpr int NUMBER_BUTTONS = sizeof(buttons)/sizeof(*buttons);
static constexpr int BUTTON_MSG_ON_LENGTH  = 9;
static constexpr int BUTTON_MSG_OFF_LENGTH = 7;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

Sensel sensel(huart1);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
extern "C" void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t line_y_offset(char key) {
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
      return 1;
    }
    case '6': {
      return 2;
    }
    case '7': {
      return 3;
    }
    case '8': {
      return 4;
    }
  }

  return 0;
}

void handleButtons() {
  for (uint32_t i = 0; i < NUMBER_BUTTONS; i++) {
    uint8_t reading = HAL_GPIO_ReadPin(buttons[i].bank_, buttons[i].pin_);
    if (buttons[i].state_ != reading) {
      buttons[i].state_ = reading;

      // is button pressed
      if (buttons[i].state_ == GPIO_PIN_SET) {
            CDC_Transmit_FS((uint8_t*)buttons[i].msgOn_, buttons[i].msgOnLen_);  
      }
      else if (buttons[i].msgOff_ != NULL) { // or released and off message not NULL
            CDC_Transmit_FS((uint8_t*)buttons[i].msgOff_, buttons[i].msgOffLen_);
      }
    }
  }
}

void handleEncoders() {
  static char buffer[512];
  
  for (uint32_t i = 0; i < NUMBER_ENCODERS; i++) {
    Encoder& encoder = encoders[i];
    if (encoder.changed_) {
      //int32_t pos = encoder.position(); 
      buffer[0] = encoder.msgP1_[0];
      buffer[1] = encoder.msgP1_[1];
      buffer[2] = encoder.msgP1_[2];
      buffer[3] = encoder.msgP1_[3];
      buffer[4] = ' ';
      itoa(encoder.position_, &buffer[5], 10);
      int len = strlen(buffer);
      buffer[len++] = '\n';
      CDC_Transmit_FS((uint8_t*)buffer, len);
      encoder.changed_ = false;
    }
  }
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin) {
  uint32_t index = 0;

  if (gpio_pin == ENCODER_1_PIN_A || gpio_pin == ENCODER_1_PIN_B) {
    index = 0;
  }
  else if (gpio_pin == ENCODER_2_PIN_A || gpio_pin == ENCODER_2_PIN_B) {
    index = 1;
  }
  else if (gpio_pin == ENCODER_3_PIN_A || gpio_pin == ENCODER_3_PIN_B) {
    index = 2;
  }
  else if (gpio_pin == ENCODER_4_PIN_A || gpio_pin == ENCODER_4_PIN_B) {
    index = 3;
  }
  else {
    counter = gpio_pin;
    return ;
  }

  Encoder& encoder = encoders[index];
  encoder.idx_ = ((encoder.idx_<< 2) & 0xF) | 
          (HAL_GPIO_ReadPin(encoder.bankA_, encoder.pinA_)<< 1) | 
          HAL_GPIO_ReadPin(encoder.bankB_, encoder.pinB_);
  encoder.position_ = /*encoder.position_ +*/ encoder.position();
  encoder.changed_ = true;

  // indexs[counter] = encoder.idx_;
  // pins[counter]   = gpio_pin;
  // // as[counter] = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A);
  // // bs[counter] = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B);
  // pcounts[counter] = pos_count;
  counter = gpio_pin;

  //uint8_t index =  (((gpio_pin == ENCODER_4_B)<<3) | (HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A) << 2));
                  //| (((gpio_pin == ENCODER_4_A)<<1) | HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B));
  //pos_count = index; // pos_count + add_subt[index];

  // if (gpio_pin == ENCODER_4_B && HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A)) {
  //   pos_count = pos_count - 1;
  // }
  // else if (gpio_pin == ENCODER_4_A && HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B)) {
  //   pos_count = pos_count + 1;
  // }

//gpio_pin == ENCODER_4_A ||
  // if (  gpio_pin == ENCODER_4_B) {
  //   //goo();
  //   //update();
  //   idx = ((idx<< 2) & 0x0F) + 
  //         HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A)<< 1 + 
  //         HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B);

  //   pos_count = pos_count + add_subt[idx & 0x0F];
  //   // indexs[counter] = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A); //idx;

  //   // pins[counter] = gpio_pin;
  //   // as[counter] = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A);
  //   // bs[counter] = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B);
  //   // counter = counter + 1;
  //   // pos_count = idx; //(pos_count << 2 & 0x0F) + HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A) << 1; //pos_count + 1;
  // }
}

/* USER CODE END 0 */
bool do_once = true;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
 

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  ssd1306_init_display();
  ssd1306_clear_display();
  
  //sprintf(buffer, "###IRONLINK###");
  //char str[] = "   Muses xyz";
  ssd1306_set_cursor(0,0);
  ssd1306_write_string("   Module", Font_7x10, 1);
  ssd1306_set_cursor(0,10);
  ssd1306_write_string("   Ctrl 1", Font_7x10, 1);
  ssd1306_set_cursor(0,20);
  ssd1306_write_string("   Ctrl 2", Font_7x10, 1);
  ssd1306_set_cursor(0,30);
  ssd1306_write_string("   Ctrl 3", Font_7x10, 1);
  ssd1306_set_cursor(0,40);
  ssd1306_write_string("   Ctrl 4", Font_7x10, 1);
  ssd1306_set_cursor(0,50);
  ssd1306_write_string("   Page", Font_7x10, 1);
  ssd1306_update_display();

//#if defined(__SEMIHOSTING__)
  //pcaudio::write(2, "Hello\n", 6);
//#endif

  //HAL_Delay(3000);

  //Set frame content to scan. No pressure or label support.
  //sensel.setFrameContent(SENSEL_REG_CONTACTS_FLAG);
  //Start scanning the Sensel device
  //sensel.startScanning();
  
  /* USER CODE END 2 */

  // setup encoder 4
  idx = HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A) << 3 + 
        HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B) << 2 + 
        HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_A) << 1 + 
        HAL_GPIO_ReadPin(ENCODER_4_BANK, ENCODER_4_B);

#if defined(__SEMIHOSTING__)
  sprintf(buffer, "%d\n", idx);
  pcaudio::write(2, buffer, strlen(buffer));
#endif

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET );

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET );

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET );

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET );


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);

    // Button 4
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET); //works
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET ); //works green
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3); // works blue
    HAL_Delay(100);

    // Button 3
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET ); //works red
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET ); // works green
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // works blue

    // //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

    // Button 2
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET ); // works red
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET ); // works green
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // blue works
    
    // //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
    // //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,  GPIO_PIN_SET); // working red
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,  GPIO_PIN_SET); // working green
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // works blue
    
    //HAL_Delay(10);
    //if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET) {
      // itoa(pos_count, buffer4, 10);
      // int len = strlen(buffer4);
      // buffer4[len++] = '\r';
      // buffer4[len++] = '\n';
      // buffer4[len++] = '\0';
      // CDC_Transmit_FS((uint8_t*)buffer4, len);
    //}

    // handle buttons
    handleButtons();
    handleEncoders();

    // handle OLED
    
    // if (oled_values_changed) {

    //   oled_values_changed = false;
    //   tmpBuffer[0] = 'C';
    //   tmpBuffer[1] = '1';
    //   tmpBuffer[2] = ' ';
    //   itoa(ctrl1, &tmpBuffer[3], 10);
    //   ssd1306_set_cursor(0,11);
    //   ssd1306_write_string(tmpBuffer, Font_7x10, 1);
    //   ssd1306_update_display();

    //   // we probably need to lock here...
    //   // "num num \n"
    //   // if (cdcRxBuffer[0] == '1') {
    //   // }
    //   // cdcRxBufferLength = 0;
    // }


    // if (cdcRxBufferLength != 0) {
    //   // message /module
    //   if (cdcRxBuffer[0] == 'm') {
    //      int i = 1;
    //      for (; cdcRxBuffer[i] != '\n'; i++) { }
    //      ssd1306_set_cursor(0,0);
    //      ssd1306_write_string(blankLine, Font_7x10, 1);
    //      cdcRxBuffer[i] = '\0';
    //      ssd1306_set_cursor(0,0);
    //      ssd1306_write_string(&cdcRxBuffer[1], Font_7x10, 1);
    //      ssd1306_update_display();
    //   }
    //   else if (cdcRxBuffer[0] == 'p') {
    //      int i = 1;
    //      for (; cdcRxBuffer[i] != '\n'; i++) { }
    //      ssd1306_set_cursor(0,50);
    //      cdcRxBuffer[i] = '\0';
    //      ssd1306_write_string(&cdcRxBuffer[1], Font_7x10, 1);
    //      ssd1306_update_display();
    //   }
    //   cdcRxBufferLength = 0;
    // }

    while (!cbuffer.empty()) {
      char c = cbuffer.pop();
      // update OLED line, if at end of update message
      if (c == '\n') {
        uint32_t y = line_y_offset(oled_line[0]) * 10;
        ssd1306_set_cursor(0,y);
        ssd1306_write_string(blankLine, Font_7x10, 1);
        oled_line[oled_current_offset] = '\0';
        ssd1306_set_cursor(0,y);
        ssd1306_write_string(&oled_line[1], Font_7x10, 1);
        ssd1306_update_display();
        // reset ready for next message
        oled_current_offset = 0;
      }
      // otherwise, just store the next char of current message
      else { 
        oled_line[oled_current_offset++] = c;
      }
    }

    // ---------------------------------------------------------
    // big buttons
    //----------------------------------------------------------
    // if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/1 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/2 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/3 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/4 1\n", 7);
      
    // }

    // // ---------------------------------------------------------
    // // handle encoders...
    // //----------------------------------------------------------
    // // buttons
    // if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/5 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/6 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/7 1\n", 7);
    // }

    // if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET) {
    //   CDC_Transmit_FS((uint8_t*)"/b/8 1\n", 7);
    // }

      //pcaudio::write(2, "3\n", 2);
      //Read the frame of contacts from the Sensel device
      //sensel.senselStartScanning();
      //if (do_once) {
      //sensel.getSendFrame();
      
      // sensel.getFrame(frame);
      // if (frame.n_contacts > 0) {
      //   sensel.outputFrame(buffer, frame);
      // }
      
      //HAL_Delay(10);
      //sensel.senselStopScanning();
        

        //do_once = false;
      //}
    //}

#if defined(__SEMIHOSTING__)
    
    //if (pos_count > 20) {
    // sprintf(buffer, "%d\n", pos_count);
    // sprintf(buffer2, "%d\n", counter);
    // pcaudio::write(2, buffer2, strlen(buffer2));
    //}

    //itoa (idx,buffer,2);
    // if (counter > 10) {
    //   sprintf(buffer, "%d\n", pos_count);
    //   pcaudio::write(2, buffer, strlen(buffer));

    //   for (int i = 0; i < counter; i++) {
    //     itoa(indexs[i], buffer, 2);
    //     //itoa(pins[i], buffer3, 2);
    //     itoa(as[i], buffer4, 2);
    //     itoa(bs[i], buffer5, 2);

    //     sprintf(buffer2, "%d\t\t%d %s %s %d\n", indexs[i], pins[i], buffer4, buffer5, pcounts[i]);
    //     //sprintf(buffer2, "%d \n", pins[i], buffer4, buffer5, pos_count);
    //     pcaudio::write(2, buffer2, strlen(buffer2));
    //   }
    //   //pos_count = 0;
    //   counter = 0;
    // }
#endif

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
extern "C" void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI0_1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
  /* EXTI2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
  /* EXTI4_15_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC2 PC7 PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  // GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // GPIO_MODE_IT_RISING;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; //GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB13 
                           PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB11 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  // HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  // HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  // HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
extern "C" void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
