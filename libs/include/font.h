/*
 * font.h
 *
 *  Created on: 3 May 2019
 *      Author: frazer
 */

#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;


extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#ifdef __cplusplus
}
#endif

#endif /* INC_FONT_H_ */
