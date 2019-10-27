/*
 * Name: semihost.hpp
 * Author: Benedict R. Gaster
 * Date: 6th June 2018
 * Desc:
 *       Interface to ARM's semihost IO capabilities via the host/debugger.
 *
 *    TODO: Add missing SYS_XXX calls
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

namespace pcaudio {

enum class SemihostOp {
    SYS_OPEN  = 0x1,
    SYS_CLOSE = 0x2,
    SYS_WRITE = 0x5,
    SYS_READ  = 0x6
};

//-----------------------------------------------------------------------------
// file routines
//-----------------------------------------------------------------------------

typedef uint32_t Handle;

/**
 * @brief  Opens a file on the host system.
 * @param  pathname pointer to null terminated string containing a file or
 *         device name
 * @param  mode     descriptor specifing the file opening mode, follows fopen
 * @return          -1 if the call is not successful, otherwise a nonzero handle.
 */
Handle open(const char *pathname, const char* mode);

/**
 * @brief close a file on the host system.
 * @param  fd contains a handle for an open file
 * @return  0 if successful, otherwise -1
 */
uint32_t close(Handle fd);

/**
 * @brief writes the contents of buffer to a specified file handle at the
 *        current position
 * @param  fd     handle to a file previosuly opened with open or 1 for stdout
 *         and 2 for stderr
 * @param  buffer points to memory containing data to be written
 * @param  count  number of bytes to be written to file
 * @return        0 if successful, otherwise number of bytes not written
 */
size_t write(Handle fd, const uint8_t* buffer, size_t count);

/**
 * @brief writes the contents of buffer to a specified file handle at the
 *        current position
 * @param  fd     handle to a file previosuly opened with open or 1 for stdout
 *         and 2 for stderr
 * @param  buffer points to memory containing data to be written
 * @param  count  number of bytes to be written to file
 * @return        0 if successful, otherwise number of bytes not written
 */
__inline__
size_t write(Handle fd, const char* buffer, size_t count) {
    return write(fd, reinterpret_cast<const uint8_t*>(buffer), count);
}

} // pcaudio
