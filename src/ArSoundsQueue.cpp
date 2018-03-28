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

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "ArExport.h"
#include "ArLog.h"
#include "ariaUtil.h"
#include "ArSoundsQueue.h"
#include "ArSoundPlayer.h"
#include <assert.h>


// For debugging:
//#define ARSOUNDSQUEUE_DEBUG 1


#ifdef ARSOUNDSQUEUE_DEBUG
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
#define debuglog(msg) ArLog::log(ArLog::Verbose, "%s: %s", __PRETTY_FUNCTION__, (msg));
#else
#define debuglog(msg) {}
#endif


using namespace std;


AREXPORT ArSoundsQueue::Item::Item() :
  data(""), type(OTHER), params(""), priority(0)
{
}

AREXPORT ArSoundsQueue::Item::Item(std::string _data, ItemType _type, std::string _params, int _priority, std::list<PlayItemFunctor*> _callbacks) : 
      data(_data), type(_type), params(_params), priority(_priority), playCallbacks(_callbacks)
{
}

AREXPORT ArSoundsQueue::Item::Item(std::string _data, ItemType _type, std::string _params, int _priority) : 
  data(_data), type(_type), params(_params), priority(_priority)
{
}

AREXPORT ArSoundsQueue::Item::Item(const ArSoundsQueue::Item& toCopy) 
{
  data = toCopy.data;
  type = toCopy.type;
  params = toCopy.params;
  priority = toCopy.priority;
  playCallbacks = toCopy.playCallbacks;
  interruptCallbacks = toCopy.interruptCallbacks;
  doneCallbacks = toCopy.doneCallbacks;
  playbackConditionCallbacks = toCopy.playbackConditionCallbacks;
}

void ArSoundsQueue::Item::play()
{
  for(std::list<PlayItemFunctor*>::const_iterator i = playCallbacks.begin(); i != playCallbacks.end(); i++) 
  {
    if(*i) {
      (*i)->invokeR(data.c_str(), params.c_str());
    }
  }
}

void ArSoundsQueue::Item::interrupt()
{
  for(std::list<ArFunctor*>::const_iterator i = interruptCallbacks.begin(); i != interruptCallbacks.end(); i++) 
    if(*i) (*i)->invoke();
}


void ArSoundsQueue::Item::done()
{
  for(std::list<ArFunctor*>::const_iterator i = doneCallbacks.begin(); i != doneCallbacks.end(); i++) 
    if(*i) {
      (*i)->invoke();
    }
}

/** @cond INTERNAL_CLASSES */

/** STL list comparator function object base class
 * @internal
 */
class ItemComparator {
protected:
  ArSoundsQueue::Item myItem;
public:
  ItemComparator(string data = "", ArSoundsQueue::ItemType type = ArSoundsQueue::OTHER, string params = "", int priority = 0) : 
    myItem(data, type, params, priority)
  {}
  virtual ~ItemComparator() {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return (other == myItem && other.priority == myItem.priority);
  }
};

/** Function object which compares the data portion of list items 
 * @internal
 */
class ItemComparator_OnlyData : public virtual ItemComparator {
public:
  ItemComparator_OnlyData(string data) : 
    ItemComparator(data)
  {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return(other.data == myItem.data);
  }
};

/** Function object which compares the data and type fields of list items 
 * @internal
 */
class ItemComparator_TypeAndData : public virtual ItemComparator {
public:
  ItemComparator_TypeAndData(string data, ArSoundsQueue::ItemType type) : 
    ItemComparator(data, type)
  {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return(other.type == myItem.type && other.data == myItem.data);
  }
};

/** Function object which compares the  priority fields of list items 
 * @internal
 */
class ItemComparator_PriorityLessThan : public virtual ItemComparator {
  int myPriority;
public:
  ItemComparator_PriorityLessThan(int priority) : myPriority(priority)
  {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return(other.priority < myPriority);
  }
};

/** Function object which compares the  priority fields of list items 
 * @internal
 */
class ItemComparator_WithTypePriorityLessThan : public virtual ItemComparator {
  ArSoundsQueue::ItemType myType;
  int myPriority;
public:
  ItemComparator_WithTypePriorityLessThan(ArSoundsQueue::ItemType type, int priority) : myType(type), myPriority(priority)
  {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return(other.type == myType && other.priority < myPriority);
  }
};

/** Function object which compares the type field of list items to a supplied
 * type 
 * @internal
 */ 
class ItemComparator_WithType : public virtual ItemComparator {
  ArSoundsQueue::ItemType myType;
public:
  ItemComparator_WithType(ArSoundsQueue::ItemType type) : myType(type)
  {}
  virtual bool operator()(const ArSoundsQueue::Item& other)
  {
    return(other.type == myType);
  }
};

/** @endcond INTERNAL_CLASSES */


AREXPORT ArSoundsQueue::ArSoundsQueue()  :
  myInitialized(false),
  myPlayingSomething(false),
  myDefaultSpeakCB(0), myDefaultInterruptSpeechCB(0),
  myDefaultPlayFileCB(0), myDefaultInterruptFileCB(0),
  myPauseRequestCount(0),
  myDefaultPlayConditionCB(0)
{
  setThreadName("ArSoundsQueue");
  myQueueMutex.setLogName("ArSoundsQueue::myQueueMutex");
}

AREXPORT ArSoundsQueue::ArSoundsQueue(ArRetFunctor<bool> *speakInitCB, 
		    PlayItemFunctor *speakCB, 
        InterruptItemFunctor *interruptSpeechCB,
        ArRetFunctor<bool> *playInitCB, 
        PlayItemFunctor *playCB,
        InterruptItemFunctor *interruptFileCB) :
  myInitialized(false), myPlayingSomething(false),
  myDefaultSpeakCB(speakCB), myDefaultInterruptSpeechCB(interruptSpeechCB),
  myDefaultPlayFileCB(playCB), myDefaultInterruptFileCB(interruptFileCB),
  myPauseRequestCount(0),
  myDefaultPlayConditionCB(0)
{ 
  setThreadName("ArSoundsQueue");
  if(speakInitCB)
    myInitCallbacks.push_back(speakInitCB);
  if(playInitCB)
    myInitCallbacks.push_back(playInitCB);
  if(playCB == 0)
    myDefaultPlayFileCB = ArSoundPlayer::getPlayWavFileCallback();
  if(interruptFileCB == 0)
    myDefaultInterruptFileCB = ArSoundPlayer::getStopPlayingCallback();
}

/*
AREXPORT ArSoundsQueue::ArSoundsQueue(ArSpeechSynth* speechSynth, 
		    ArRetFunctor<bool> *playInitCB,
		    PlayItemFunctor *playFileCB,
        InterruptItemFunctor *interruptFileCB) 
  : myInitialized(false), myPlayingSomething(false),
  myDefaultSpeakCB(0), myDefaultInterruptSpeechCB(0),
  myDefaultPlayFileCB(playFileCB), myDefaultInterruptFileCB(interruptFileCB),
  myPauseRequestCount(0),
  myDefaultPlayConditionCB(0)
{
  setThreadName("ArSoundsQueue");
  if(playInitCB)
    myInitCallbacks.push_back(playInitCB);
  if(speechSynth)
  {
    myInitCallbacks.push_back(speechSynth->getInitCallback());
    myDefaultSpeakCB = speechSynth->getSpeakCallback();
    myDefaultInterruptSpeechCB = speechSynth->getInterruptCallback();
  }
}
*/

AREXPORT ArSoundsQueue::~ArSoundsQueue()
{

}


void ArSoundsQueue::invokeCallbacks(const std::list<ArFunctor*>& lst)
{
  for(std::list<ArFunctor*>::const_iterator i = lst.begin(); i != lst.end(); i++)
  {
    if(*i) (*i)->invoke();
    else ArLog::log(ArLog::Verbose, "ArSoundsQueue: warning: skipped NULL callback (simple functor).");
  }
}

void ArSoundsQueue::invokeCallbacks(const std::list<ArRetFunctor<bool>*>& lst)
{
  for(std::list<ArRetFunctor<bool>*>::const_iterator i = lst.begin(); i != lst.end(); i++)
  {
    if(*i) (*i)->invokeR();
    else ArLog::log(ArLog::Verbose, "ArSoundsQueue: warning: skipped NULL callback (bool ret. funct.).");
  }
}


// This is the public method, but all we have to do is call the private push
// method.
AREXPORT void ArSoundsQueue::addItem(ItemType type, const char* data, std::list<PlayItemFunctor*> callbacks, int priority, const char* params)
{
  assert(data);
  pushQueueItem(Item(data, type, params?params:"", priority, callbacks));
}

// This is the public method, but all we have to do is call the private push
// method.
AREXPORT void ArSoundsQueue::addItem(ArSoundsQueue::Item item)
{
  pushQueueItem(item);
}

// Class-protected version.
void ArSoundsQueue::pushQueueItem(ArSoundsQueue::Item item)
{
  lock();
  pushQueueItem_NoLock(item);
  unlock();
}

// Class-protected version that does not lock (so caller can do it manually as
// needed)
void ArSoundsQueue::pushQueueItem_NoLock(ArSoundsQueue::Item item)
{
  ArLog::log(ArLog::Verbose, "ArSoundsQueue: pushing \"%s\" with type=%d, priority=%d, params=\"%s\".", item.data.c_str(), item.type, item.priority, item.params.c_str());
  myQueue.push_back(item);
}

ArSoundsQueue::Item ArSoundsQueue::popQueueItem()
{
  lock();
  ArSoundsQueue::Item item = *(myQueue.begin());
  myQueue.pop_front();
  unlock();
  return item;
}

ArSoundsQueue::Item ArSoundsQueue::popQueueItem_NoLock()
{
  ArSoundsQueue::Item item = *(myQueue.begin());
  myQueue.pop_front();
  return item;
}

AREXPORT ArSoundsQueue::Item ArSoundsQueue::createDefaultSpeechItem(const char* speech)
{
  Item item;
  item.type = SPEECH;
  if(myDefaultSpeakCB)
    item.playCallbacks.push_back(myDefaultSpeakCB);
  if(myDefaultInterruptSpeechCB)
    item.interruptCallbacks.push_back(myDefaultInterruptSpeechCB);
  if(speech)
    item.data = speech; // copy char* contents into std::string
  if(myDefaultPlayConditionCB)
    item.playbackConditionCallbacks.push_back(myDefaultPlayConditionCB);
  return item;
}

AREXPORT void ArSoundsQueue::speak(const char *str)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);
  Item item = createDefaultSpeechItem();
  item.data = str;
  pushQueueItem(item);
}

AREXPORT void ArSoundsQueue::play(const char *str)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);
  Item item = createDefaultFileItem();
  item.data = str;
  pushQueueItem(item);
}

#if !(defined(WIN32) && defined(_MANAGED)) // MS Managed C++ does not allow varargs

AREXPORT void ArSoundsQueue::speakf(const char *str, ...)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);

  char *buf;
  size_t buflen = (strlen(str) + 1000 * 2);
  buf = new char[buflen];

  Item item = createDefaultSpeechItem();

  lock(); // need to lock out here to protect the un-threadsafe va_list functions
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, buflen, str, ptr);
  item.data = buf; // std::string constructor will duplicate char* contents
  pushQueueItem_NoLock(item);   // must use NoLock, since we lock() and unlock() in this function
  va_end(ptr);
  delete[] buf;
  unlock();
}


AREXPORT void ArSoundsQueue::speakWithVoice(const char* voice, const char* str, ...)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);

  char *buf;
  size_t buflen = (strlen(str) + 1000 * 2);
  buf = new char[buflen];

  Item item = createDefaultSpeechItem();
  string params = "name=";
  params += voice;
  item.params = params;

  lock(); // need to lock out here to protect the un-threadsafe va_list functions
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, buflen, str, ptr);
  item.data = buf;// std::string constructor will duplicate char* contents
  pushQueueItem_NoLock(item);
  va_end(ptr);
  delete[] buf;
  unlock();
}

AREXPORT void ArSoundsQueue::speakWithPriority(int priority, const char* str, ...)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);

  char *buf;
  size_t buflen = (strlen(str) + 1000 * 2);
  buf = new char[buflen];

  Item item = createDefaultSpeechItem();
  item.priority = priority;

  lock(); // need to lock out here to protect the un-threadsafe va_list functions
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, buflen, str, ptr);
  item.data = buf;// std::string constructor will duplicate char* contents
  pushQueueItem_NoLock(item);
  va_end(ptr);
  delete[] buf;
  unlock();
}


AREXPORT void ArSoundsQueue::playf(const char *str, ...)
{
  if(myQueue.size() == 0)
    invokeCallbacks(myQueueNonemptyCallbacks);

  char buf[2048];
  Item item = createDefaultFileItem();

  lock();  // va_list is not threadsafe
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, 2048, str, ptr);
  item.data = buf;  // std::string constructor will duplicate char* contents
  va_end(ptr);
  unlock();

  pushQueueItem(item);
}

#endif // MS Managed C++

AREXPORT ArSoundsQueue::Item ArSoundsQueue::createDefaultFileItem(const char* filename)
{
  Item item;
  item.type = SOUND_FILE;
  if(myDefaultPlayFileCB)
    item.playCallbacks.push_back(myDefaultPlayFileCB);
  else
    ArLog::log(ArLog::Normal, "ArSoundsQueue: Internal Warning: no default PlayFile callback.");
  if(myDefaultInterruptFileCB)
    item.interruptCallbacks.push_back(myDefaultInterruptFileCB);
  if(filename)
    item.data = filename; // copy into std::string
  if(myDefaultPlayConditionCB)
    item.playbackConditionCallbacks.push_back(myDefaultPlayConditionCB);
  return item;
}



AREXPORT void *ArSoundsQueue::runThread(void *arg)
{
  threadStarted();
  invokeCallbacks(myInitCallbacks);
  debuglog("the init callbacks were called.");
  myInitialized = true;

  while (getRunning())
  {
    lock();
    if(myPauseRequestCount > 0) 
    {
      unlock();
      myPausedCondition.wait();
      lock();
    }
    if (myQueue.size() > 0)
    {
      myLastItem = popQueueItem_NoLock();

#ifdef DEBUG
      ArLog::log(ArLog::Normal, "* DEBUG * ArSoundsQueue: Popped an item from the queue. There are %d condition callbacks for this item.", myLastItem.playbackConditionCallbacks.size());
#endif

      // Call some callbacks to tell them that play is about to begin
      invokeCallbacks(myStartPlaybackCBList);
      std::list<ArFunctor1<ArSoundsQueue::Item> *>::iterator lIt;
      for (lIt = myStartItemPlaybackCBList.begin(); 
	   lIt != myStartItemPlaybackCBList.end(); 
	   lIt++)
      {
	(*lIt)->invoke(myLastItem);
      }


      // Abort if any conditions return false
      bool doPlayback = true;
      for(std::list<PlaybackConditionFunctor*>::const_iterator i = myLastItem.playbackConditionCallbacks.begin(); 
          i != myLastItem.playbackConditionCallbacks.end(); i++)
      {
        if( (*i) && (*i)->invokeR() == false) {
          if( (*i)->getName() && strlen((*i)->getName()) > 0 )
            ArLog::log(ArLog::Normal, "ArSoundsQueue: the \"%s\" condition is preventing this item from playing. Skipping this item.", (*i)->getName());
          else
            ArLog::log(ArLog::Normal, "ArSoundsQueue: an unnamed condition is preventing this item from playing. Skipping this item.");
          doPlayback = false;
          break;
        }
#ifdef DEBUG
        else {
          ArLog::log(ArLog::Normal, "* DEBUG * ArSoundsQueue: Condition callback returned true.");
        }
#endif
      }

      if(doPlayback)
      {

        myPlayingSomething = true;
        unlock();

      // Play the item.
#ifdef DEBUG
        ArLog::log(ArLog::Normal, "* DEBUG* Acting on item. type=%d", myLastItem.type);
#endif
        myLastItem.play();

        // Sleep a bit for some "recover" time (especially for sound playback
        // processing)
        ArUtil::sleep(200);

        lock();
        myPlayingSomething = false;
      }


      // Call some more callbacks to tell them that play ended.
      debuglog("Finished acting on item. Invoking endPlayback callbacks...");
      unlock();
      myLastItem.done();
      invokeCallbacks(myEndPlaybackCBList);
      for (lIt = myEndItemPlaybackCBList.begin(); 
	   lIt != myEndItemPlaybackCBList.end(); 
	   lIt++)
      {
	(*lIt)->invoke(myLastItem);
      }
      lock();

      if(myQueue.size() == 0)
      {
        debuglog("invoking queue-empty callbacks!");
        unlock();
        invokeCallbacks(myQueueEmptyCallbacks);
      }
      else
      {
        unlock();
      }

    }
    else
    {
      unlock();
      ArUtil::sleep(20);
    }
  }
  threadFinished();
  return NULL;
}


AREXPORT void ArSoundsQueue::pause()
{
  lock();
  myPauseRequestCount++;
  unlock();
}

AREXPORT void ArSoundsQueue::resume()
{
  ArLog::log(ArLog::Verbose, "ArSoundsQueue::resume: requested.");
  lock();
  if(--myPauseRequestCount <= 0) 
  {
    myPauseRequestCount = 0;
    ArLog::log(ArLog::Verbose, "ArSoundsQueue::resume: unpausing.");
    unlock();
    myPausedCondition.signal();
  } else {
    unlock();
  }
}

AREXPORT void ArSoundsQueue::stop()
{
  stopRunning();
}

AREXPORT bool ArSoundsQueue::isPaused()
{
  return (myPauseRequestCount > 0);
}

AREXPORT void ArSoundsQueue::interrupt()
{
  lock();
  // Don't try to interrupt the last item removed from the queue if
  // it's not currently being played.
  //printf("interrupt: myPlayingSomething=%d\n", myPlayingSomething);
  if(myPlayingSomething)
    myLastItem.interrupt();
  myPlayingSomething = false;
  unlock();
}

AREXPORT void ArSoundsQueue::clearQueue() 
{
  lock();
  myQueue.clear();
  unlock();
  invokeCallbacks(myQueueEmptyCallbacks);
}

AREXPORT set<int> ArSoundsQueue::findPendingItems(const char* item)
{
  lock();
  set<int> found;
  int pos = 0;
  for(list<Item>::const_iterator i = myQueue.begin(); i != myQueue.end(); i++)
  {
    if((*i).data == item)
      found.insert(pos);
    pos++;
  }
  unlock();
  return found;
}

AREXPORT void ArSoundsQueue::removePendingItems(const char* item, ItemType type) 
{
  lock();
  myQueue.remove_if<ItemComparator_TypeAndData>(ItemComparator_TypeAndData(item, type));
  unlock();
}


AREXPORT void ArSoundsQueue::removePendingItems(const char* data)
{
  lock();
  myQueue.remove_if<ItemComparator_OnlyData>(ItemComparator_OnlyData(data));
  unlock();
}

AREXPORT void ArSoundsQueue::removePendingItems(int priority) 
{
  lock();
  myQueue.remove_if<ItemComparator_PriorityLessThan>(ItemComparator_PriorityLessThan(priority));
  unlock();
}

AREXPORT void ArSoundsQueue::removePendingItems(int priority, ItemType type)
{
  lock();
  myQueue.remove_if<ItemComparator_WithTypePriorityLessThan>(ItemComparator_WithTypePriorityLessThan(type, priority));
  unlock();
}

AREXPORT void ArSoundsQueue::removePendingItems(ItemType type)
{
  lock();
  myQueue.remove_if<ItemComparator_WithType>(ItemComparator_WithType(type));
  unlock();
}

AREXPORT void ArSoundsQueue::removeItems(int priority) 
{

  lock();
  removePendingItems(priority);
  if (myPlayingSomething && myLastItem.priority < priority)
  {
    interrupt();
  }
  unlock();
}


AREXPORT void ArSoundsQueue::removeItems(Item item) 
{

  lock();
  removePendingItems(item.data.c_str(), item.type);
  if (myPlayingSomething && myLastItem.type == item.type && 
      myLastItem.data == item.data)
  {
    interrupt();
  }
  unlock();
}


AREXPORT string ArSoundsQueue::nextItem(ItemType type)
{
  lock();
  for(list<Item>::const_iterator i = myQueue.begin(); i != myQueue.end(); i++)
  {
    if(type == (*i).type) {
      string found = (*i).data;
      unlock();
      return found;
    }
  }
  unlock();
  return "";
}

AREXPORT string ArSoundsQueue::nextItem(int priority)
{
  lock();
  for(list<Item>::const_iterator i = myQueue.begin(); i != myQueue.end(); i++)
  {
    if((*i).priority >= priority) {
      string found = (*i).data;
      unlock();
      return found;
    }
  }
  unlock();
  return "";
}

AREXPORT string ArSoundsQueue::nextItem(ItemType type, int priority)
{
  lock();
  for(list<Item>::const_iterator i = myQueue.begin(); i != myQueue.end(); i++)
  {
    if(type == (*i).type && (*i).priority >= priority) {
      string found = (*i).data;
      unlock();
      return found;
    }
  }
  unlock();
  return "";
}


