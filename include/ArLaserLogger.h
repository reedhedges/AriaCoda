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
#ifndef ARLASERLOGGER_H
#define ARLASERLOGGER_H

#include <stdio.h>

#include "ariaUtil.h"
#include "ArFunctor.h"

class ArLaser;
class ArRobot;
class ArJoyHandler;
class ArRobotJoyHandler;
class ArRobotPacket;

/// This class can be used to create log files for the laser mapper
/**
   This class has a pointer to a robot and a laser... every time the
   robot has EITHER moved the distDiff, or turned the degDiff, it will
   take the current readings from the laser and log them into the log
   file given as the fileName to the constructor.  Readings can also
   be taken by calling takeReading which explicitly tells the logger
   to take a reading.  

   The class can also add goals, see the constructor arg addGoals for
   information about that... you can also explicitly have it add a
   goal by calling addGoal.

   @see @ref LaserLogFileFormat for details on the laser scan log output file format.
**/
class ArLaserLogger
{
public:
  /// Constructor
  AREXPORT ArLaserLogger(
	  ArRobot *robot, ArLaser *laser, double distDiff, 
	  double degDiff, const char *fileName, 
	  bool addGoals = false, 
	  ArJoyHandler *joyHandler = NULL,
	  const char *baseDirectory = NULL,
	  bool useReflectorValues = false,
	  ArRobotJoyHandler *robotJoyHandler = NULL,
	  const std::map<std::string, 
	  ArRetFunctor3<int, ArTime, ArPose *, ArPoseWithTime *> *, 
	  ArStrCaseCmpOp> *extraLocationData = NULL,
	  std::list<ArLaser *> *extraLasers = NULL);
  /// Destructor
  AREXPORT virtual ~ArLaserLogger();

#ifndef SWIG
  /** @brief Adds a string to the log file with a tag at the given moment
   *  @swigomit
   */
  AREXPORT void addTagToLog(const char *str, ...);
#endif

  /// Same ass addToLog, but no varargs, wrapper for java
  AREXPORT void addTagToLogPlain(const char *str);

#ifndef SWIG
  /** @brief Adds a string to the log file without a tag for where or when we are
   *  @swigomit
   */
  AREXPORT void addInfoToLog(const char *str, ...);
#endif

  /// Same as addToInfo, but does it without marking robot position
  AREXPORT void addInfoToLogPlain(const char *str);
  /// Sets the distance at which the robot will take a new reading
  void setDistDiff(double distDiff) { myDistDiff = ArMath::fabs(distDiff); }
  /// Gets the distance at which the robot will take a new reading
  double getDistDiff(void) { return myDistDiff; }
  /// Sets the degrees to turn at which the robot will take a new reading
  void setDegDiff(double degDiff) { myDistDiff = ArMath::fabs(degDiff); }
  /// Gets the degrees to turn at which the robot will take a new reading
  double getDegDiff(void) { return myDegDiff; }
  /// Explicitly tells the robot to take a reading
  void takeReading(void) { myTakeReadingExplicit = true; }
  /// Adds a goal where the robot is at the moment
  void addGoal(void) { myAddGoalExplicit = true; }
  /// Sees if the file was opened successfully
  bool wasFileOpenedSuccessfully(void) 
    { if (myFile != NULL) return true; else return false; }
  /// Gets if we're taking old (sick1:) readings
  bool takingOldReadings(void) { return myOldReadings; }
  /// Sets if we're taking old (sick1:) readings
  void takeOldReadings(bool takeOld) { myOldReadings = takeOld; }
  /// Gets if we're taking new (scan1:) readings
  bool takingNewReadings(void) { return myNewReadings; }
  /// Sets if we're taking old (scan1:) readings
  void takeNewReadings(bool takeNew) { myNewReadings = takeNew; }
protected:
  /// The task which gets attached to the robot
  AREXPORT void robotTask(void);
  // internal function that adds goals if needed (and specified)
  void internalAddGoal(void);
  // internal function that writes tags
  void internalWriteTags(void);
  // internal function that takes a reading
  void internalTakeReading(void);
  // internal function that takes a reading from one laser
  void internalTakeLaserReading(ArLaser *laser, int laserNumber);
  // internal function that prints the position
  void internalPrintPos(ArPose encoderPoseTaken, ArPose goalPoseTaken, 
			ArTime timeTaken);
  // internal function that logs the pose and conf
  void internalPrintLaserPoseAndConf(ArLaser *laser, int laserNumber);
  // internal packet for handling the loop packets
  AREXPORT bool loopPacketHandler(ArRobotPacket *packet);


  // what type of readings we are taking
  bool myOldReadings;
  // what type of readings we are taking
  bool myNewReadings;
  // if we're taking reflector values too
  bool myUseReflectorValues;
  std::list<std::string> myTags;
  std::list<std::string> myInfos;
  bool myWrote;
  ArRobot *myRobot;
  // note that this is now in the list of lasers, but this pointer is
  // kept to denote the primary laser (so that it can always be called
  // number 1)
  ArLaser *myLaser;
  bool myAddGoals;
  ArJoyHandler *myJoyHandler;
  ArRobotJoyHandler *myRobotJoyHandler;
  std::string myFileName;
  std::string myBaseDirectory;
  FILE *myFile;
  bool myFirstTaken;

  ArPose myLast;
  double myLastVel;
  double myDistDiff;
  double myDegDiff;
  ArSectors mySectors;
  ArFunctorC<ArLaserLogger> myTaskCB;
  int myScanNumber;
  ArTime myStartTime;
  bool myTakeReadingExplicit;
  bool myAddGoalExplicit;
  bool myAddGoalKeyboard;
  bool myLastAddGoalKeyboard;
  bool myLastJoyButton;
  bool myLastRobotJoyButton;
  bool myFirstGoalTaken;
  int myNumGoal;
  ArPose myLastGoalTakenPose;
  ArTime myLastGoalTakenTime;

  void goalKeyCallback(void);
  unsigned char myLastLoops;
  
  // the lasers all have this, so shouldn't need it anymore...
  //bool myFlipped;
  
  bool myIncludeRawEncoderPose;
  std::map<std::string, ArRetFunctor3<int, ArTime, ArPose *, ArPoseWithTime *> *, 
	   ArStrCaseCmpOp> myExtraLocationData;
  
  // holders for the extra lasers 
  std::list<ArLaser *> myLasers;

  ArFunctorC<ArLaserLogger> myGoalKeyCB;
  ArRetFunctor1C<bool, ArLaserLogger, ArRobotPacket *> myLoopPacketHandlerCB;
};

/// @deprecated
typedef ArLaserLogger ArSickLogger;

#endif // ARLASERLOGGER_H
