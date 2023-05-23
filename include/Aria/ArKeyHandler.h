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
#ifndef ARKEYHANDLER_H
#define ARKEYHANDLER_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArFunctor.h"
#include "Aria/ariaUtil.h"

#include <map>
#include <stdio.h>

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#include <assert.h>


/// Perform actions in response to keyboard keys in terminal/console input
/**
   This class is used for handling input from the keyboard (via terminal/console
   window), and invoking callback functions when specific keys are pressed.
   Use addKeyHandler() to associate a callback function with a keyboard key.
   ArKeyHandler will invoke that callback when the key is pressed.

   ArKeyHandler can handle all alphanumeric and punctuation keys (lower and
   upper case), as well as arrow keys, function keys (F-keys), escape, page up, 
   page down, delete, and insert.  It does not handle control-keys alt-shifted keys.

   To avoid accidentally creating more than one ArKeyHandler object, you should also 
   register the keyhandler with Aria::setKeyHandler(),
   and before you create a key handler you should see if one is
   already there with Aria::getKeyHandler().  Only one key handler
   can be created in a program, and this lets independent parts
   of a program use the same key handler.
   
   You can attach a key handler to a robot with
   ArRobot::attachKeyHandler() which will create a task
   list of tasks to automatically check for new keyboard input in each cycle, so
   you do not need to check for keyboard input elsewhere in the program. This
   will also add a handler to end the robot's task cycle when Escape is pressed
   (you can override this by replacing it with your own handler).
   (If you have multiple ArRobot objects, only create one key handler and attach
   it to one robot.)

   Alternatively, you can call checkKeys() periodically to check for new key
   input, and invoke any associated functions for that key.

   If you only want to poll keyboard input and not invoke functions, you can
   instead use getKey() to check for one keypress.

    @bug does not handle multi-byte special control sequences (high f keys etc)
    correctly in blocking mode

  @ingroup OptionalClasses
**/

class ArKeyHandler
{
public:
  AREXPORT ArKeyHandler(bool blocking = false, bool addAriaExitCB = true, 
			FILE *stream = NULL, 
			bool takeKeysInConstructor = true);

  /// Destructor. Reseases control of the keyboard and restores state before
  /// this key handler was created.
  AREXPORT ~ArKeyHandler();

  /// These are symbols for the non-ascii keys
  enum KEY : int {
    UP = 256, ///< Up arrow (keypad or 4 key dirs)
    DOWN, ///< Down arrow (keypad or 4 key dirs)
    LEFT, ///< Left arrow (keypad or 4 key dirs)
    RIGHT, ///< Right arrow (keypad or 4 key dirs)
    ESCAPE, ///< Escape key
    SPACE, ///< Space key
    TAB, ///< Tab key
    ENTER, ///< Enter key
    BACKSPACE, ///< Backspace key
    _StartFKeys,  ///< F key offset (internal; don't use).
    F1, ///< F1
    F2, ///< F2
    F3, ///< F3
    F4, ///< F4
    F5, ///< F5 (not supported on Windows yet)
    F6, ///< F6 (not supported on Windows yet)
    F7, ///< F7 (not supported on Windows yet)
    F8, ///< F8 (not supported on Windows yet)
    F9, ///< F9 (not supported on Windows yet)
    F10, ///< F10 (not supported on Windows yet)
    F11, ///< F11 (not supported on Windows yet)
    F12, ///< F12 (not supported on Windows yet)
    _EndFKeys, ///< F key range (internal; don't use)
    PAGEUP, ///< Page Up (not supported on Windows yet)
    PAGEDOWN, ///< Page Down (not supported on Windows yet)
    HOME,   ///< Home key (not supported on Windows yet)
    END,    ///< End key (not supported on Windows yet)
    INSERT, ///< Insert key (not supported on Windows yet)
    DEL  ///< Special delete key (often forward-delete) (not supported on Windows yet)
  };

  /// This adds a keyhandler, when the keyToHandle is hit, functor will fire
  AREXPORT bool addKeyHandler(int keyToHandle, ArFunctor *functor);

  /// This removes a key handler, by key
  AREXPORT bool remKeyHandler(int keyToHandler);
  /// This removes a key handler, by key
  AREXPORT bool remKeyHandler(ArFunctor *functor);

  /// Takes the key control over. For internal or special use, since it's
  /// called in the constructor.
  AREXPORT void takeKeys(bool blocking = false);

  /// Sets stdin back to its original settings, if its been restored
  /// it won't read anymore. For internal or special use, since it's 
  /// called in the destructor.
  AREXPORT void restore();

  /// Internal: Use addKeyHandler() instead. This internal method Checks for keys and handles them. This is automatically done in an
  /// ArRobot task if a keyhandler attached to ArRobot with
  /// ArRobot::attachKeyHandler() or Aria::setKeyHandler(), in which case 
  /// you do not need to call it.  If not using or running an ArRobot 
  /// task cycle, call this instead.
  /// @internal
  AREXPORT void checkKeys();

  /// Gets a key from the stdin if ones
  /// available, -1 if there aren't any available
  AREXPORT int getKey();

protected:


  std::map<int, ArFunctor *> myMap;
  bool myBlocking;
  
  bool myRestored;
  ArFunctorC<ArKeyHandler> myAriaExitCB;
#ifndef _WIN32
  struct termios myOriginalTermios;
#endif
  
  FILE *myStream;
  bool myTookKeys;

#ifndef WIN32
  int getChar();
  int waitForChar(long timeout)
  {
    assert(timeout >= 0);
    ArTime start;
    while(start.mSecSince() < timeout)
    {
      const int key = getChar();
      if(key != -1)
      return key;
    }
    return -1;
  }


  int mySavedChar = -1; 
  void saveChar(int c)
  {
    mySavedChar = c;
  }
#endif

};


#endif // ARKEYHANDLER_H
