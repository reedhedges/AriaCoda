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
#ifndef ARACTIONGOTOSTRAIGHT_H
#define ARACTIONGOTOSTRAIGHT_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArAction.h"

/** This action sets a heading and forward velocity based on angle and distance
    from current robot position to a given goal point.  The robot will rotate towards 
    this heading and move forward at the same time.

   This action naively drives straight towards a given ArPose. The
   action stops the robot when it is within a distance threshold (Close Dist.)
   to the goal point, or alternatively, if it has gone the expected distance 
   to the goal point.  Maximum forward velocity can be set in the constructor
   or by calling setSpeed().  The velocity requested is proportional to the 
   remaining distance to the goal, but is limited to the speed set with
   setSpeed().  The threshold for considering the goal achieved may be 
   set by calling setCloseDist().

   You can give it a new goal pose with setGoal(), cancel its movement
   with cancelGoal(), and see if it got there with haveAchievedGoal().
   You can reverse the motion by setting the backwards flag to true when
   setting a new goal.  If you set the justDistance flag to true, the action
   stops after covering the calculated distance to the goal rather than 
   checking each iteration against the given Close Distance threshold.

   Normally a new heading and forward velocity will be commanded simultaneously.
   To make the robot rotate in place before forward velocity, set the
   turn threshold by calling setTurnThreshold().  This may be desired for
   goals that are only a short distance away from the robot, to avoid a large
   arc of motion.

   This action does not check sensors.  You may use a higher priority action to
   stop or otherwise override motion based on sensor data (e.g.
   ArActionLimiterForward, or you own custom action.)

   @ingroup ActionClasses
**/


class ArActionGotoStraight : public ArAction
{
public:
  AREXPORT ArActionGotoStraight(const char *name = "goto", 
				double speed = 400);
  AREXPORT virtual ~ArActionGotoStraight();

  /// Sees if the goal has been achieved
  AREXPORT bool haveAchievedGoal(void);
  /// Cancels the goal the robot has
  AREXPORT void cancelGoal(void);
  /// Sets a new goal and sets the action to go there
  AREXPORT void setGoal(ArPose goal, bool backwards = false, 
			bool justDistance = true);
  /// Sets the goal in a relative way
  AREXPORT void setGoalRel(double dist, double deltaHeading, 
			   bool backwards = false, bool justDistance = true);
  /// Gets the goal the action has
  ArPose getGoal(void) { return myGoal; }
  /// Gets whether we're using the encoder goal or the normal goal
  bool usingEncoderGoal(void) { return myUseEncoderGoal; }
  /// Sets a new goal and sets the action to go there
  AREXPORT void setEncoderGoal(ArPose encoderGoal, bool backwards = false,
			       bool justDistance = true);
  /// Sets the goal in a relative way
  AREXPORT void setEncoderGoalRel(double dist, double deltaHeading, 
				  bool backwards = false, 
				  bool justDistance = true);
  /// Gets the goal the action has
  ArPose getEncoderGoal(void) { return myEncoderGoal; }
  /// Sets the speed the action will travel to the goal at (mm/sec)
  void setSpeed(double speed) { mySpeed = speed; }
  /// Gets the speed the action will travel to the goal at (mm/sec)
  double getSpeed(void) { return mySpeed; }
  /// Sets how close we have to get if we're not in just distance mode
  void setCloseDist(double closeDist = 100) { myCloseDist = closeDist; } 
  /// Gets how close we have to get if we're not in just distance mode
  double getCloseDist(void) { return myCloseDist; }
  /// Sets whether we're backing up there or not (set in the setGoals)
  bool getBacking(void) { return myBacking; }
  
  /// If the angle at which the robot must rotate to face the goal point is
  /// greater than @a threshAngle, turn in place rather than moving forward and
  /// turning simultaneously.  This is useful when the goal is close to the robot
  /// but the robot is facing away from the goal point.
  /// @warning if threshAngle is too small (smaller than about 20 deg) then
  /// the robot may not achieve that heading, and it could become stuck with no
  /// movement. To set the speed used when turning in place,  use
  /// setMaxTurnSpeed().
  void setTurnThreshold(double threshAngle) { myTurnThresh = threshAngle; }

  /// Set speed used when turning in place (because angle difference is above
  /// turn threshold, see setTurnThreshold()). If 0, use robot's default
  /// HEAD speed, which is usually its maximum rotational velocity capability.
  /// On some robots and for some applications this may be too fast, so you 
  /// may want to reduce it here.
  void setTurnSpeed(double speed) { myMaxTurnSpeed = speed; }

  /// Set maximum rotational velocity allowed when adjusting heading while
  /// also driving forward. If 0, use default.
  /// Normally, you will want this to be the default or a high value, to
  /// quickly correct any heading deviation while driving to the goal.
  void setMaxRotVel(double rv) { myMaxRotVel = rv; }

protected:
  AREXPORT virtual ArActionDesired *fire(ArActionDesired currentDesired);
  AREXPORT virtual ArActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const ArActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  ArPose myGoal;
  bool myUseEncoderGoal;
  ArPose myEncoderGoal;
  double mySpeed;
  bool myBacking;
  ArActionDesired myDesired;
  const bool myPrinting;
  double myDist;
  double myCloseDist;

  bool myJustDist;

  double myDistTravelled;
  ArPose myLastPose;
  double myTurnThresh;
  double myMaxTurnSpeed;
  double myMaxRotVel;
  
  enum State
  {
    STATE_NO_GOAL, 
    STATE_ACHIEVED_GOAL,
    STATE_GOING_TO_GOAL
  };
  State myState;
};

#endif // ARACTIONGOTO
