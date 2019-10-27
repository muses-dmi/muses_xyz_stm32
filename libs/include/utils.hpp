/*
 * Name: utils.hpp
 * Author: Benedict R. Gaster
 * Date: 6th June 2018
 * Desc: Some handy utilities that don't fit anywhere else.
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
   // x86 for debug
#endif

#include <cstdlib>
#include <cstring>

namespace pcaudio {

    constexpr unsigned int str2int(const char* str, int h = 0){
        return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
    }

    // the following provide typed vesions of C's malloc, free, realloc, ...

    /**
     * [malloc description]
     * @param  bytes [description]
     * @return       [description]
     */
    template<class T>
    T * malloc(size_t bytes) {
        return static_cast<T*>(std::malloc(bytes));
    }

    /**
     * [free  description]
     * @param ptr [description]
     */
    template<class T>
    void free (T* ptr) {
        std::free(static_cast<void *>(ptr));
    }

    /**
     * [realloc description]
     * @param  ptr  [description]
     * @param  size [description]
     * @return      [description]
     */
    template<class T>
    T * realloc(T* ptr, size_t size) {
        static_cast<T*>(std::realloc(static_cast<void*>(ptr), size));
    }

    /**
     * [memcpy description]
     * @param  dest [description]
     * @param  src  [description]
     * @param  num  [description]
     * @return      [description]
     */
    template<class T>
    T * memcpy(T* dest, T* src, size_t num) {
        return static_cast<T*>(
            std::memcpy(
                static_cast<void*>(dest),
                static_cast<void*>(src),
                num* sizeof(T)));
    }
} // namespace pcaudio
