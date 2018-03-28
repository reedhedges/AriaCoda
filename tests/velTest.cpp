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
  This demo starts up the robot in its own thread, then runs a bunch
  of the direct motion commands and prints the results ... note that
  if you want to stop direct motion commands and let actions take
  over, you should use clearDirectMotion, see the docs for details.
  Also note how it locks the robot before doing anything with the
  robot, then unlocks it when done, this is vital for threaded code.
  You shouldn't really use threads if you don't understand them or
  don't need them.  */

/*
  This is a connection handler, fairly simple, but quite useful, esp when
  the robot is running in another thread.  Its not really needed here
  since blockingConnect is used.  But it'd still be useful if there is an
  error after connected
*/
class ConnHandler
{
public:
  // Constructor
  ConnHandler(ArRobot *robot);
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
  // the functor callbacks
  ArFunctorC<ConnHandler> myConnectedCB;
  ArFunctorC<ConnHandler> myConnFailCB;
  ArFunctorC<ConnHandler> myDisconnectedCB;
};

/*
  The constructor, note its use of contructor chaining to initalize the 
  callbacks.
*/
ConnHandler::ConnHandler(ArRobot *robot) :
  myConnectedCB(this, &ConnHandler::connected),  
  myConnFailCB(this, &ConnHandler::connFail),
  myDisconnectedCB(this, &ConnHandler::disconnected)

{
  // set the robot poitner
  myRobot = robot;

  // add the callbacks to the robot
  myRobot->addConnectCB(&myConnectedCB, ArListPos::FIRST);
  myRobot->addFailedConnectCB(&myConnFailCB, ArListPos::FIRST);
  myRobot->addDisconnectNormallyCB(&myDisconnectedCB, ArListPos::FIRST);
  myRobot->addDisconnectOnErrorCB(&myDisconnectedCB, ArListPos::FIRST);
}

// just exit if the connection failed
void ConnHandler::connFail(void)
{
  printf("Failed to connect.\n");
  myRobot->stopRunning();
  Aria::shutdown();
  return;
}

// turn on motors, and off sonar, and off amigobot sounds, when connected
void ConnHandler::connected(void)
{
  printf("Connected\n");
  myRobot->comInt(ArCommands::SONAR, 0);
  myRobot->comInt(ArCommands::ENABLE, 1);
  myRobot->comInt(ArCommands::SOUNDTOG, 0);
}

// lost connection, so just exit
void ConnHandler::disconnected(void)
{
  printf("Lost connection\n");
  exit(0);
}



int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  ArTime start;
  
  // connection to the robot
  ArSerialConnection con;
  // the robot
  ArRobot robot;
  // the connection handler from above
  ConnHandler ch(&robot);

  // init area with a dedicated signal handling thread
  Aria::init(Aria::SIGHANDLE_THREAD);

  // open the connection with the defaults, exit if failed
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }

  // set the robots connection
  robot.setDeviceConnection(&con);
  // try to connect, if we fail, the connection handler should bail
  if (!robot.blockingConnect())
  {
    // this should have been taken care of by the connection handler
    // but just in case
    printf(
    "asyncConnect failed because robot is not running in its own thread.\n");
    Aria::shutdown();
    return 1;
  }
  // run the robot in its own thread, so it gets and processes packets and such
  robot.runAsync(false);

  // just a big long set of printfs, direct motion commands and sleeps,
  // it should be self-explanatory
  printf("Telling the robot to go 300 mm for 5 seconds\n");
  robot.lock();
  robot.setVel(500);
  robot.unlock();
  start.setToNow();
  while (1)
  {
    robot.lock();
    if (start.mSecSince() > 5000)
    {
      robot.unlock();
      break;
    }   
    printf("Trans: %10g Rot: %10g\n", robot.getVel(), robot.getRotVel());
    robot.unlock();
    ArUtil::sleep(100);
  }
  
  printf("Telling the robot to turn at 50 deg/sec for 10 seconds\n");
  robot.lock();
  robot.setVel(0);
  robot.setRotVel(50);
  robot.unlock();
  start.setToNow();
  while (1)
  {
    robot.lock();
    if (start.mSecSince() > 10000)
    {
      robot.unlock();
      break;
    }   
    printf("Trans: %10g Rot: %10g\n", robot.getVel(), robot.getRotVel());
    robot.unlock();
    ArUtil::sleep(100);
  }

  printf("Telling the robot to turn at 100 deg/sec for 10 seconds\n");
  robot.lock();
  robot.setVel(0);
  robot.setRotVel(100);
  robot.unlock();
  start.setToNow();
  while (1)
  {
    robot.lock();
    if (start.mSecSince() > 10000)
    {
      robot.unlock();
      break;
    }   
    printf("Trans: %10g Rot: %10g\n", robot.getVel(), robot.getRotVel());
    robot.unlock();
    ArUtil::sleep(100);
  }

  printf("Done with tests, exiting\n");
  robot.disconnect();
  // shutdown and ge tout
  Aria::shutdown();
  return 0;
}

