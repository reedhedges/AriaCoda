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
#include <string>

#ifndef WIN32
#include <signal.h>
#endif

/*
 * This is useful as a diagnostic tool, plus it shows all the many accessor
 * methods of ArRobot for robot state. It makes the robot wander, using sonar
 * to avoid obstacles, and prints out various pieces of robot state information
 * each second. Refer to the ARIA ArRobot documentation and to your robot manual
 * (section on standard ARCOS SIP packet contents) for details on the data.
 *
 * If lasers are configured in robot parameters (most robots are configured by
 * default for typical laser accessories), and laser connection is successful,
 * it will use the laser as well as the sonar to wander.
 *
 * ARIA also contains a class called ArDataLogger which is configurable through
 * ArConfig.  You can use ArDataLogger in your applications to incorporate 
 * a similar data logging feature.
 */



/* function to display a byte as a string of 8 '1' and '0' characters. */
std::string byte_as_bitstring(char byte) 
{
  char tmp[9];
  int bit; 
  int ch;
  for(bit = 7, ch = 0; bit >= 0; bit--,ch++)
    tmp[ch] = ((byte>>bit)&1) ? '1' : '0';
  tmp[8] = 0;
  return std::string(tmp);
}

/* function to display a 2-byte int as a string of 16 '1' and '0' characters. */
std::string int_as_bitstring(ArTypes::Byte2 n) 
{
  char tmp[17];
  int bit;
  int ch;
  for(bit = 15, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[16] = 0;
  return std::string(tmp);
}

std::string charge_state_string(int state)
{
  switch(state)
  {
    case ArRobot::CHARGING_UNKNOWN:
      return "unknown";
    case ArRobot::CHARGING_NOT:
      return "not";
    case ArRobot::CHARGING_BULK:
      return "bulk";
    case ArRobot::CHARGING_FLOAT:
      return "float";
    case ArRobot::CHARGING_OVERCHARGE:
      return "over";
    default:
      return "UNREC VAL";
  }
}

/* Some events might only be detectable in one robot cycle, not over the
 * 1-second period that the main thread sleeps. This cycle callback will detect
 * those and save them in some global variables. */
bool wasLeftMotorStalled = false;
bool wasRightMotorStalled = false;
ArTypes::UByte2 cumulativeStallVal = 0;
ArTypes::UByte2 cumulativeRobotFlags = 0;
bool wasLeftIRTriggered = false;
bool wasRightIRTriggered = false;
bool wasEStopTriggered = false;

bool cycleCallback(ArRobot* robot)
{
  cumulativeStallVal |= robot->getStallValue();
  wasLeftMotorStalled = wasLeftMotorStalled || robot->isLeftMotorStalled();
  wasRightMotorStalled = wasRightMotorStalled || robot->isRightMotorStalled();
  wasEStopTriggered = wasEStopTriggered || robot->getEstop();
  wasLeftIRTriggered = wasLeftIRTriggered || (robot->hasTableSensingIR() && robot->isLeftTableSensingIRTriggered());
  wasRightIRTriggered = wasRightIRTriggered || (robot->hasTableSensingIR() && robot->isRightTableSensingIRTriggered());
  return true;
}


unsigned int encoderPacketCountPrevSec;
unsigned int encoderPacketCount;
ArTime encoderPacketTimer;

bool packetCallback(ArRobotPacket *packet)
{
  if(packet->getID() == 0x90)
  {
    // encoder packet
    ++encoderPacketCount;
    if(encoderPacketTimer.secSince() >= 1)
    {
      encoderPacketCountPrevSec = encoderPacketCount;
      encoderPacketCount = 0;
      encoderPacketTimer.setToNow();
    }
    //printf("got an encoder packet. Count=%d, CountPrevSec=%d\n", encoderPacketCount, encoderPacketCountPrevSec);
  }
  return false;  // let other packet handlers (e.g. in ArRobot) be called
}


ArActionGroup *ToggleActionGroup = NULL;
bool ToggleActionGroupActive = false;
void toggleaction(int signal)
{
  ArLog::log(ArLog::Normal, "%s action group.", ToggleActionGroupActive?"Deactivating":"Activating");
  if(ToggleActionGroupActive)
  {
    ToggleActionGroup->deactivate();
    ToggleActionGroupActive = false;
  }
  else
  {
    ToggleActionGroup->activate();
    ToggleActionGroupActive = true;
  }
}


/* main function */
int main(int argc, char **argv)
{
  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);

  // robot and devices
  ArRobot robot;
  ArSonarDevice sonar;
  ArBumpers bumpers;
  ArIRs ir;

  // the actions we'll use to wander and avoid obstacles
  ArActionStallRecover recoverAct;
  ArActionBumpers bumpAct;
  ArActionAvoidFront avoidFrontNearAct("Avoid Front Near", 225, 0);
  ArActionAvoidFront avoidFrontFarAct;
  ArActionConstantVelocity constantVelocityAct("Constant Velocity", 400);

  printf("This program will make the robot wander around, avoiding obstacles, and print some data and events.\nPress Ctrl-C to exit. Send SIGUSR1 signal to toggle wandering.\n");


  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobotConnector robotConnector(&parser, &robot);

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    ArLog::log(ArLog::Terse, "wanderAndLogData: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        Aria::logOptions();
        Aria::exit(1);
        return 1;
    }
  }

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "wanderAndLogData: Could not connect to the robot.");
    Aria::exit(2);
    return 2;
  }

  ArLog::log(ArLog::Normal, "wanderAndLogData: Connected.");

  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);
  if(!laserConnector.connectLasers())
    ArLog::log(ArLog::Normal, "Warning: unable to connect to requested lasers, will wander using robot sonar only.");
    

  // add the range devices to the robot
  robot.addRangeDevice(&sonar);
  robot.addRangeDevice(&bumpers);
  robot.addRangeDevice(&ir);
  
  // turn on the motors, turn off amigobot sound effects (for old h8-model amigobots)
  robot.enableMotors();
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions created above
  ArActionGroup wanderGroup(&robot);
  wanderGroup.addAction(&recoverAct, 100);
  wanderGroup.addAction(&bumpAct, 75);
  wanderGroup.addAction(&avoidFrontNearAct, 50);
  wanderGroup.addAction(&avoidFrontFarAct, 49);
  wanderGroup.addAction(&constantVelocityAct, 25);
  ToggleActionGroup = &wanderGroup;

#ifndef WIN32
  // can use SIGUSR1 to disable wandering (use Linux kill command)
  signal(SIGUSR1, toggleaction);
#endif

  // Cycle callback to check for events
  robot.addUserTask("checkevents", 1, new ArGlobalRetFunctor1<bool, ArRobot*>(&cycleCallback, &robot));

  // Packet callback to count packets recieved of different types
  encoderPacketCount = 0;
  encoderPacketCountPrevSec = 0;
  encoderPacketTimer.setToNow();
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&packetCallback), ArListPos::FIRST);

  // Activate the wander action
  if(!parser.checkArgument("nowander"))
  {
    ArLog::log(ArLog::Normal, "Beginning wandering actions. Send SIGUSR1 to deactivate wander.");
    wanderGroup.activate();
    ToggleActionGroupActive = true;
  }
  else
  {
    ArLog::log(ArLog::Normal, "Not activating wandering actions since -nowander option was given. Send SIGUSR1 to activate wander.");
    wanderGroup.deactivate();
    ToggleActionGroupActive = false;
  }

  // start the robot running, true means that if we lose robot connection the 
  // ArRobot runloop stops
  robot.runAsync(true);
  
  // Print data header
#define HEADFORMAT "%-24s %-5s %-16s %-5s %-6s %-6s %-16s %-8s %-8s %-8s %-8s %-8s %-8s %-10s %-10s %-5s %-5s %-5s %8s %s"
#define DATAFORMAT "%-24s %03.02f %-16s %-5s %-6s %-6s %-16s %-8d %-8d %-8g %-8g %-8s %-8s %-10lu %-10lu %-5s %-5s %-5d %6s " // doesn't include bumps details on end
  printf("\n" HEADFORMAT "\n\n",
        "Time",
        "Volts",
        "Flags",
        "EStop",
        "StallL",
        "StallR",
        "StallVal",
        "#SIP/s",
        "#Son/s",
        "Vel L",
        "Vel R",
        "DigIns",
        "DigOuts",
        "Enc L",
        "Enc R",
        "IR L",
        "IR R",
        "#Enc/s",
        "Chargestate",
        "Cur Bumps, (Last Bump Pose)"
    );

  // Request that we will want encoder data
  robot.requestEncoderPackets();

  // Print data every second
  char timestamp[24];
  while(robot.isRunning()) {
    robot.lock();
    time_t t = time(NULL);
    strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf( DATAFORMAT,
        timestamp,
        robot.getRealBatteryVoltage(),
        int_as_bitstring(cumulativeRobotFlags).c_str(),
        (wasEStopTriggered ? "YES" : "   "),
        (wasLeftMotorStalled?"YES":"   "), 
        (wasRightMotorStalled?"YES":"   "),
        int_as_bitstring(cumulativeStallVal).c_str(),
        robot.getMotorPacCount(),
        robot.getSonarPacCount(),
        robot.getLeftVel(), 
        robot.getRightVel(),
        byte_as_bitstring(robot.getDigIn()).c_str(),
        byte_as_bitstring(robot.getDigOut()).c_str(),
        robot.getLeftEncoder(),
        robot.getRightEncoder(),
        wasLeftIRTriggered?"YES": "   ",
        wasRightIRTriggered?"YES":"   ",
        encoderPacketCountPrevSec,
	charge_state_string(robot.getChargeState()).c_str()
      );

    // list indices of bumpers flaged in stallval
    // skip the last bit which is a motor stall flag
    ArTypes::UByte2 bumpmask = ArUtil::BIT15;
    int bump = 0;
    for(int bit = 16; bit > 0; bit--) 
    {
      if(bit == 9) // this is also a motor stall bit
      {
        bumpmask = bumpmask >> 1; 
        bit--;
        continue;
      }
      //printf("\n\tComparing stallval=%s to bumpmask=%s... ", int_as_bitstring(stallval).c_str(), int_as_bitstring(bumpmask).c_str());
      if(cumulativeStallVal & bumpmask)
        printf("%d ", bump);
      bumpmask = bumpmask >> 1;
      bump++;
    }

    // print pose of last bump sensor reading
    const std::list<ArPoseWithTime*>* bumpsensed = bumpers.getCurrentBuffer();
    if(bumpsensed)
    {
      //printf("%d readings. ", bumpsensed->size());
      if(bumpsensed->size() > 0 && bumpsensed->front()) {
        printf("(%.0f,%.0f)", bumpsensed->front()->getX(), bumpsensed->front()->getY());
      }
    }


    puts(""); // newline

    // clear events to accumulate for the next second
    cumulativeRobotFlags = cumulativeStallVal = 0;
    wasLeftMotorStalled = wasRightMotorStalled = wasLeftIRTriggered = wasRightIRTriggered = wasEStopTriggered = false;

    robot.unlock();
    ArUtil::sleep(1000);
  }
  
  // robot cycle stopped, probably because of lost robot connection
  Aria::exit(0); // exit program
  return 0;
}
