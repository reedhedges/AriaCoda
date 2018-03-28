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
  Takes two or three arguments:

  peoplebotTest <wanderTime> <restingTime> <hostname to export ACTS to>

  This program will just have the robot wander around for a specificed period of
  time, and then rest for a period of time.  While wandering it runs acts
  and pipes it the the machine name listed as the third argument.  Otherwise
  it defaults to try to pipe to prod.local.net.

  It pings the front sonar when moving forwards, and the read sonar when moving
  backwards.  It assumes that the robot is a Performance Peoplbot, and has three
  rings of sonar.
  
  It uses some avoidance routines, then just has a constant velocity when
  wandering.

  You can press escape while it was running to cause the program to
  close up and exit.  Otherwise it exits when the batteries dies, and then
  displays the total runtime, and the time spent driving.
*/

class PeoplebotTest
{
  public:
    PeoplebotTest(ArRobot *robot, int wanderTime, int restTime, std::string hostname);
    ~PeoplebotTest(void);

  private:
    ArRobot *myRobot;
    ArTime myStateTime;

    enum State {
      IDLE,
      WANDERING,
      RESTING,
      OTHER
    };

    ArFunctorC<PeoplebotTest> myPeoplebotTestCB;
    State myState;
    void userTask(void);
    bool timeout(int mSec);
    int myWanderingTimeout;
    int myRestingTimeout;
    int myTotalRestTime;
    int myTotalWanderTime;
    int myTotalRunTime;
    std::string myHostname;
    std::string myCmd;
    int mySonar; // state of sonar -  -1 for rear, 0 for none, 1 for front
    ArActionConstantVelocity *myConstantVelocity;
};

PeoplebotTest::PeoplebotTest(ArRobot *robot, int wanderTime, int restTime, std::string hostname) :
myPeoplebotTestCB(this, &PeoplebotTest::userTask)
{
  myRobot = robot;
  myConstantVelocity = new ArActionConstantVelocity();

  if (myRobot != NULL)
    myRobot->addUserTask("peoplebotTest", 100, &myPeoplebotTestCB);

  myStateTime.setToNow();
  
  myState = IDLE;
  mySonar = 0;

  myTotalRestTime = 0;
  myTotalRunTime = 0;
  myTotalWanderTime = 0;

  // set the timeouts.  These are minutes
  myWanderingTimeout = wanderTime;
  myRestingTimeout = restTime;

  myHostname = hostname;
}

PeoplebotTest::~PeoplebotTest(void)
{
  if (myRobot != NULL)
    myRobot->remUserTask(&myPeoplebotTestCB);

  if (myState == WANDERING)
    myTotalWanderTime += (int)myStateTime.mSecSince()/1000/60;
  else if (myState == RESTING)
    myTotalRestTime += (int)myStateTime.mSecSince()/1000/60;

  myTotalRunTime = myTotalWanderTime + myTotalRestTime;
  if (myTotalRunTime != 0)
  {
    printf("Percent wander time - %.2f%% \n", myTotalWanderTime*100.0/myTotalRunTime);
    printf("Total run time - %d minutes\n", myTotalRunTime);
  }
  printf("Killing acts\n");
  system("killall -9 acts &> /dev/null");
}

bool PeoplebotTest::timeout(int min)
{
  return (myStateTime.mSecSince() > 60*1000*min);
}

void PeoplebotTest::userTask(void)
{
  switch (myState)
  {
    case IDLE:
      // start wandering
      printf("Starting to wander for the first time\n");
      myStateTime.setToNow();
      myState = WANDERING;
      myRobot->addAction(myConstantVelocity, 25);
      printf("Opening up ACTS\n");
      myCmd = "DISPLAY=";
      myCmd += myHostname.c_str();
      myCmd += ":0; /usr/local/acts/bin/acts -G bttv -n 0 &> /dev/null &";
      system(myCmd.c_str());
      break;
    case WANDERING:
      if (timeout(myWanderingTimeout))
      {
	myRobot->comInt(ArCommands::SONAR,0);
	myRobot->remAction(myConstantVelocity);
	myRobot->setVel(0);
	myRobot->setRotVel(0);
	myState = RESTING;
	mySonar = 0;
	printf("Going to rest now\n");
	printf("Killing ACTS\n");
	system("killall -9 acts &> /dev/null");
	myStateTime.setToNow();
	myTotalWanderTime += myWanderingTimeout;
      }
      else if (myRobot->getVel() > 0 && mySonar != 1)
      {
	// ping front sonar
	//printf("Enabling front sonar\n");
	mySonar = 1;
	myRobot->comInt(ArCommands::SONAR, 0);
	myRobot->comInt(ArCommands::SONAR, 1);
	myRobot->comInt(ArCommands::SONAR, 4);
      }
      else if (myRobot->getVel() < 0 && mySonar != -1)
      {
	// ping rear sonar
	//printf("Enabling rear sonar\n");
	mySonar = -1;
	myRobot->comInt(ArCommands::SONAR, 0);
	myRobot->comInt(ArCommands::SONAR, 5);
      }
      break;
    case RESTING:
      if (timeout(myRestingTimeout))
      {
	printf("Going to wander now\n");
	myState = WANDERING;
	myStateTime.setToNow();
	myTotalRestTime += myRestingTimeout;
	myRobot->clearDirectMotion();
	myRobot->addAction(myConstantVelocity, 25);
	printf("Opening up ACTS\n");
	myCmd = "DISPLAY=";
	myCmd += myHostname.c_str();
	myCmd += ":0; /usr/local/acts/bin/acts -G bttv -n 0 &> /dev/null &";
	system(myCmd.c_str());
      }
      break;
    case OTHER:
    default:
      break;
  };
}
  


int main(int argc, char **argv)
{
  int ret;
  std::string str;
  // the serial connection (robot)
  ArSerialConnection serConn;
  // tcp connection (sim)
  ArTcpConnection tcpConn;
  // the robot
  ArRobot robot;
  // the laser
  ArSick sick;
  // the laser connection
  ArSerialConnection laserCon;

  bool useSimForLaser = false;


  std::string hostname = "prod.local.net";

  // timeouts in minutes
  int wanderTime = 0;
  int restTime = 0;


  // check arguments
  if (argc == 3 || argc == 4)
  {
    wanderTime = atoi(argv[1]);
    restTime = atoi(argv[2]);
    if (argc == 4)
      hostname = argv[3];
  }
  else
  {
    printf("\nUsage:\n\tpeoplebotTest <wanderTime> <restTime> <hostname>\n\n");
    printf("Times are in minutes.  Hostname is the machine to pipe the ACTS display to\n\n");
    wanderTime = 15;
    restTime = 45;
  }

  printf("Wander time - %d minutes\nRest time - %d minutes\n", wanderTime, restTime);
  printf("Sending display to %s.\n\n", hostname.c_str());

  // sonar, must be added to the robot
  ArSonarDevice sonar;

  // the actions we'll use to wander
  ArActionStallRecover recover;
  ArActionBumpers bumpers;
  ArActionAvoidFront avoidFrontNear("Avoid Front Near", 225, 0);
  ArActionAvoidFront avoidFrontFar;

  // Make a key handler, so that escape will shut down the program
  // cleanly
  ArKeyHandler keyHandler;

  // mandatory init
  Aria::init();

  // Add the key handler to Aria so other things can find it
  Aria::setKeyHandler(&keyHandler);

  // Attach the key handler to a robot now, so that it actually gets
  // some processing time so it can work, this will also make escape
  // exit
  robot.attachKeyHandler(&keyHandler);


  // First we see if we can open the tcp connection, if we can we'll
  // assume we're connecting to the sim, and just go on...  if we
  // can't open the tcp it means the sim isn't there, so just try the
  // robot

  // modify this next line if you're not using default tcp connection
  tcpConn.setPort();

  // see if we can get to the simulator  (true is success)
  if (tcpConn.openSimple())
  {
    // we could get to the sim, so set the robots device connection to the sim
    printf("Connecting to simulator through tcp.\n");
    robot.setDeviceConnection(&tcpConn);
  }
  else
  {
    // we couldn't get to the sim, so set the port on the serial
    // connection and then set the serial connection as the robots
    // device

    // modify the next line if you're not using the first serial port
    // to talk to your robot
    serConn.setPort();
    printf(
      "Could not connect to simulator, connecting to robot through serial.\n");
    robot.setDeviceConnection(&serConn);
  }
  
  
  // add the sonar to the robot
  robot.addRangeDevice(&sonar);

  // add the laser
  robot.addRangeDevice(&sick);

  // try to connect, if we fail exit
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  // turn on the motors, turn off amigobot sounds
  //robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // turn off the sonar to start with
  robot.comInt(ArCommands::SONAR, 0);

  // add the actions
  robot.addAction(&recover, 100);
  robot.addAction(&bumpers, 75);
  robot.addAction(&avoidFrontNear, 50);
  robot.addAction(&avoidFrontFar, 49);
  
  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  if (!useSimForLaser)
  { 
    sick.setDeviceConnection(&laserCon);

    if ((ret = laserCon.open("/dev/ttyS2")) != 0)
    {
      str = tcpConn.getOpenMessage(ret);
      printf("Open failed: %s\n", str.c_str());
      Aria::shutdown();
      return 1;
    }
    sick.configureShort(false);
  }
  else 
  {
    sick.configureShort(true);
  }

  sick.runAsync();

  if (!sick.blockingConnect())
  {
    printf("Could not connect to SICK laser... exiting\n");
    Aria::shutdown();
    return 1;
  }

  robot.lock();
  robot.comInt(ArCommands::ENABLE, 1);
  robot.unlock();

  // add the peoplebot test
  PeoplebotTest pbTest(&robot, wanderTime, restTime, hostname);

  robot.waitForRunExit();

  // now exit
  Aria::shutdown();
  return 0;
}
