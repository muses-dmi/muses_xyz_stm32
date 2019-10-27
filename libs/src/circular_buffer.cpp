
#include <circular_buffer.hpp>

extern "C" char pop(void * cbuffer) {
    return static_cast<circular_buffer<char, 1024>*>(cbuffer)->pop();
}

extern "C" void push(void * cbuffer, char value) {
    static_cast<circular_buffer<char, 1024>*>(cbuffer)->push(value);
}