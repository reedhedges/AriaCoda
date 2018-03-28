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

// Includes
#include "Aria.h"

// A class that just wraps the robot so that people don't forget
// to unlock and lock the robot when using direct motion commands
class ArDirectMotion
{
public:
  // Constructor
  ArDirectMotion(ArRobot *robot)
  {myRobot = robot;}

  // Destructor
  ~ArDirectMotion() {}

  // Wait until the turn times out or has been completed
  // within the required range in degrees
  void finishTurn(double timeOut, double withinDeg)
  {
    ArTime start;
    while (1)
      {
	myRobot->lock();
	if (myRobot->isHeadingDone(withinDeg))
	  {
	    printf("Finished turn\n");
	    myRobot->unlock();
	    break;
	  }
	if (start.mSecSince() > timeOut)
	  {
	    printf("Turn timed out\n");
	    myRobot->unlock();
	    break;
	  }
	myRobot->unlock();
      }
  }

  // Wait until the move times out or has been completed
  // within the required range in mm
  void finishMove(double timeOut, double withinDist)
  {
    ArTime start;
    while (1)
      {
	myRobot->lock();
	if (myRobot->isMoveDone(withinDist))
	  {
	    printf("Finished move\n");
	    myRobot->unlock();
	    break;
	  }
	if (start.mSecSince() > timeOut)
	  {
	    printf("Turn timed out\n");
	    myRobot->unlock();
	    break;
	  }
	myRobot->unlock();
      }
  }
  
  // Move a distance if that distance + the buffer space
  // is clear, otherwise do nothing
  void move(double distance, double bufferSpace)
  {
    myRobot->lock();

    if(myRobot->checkRangeDevicesCurrentBox(0, -myRobot->getRobotRadius(),
					    distance + bufferSpace, myRobot->getRobotRadius())
       >= distance + myRobot->getRobotRadius() + bufferSpace)
      {
	myRobot->move(distance);
      }
    myRobot->unlock();
  }

  // Change heading by
  void setDeltaHeading(double heading)
  {
    myRobot->lock();
    myRobot->setDeltaHeading(heading);
    myRobot->unlock();
  }

  // Set to absolute heading
  void setHeading(double degree)
  {
    myRobot->lock();
    myRobot->setHeading(degree);
    myRobot->unlock();
  }

  // Set the maximum translational velocity
  void setTransVelMax(double max)
  {
    myRobot->lock();
    myRobot->setTransVelMax(max);
    myRobot->unlock();
  }

  // Set the translational acceleration
  void setTransAccel(double acc)
  {
    myRobot->lock();
    myRobot->setTransAccel(acc);
    myRobot->unlock();
  }

  // Set the translational deceleration
  void setTransDecel(double decel)
  {
    myRobot->lock();
    myRobot->setTransDecel(decel);
    myRobot->unlock();
  }

  // Set the rotational acceleration
  void setRotAccel(double acc)
  {
    myRobot->lock();
    myRobot->setRotAccel(acc);
    myRobot->unlock();
  }

  // Set the rotational deceleration
   void setRotDecel(double decel)
  {
    myRobot->lock();
    myRobot->setRotDecel(decel);
    myRobot->unlock();
  }

  // Set the maximum rotational velocity
  void setRotVelMax(double max)
  {
    myRobot->lock();
    myRobot->setRotVelMax(max);
    myRobot->unlock();
  }

  // Set the rotational velocity
  void setRotVel(double velocity)
  {
    myRobot->lock();
    myRobot->setRotVel(velocity);
    myRobot->unlock();
  }

  // Set the translational veocity
  void setVel(double velocity)
  {
    myRobot->lock();
    myRobot->setVel(velocity);
    myRobot->unlock();
  }

  // Set the velocities of each wheel
  void setVel2(double left, double right)
  {
    myRobot->lock();
    myRobot->setVel2(left, right);
    myRobot->unlock();
  }

  // Stop the wheels
  void stop()
  {
    myRobot->lock();
    myRobot->stop();
    myRobot->unlock();
  }
  

protected:
  ArRobot *myRobot;
};



int main(int argc, char** argv)
{
  // To simply connect
  ArSimpleConnector simpleConnector(&argc, argv);

  // The robot
  ArRobot robot;

  // The key handler
  ArKeyHandler keyHandler;
  
  // Sonar
  ArSonarDevice sonarDev;
  // Laser
  //ArSick laserDev;

  //Direct Motion Commands
  ArDirectMotion motion(&robot);

  // Parse the arguments from the simple connector
  simpleConnector.parseArgs();

  // Some arguments did not parse....
  // The program fails to understand and shuts down.
  if (argc > 1)
  {    
    simpleConnector.logOptions();
    keyHandler.restore();
    exit(1);
  }

  // Initialize Aria
  Aria::init();

  // Give Aria the key handler
  Aria::setKeyHandler(&keyHandler);
  
  // Attach the key handler to the robot
  robot.attachKeyHandler(&keyHandler);

  // Add the sonar to the robot
  robot.addRangeDevice(&sonarDev);
  // Add the laser (if we have it) to the robot
  //robot.addRangeDevice(&laserDev);

 // Connect to the robot
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    keyHandler.restore();
    return 1;
  }
 
  
// Run the robot in its own thread
  robot.runAsync(false);

  //simpleConnector.setupLaser(&laserDev);
  //laserDev.runAsync();
  /*
   if (!laserDev.blockingConnect())
  {
    printf("Could not connect to SICK laser... exiting\n");
    Aria::shutdown();
    return 1;
  }
  */

  // turn on the motors
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::JOYDRIVE, 1);





  //----------------------------------------------------------------
  // The robot's settings for this run (feel free to change these)
  //----------------------------------------------------------------

  // Set the Robot's PIDs
  robot.comInt(82, 50);  // rotkp
  robot.comInt(83, 300); // rotkv
  robot.comInt(84, 10);  // rotki
  robot.comInt(85, 25);  // transkp
  robot.comInt(86, 600); // transkv
  robot.comInt(87, 10);  // transki  
 
  // Set the robot's velocities and accelerations
  motion.setTransVelMax(2999);
  motion.setRotVelMax(2999);
  motion.setTransAccel(2999);
  motion.setTransDecel(2999);
  motion.setRotAccel(2999);
  motion.setRotDecel(2999);



  //----------------------------------------------------------------
  // The robot's test pattern for this run (feel free to change this)
  //----------------------------------------------------------------

  // The test pattern described below
  printf("Executing random test pattern\n");

  // Some constants for this run
  const double TIMOUT_TIME  = 5000; // Time(msec) before a movement times out
  const double WITHIN_DIST  = 50;   // If within this(mm) of target, good enough
  const double WITHIN_DEG   = 10;   // If within this(deg) of target, good enough
  const double SPACE_BUFFER = 600;  // Extra space to give robot

  // Distances and angles for pattern
  double distance = 100;
  double angle    = 30;

  /*
    A figure 8 test pattern

  // The initial move
  motion.move(distance, SPACE_BUFFER);
  motion.finishMove(TIMOUT_TIME, WITHIN_DIST);

  while(robot.isRunning())
    {
      // Turn right
      if(!robot.isRunning())break;
      motion.setDeltaHeading(-angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);

      // Move forward
      if(!robot.isRunning())break;
      motion.move(distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);
      
      // Turn right
      if(!robot.isRunning())break;
      motion.setDeltaHeading(-angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);


      // Move forward
      if(!robot.isRunning())break;
      motion.move(distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);
      
      // Turn right
      if(!robot.isRunning())break;
      motion.setDeltaHeading(-angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);


      // Move forward
      if(!robot.isRunning())break;
      motion.move(2 * distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);

      // Turn left
      if(!robot.isRunning())break;
      motion.setDeltaHeading(angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);


      // Move forward
      if(!robot.isRunning())break;
      motion.move(distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);

      // Turn left
      if(!robot.isRunning())break;
      motion.setDeltaHeading(angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);


      // Move forward
      if(!robot.isRunning())break;
      motion.move(distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);

      // Turn left
      if(!robot.isRunning())break;
      motion.setDeltaHeading(angle);
      motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);


      // Move forward
      if(!robot.isRunning())break;
      motion.move(2 * distance, SPACE_BUFFER);
      motion.finishMove(TIMOUT_TIME, WITHIN_DIST);
    }
  */


   while(robot.isRunning())
    {
      double randomNumber = rand();

      int choice = (int)randomNumber % 3;
 
      switch(choice)
	{
	case 0: 
	  // Turn right
	  motion.setDeltaHeading(-angle);
	  motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);
	  break;
	case 1:
	  // Turn left
	  motion.setDeltaHeading(angle);
	  motion.finishTurn(TIMOUT_TIME, WITHIN_DEG);
	  break;
	case 2:
	  // Move forward
	  motion.move(distance, SPACE_BUFFER);
	  motion.finishMove(TIMOUT_TIME, WITHIN_DIST);
	default:
	  motion.stop();
	  break;
	}
    }


  // now exit
  Aria::shutdown();
  return 0;
}
























