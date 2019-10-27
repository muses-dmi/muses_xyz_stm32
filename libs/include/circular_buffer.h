/*
 * Name: circular_buffer.h
 * Author: Benedict R. Gaster
 * Date: 9th Oct 2019
 * Desc:
 *       
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char pop(void * cbuffer);
void push(void * cbuffer, char value);

#ifdef __cplusplus
}
#endif
