//===================================================================
// File: circular_buffer.cpp
//
// Desc: A Circular Buffer implementation in C++.
// from https://medium.com/@edwin.cloud/wrapping-your-head-around-circular-buffers-e84cbf549a04
//
// Copyright © 2019 Edwin Cloud. All rights reserved.
//
//===================================================================

#include <memory>
#include <queue>
#include <deque>
#include <iostream>
//#include "local.hpp"

/*

// from https://stackoverflow.com/questions/56334492/c-create-fixed-size-queue
// https://stackoverflow.com/questions/5877504/access-c-queue-elements-like-an-array
template <typename T, int maxLen>
class fixedQueue : public std::deque<T> {

public:
    fixedQueue<T, maxLen>() {
        this->resize(maxLen);
        this->clear();
    }
    void push(const T& value) {
        if (this->size() == maxLen) {
           //std::cout << this << " : reached maxlen = " << maxLen << " so clearing...\n";
           this->pop_front();
        }
        std::deque<T>::push_back(value);
    }
    void pop(void) {
        return std::deque<T>::pop_front();
    }
};



//-------------------------------------------------------------------
// Circular_Buffer (Class)
//     We will implement the buffer with a templated class so
//     the buffer can be a buffer of specified type.
//-------------------------------------------------------------------
const int max_size = 128;
template <class T> class circularBuffer {
private:
  //---------------------------------------------------------------
  // Circular_Buffer - Private Member Variables
  //---------------------------------------------------------------

  int _head = 0;
  int _tail = 0;
  int _size = 0;
  T buffer[max_size];
  T empty_item; // we will use this to clear data
public:
  //---------------------------------------------------------------
  // Circular_Buffer - Public Methods
  //---------------------------------------------------------------
  
  circularBuffer<T>() {
  }

  void push(T item) {
    if (is_full())
        pop();
    
    if (_head == max_size)
        _head = 0;
    buffer[_head++] = item;

    _size++;
  }

  T pop() {
    if (is_empty())
        return empty_item;

    _size--;
    if (_tail == max_size)
        _tail = 0;
    T item = buffer[_tail];
    buffer[_tail] = empty_item;
    _tail++;
    return item;
  }

  T front() { return buffer[_tail]; }

  bool is_empty() { return _size == 0;}
  bool is_full()  { return _size == max_size; }

  // Return the size of this circular buffer.
  int size() {
      //uprintf(" s=%d h=%d t=%d \n", _size, _head, _tail);
      return _size;
  }

};

*/
