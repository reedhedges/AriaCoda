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
#include "ariaOSDef.h"
#include "ArMode.h"
#include "ArRobot.h"
#include "ariaInternal.h"

ArMode *ArMode::ourActiveMode = NULL;
ArGlobalFunctor *ArMode::ourHelpCB = NULL;
std::list<ArMode *> ArMode::ourModes;

/**
   @param robot the robot we're attaching to
   
   @param name the name of this mode

   @param key the primary key to switch to this mode on... it can be
   '\\0' if you don't want to use this

   @param key2 an alternative key to switch to this mode on... it can be
   '\\0' if you don't want a second alternative key
**/
AREXPORT ArMode::ArMode(ArRobot *robot, const char *name, char key, 
			char key2) :
  myActivateCB(this, &ArMode::activate),
  myDeactivateCB(this, &ArMode::deactivate),
  myUserTaskCB(this, &ArMode::userTask)
{
  ArKeyHandler *keyHandler;
  myName = name;
  myRobot = robot;
  myKey = key;
  myKey2 = key2;
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    if (myRobot != NULL)
      myRobot->attachKeyHandler(keyHandler);
    else
      ArLog::log(ArLog::Terse, "ArMode: No robot to attach a keyHandler to, keyHandling won't work... either make your own keyHandler and drive it yourself, make a keyhandler and attach it to a robot, or give this a robot to attach to.");
  }  
  if (ourHelpCB == NULL)
  {
    ourHelpCB = new ArGlobalFunctor(&ArMode::baseHelp);
    if (!keyHandler->addKeyHandler('h', ourHelpCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for 'h', ArMode will not be invoked on an 'h' keypress.");
    if (!keyHandler->addKeyHandler('H', ourHelpCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for 'H', ArMode will not be invoked on an 'H' keypress.");
    if (!keyHandler->addKeyHandler('?', ourHelpCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for '?', ArMode will not be invoked on an '?' keypress.");
    if (!keyHandler->addKeyHandler('/', ourHelpCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for '/', ArMode will not be invoked on an '/' keypress.");

  }

  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (myKey != '\0')
    if (!keyHandler->addKeyHandler(myKey, &myActivateCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for '%c', ArMode will not work correctly.", myKey);
  if (myKey2 != '\0')
    if (!keyHandler->addKeyHandler(myKey2, &myActivateCB))
      ArLog::log(ArLog::Terse, "The key handler already has a key for '%c', ArMode will not work correctly.", myKey2);

  // toss this mode into our list of modes
  ourModes.push_front(this);
}

AREXPORT ArMode::~ArMode()
{
  ArKeyHandler *keyHandler;
  if ((keyHandler = Aria::getKeyHandler()) != NULL)
  {
    if (myKey != '\0')
      keyHandler->remKeyHandler(myKey);
    if (myKey2 != '\0')
      keyHandler->remKeyHandler(myKey2);
  }
  if (myRobot != NULL)
    myRobot->remUserTask(&myUserTaskCB);
}

/** 
   Inheriting modes must first call this to get their user task called
   and to deactivate the active mode.... if it returns false then the
   inheriting class must return, as it means that his mode is already
   active
**/
AREXPORT bool ArMode::baseActivate(void)
{
  if (ourActiveMode == this)
    return false;
  myRobot->deactivateActions();
  if (myRobot != NULL)
  {
    myRobot->addUserTask((myName+"_mode_usertask").c_str(), 50, &myUserTaskCB);
  }
  if (ourActiveMode != NULL)
    ourActiveMode->deactivate();
  ourActiveMode = this;
  if (myRobot != NULL)
  {
    myRobot->stop();
    myRobot->clearDirectMotion();
  }
  
  baseHelp();
  return true;
}

/**
   This gets called when the mode is deactivated, it removes the user
   task from the robot
**/
AREXPORT bool ArMode::baseDeactivate(void)
{
  if (myRobot != NULL)
    myRobot->remUserTask(&myUserTaskCB);
  if (ourActiveMode == this)
  {
    ourActiveMode = NULL;
    return true;
  }
  return false;
}

AREXPORT const char *ArMode::getName(void)
{
  return myName.c_str();
}

AREXPORT char ArMode::getKey(void)
{
  return myKey;
}

AREXPORT char ArMode::getKey2(void)
{
  return myKey2;
}

AREXPORT void ArMode::baseHelp(void)
{
  std::list<ArMode *>::iterator it;
  ArLog::log(ArLog::Terse, "\n\nYou can do these actions with these keys:\n");
  ArLog::log(ArLog::Terse, "quit: escape");
  ArLog::log(ArLog::Terse, "help: 'h' or 'H' or '?' or '/'");
  ArLog::log(ArLog::Terse, "\nYou can switch to other modes with these keys:");
  for (it = ourModes.begin(); it != ourModes.end(); ++it)
  {
    ArLog::log(ArLog::Terse, "%30s mode: '%c' or '%c'", (*it)->getName(), 
	       (*it)->getKey(), (*it)->getKey2());
  }
  if (ourActiveMode == NULL)
    ArLog::log(ArLog::Terse, "You are in no mode currently.");
  else
  {
    ArLog::log(ArLog::Terse, "You are in '%s' mode currently.\n",
	       ourActiveMode->getName());
    ourActiveMode->help();
  }
}

AREXPORT void ArMode::addKeyHandler(int keyToHandle, ArFunctor *functor)
{
  ArKeyHandler *keyHandler;
  std::string charStr;

  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse,"ArMode '%s'::keyHandler: There should already be a key handler, but there isn't... mode won't work right.", getName());
    return;
  }

  if (!keyHandler->addKeyHandler(keyToHandle, functor))
  {
    bool specialKey = true;
    switch (keyToHandle) {
    case ArKeyHandler::UP:
      charStr = "Up";
      break;
    case ArKeyHandler::DOWN:
      charStr = "Down";
      break;
    case ArKeyHandler::LEFT:
      charStr = "Left";
      break;
    case ArKeyHandler::RIGHT:
      charStr = "Right";
      break;
    case ArKeyHandler::ESCAPE:
      charStr = "Escape";
      break;
    case ArKeyHandler::F1:
      charStr = "F1";
      break;
    case ArKeyHandler::F2:
      charStr = "F2";
      break;
    case ArKeyHandler::F3:
      charStr = "F3";
      break;
    case ArKeyHandler::F4:
      charStr = "F4";
      break;
    case ArKeyHandler::SPACE:
      charStr = "Space";
      break;
    case ArKeyHandler::TAB:
      charStr = "Tab";
      break;
    case ArKeyHandler::ENTER:
      charStr = "Enter";
      break;
    case ArKeyHandler::BACKSPACE:
      charStr = "Backspace";
      break;
    default:
      charStr = (char)keyToHandle;
      specialKey = false;
      break;
    }
    if (specialKey || (keyToHandle >= '!' && keyToHandle <= '~'))
      ArLog::log(ArLog::Terse,  
		 "ArMode '%s': The key handler has a duplicate key for '%s' so the mode may not work right.", getName(), charStr.c_str());
    else
      ArLog::log(ArLog::Terse,  
		 "ArMode '%s': The key handler has a duplicate key for number %d so the mode may not work right.", getName(), keyToHandle);
  }
  
}

AREXPORT void ArMode::remKeyHandler(ArFunctor *functor)
{
  ArKeyHandler *keyHandler;
  std::string charStr;

  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse,"ArMode '%s'::keyHandler: There should already be a key handler, but there isn't... mode won't work right.", getName());
    return;
  }
  if (!keyHandler->remKeyHandler(functor))
    ArLog::log(ArLog::Terse,  
	       "ArMode '%s': The key handler already didn't have the given functor so the mode may not be working right.", getName());
}
  
