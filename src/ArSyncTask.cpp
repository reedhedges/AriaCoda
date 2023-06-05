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
#include "Aria/ariaUtil.h"
#include "Aria/ArSyncTask.h"
#include "Aria/ArLog.h"

/**
   New should never be called to create an ArSyncTask except to create the 
   root node.  Read the detailed documentation of the class for details.
*/
AREXPORT ArSyncTask::ArSyncTask(const char *name, ArFunctor *functor,
				ArTaskState::State *state, ArSyncTask *parent)
{
  if(name) myName = name;
  myStatePointer = state;
  myFunctor = functor;
  myParent = parent;
  myIsDeleting = false;
  setState(ArTaskState::INIT);
  if (myParent != NULL)
  {
    setWarningTimeCB(parent->getWarningTimeCB());
    setNoTimeWarningCB(parent->getNoTimeWarningCB());
  }
  else
  {
    setWarningTimeCB(NULL);
    setNoTimeWarningCB(NULL);
  }
}

/**
   If you delete the task it deletes everything in its list, so to 
   delete the whole tree just delete the top one... also note that if you
   delete a node, it will remove itself from its parents list.
*/
AREXPORT ArSyncTask::~ArSyncTask()
{
  myIsDeleting = true;
  if (myParent != NULL && !myParent->isDeleting())
    myParent->remove(this);
  
  ArUtil::deleteSetPairs(myMultiMap.begin(), myMultiMap.end());  
  myMultiMap.clear();
}

AREXPORT ArTaskState::State ArSyncTask::getState()
{
  if (myStatePointer != NULL)
    return *myStatePointer;
  else
    return myState;
}

AREXPORT void ArSyncTask::setState(ArTaskState::State state)
{
  if (myStatePointer != NULL)
    *myStatePointer = state;
  else
    myState = state;
}

AREXPORT std::string ArSyncTask::getName()
{
  return myName;
}


/**
   Finds a node below (or at) this level in the tree with the given functor
   @param functor The task functor pointer to search for. Must not be NULL.
   @return The task, if found.  If not found, NULL.
*/
AREXPORT ArSyncTask *ArSyncTask::find(ArFunctor *functor)
{
  //ArSyncTask *proc;
  //std::multimap<int, ArSyncTask *>::iterator it;
  
  if (myFunctor == functor)
    return this;

  for (auto it = myMultiMap.cbegin(); it != myMultiMap.cend(); ++it)
  {
    ArSyncTask *proc = (*it).second;
    if (proc->find(functor) != NULL)
      return proc;
  }
  return NULL;
  
}

/**
   Finds a node below (or at) this level in the tree with the given name
   @param name The name of the child we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
AREXPORT ArSyncTask *ArSyncTask::find(const char *name)
{
  //ArSyncTask *proc;
  //std::multimap<int, ArSyncTask *>::iterator it;
  
  if (strcmp(myName.c_str(), name) == 0)
    return this;

  for (auto it = myMultiMap.cbegin(); it != myMultiMap.cend(); ++it)
  {
    ArSyncTask *proc = (*it).second;
    if (proc->find(name) != NULL)
      return proc;
  }
  return NULL;
  
}

/**
   Finds a child of this node with the given name
   @param name The name of the child we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
AREXPORT ArSyncTask *ArSyncTask::findNonRecursive(const char * name)
{
  //ArSyncTask *proc;
  //std::multimap<int, ArSyncTask *>::iterator it;
  
  for (auto it = myMultiMap.cbegin(); it != myMultiMap.cend(); ++it)
  {
    ArSyncTask *proc = (*it).second;
    if (strcmp(proc->getName().c_str(), name) == 0)  
      return proc;
  }
  return NULL;
}

/**
   Finds a child of this node with the given functor
   @param functor the functor we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
AREXPORT ArSyncTask *ArSyncTask::findNonRecursive(ArFunctor *functor)
{
  //ArSyncTask *proc;
  //std::multimap<int, ArSyncTask *>::iterator it;
  
  for (auto it = myMultiMap.cbegin(); it != myMultiMap.cend(); ++it)
  {
    ArSyncTask *proc = (*it).second;
    if (proc->getFunctor() == functor)
      return proc;
  }
  return NULL;
}

/**
   Creates a new task with the given name and puts the task into its 
   own internal list at the given position.  
   @param nameOfNew Name to give to the new task.
   @param position place in list to put the branch, things are run/printed in 
   the order of highest number to lowest number, no limit on numbers (other 
   than that it is an int).  ARIA uses 0 to 100 just as a convention.
   @param state Pointer to external variable to store task state in, or NULL to use a new
   internal variable instead.
*/
AREXPORT void ArSyncTask::addNewBranch(const char *nameOfNew, int position,
				       ArTaskState::State *state)
{
  ArSyncTask *proc = new ArSyncTask(nameOfNew, NULL, state, this);
  //myMultiMap.insert(std::pair<int, ArSyncTask *>(position, proc));
  using map_type = decltype(myMultiMap);
  using map_entry_type = map_type::value_type;
  myMultiMap.insert(map_entry_type{position, proc});
}

/**
   Creates a new task with the given name and puts the task into its 
   own internal list at the given position.  Sets the nodes functor so that
   it will call the functor when run is called.
   @param nameOfNew Name to give to the new task.
   @param position place in list to put the branch, things are run/printed in 
   the order of highest number to lowest number, no limit on numbers (other 
   than that it is an int).  ARIA uses 0 to 100 just as a convention.
   @param functor ArFunctor which contains the functor to invoke when run is 
   called.
   @param state Pointer to external variable to store task state in, or NULL to
   use an internal variable instead.
*/
AREXPORT void ArSyncTask::addNewLeaf(const char *nameOfNew, int position, 
				     ArFunctor *functor, 
				     ArTaskState::State *state)
{
  ArSyncTask *proc = new ArSyncTask(nameOfNew, functor, state, this);
  //myMultiMap.insert(std::pair<int, ArSyncTask *>(position, proc));
  using map_type = decltype(myMultiMap);
  using map_entry_type = map_type::value_type;
  myMultiMap.insert(map_entry_type{position, proc});
}

AREXPORT void ArSyncTask::remove(ArSyncTask *proc)
{
  std::multimap<int, ArSyncTask *>::iterator it;
  
  for (it = myMultiMap.begin(); it != myMultiMap.end(); it++)
  {
    if ((*it).second == proc)
    {
      myMultiMap.erase(it);
      return;
    }
  }
}

AREXPORT bool ArSyncTask::isDeleting()
{
  return myIsDeleting;
}

AREXPORT ArFunctor *ArSyncTask::getFunctor()
{
  return myFunctor;
}

/**
   If this node is a leaf it calls the functor for the node, if it is
   a branch it goes through all of the children in the order of
   highest position to lowest position and calls run on them.
**/
AREXPORT void ArSyncTask::run()
{
  myRunning = true;

  switch (getState()) 
  {
  case ArTaskState::SUSPEND:
  case ArTaskState::SUCCESS:
  case ArTaskState::FAILURE:
    // The task isn't running so just return  
    myRunning = true;
    return;
  case ArTaskState::INIT:
  case ArTaskState::RESUME:
  case ArTaskState::ACTIVE:
  default:
    break;
  }

  ArTime runTime;
  if (myFunctor != NULL)
    myFunctor->invoke();
  long took = runTime.mSecSince();
  assert(took >= 0);
  long warningTime = 0;
  if (myNoTimeWarningCB != NULL && !myNoTimeWarningCB->invokeR() && 
      myFunctor != NULL && myWarningTimeCB != NULL &&
      myWarningTimeCB->invokeR() > 0 && 
      (took > (warningTime = (long) myWarningTimeCB->invokeR())) )
    ArLog::log(ArLog::Normal, 
	       "Warning: Task '%s' took %ld ms to run (longer than the %ld warning time)",
	       myName.c_str(), took, warningTime);
  
  
  for (auto it = myMultiMap.rbegin(); it != myMultiMap.rend(); ++it)
  {
    myInvokingOtherFunctor = (*it).second;
    myInvokingOtherFunctor->run();
  }
  myInvokingOtherFunctor = NULL;
}

/**
   This sets a functor which will be called to find the time on the
   task such that if it takes longer than this number of ms to run a
   warning message will be issued, sets this on the children too.
**/
AREXPORT void ArSyncTask::setWarningTimeCB(ArRetFunctor<unsigned int> *functor)
{
  std::multimap<int, ArSyncTask *>::reverse_iterator it;
  myWarningTimeCB = functor;
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->setWarningTimeCB(functor);
}

/**
   This gets a functor which will be called to find the time on the
   task such that if it takes longer than this number of ms to run a
   warning message will be issued, sets this on the children too.
**/
AREXPORT ArRetFunctor<unsigned int> *ArSyncTask::getWarningTimeCB()
{
  return myWarningTimeCB;
}

/**
   This sets a functor which will be called to see if we should warn
   this time through or not.
**/
AREXPORT void ArSyncTask::setNoTimeWarningCB(ArRetFunctor<bool> *functor)
{
  std::multimap<int, ArSyncTask *>::reverse_iterator it;
  myNoTimeWarningCB = functor;
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->setNoTimeWarningCB(functor);
}

/**
   This sets a functor which will be called to see if we should warn
   this time through or not.
**/
AREXPORT ArRetFunctor<bool> *ArSyncTask::getNoTimeWarningCB()
{
  return myNoTimeWarningCB;
}


/**
   Prints the node... the defaulted depth parameter controls how far over to 
   print the data (how many tabs)... it recurses down all its children.
*/
AREXPORT void ArSyncTask::log(int depth)
{
  std::string str = "";
  for (int i = 0; i < depth; i++)
    str += "\t";
  str += myName.c_str();
  str += " (";
  ArTaskState::State state = getState();
  switch (state) 
  {
  case ArTaskState::INIT:
    str += "INIT, running)";
    break;
  case ArTaskState::RESUME:
    str += "RESUME, running)";
    break;
  case ArTaskState::ACTIVE:
    str += "ACTIVE, running)";
    break;
  case ArTaskState::SUSPEND:
    str += "SUSPEND, NOT running)";
    break;
  case ArTaskState::SUCCESS:
    str += "SUCCESS, NOT running)";
    break;
  case ArTaskState::FAILURE:
    str += "FAILURE, NOT running)";
    break;
  default:
    str += state;
    str += ", running)";
    break;
  }
  ArLog::log(ArLog::Terse, const_cast<char *>(str.c_str()));
  //std::multimap<int, ArSyncTask *>::reverse_iterator it;
  for (auto it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->log(depth + 1);
  
}


/// Returns what this is running, if anything (recurses)
AREXPORT ArSyncTask *ArSyncTask::getRunning()
{
  if (!myRunning)
    return NULL;
  else if (myInvokingOtherFunctor != NULL)
    return myInvokingOtherFunctor->getRunning();
  else if (myFunctor)
    return this;
  else
  {
    ArLog::log(ArLog::Normal, "ArSyncTask::getRunning: Tried to get running, but apparently nothing was running");
    return NULL;
  }
}
