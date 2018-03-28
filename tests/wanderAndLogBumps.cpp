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

/*
 * This is useful as a diagnostic tool for the bumpers, motor stalls, table IR
 * (peoplebot) and e-stop events.
 * It makes the robot wander, using sonar to avoid obstacles, if an event happens
 * it is printed to standard output with a timestamp. 
 * Refer to the ARIA ArRobot documentation and to your robot manual
 * (section on standard ARCOS SIP packet contents) for details on the stallval
 * format.
 *
 * Note, it lists bumpers by the bit in the stallval that was set, not
 * neccesarily by other bumper numbering conventions (depending on what kind
 * of robot it is).  E.g. on a DX, the front bumper ring triggers bits 2-6 from
 * right side to left side.  A PowerBot triggers bits 0-7.
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
std::string int_as_bitstring(int16_t n) 
{
  char tmp[17];
  int bit;
  int ch;
  for(bit = 15, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[16] = 0;
  return std::string(tmp);
}


/* Things that might only be evident in one robot cycle, so are set by a cycle
 * callback then cleared after being printed. */
bool wasLeftMotorStalled = false;
bool wasRightMotorStalled = false;
ArTypes::UByte2 cumulativeStallVal = 0;
ArTypes::UByte2 cumulativeRobotFlags = 0;
bool wasLeftIRTriggered = false;
bool wasRightIRTriggered = false;
bool wasEStopTriggered = false;

/* Robot cycle callback that accumulates positive states over several cycles
 * until the main thread gets around to checking and printing them out (since it
 * only logs once a second). */
bool cycleCallback(ArRobot* robot)
{
  cumulativeStallVal |= robot->getStallValue();
  cumulativeRobotFlags |= robot->getFlags();
  wasLeftMotorStalled = wasLeftMotorStalled || robot->isLeftMotorStalled();
  wasRightMotorStalled = wasRightMotorStalled || robot->isRightMotorStalled();
  wasEStopTriggered = wasEStopTriggered || robot->getEstop();
  wasLeftIRTriggered = wasLeftIRTriggered || (robot->hasTableSensingIR() && robot->isLeftTableSensingIRTriggered());
  wasRightIRTriggered = wasRightIRTriggered || (robot->hasTableSensingIR() && robot->isRightTableSensingIRTriggered());
  return true;
}

/* main function */
int main(int argc, char **argv)
{
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

  // initialize aria and aria's logging destination and level
  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);

  // connector
  ArSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    exit(1);
  }

  printf("This program will make the robot wander around, avoiding obstacles with the sonar if possible, and print stall, bumper, e-stop and IR (Peoplebot) if and when they happen.\nPress Ctrl-C to exit.\n");
  
  // add the range devices to the robot
  robot.addRangeDevice(&sonar);
  robot.addRangeDevice(&bumpers);
  robot.addRangeDevice(&ir);
  
  // try to connect, if we fail exit
  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  // turn on the motors, turn off amigobot sound effects (for old h8-model amigobots)
  robot.enableMotors();
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions created above
  robot.addAction(&recoverAct, 100);
  robot.addAction(&bumpAct, 75);
  robot.addAction(&avoidFrontNearAct, 50);
  robot.addAction(&avoidFrontFarAct, 49);
  robot.addAction(&constantVelocityAct, 25);

  // Cycle callback to check for events
  robot.addUserTask("checkevents", 1, new ArGlobalRetFunctor1<bool, ArRobot*>(&cycleCallback, &robot));


  // start the robot running, true means that if we lose robot connection the 
  // ArRobot runloop stops
  robot.runAsync(true);
  
  // Print data header
#define HEADFORMAT "%-24s %-16s %-6s %-6s %-6s %-6s %-6s %s"
#define DATAFORMAT "%-24s %-16s %-6s %-6s %-6s %-6s %-6s" // doesn't include bumps details on end
  printf("\n" HEADFORMAT "\n\n",
        "Time",
        "StallVal",
        "StallL",
        "StallR",
        "EStop",
        "IR L",
        "IR R",
        "Bumps Triggered"
    );

  // Print data every second
  char timestamp[24];
  while(robot.isRunning()) {
    robot.lock();
    if(cumulativeStallVal || wasEStopTriggered || wasLeftIRTriggered || wasRightIRTriggered)
    {
      time_t t = time(NULL);
      strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&t));
      printf( DATAFORMAT,
          timestamp,
          int_as_bitstring(cumulativeStallVal).c_str(),
          (wasLeftMotorStalled?"YES":"   "), 
          (wasRightMotorStalled?"YES":"   "),
          (wasEStopTriggered ? "YES" : "   "),
          (wasLeftIRTriggered ? "YES" : "   " ),
          (wasRightIRTriggered ? "YES" : "   " )
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

      puts(""); // newline
    }

    // clear values to accumulate for the next second
    cumulativeStallVal = 0;
    wasLeftMotorStalled = wasRightMotorStalled = wasLeftIRTriggered = wasRightIRTriggered = wasEStopTriggered = false;

    robot.unlock();
    ArUtil::sleep(1000);
  }
  
  // robot cycle stopped, probably because of lost robot connection
  Aria::shutdown();
  return 0;
}
