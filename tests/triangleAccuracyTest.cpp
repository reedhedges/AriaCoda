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

ArRobot *robot;

/**
   This program will repeatedly test the ArActionTriangleDriveTo
   by backing away a random amount and direction, then using the 
   action to move forwards.  It'll pause to allow time to mark the 
   location relative to the world (using a laser pointer, for example)

 **/

int main(int argc, char **argv)
{
  // parse our args and make sure they were all accounted for
  ArSimpleConnector connector(&argc, argv);

  // the robot
  robot = new ArRobot;
  // the laser
  ArSick sick;

  Aria::init();

  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    exit(1);
  }
  
  // a key handler so we can do our key handling
  ArKeyHandler keyHandler;
  // let the global aria stuff know about it
  Aria::setKeyHandler(&keyHandler);
  // toss it on the robot
  robot->attachKeyHandler(&keyHandler);

  // add the laser to the robot
  robot->addRangeDevice(&sick);

  ArSonarDevice sonar;
  robot->addRangeDevice(&sonar);
  
  //ArLineFinder lineFinder(&sick);
  ArActionTriangleDriveTo triangleDriveTo;
  ArFunctorC<ArActionTriangleDriveTo> lineGoCB(&triangleDriveTo, 
				      &ArActionTriangleDriveTo::activate);
  //keyHandler.addKeyHandler('g', &lineGoCB);
  //keyHandler.addKeyHandler('G', &lineGoCB);
  ArFunctorC<ArActionTriangleDriveTo> lineStopCB(&triangleDriveTo, 
					&ArActionTriangleDriveTo::deactivate);
  //keyHandler.addKeyHandler('s', &lineStopCB);
  //keyHandler.addKeyHandler('S', &lineStopCB);
  ArLineFinder lineFinder(&sick);
  ArFunctorC<ArLineFinder> findLineCB(&lineFinder, 
				      &ArLineFinder::getLinesAndSaveThem);
  keyHandler.addKeyHandler('f', &findLineCB);
  keyHandler.addKeyHandler('F', &findLineCB);
  //lineFinder.setVerbose(true);

  ArActionLimiterForwards limiter("limiter", 150, 0, 0, 1.3);
  robot->addAction(&limiter, 70);
  ArActionLimiterBackwards limiterBackwards;
  robot->addAction(&limiterBackwards, 69);
  
  robot->addAction(&triangleDriveTo, 60);

  ArActionKeydrive keydrive;
  robot->addAction(&keydrive, 55);


  ArActionStop stopAction;
  robot->addAction(&stopAction, 50);
  
  // try to connect, if we fail exit
  if (!connector.connectRobot(robot))
  {
    printf("Could not connect to robot->.. exiting\n");
    Aria::shutdown();
    return 1;
  }

  robot->comInt(ArCommands::SONAR, 1);
  robot->comInt(ArCommands::ENABLE, 1);
  
  // start the robot running, true so that if we lose connection the run stops
  robot->runAsync(true);

  // now set up the laser
  connector.setupLaser(&sick);

  sick.runAsync();

  if (!sick.blockingConnect())
  {
    printf("Could not connect to SICK laser... exiting\n");
    Aria::shutdown();
    return 1;
  }

  //printf("If you press the 'g' key it'll go find a triangle, if you press 's' it'll stop.\nPress 'f' to save lines\n");
  printf("\nPress 'f' to save lines\n");
  bool lastSucceeded = false;
  int numGood = 0;
  int numBad = 0;
  // how many times to run, if this is negative it'll go forever (well
  // until it wraps at 2 billion)
  int timesToRun = -1;
  for (int i=0; (i != timesToRun) && (robot->isRunning()); i++) {
    bool done = false;
    printf("\nStarting run: %d",i);
    // move back

    if (lastSucceeded)
    {
      robot->lock();
      robot->move(-300);
      robot->unlock();
    }
    do {
      ArUtil::sleep(100);
      robot->lock();
      done = robot->isMoveDone();
      robot->unlock();
    } while (!done);
    // figure out how much to rotate
    int rotation = ArMath::random()%60;
    // make it positive or negative
    rotation = rotation - 30;
    printf("...rotating");
    robot->lock();
    robot->setDeltaHeading(rotation);
    robot->unlock();
    do {
      ArUtil::sleep(100);
      robot->lock();
      done = robot->isHeadingDone(5);
      robot->unlock();
    } while (!done);
    // figure out how much to move back
    int distance_back = ArMath::random()%1500;
    // make it at least half a meter
    if (lastSucceeded)
    {
      distance_back = -1*(distance_back + 1200);
      printf("...moving");
      robot->lock();
      robot->move(distance_back);
      robot->unlock();
      do {
	ArUtil::sleep(100);
	robot->lock();
	done = robot->isMoveDone(100);
	robot->unlock();
      } while (!done);
    }
    // activate the action
    printf("...activating\n");
    robot->lock();
    robot->clearDirectMotion();
    triangleDriveTo.activate();
    robot->unlock();
    // insert cool way to see if we're done with the driveTo
    // for now, just sleep for a while
    do { 
      ArUtil::sleep(100);
      robot->lock();
      if (triangleDriveTo.getState() == 
	  ArActionTriangleDriveTo::STATE_SUCCEEDED)
      {
	numGood++;
	printf("succeeded (made %d of %d)...", numGood, numGood + numBad);
	done = true;
	lastSucceeded = true;
      }
      else if (triangleDriveTo.getState() == 
	       ArActionTriangleDriveTo::STATE_FAILED)
      {
	numBad++;
	printf("failed (made %d of %d)...", numGood, numGood + numBad);
	done = true;
	lastSucceeded = false;
      }
      else
	done = false;
      robot->unlock();
    } while (!done);
    printf("\a");
    printf("...sleeping");
    ArUtil::sleep(3000);
    // deactivate the action
    printf("...deactivating");
    robot->lock();
    triangleDriveTo.deactivate();
    robot->unlock();
  }
  printf("\n\n");
  robot->lock();
  robot->disconnect();
  robot->unlock();
  Aria::shutdown();
  printf("\n");
  return 0;
}



