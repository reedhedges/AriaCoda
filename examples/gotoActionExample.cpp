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
#include "Aria.h"

/** @example gotoActionExample.cpp Uses ArActionGoto or ArActionGotoStraight 
   (modify code to switch) to drive the robot in a square
 
  This program will make the robot drive in a 2.5x2.5 meter square by
  setting each corner in turn as the goal for the action.
  It also uses speed limiting actions to avoid collisions. After some 
  time, it cancels the goal (and the robot stops due to a stopping action) 
  and exits.

  You can modify the code below to change the type of <tt>gotoPoseAction</tt>
  to either ArActionGotoStraight or ArActionGoto to compare these if you wish.
  (These are slightly different actions with similar interfaces; generally 
  ArActionGotoStraight is preferred, and has a few more options.)
*/

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArAnalogGyro gyro(&robot);
  ArSonarDevice sonar;
  ArRobotConnector robotConnector(&parser, &robot);
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);


  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "gotoActionExample: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "gotoActionExample: Connected to robot.");

  robot.addRangeDevice(&sonar);
  robot.runAsync(true);

  // Collision avoidance actions at higher priority
  ArActionLimiterForwards limiterAction("speed limiter near", 300, 600, 250);
  ArActionLimiterForwards limiterFarAction("speed limiter far", 300, 1100, 400);
  ArActionLimiterTableSensor tableLimiterAction;
  robot.addAction(&tableLimiterAction, 100);
  robot.addAction(&limiterAction, 95);
  robot.addAction(&limiterFarAction, 90);

  // Goto action at lower priority
  //ArActionGoto gotoPoseAction("goto");
  ArActionGotoStraight gotoPoseAction("gotostraight");
  robot.addAction(&gotoPoseAction, 50);
  
  // Stop action at lower priority, so the robot stops if it has no goal
  ArActionStop stopAction("stop");
  robot.addAction(&stopAction, 40);


  // turn on the motors, turn off amigobot sounds
  robot.enableMotors();
  robot.comInt(ArCommands::SOUNDTOG, 0);

  const int duration = 30000; //msec
  ArLog::log(ArLog::Normal, "Going to four goals in turn for %d seconds, then cancelling goal and exiting.", duration/1000);

  bool first = true;
  int goalNum = 0;
  ArTime start;
  start.setToNow();
  while (Aria::getRunning()) 
  {
    robot.lock();

    // Choose a new goal if this is the first loop iteration, or if we 
    // achieved the previous goal.
    if (first || gotoPoseAction.haveAchievedGoal())
    {
      first = false;
      goalNum++;
      if (goalNum > 4)
        goalNum = 1; // start again at goal #1

      // set our positions for the different goals
      if (goalNum == 1)
        gotoPoseAction.setGoal(ArPose(2500, 0));
      else if (goalNum == 2)
        gotoPoseAction.setGoal(ArPose(2500, 2500));
      else if (goalNum == 3)
        gotoPoseAction.setGoal(ArPose(0, 2500));
      else if (goalNum == 4)
        gotoPoseAction.setGoal(ArPose(0, 0));

      ArLog::log(ArLog::Normal, "Going to next goal at %.0f %.0f", 
		    gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
    }

    if(start.mSecSince() >= duration) {
      ArLog::log(ArLog::Normal, "%d seconds have elapsed. Cancelling current goal, waiting 3 seconds, and exiting.", duration/1000);
      gotoPoseAction.cancelGoal();
      robot.unlock();
      ArUtil::sleep(3000);
      break;
    }

    robot.unlock();
    ArUtil::sleep(100);
  }
  
  // Robot disconnected or time elapsed, shut down
  Aria::exit(0);
  return 0;
}
