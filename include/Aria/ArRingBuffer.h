/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/


#ifndef _AR_BUFFER_H_
#define _AR_BUFFER_H_

#include <iostream>
#include <array>
#include "Aria/ariaTypedefs.h"

/** @brief A very simple ring buffer in which all memory for data is stored in a fixed size contiguous array (std::array). A maximum size (capacity) is maintained, and 
 *  memory is reused as items are removed (popped) from the front and added (pushed)
 *  to the back. If the buffer is full (at capacity), then old items are replaced by new items (via the assignment operator).   If you want a fixed capacity container, this can have better performance than std::list or std::vector, with which items are removed and inserted (causing memory fragmentation and heap allocation in the case of std::list and re-allocation, copying and moving of objects in the case of std::vector).
 * 
 *  Items may be added to the back with push(), and
 *  the oldest item still available can be accessed from the front with pop() or front().  (But most recent item cannot be accessed from the back.)
 *
 *  The buffer starts with a fixed capacity (std::array is used internally), but those initial items are considered 'unused'.  Items
 *  are "pushed" into the queue at the "back" with push(), and "popped" from the queue at
 *  the "front". and pop() (and advance_front()) will move the front of the queue to 
 *  the next item, creating a new 'unused slot' for future use; advance_back()
 *  changes the next item in the back to the next item, which may or may not already be used.
 *  it can then be replaced via the back() iterator.
 * 
 *  If an old item is replaced by a new item by push(), then T's copy or move assignment operator is used to replace the item.   If replaced by emplace(), then the copy or move constructor is used.
 *
 *  Note: currently if reset() is called or items are popped(), old items are not 
 *  destroyed (destructors not called).  Destructors may be called if the item is later replaced by a new item. 
 *
 *  @todo Ideally, this class would be fully threadsafe/non-locking (or with occasional
 *  mutex locking in certain cases), but it is not currently. Should be updated to use
 *  atomic support in modern C++ standard library if possible.
 *
 *  @ingroup UtilityClasses
 */
template<class T, size_t Capacity> 
class ArRingBuffer {
public:

  using array_type = std::array<T, Capacity>;

  /** @param capacity Initial capacity of the ring queue. 
   *  @param init_value Initial value for new, unused items in the queue. 
   *  */
  ArRingBuffer() : curSize(0), front_it(arr.end()), back_it(arr.begin())
    // note front_it at end indicates empty state
  {
  }


  /** Get an iterator for the front item of the ring queue (the item that would
   * be returned by pop()). If the queue is currently empty, nil() will be
   * returned.
   * 
   * To remove an item from the queue without
   * making a copy with pop(), first check if the queue is empty(). Then  use this 
   * function to get the data. Then call advance_front(). 
   */
  typename array_type::iterator front() {
    if(empty())
      return nil();
    return front_it;
  }

  /** Get an iterator for the back of the queue (the item that would be
   * replaced by push()). This is not the last item in the queue, rather it is the
   * next, unused, "slot".  If the queue is full, an iterator equivalent to that
   * returned by nil() is returned.
   *
   * To add an item to the queue without pushing
   * a copy with push(), first check if the queue is full (in which case you
   * must push() your item). Then use this function to write the data into the
   * next unused 'slot'. Then call advance_back() to advance the back of the queue
   * to your new item. 
   */
  typename array_type::iterator back() {
    if(front_it == back_it)
    {
      //std::cerr << "ArRingBuffer: back(): 0-capacity or full, returning nil.\n";
      return nil();
    }
    return back_it;
  }

  /** Advance the front of the queue. 'Used' size will be decremented.  */
  void advance_front() {
    if(front_it == arr.end())  // initial or  empty state.
      front_it = arr.begin();
    else if(++front_it == arr.end()) 
      front_it = arr.begin();
    if(front_it == back_it) { // it's now empty (not full)
      front_it = arr.end();
      back_it = arr.begin();
    }
    curSize--;
  }

  /** Same as advance_front() */
  void pop_front() { advance_front(); };

  /** Advance the back (an 'empty' push), if the queue is not full.  'Used' size will be incremented.  */
  void advance_back() {
    if(front_it == back_it) // full or 0-capacity TODO should we remove this check and just advance the front and discard it
    {
      // debugging:
      /*
      if(empty()) {
        std::cerr << "ArRingBuffer: advance_back(): queue is *empty*, can't advance back.\n";
        return;
      }
      std::cerr << "ArRingBuffer: advance_back(): queue is full, can't advance back.\n";
      */
      return;
    }
    if(++back_it == arr.end())
      back_it = arr.begin();
    if(front_it == arr.end())
      front_it = arr.begin();  // no longer empty.
    curSize++;
  }

  /** Push a new item.  If the buffer is full, then the oldest item is replaced with the new item.
   */
  void push(const T& item) {
    if(full())
      advance_front(); // throw away the item at the front, when we advance_back(), then back will be pointing to it.
    *back_it = item;
    advance_back();
  }

  /** Take rvalue reference, so item could be moved into buffer. */
  void push(T&& item) {
    if(full())
      advance_front();
    *back_it = item;
    advance_back();
  }

  /** Print the current contents of the queue. 
   *  @pynote use printQueue() instead of print() (which is a reserved word in Python)
  */
  void print() const {
    for(typename array_type::const_iterator i = arr.begin(); i != arr.end(); i++) {
      if(i == back_it)
        std::cerr << "]";
      if(i != arr.begin())
        std::cerr << ",";
      if(i == front_it || (i == arr.begin() && front_it == arr.end()) )
        std::cerr << "[";
      std::cerr << (*i); // << "," ;
    }
    std::cerr << std::endl;
  }

  /** Get the number of items currently 'used' in the queue. */
  size_t size() const {
    return curSize;
  }

  /** Get the current capacity of the queue. */
  size_t capacity() const {
    return arr.size();
  }

  /** Return true if the queue is empty (has no 'used' items), false otherwise.  */
  bool empty() const {
    return (front_it == arr.end());
  }

  /** Logically clear the queue, resetting to initial empty state
   * The contents are not
   * destroyed, but will be when replaced by new data later. */
  void reset() {
    front_it = arr.end();
    back_it = arr.begin();
    curSize = 0;
  }

  /** Return true if the queue is full, false otherwise. */
  bool full() const {
    return (back_it == front_it);
  }

  /** Return an iterator representing an invalid item. Compare to the return
   * values of front(), back(), pop(), etc. */
  typename array_type::iterator nil() {
    return arr.end();
  }

private:
  size_t curSize;
  typename array_type::iterator front_it, back_it;   
  // push to back, pop from front; front will point to first item, 
  // back to one past last. 

  array_type arr;
};



#endif
