/*
 * Name: semihost.cpp
 * Author: Benedict R. Gaster
 * Date: 6th June 2018
 * Desc: Implementation for semihost.hpp interface.
 *
 */

#include <cstring>

#include <semihost.hpp>
#include <utils.hpp>

namespace pcaudio {

namespace {
    /**
     * @brief preform semihost system call
     * @param  op   code for system call
     * @param  args pointer to buffer containing arguments for system call
     * @return      value returned by system call
     */
    int semihostingOp(unsigned int op, const void *args) {
        register uint32_t op_ asm ("r0") = op;
        register uint32_t args_ asm ("r1") = (uintptr_t)args;
        register int ret asm ("r0");

        __asm__ volatile ("bkpt 0xab" : "=r" (ret) : "0" (op_), "r" (args_));
        return (ret);
    }
}

//-----------------------------------------------------------------------------
//-- file routines
//-----------------------------------------------------------------------------

Handle open(const char *pathname, const char* mode) {
    uint32_t m;

    switch(str2int(mode)) {
    case str2int("r"):
        m = 0;
        break;
    case str2int("rb"):
        m = 1;
        break;
    case str2int("r+b"):
        m = 3;
        break;
    case str2int("w"):
        m = 4;
        break;
    case str2int("wb"):
        m = 5;
        break;
    case str2int("w+"):
        m = 6;
        break;
    case str2int("w+b"):
        m = 7;
        break;
    case str2int("a"):
        m = 8;
        break;
    case str2int("ab"):
        m = 9;
        break;
    case str2int("a+"):
        m = 10;
        break;
    case str2int("a+b"):
        m = 11;
        break;
    default:
        return -1;
    }

    struct args {
            const void *buf;
            uint32_t mode;
            size_t len;
    } args = {pathname, m, strlen(pathname)};

    return semihostingOp(
        static_cast<uint32_t>(SemihostOp::SYS_OPEN), &args);
}

uint32_t close(Handle fd) {
    struct args {
            uint32_t handle;
    } args = {fd};

    return semihostingOp(
        static_cast<uint32_t>(SemihostOp::SYS_CLOSE), &args);
}

//----------------------------------------------------------------------------
// -- READ/WRITE routines
//----------------------------------------------------------------------------

size_t write(Handle fd, const uint8_t* buffer, size_t count) {
    struct args {
            uint32_t handle;
            const void *buf;
            size_t len;
    } args = {fd, buffer, count};

    return semihostingOp(
        static_cast<uint32_t>(SemihostOp::SYS_WRITE), &args);
}

} // namespace pcaudio
