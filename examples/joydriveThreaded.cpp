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

/*
  This program just drives the robot around with a joystick.

  This example shows an example of a program making a thread for its own use 
  (reading the joystick and driving the robot), having the robot run in its
  own thread, and then keeping its main thread to itself.  Demonstrates the
  thread locking that must be done for threads to work safely.  If you don't
  know or understand threading, or you don't need threading, you probably 
  shouldn't do it this way, as it is more complicated.
*/

/*
  This class creates its own thread, and then runs in the thread, controlling
  the robot with the joystick.
*/
class Joydrive : public ArASyncTask
{
public:
  // constructor
  Joydrive(ArRobot *robot);
  // empty destructor
  ~Joydrive(void) {}

  // the function to run in the new thread, this just is called once, so 
  // only return when you want th ethread to exit
  virtual void * runThread(void *arg);

protected:
  // joystick handler
  ArJoyHandler myJoyHandler;
  // robot pointer
  ArRobot *myRobot;
};

// a nice simple constructor
Joydrive::Joydrive(ArRobot *robot)
{
  setThreadName("Joydrive");
  // set the robot pointer
  myRobot = robot;
  // initialize the joystick
  myJoyHandler.init();
  // set up the joystick so we'll get the speeds out we want
  myJoyHandler.setSpeeds(40, 700);

  // see if we have a joystick, and let the users know
  if (myJoyHandler.haveJoystick())
  {
    printf("Have a joystick\n\n");
  }
  // if we don't have a joystick, then print error message and exit
  else
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    Aria::exit(1);  // exit program with error code 1
  }

  // this is what creates are own thread, its from the ArASyncTask
  create();
}

// this is the function called in the new thread
void *Joydrive::runThread(void *arg)
{
  threadStarted();

  int trans, rot;

  // only run while running, ie play nice and pay attention to the thread 
  //being shutdown
  while (myRunning)
  {
    // lock the robot before touching it
    myRobot->lock();
    if (!myRobot->isConnected())
    {
      myRobot->unlock();
      break;
    }
    // print out some information about the robot
    printf("\rx %6.1f  y %6.1f  tth  %6.1f vel %7.1f mpacs %3d   ", 
	   myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	   myRobot->getVel(), myRobot->getMotorPacCount());
    fflush(stdout);
    // if one of the joystick buttons is pushed, drive the robot
    if (myJoyHandler.haveJoystick() && (myJoyHandler.getButton(1) ||
					myJoyHandler.getButton(2)))
    {
      // get out the values from the joystick
      myJoyHandler.getAdjusted(&rot, &trans);
      // drive the robot
      myRobot->setVel(trans);
      myRobot->setRotVel(-rot);
    }
    // if no buttons are pushed stop the robot
    else
    {
      myRobot->setVel(0);
      myRobot->setRotVel(0);
    }
    // unlock the robot, so everything else can run
    myRobot->unlock();
    // now take a little nap
    ArUtil::sleep(50);
  }
  // return out here, means the thread is done
  return NULL;
}

/*
  This is a connection handler, fairly simple, but quite useful, esp when
  the robot is running in another thread. 
*/
class ConnHandler
{
public:
  // constructor
  ConnHandler(ArRobot *robot, Joydrive *jd);
    // Destructor, its just empty
  ~ConnHandler(void) {}
  // to be called if the connection was made
  void connected(void);
  // to call if the connection failed
  void connFail(void);
  // to be called if the connection was lost
  void disconnected(void);
protected:
  // robot pointer
  ArRobot *myRobot;
  // pointer to joydrive
  Joydrive *myJoydrive;
  // the functor callbacks
  ArFunctorC<ConnHandler> *myConnectedCB;
  ArFunctorC<ConnHandler> *myConnFailCB;
  ArFunctorC<ConnHandler> *myDisconnectedCB;

};

// the mythical constructor
ConnHandler::ConnHandler(ArRobot *robot, Joydrive *jd) 
{
  // set the pointers
  myRobot = robot;
  myJoydrive = jd;

  // now create the functor callbacks, then set them on the robot
  myConnectedCB = new ArFunctorC<ConnHandler>(this, &ConnHandler::connected);
  myRobot->addConnectCB(myConnectedCB, ArListPos::FIRST);
  myConnFailCB = new ArFunctorC<ConnHandler>(this, &ConnHandler::connFail);
  myRobot->addFailedConnectCB(myConnFailCB, ArListPos::FIRST);
  myDisconnectedCB = new ArFunctorC<ConnHandler>(this, 
						 &ConnHandler::disconnected);
  myRobot->addDisconnectNormallyCB(myDisconnectedCB, ArListPos::FIRST);
  myRobot->addDisconnectOnErrorCB(myDisconnectedCB, ArListPos::FIRST);
}

// when we connect turn off the sonar, turn on the motors, and disable amigobot
// sound
void ConnHandler::connected(void)
{
  myRobot->comInt(ArCommands::SONAR, 0);
  myRobot->comInt(ArCommands::ENABLE, 1);
  myRobot->comInt(ArCommands::SOUNDTOG, 0);
}

// just exit if we failed to connect
void ConnHandler::connFail(void)
{
  printf("Failed to connect.\n");
  myRobot->stopRunning();
  myJoydrive->stopRunning();
  Aria::exit(2);  // exit program with error code 2
}

// if we lost connection then exit
void ConnHandler::disconnected(void)
{
  printf("Lost connection\n");
  myRobot->stopRunning();
  myJoydrive->stopRunning();
  Aria::exit(3);  // exit program with error code 3
}

int main(int argc, char **argv) 
{
  std::string str;
  int ret;

  // connection to the robot
  ArTcpConnection con;
  // the robot
  ArRobot robot;
  
  // ake the joydrive object, which also creates its own thread
  Joydrive joyd(&robot);
  
  // the connection handler
  ConnHandler ch(&robot, &joyd);

  // init aria, which will make a dedicated signal handling thread
  Aria::init(Aria::SIGHANDLE_THREAD);

  // open the connection with default args, exit if it fails
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }


  // set the connection on the robot
  robot.setDeviceConnection(&con);

  // run the robot in its own thread
  robot.runAsync(false);
  
  // have the robot connect asyncronously (so its loop is still running)
  // if this fails it means that the robot isn't running in its own thread
  if (!robot.asyncConnect())
  {
    printf(
    "asyncConnect failed because robot is not running in its own thread.\n");
    Aria::shutdown();
    return 1;
  }

  // now we just wait for the robot to be done running
  printf("Waiting for the robot's run to exit.\n");
  robot.waitForRunExit();
  // then we exit
  printf("exiting main\n");
  Aria::exit(0);  // exit program
  return 0;
}


