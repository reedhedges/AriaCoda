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

/* This program monitors the time it takes to receive a new value
 * for Sonar #3.
 *
 * (Or, you can uncomment code in main() to experiment with different
 * sonar polling sequences).
 */


class SonarMonitor
{
private:
  ArRobot* myRobot;
  int mySonarNum;
  ArTime myTimer;
  ArFunctorC<SonarMonitor> myCB;

public:

  SonarMonitor(ArRobot* r, int s) : 
    myRobot(r), mySonarNum(s), myCB(this, &SonarMonitor::cycleCallback)
  {
    ArLog::log(ArLog::Normal, "Sonar Monitor: Monitoring sonar transducer #%d", s);
    myTimer.setToNow();
    myRobot->addSensorInterpTask("SonarMonitor", 1, &myCB);
  }

  void cycleCallback()
  {
    if(myRobot->isSonarNew(mySonarNum))
    {
      ArLog::log(ArLog::Normal, "Sonar Monitor: It took %d ms for sonar #%d to be refreshed (range value=%d)",
        myTimer.mSecSince(), mySonarNum, myRobot->getSonarRange(mySonarNum));
      myTimer.setToNow();
    }
  }
};

int main(int argc, char **argv)
{
  ArRobot robot;
  Aria::init();
  ArSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    return 1;
  }
  
  if (!connector.connectRobot(&robot))
  {
    ArLog::log(ArLog::Terse, "Sonar Monitor: Error: Could not connect to robot... exiting\n");
    return 2;
  }

  ArLog::log(ArLog::Normal, "Sonar Monitor: Connected.\n");

  // Check the state of sonar index 1 (the second sonar) every cycle
  SonarMonitor sonarMon(&robot, 1);

  // Check the state of sonar index 4 (the fifth sonar) every cycle
  SonarMonitor sonarMon2(&robot, 4);

  // You can experiment with different polling sequences by commenting
  // out one of the following commands:
  
  /* 
   * Change the polling sequence to only enable one sonar (#0 if you are
   * counting from 0 like ARIA, but 1 counting from 1 like ARCOS):
   */
  //char sequence[1] = {2};
  //robot.comStrN(ArCommands::POLLING, sequence, 2);

  /* 
   * This would work too:
   */
  //char sequence[8] = {2, 2, 2, 2, 2, 2, 2, 2};
  //robot.comStrN(ArCommands::POLLING, sequence, 8);

  /* 
   * Only enable the left-front side of a pioneer, or the front of an amigo:
   */
  //char sequence[4] = {2, 3, 4, 5};
  //robot.comStrN(ArCommands::POLLING, sequence, 4);

  // Run robot loop until killed.
  robot.run(true);
  return 0;
}
