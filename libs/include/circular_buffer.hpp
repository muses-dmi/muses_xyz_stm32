/*
 * Name: circular_buffer.hpp
 * Author: Benedict R. Gaster
 * Date: 9th Oct 2019
 * Desc:
 *       
 *
 */
#pragma once

#include <cstddef>
#include <utility>

template <typename T, std::size_t mcapacity>
class circular_buffer
{
private:
    // data buffer
    T* data_;
    // current write index
    int write_index_;
    // current read index
    int read_index_;
    // current size of buffer
    std::size_t msize_;

public:
    using container = circular_buffer<T, mcapacity>;
    using value_type = T;
    using reference = value_type&;
    using pointer = T*;
    using const_reference = const reference;
    using size_type = std::size_t;

    // create buffer
    circular_buffer() : 
        data_((pointer)::operator new(sizeof(value_type) * mcapacity)),
        write_index_(0),
        read_index_(0),
        msize_(0) {
    }

    // push value on to buffer, does nothing if full
    void push(T value) {
        //if (!full()) {
        data_[write_index_] = value;
        write_index_ = (write_index_+1) % mcapacity;
        msize_++;
        //}
    }

    // push value on to buffer, does nothing if full
    // void push(const T& value) {
    //     if (!full()) {
    //         data_[write_index_] = value;
    //         write_index_ = (write_index_+1) % mcapacity;
    //         msize_++;
    //     }
    // }

    // push value on to buffer
    // does nothing is full
    void push(T&& value) {
        if (!full()) {
            data_[write_index_] = value;
            write_index_ = (write_index_+1) % mcapacity;
            msize_++;
        }
    }

    // pop value from buffer
    // invariant: must not be empty
    T pop() {
        T tmp = data_[read_index_];
        read_index_ = (read_index_+1) % mcapacity;
        msize_--;
        return tmp;
    }

    // buffer's current size...
    size_type size() {
        return msize_;
    }

    // buffer's capacity
    constexpr size_type capacity() {
        return mcapacity;
    }

    // is buffer full
    bool full() const {
        return msize_ == mcapacity;
    }

    // is buffer empty
    bool empty() const {
        return msize_ == 0;
    }

    // destroy buffer
    ~circular_buffer() {
        delete data_;
    }
};