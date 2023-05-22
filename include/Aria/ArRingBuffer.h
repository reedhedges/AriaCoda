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
     but WITHOUT ANY WARRANTY; without even the implied wcontaineranty of
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

/** @brief Adapts a container type (see requirements below) into a very simple ring buffer.

    A maximum size (capacity) is maintained, and 
    memory can be reused as items are removed (popped) from the front and added (pushed)
    to the back. If the buffer is full (at capacity), then old items are replaced by new items (via the assignment operator).   

    Specify a standard container type as the ContainerT parameter such as std::array, std::vector, or any other type providing standard iterator interface as the underlying storage.
    For example, if std::array is used as ContainerT, then all memory is stored in a fixed size condiguous array inside this object, and no heap allocation is needed. The 
    ring buffer Capacity parameter should be specified as the same as the std::array size, and new items will begin to replace old items as soon as the buffer reaches the capacity.
    If std::array is used, the a small ring buffer can be efficiently kept in a stack object rather than allocated on the heap.
    Or, if std::vector is used, or any other item whose size increments as items are added with push_back(),  new items are added with push_back() until the size of the 
    container reaches the given Capacity, in which case the ring buffer will begin reusing the memory previously used in the std::vector, replacing old items.  I.e. the size of
    the vector will be expanded as usual until reaching Capacity.  

    The underlying container object can be accessed as @a container after the ring_buffer is created. (For example,
    to choose an initial size with reserve(), or similar.)   The container type given as ContainerT should support the following:
      * Provide iterators via begin() and end().
      * Provide a size via size().
      * Optionally provide a push_back() method if it can be expanded (until size() == Capacity).  If it does not have a push_back() method, then this feature will not be used and only the container's current size is used.

 *  Items may be added to the ring_buffer with push() or emplace(), and
 *  the oldest item still available can be accessed from the front with pop() or front().  (But most recent item cannot be accessed from the back.)
 *  If an old item is replaced by a new item by push(), then T's copy or move assignment operator is used to replace the item.   If replaced by emplace(), then the copy or move constructor is used.
 *
 *  Note: a std::containeray is used internally, so the size of an ArRingBuffer object is fixed at Capacity * sizeof(T) + a few pointers and size. (two iterators and size_t).  So a very large ArRingBuffer should be allocated and stored as a pointer (e.g. use std::make_unique or std::make_shared.) But smaller ArRingBufferObjects can be stored on the stack (e.g. as regular data members of a class.) 
 *
 *  Note: currently if reset() is called or items are popped(), old items are not 
 *  destroyed (destructors not called).  Destructors may be called if the item is later replaced by a new item. 
 *
 *  Note: This implementation is not thread safe; it is not designed or optimized for simultaneous reads and writes from different threads. (A mutex lock or similar may be required.) A future version could do this.
 *
 *  @todo Ideally, this class would be fully threadsafe/non-locking (or with occasional
 *  mutex locking in certain cases), but it is not currently. Should be updated to use
 *  atomic support in modern C++ standard library if possible, or using techniques
 *  described in <https://rigtorp.se/ringbuffer/>
 *
 *  @ingroup UtilityClasses
 */
template<class ItemT, size_t Capacity, class ContainerT = std::array<ItemT, Capacity>>
class ArRingBuffer {
public:

  /** @param capacity Initial capacity of the ring queue. 
   *  @param init_value Initial value for new, unused items in the queue. 
   *  */
  ArRingBuffer() : curSize(0), front_it(container.end()), back_it(container.begin())
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
  typename ContainerT::iterator front() {
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
  typename ContainerT::iterator back() {
    if(front_it == back_it)
    {
      //std::cerr << "ArRingBuffer: back(): 0-capacity or full, returning nil.\n";
      return nil();
    }
    return back_it;
  }

  /** Advance the front of the queue. 'Used' size will be decremented.  */
  void advance_front() {
    if(front_it == container.end())  // initial or  empty state.
      front_it = container.begin();
    else if(++front_it == container.end()) 
      front_it = container.begin();
    if(front_it == back_it) { // it's now empty (not full)
      front_it = container.end();
      back_it = container.begin();
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
    if(++back_it == container.end())
      back_it = container.begin();
    if(front_it == container.end())
      front_it = container.begin();  // no longer empty.
    curSize++;
  }

  /** Push a new item.  If the buffer is full, then the oldest item is replaced with the new item.
   */
  void push(const ItemT& item) {
    // todo push_back to expand container if neccesary (container.size() < Capacity
    if(full())
      advance_front(); // throw away the item at the front, when we advance_back(), then back will be pointing to it.
    *back_it = item;
    advance_back();
  }

  /** Take rvalue reference, so item could be moved into buffer. */
  void push(ItemT&& item) {
    // todo push_back to expand container if neccesary and possible (container.size() < Capacity)
    if(full())
      advance_front();
    *back_it = item;
    advance_back();
  }

  /** Print the current contents of the queue. 
   *  @pynote use printQueue() instead of print() (which is a reserved word in Python)
  */
  void print() const {
    for(typename ContainerT::const_iterator i = container.begin(); i != container.end(); i++) {
      if(i == back_it)
        std::cerr << "]";
      if(i != container.begin())
        std::cerr << ",";
      if(i == front_it || (i == container.begin() && front_it == container.end()) )
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
  constexpr size_t capacity() const {
    if(Capacity == 0)
      return container.size();
    else
      return Capacity;
  }

  /** Return true if the queue is empty (has no 'used' items), false otherwise.  */
  bool empty() const {
    return (front_it == container.end());
  }

  /** Logically clear the queue, resetting to initial empty state
   * The contents are not
   * destroyed, but will be when replaced by new data later. */
  void reset() {
    front_it = container.end();
    back_it = container.begin();
    curSize = 0;
  }

  /** Return true if the queue is full, false otherwise. */
  bool full() const {
    return (back_it == front_it);
  }

  /** Return an iterator representing an invalid item. Compare to the return
   * values of front(), back(), pop(), etc. */
  typename ContainerT::iterator nil() {
    return container.end();
  }


private:
  size_t curSize;
  typename ContainerT::iterator front_it, back_it;   
  // push to back, pop from front; front will point to first item, 
  // back to one past last. 

public:

  ContainerT container;

};



#endif
