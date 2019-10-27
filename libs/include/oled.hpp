/*
 * Name: oled.cpp
 * Author: Benedict R. Gaster
 * Date: 27th Oct 2019
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

#include <ssd1306.h>    