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

/** @example actsColorFollowingExample.cpp Connects to the ACTS program, and
 * uses an ArAction subclass to drive the robot towards the largest detected
 * blob.
 */

#include "Aria.h"



// Chase is an action that moves the robot toward the largest blob that
// appears in it's current field of view.
class Chase : public ArAction
{
  
public:
  
  // The state of the chase action
  enum State {
    NO_TARGET,      // There is no target in view
    TARGET,         // This is a target in view
  };

  // Constructor
  Chase(ArACTS_1_2 *acts, ArVCC4 *camera);
  
  // Destructor
  ~Chase(void);
  
  // The action
  ArActionDesired *fire(ArActionDesired currentDesired);

  // Set the ACTS channel that we want to get blob info from
  bool setChannel(int channel);

  // Return the current state of this action
  State getState(void) { return myState; }

  // Height and width of pixels from frame-grabber
  enum {
    WIDTH = 160,
    HEIGHT = 120
  };

protected:
  ArActionDesired myDesired;
  ArACTS_1_2 *myActs;
  ArVCC4 *myCamera;
  ArTime myLastSeen;
  State myState;
  int myChannel;
  int myMaxTime;
};


// Constructor: Initialize the chase action
Chase::Chase(ArACTS_1_2 *acts, ArVCC4 *camera) :
    ArAction("Chase", "Chases the largest blob.")
{
  myActs = acts;
  myCamera = camera;
  myChannel = 0;
  myState = NO_TARGET;
  setChannel(1);
  myLastSeen.setToNow();
  myMaxTime = 1000;
}

// Destructor
Chase::~Chase(void) {}


// The chase action
ArActionDesired *Chase::fire(ArActionDesired currentDesired)
{
  ArACTSBlob blob;
  ArACTSBlob largestBlob;

  bool flag = false;

  int numberOfBlobs;
  int blobArea = 10;

  double xRel, yRel;

  // Reset the desired action
  myDesired.reset();
  
  numberOfBlobs = myActs->getNumBlobs(myChannel);

  // If there are blobs to be seen, set the time to now
  if(numberOfBlobs != 0)
  {
    for(int i = 0; i < numberOfBlobs; i++)
    {
      myActs->getBlob(myChannel, i + 1, &blob);
      if(blob.getArea() > blobArea)
      {
        flag = true;
        blobArea = blob.getArea();
        largestBlob = blob;
      }
    }

    myLastSeen.setToNow();
  }

  // If we have not seen a blob in a while...
  if (myLastSeen.mSecSince() > myMaxTime)
  {
    if(myState != NO_TARGET) ArLog::log(ArLog::Normal, "Target Lost");
    myState = NO_TARGET;
  }
  else
  {
    // If we see a blob and haven't seen one before..
    if(myState != TARGET) {
      ArLog::log(ArLog::Normal, "Target Aquired");
      ArLog::log(ArLog::Normal, "(Using channel %d with %d blobs)", myChannel, numberOfBlobs);
    }
    myState = TARGET;
  }

  if(myState == TARGET && flag == true)
  { 
    // Determine where the largest blob's center of gravity
    // is relative to the center of the camera
    xRel = (double)(largestBlob.getXCG() - WIDTH/2.0)  / (double)WIDTH;
    yRel = (double)(largestBlob.getYCG() - HEIGHT/2.0) / (double)HEIGHT;
      
    // Tilt the camera toward the blob
    if(!(ArMath::fabs(yRel) < .20))
    {
      if (-yRel > 0)
        myCamera->tiltRel(1);
      else
        myCamera->tiltRel(-1);
    }

    // Set the heading and velocity for the robot
    if (ArMath::fabs(xRel) < .10)
    {
      myDesired.setDeltaHeading(0);
    }
    else
    {
      if (ArMath::fabs(-xRel * 10) <= 10)
        myDesired.setDeltaHeading(-xRel * 10);
      else if (-xRel > 0)
        myDesired.setDeltaHeading(10);
      else
        myDesired.setDeltaHeading(-10);
     
    }

    myDesired.setVel(200);
    return &myDesired;    
  }

  // If we have no target, then don't set any action and let lower priority
  // actions (e.g. stop) control the robot.
  return &myDesired;
}

// Set the channel that the blob info will be obtained from
bool Chase::setChannel(int channel)
{
  if (channel >= 1 && channel <= ArACTS_1_2::NUM_CHANNELS)
  {
    myChannel = channel;
    return true;
  }
  else
    return false;
}




// Callback to enable/disable the keyboard driving action
void toggleAction(ArAction* action)
{
  if(action->isActive()) {
    action->deactivate();
    ArLog::log(ArLog::Normal, "%s action is now deactivated.", action->getName());
  }
  else {
    action->activate();
    ArLog::log(ArLog::Normal, "%s action is now activated.", action->getName());
  }
}



// Main function
int main(int argc, char** argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

  // Sonar for basic obstacle avoidance
  ArSonarDevice sonar;

  // The camera (Cannon VC-C4) PTZ control
  // TODO replace with ArPTZConnector for other camera types
  ArVCC4 vcc4 (&robot);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "actsColorFollowingExample: Could not connect to the robot.");
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

  ArLog::log(ArLog::Normal, "actsColorFollowingExample: Connected to robot.");

  robot.runAsync(true);

  // Connect to laser(s) as defined in parameter files.
  // (Some flags are available as arguments to connectLasers() to control error behavior and to control which lasers are put in the list of lasers stored by ArRobot. See docs for details.)
  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "Warning: Could not connect to configured lasers. ");
  }

  // A key handler to take input from keyboard
  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);

  // ACTS, for tracking blobs of color
  ArACTS_1_2 acts;

  // Robot motion limiter actions (if obstacles are detected by sonar or laser)
  ArActionLimiterForwards limiter("speed limiter near", 350, 800, 200);
  ArActionLimiterForwards limiterFar("speed limiter far", 400, 1250, 300);
  ArActionLimiterBackwards backwardsLimiter;
  ArActionConstantVelocity stop("stop", 0);
  //ArActionConstantVelocity backup("backup", -200);
  

  // The color following action, defined above
  Chase chase(&acts, &vcc4);

  // Keyboard teleoperation action
  ArActionKeydrive keydriveAction;

  // Use the "a" key to activate/deactivate keydrive mode
  keyHandler.addKeyHandler('a', new ArGlobalFunctor1<ArAction*>(&toggleAction, &keydriveAction));

  // Let Aria know about the key handler
  Aria::setKeyHandler(&keyHandler);

  // Add the key handler to the robot
  robot.attachKeyHandler(&keyHandler);

  robot.lock();

  // Add the sonar to the robot
  robot.addRangeDevice(&sonar);

  // Open a connection to ACTS
  if(!acts.openPort(&robot)) 
  {
    ArLog::log(ArLog::Terse, "Error: Could not connect to ACTS... exiting.");
    keyHandler.restore();
    Aria::exit(2);
  }
  

  // Initialize the camera
  vcc4.init();

  // Artificially keep the robot from going too fast
  robot.setAbsoluteMaxTransVel(400);

  // Enable the motors
  robot.comInt(ArCommands::ENABLE, 1);
  
  // Turn off the amigobot sounds
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // Add the actions to the robot in descending order of importance.
  robot.addAction(&limiter, 7);
  robot.addAction(&limiterFar, 6);
  robot.addAction(&backwardsLimiter, 5);
  robot.addAction(&keydriveAction, 4);
  robot.addAction(&chase, 3);
  robot.addAction(&stop, 1);

  robot.unlock();

  // Start with keydrive action disabled. Use the 'a' key to turn it on.
  keydriveAction.deactivate();

  // Run the robot processing cycle until the connection is lost
  ArLog::log(ArLog::Normal, "Running. Train ACTS to detect a color to drive towards an object, or use 'a' key to switch to keyboard driving mode.");
  
  robot.waitForRunExit();

  Aria::exit(0);
  return 0;
}

