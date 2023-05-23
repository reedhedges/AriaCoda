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
#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArKeyHandler.h"
#include "Aria/ArLog.h"

#ifdef WIN32
#include <conio.h>
#else // if not win32
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#include <utility>

#include "Aria/ariaInternal.h"

/**
   @param blocking whether checkKeys() or getKey() should block and wait for 
   input, or be nonblocking, and return -1 immediately if there is no input.
   Nonblocking input (blocking=false) is recommended for most use cases
   of ArKeyHandler(), and must be false if attached to ArRobot task cycle
   with ArRobot::attachKeyHandler().
   @param addAriaExitCB true to add an Aria exit callback to restore
   keys (remove callbacks) and reset input state when Aria or program exits.
   @param stream an alternative input stream to use. if this is NULL (the default)
   then standard input (stdin) is used.  
   @param takeKeysInConstructor whether to initialize input terminal state and
   begin capturing all input during this call to the constructor, or not.  If false, call takeKeys() later.
**/
AREXPORT ArKeyHandler::ArKeyHandler(bool blocking,  // default false
            bool addAriaExitCB,                     // default true
				    FILE *stream,                           // default NULL (will use stdin)
				    bool takeKeysInConstructor) :           // default true
  myAriaExitCB(this, &ArKeyHandler::restore)
{
  myAriaExitCB.setName("ArKeyHandlerExit");
  if (addAriaExitCB)
    Aria::addExitCallback(&myAriaExitCB);

  myStream = stream;
  myTookKeys = false;

  if (takeKeysInConstructor)
    takeKeys(blocking);
}

AREXPORT ArKeyHandler::~ArKeyHandler()
{
  Aria::remExitCallback(&myAriaExitCB);
  restore();
}

AREXPORT void ArKeyHandler::takeKeys(bool blocking)
{
  myBlocking = blocking;
#ifndef _WIN32
  mySavedChar = -1;
  struct termios newTermios;

  if (myStream == NULL)
  {
    tcgetattr(fileno(stdin), &myOriginalTermios);
    tcgetattr(fileno(stdin), &newTermios);
  }
  else
  {
    tcgetattr(fileno(myStream), &myOriginalTermios);
    tcgetattr(fileno(myStream), &newTermios);
  }

  //newTermios.c_cc[VTIME] = 0;
  if (myBlocking)
  {
    // always wait for 1 byte, no timeout
    newTermios.c_cc[VTIME] = 0;
    newTermios.c_cc[VMIN] = 1;
  }
  else
  {
    // VTIME=1 VMIN=0 means 0.1s timeout only, don't wait for any bytes
    // VTIME=0 VMIN=0 means immediate timeout if no character, don't wait for any characters
    newTermios.c_cc[VTIME] = 0;
    newTermios.c_cc[VMIN] = 0;
    const int fd = myStream ? fileno(myStream) : STDIN_FILENO;
    const int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags & O_NONBLOCK);
  }
  newTermios.c_lflag &= (unsigned int) (~ECHO & ~ICANON);

  if (myStream == NULL)
    tcsetattr(fileno(stdin), TCSANOW, &newTermios);
  else
    tcsetattr(fileno(myStream), TCSANOW, &newTermios);

#endif
  myRestored = false;
  myTookKeys = true;
}

AREXPORT void ArKeyHandler::restore()
{
  if (!myTookKeys)
    return;

#ifndef _WIN32
  if (myStream == NULL)
    tcsetattr(fileno(stdin), TCSANOW, &myOriginalTermios);
  else
    tcsetattr(fileno(myStream), TCSANOW, &myOriginalTermios);

  mySavedChar = -1;
#endif
  myRestored = true;
  myTookKeys = false;
}

/**
   @param keyToHandle A character value, such as 'a' or '1'
     or '[', or a member of the KEY enum.

   @param functor Functor to call when @a keyToHandle is received

   @return true If no previous key handler functor exists for @a keyToHandle and
   the handler functor was stored, or false if a handler for that key already 
   exists.
   that key
*/
AREXPORT bool ArKeyHandler::addKeyHandler(int keyToHandle, ArFunctor *functor)
{
  if (myMap.find(keyToHandle) != myMap.end())
  {
    if (keyToHandle >= '!' && keyToHandle <= '~')
      ArLog::log(ArLog::Normal, "There is already a key to handle '%c' which is number %d 0x%x", keyToHandle, keyToHandle, keyToHandle);
    else
      ArLog::log(ArLog::Normal, "There is already a key to handle number %d 0x%x", keyToHandle, keyToHandle);
    return false;
  }
  //ArLog::log(ArLog::Verbose, "keyhandler %p added key '%c' number '%d'", 
  //this, keyToHandle, keyToHandle);
  myMap[keyToHandle] = functor;

  return true;
}

/**
   @param keyToHandle The character value or code to clear the handler for.

    @return true if the remKeyHandler succeeded, which means that a
     key handler for @a keyToHandle was found and rmeoved, or false if no
     handler for that value was found.
 **/
AREXPORT bool ArKeyHandler::remKeyHandler(int keyToHandle)
{
  if (myMap.find(keyToHandle) == myMap.end())
  {
    //ArLog::log(ArLog::Normal, "There is no key to handle '%c' which is number %d 0x%x", keyToHandle, keyToHandle, keyToHandle);
    return false;
  }
  if (keyToHandle >= '!' && keyToHandle <= '~')
    ArLog::log(ArLog::Verbose, "keyhandler %p removed key '%c' number '%d'",
	       this, keyToHandle, keyToHandle);
  else
    ArLog::log(ArLog::Verbose, "keyhandler %p removed key number '%d'",
	       this, keyToHandle);
  myMap.erase(keyToHandle);
  return true;
}

/**
   @param functor the functor of the handler to remove

   @return true if the remKeyHandler succeeded, which means that the
    functor was found and removed from the handlers, or false if no
    handler with the given functor was found.
 **/
AREXPORT bool ArKeyHandler::remKeyHandler(ArFunctor *functor)
{
  std::map<int, ArFunctor *>::iterator it;
  std::list<std::map<int, ArFunctor *>::iterator> iters;
  std::list<std::map<int, ArFunctor *>::iterator>::iterator iterIter;

  for (it = myMap.begin(); it != myMap.end(); ++it)
  {
    if (it->second == functor)
    {
      iters.push_front(it);
    }
  }
  if (iters.size() > 0)
  {
    while((iterIter = iters.begin()) != iters.end())
    {
      myMap.erase((*iterIter));
      iters.pop_front();
    }
    ArLog::log(ArLog::Verbose, "keyhandler %p removed functor %p", this, 
	       functor);
    return true;
  }
  return false;
}

AREXPORT void ArKeyHandler::checkKeys()
{
  int key;
  std::map<int, ArFunctor *>::iterator it;

  if (myRestored)
    return;

  // get keys until they're gone
  while (!myRestored && (key = getKey()) != -1)
  {
    // if there's something to handle it, handle it
    if ((it = myMap.find(key)) != myMap.end())
    {
      //ArLog::log(ArLog::Verbose, "key '%c' num %d pressed\n", key, key);
      it->second->invoke();
    }
  }
}

#ifndef _WIN32

#ifdef __MACH__


/* getChar() implementation for Mac: */

AREXPORT int ArKeyHandler::getChar()
{
  // NOTE uses ::read which is unbuffered, so if we call getChar() immediately
  // after a first call to getChar() returns the beginning of a control sequence
  // (27), we may not have the next character yet.  Use waitForChar() to avoid
  // this, or find a way to use buffered input here.

  
	// TODO  since the linux implementation is now pretty much the same as this,
	// use that instead?

  if(mySavedChar != -1)
  {
    return std::exchange(mySavedChar, -1);
  }

	int c = EOF;
	int fd = -1;
	{
		if(myStream)
			fd = fileno(myStream);
		else
			fd = STDIN_FILENO;
		
		if(::read(fd, &c, 1) <= 0)
			c = -1;
		else
			c += 256; // hack to get an ascii value from latin1?
	}
	return c;
}

#else

/* getChar() implementation for Linux: */
AREXPORT int ArKeyHandler::getChar()
{

/* getchar() (and getc() and fgetc()) stopped working at some point? 
    note can't use ungetc() if not using getc()/getchar() anymore.
    use saveChar()/check mySavedChar instead.
*/
/*
  if (myStream == NULL)
  {
    const int c = getchar();
    return c;
  }
  else
    return getc(myStream);
*/

  if(mySavedChar != -1)
  {
    return std::exchange(mySavedChar, -1);
  }

  // NOTE uses ::read which is unbuffered, so if we call getChar() immediately
  // after a first call to getChar() returns the beginning of a control sequence
  // (27), we may not have the next character yet.  Use waitForChar() to avoid
  // this, or find a way to use buffered input here.

  unsigned char c;
  const ssize_t nr = ::read(myStream?fileno(myStream):STDIN_FILENO, &c, 1);
  if(nr == 0) return -1; // no input
  if(nr < 0) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      return -1; // no input
    } else {
      ArLog::logErrorFromOS(ArLog::Normal, "ArKeyHandler: Error reading input");
      return -1;
    }
  }
  return (int)c;

}

#endif

// Linux and Mac:
AREXPORT int ArKeyHandler::getKey()
{
 /*
  * What follows is a somewhat poor implementation of getch(), basically, since
  * we want to get control keys but don't want to have to use curses.
  */

  int k[5] = {-1, -1, -1, -1, -1};   // used to store escape sequences while parsing

  const long NextControlKeyTimeout = 10; // ms to wait for next control key character

  // check first key
  int key = getChar();
  switch(key)
  {
    case -1:
    case 0:
      return -1;

    //case -1: return ESCAPE; //why was this also here?
    case ' ': return SPACE;
    case '\t': return TAB;
    case 10: return ENTER;
    case 13: return ENTER;
    case 8: return BACKSPACE;
    case 127: return BACKSPACE;

    case 27: // Escape key, or Begin special control key sequence, get next byte
      key = waitForChar(NextControlKeyTimeout);
      switch(key)
      {
        case -1: return ESCAPE;
        case 79: // first four F keys, check third byte
          key = waitForChar(NextControlKeyTimeout);
          switch(key)
          {
            case 80: return F1;
            case 81: return F2;
            case 82: return F3;
            case 83: return F4;
            case 70: return END;
            case 72: return HOME;
            default: return key;
          }
        case '[':  // keypad keys and extended F-key sequences start with this 
          // Read in all characters in the special key sequence. -1 is the
          // terminator (no more keys). We also check for the beginning of a new special key
          // sequence (27) since if the key repeat is high enough, they begin
          // overlapping. TODO: check trailing keys for modifiers line shift and set an
          // (optional parameter) bitmask.
          for(short i = 0; key != -1 && key != 27 && i < 5; i++) 
          {
            k[i] = key = waitForChar(NextControlKeyTimeout);
            //printf("ArKeyHandler::getKey: read extended key component %d/%d.\n", k[i], key);
          }
          //ungetc(key, stdin); // put last key back. (Esp. important if it was the beginning of a new control sequence (27).
          saveChar(key); // put last key back. (Esp. important if it was the beginning of a new control sequence (27).

          switch(k[0])
          {
            case 65: return UP;
            case 66: return DOWN;
            case 67: return RIGHT;
            case 68: return LEFT; 

            case 51: return DEL;
            case 53: return PAGEUP;
            case 54: return PAGEDOWN;

            case 50: 
              switch(k[1])
              {
                case 126: return INSERT;
                case 48:  return F9;
                case 49:  return F10;
                case 51:  return F11;
                case 52:  return F12;
              }
              return k[1];


            case 49:
              switch(k[1])
              {
                case 53: return F5;
                case 55: return F6;
                case 56: return F7;
                case 57: return F8;
              }
              return k[1];
            default: return -1;
          }
        default: return -1; // TODO should this be ESCAPE?
      }

    default: return key;
  }
}

#if 0
/* This is a previous implementation of getKey(), just for reference or
 * quick reversion: */
AREXPORT int ArKeyHandler::getKey()
{
  char key;

  key = getChar();
  if (key == -1 || key == 0)
      return -1;

  if (key == 27)
  {
    key = getChar();
    if (key == '[')
    {
      key = getChar();
      if (key == 'A')
        return UP;
      else if (key == 'B')
        return DOWN;
      else if (key == 'C')
        return RIGHT;
      else if (key == 'D')
        return LEFT;
      else
        return getKey();
    }
    else if (key == -1)
      return ESCAPE;
    else if (key == 79)
    {
       key = getChar();
       if (key == 'P')
         return F1;
       else if (key == 'Q')
         return F2;
       else if (key == 'R')
         return F3;
       else if (key == 'S')
         return F4;
       else
         return getKey();
    }
  }
  else if (key == ' ')
    return SPACE;
  else if (key == '\t')
    return TAB;
  else if (key == 10)
    return ENTER;
  else if (key == 8 || key == 127)
    return BACKSPACE;
  return key;
}
#endif


#else // if it is win32

// Windows implementation:

AREXPORT int ArKeyHandler::getKey()
{
  int key;

  if (!myBlocking && !kbhit())
    return -1;

  key = _getch();
  if (key == 224)
  {
    key = _getch();
    if (key == 'H')
      return UP;
    else if (key == 'P')
      return DOWN;
    else if (key == 'K')
      return LEFT;
    else if (key == 'M')
      return RIGHT;
    else 
      return getKey();
  }
  else if (key == 0)
  {
    key = _getch();
    if (key == ';')
      return F1;
    else if (key == '<')
      return F2;
    else if (key == '=')
      return F3;
    else if (key == '>')
      return F4;
    else if (key == 'H')
      return UP;
    else if (key == 'P')
      return DOWN;
    else if (key == 'K')
      return LEFT;
    else if (key == 'M')
      return RIGHT;
    else
      return getKey();
  }
  else if (key == ' ')
    return SPACE;
  else if (key == '\t')
    return TAB;
  else if (key == 10)
    return ENTER;
  else if (key == 13)
    return ENTER;
  else if (key == 8)
    return BACKSPACE;
  else if (key == 27)
	return ESCAPE;

  return key;

}

#endif // WIN32


