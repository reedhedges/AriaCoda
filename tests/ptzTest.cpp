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
  This is a test of Pan/Tilt/Zoom devices using ArPTZ.
*/

#define WAIT 4000

void testPan(ArPTZ *ptz, double pos, const char *tag = "")
{
  ArLog::log(ArLog::Normal, "Pan %s %f...", tag, pos);
  ptz->pan(pos);
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, " => At pan %f.", ptz->getPan());
}
void testTilt(ArPTZ *ptz, double pos, const char *tag = "")
{
  ArLog::log(ArLog::Normal, "Tilt %s %f...", tag, pos);
  ptz->tilt(pos);
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, " => At tilt %f.", ptz->getTilt());
}
void testZoom(ArPTZ *ptz, int pos, const char *tag = "")
{
  ArLog::log(ArLog::Normal, "Zoom %s %d...", tag, pos);
  ptz->zoom(pos);
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, " => At zoom %d.", ptz->getZoom());
}

void testPanRel(ArPTZ *ptz, double pos, const char *tag = "")
{
  ArLog::log(ArLog::Normal, "PanRel %s %f...", tag, pos);
  ptz->panRel(pos);
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, " => At pan %f.", ptz->getPan());
}
void testTiltRel(ArPTZ *ptz, double pos, const char *tag = "")
{
  ArLog::log(ArLog::Normal, "TiltRel %s %f...", tag, pos);
  ptz->tiltRel(pos);
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, " => At tilt %f.", ptz->getTilt());
}

int main(int argc, char **argv) 
{ 
  Aria::init();
  ArRobot robot;
  ArArgumentParser parser(&argc, argv);
  ArRobotConnector con(&parser, &robot);
  if (!con.connectRobot())
  {
  	ArLog::log(ArLog::Terse, "Error connecting to robot.");
    Aria::exit(1);
  }
  if(!Aria::parseArgs())
  {
  	ArLog::log(ArLog::Terse, "Error parsing program arguments.");
    Aria::logOptions();
    Aria::exit(2);
  }

  //ArRVisionPTZ ptz(&robot);
  ArVCC4 ptz(&robot);
  //ArSonyPTZ ptz(&robot);
  //ArDPPTU ptz(&robot);

  ArLog::log(ArLog::Normal, "Using robot connection. (Aux. serial port 1)");

  robot.runAsync(true);
  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 0);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  if(!ptz.init())
  {
  	ArLog::log(ArLog::Terse, "Error connecting to and initializing PTZ.");
    Aria::exit(3);
  }

  // ptz.getRealPanTilt();  // vcc4 only
  if(ptz.canGetRealPanTilt()) ArLog::log(ArLog::Normal, "PTU will report real measured pan/tilt positions.");
  else ArLog::log(ArLog::Normal, "PTU cannot report real measured pan/tilt positions, will report commanded positions instead.");
  testPan(&ptz, ptz.getMaxPosPan(), "max");
  testPan(&ptz, ptz.getMaxNegPan(), "min");
  testPan(&ptz, 45);
  testPan(&ptz, -45);
  testPan(&ptz, 0);
  ArLog::log(ArLog::Normal, "Reset...");
  ptz.reset();
  ArUtil::sleep(WAIT);
  testPanRel(&ptz, 45);
  testPanRel(&ptz, -45);
  testPan(&ptz, 0);
  ArLog::log(ArLog::Normal, "Reset...");
  ptz.reset();
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, "");

  testTilt(&ptz, ptz.getMaxPosTilt(), "max");
  testTilt(&ptz, ptz.getMaxNegTilt(), "min");
  testTilt(&ptz, 45);
  testTilt(&ptz, -45);
  testTilt(&ptz, 0);
  ArLog::log(ArLog::Normal, "Reset...");
  ptz.reset();
  ArUtil::sleep(WAIT);
  testTiltRel(&ptz, 45);
  testTiltRel(&ptz, -45);
  testTilt(&ptz, 0);
  ArLog::log(ArLog::Normal, "Reset...");
  ptz.reset();
  ArUtil::sleep(WAIT);
  ArLog::log(ArLog::Normal, "");

  if(ptz.canZoom())
  {
    if(ptz.canGetRealZoom()) ArLog::log(ArLog::Normal, "PTZ will report real measured zoom positions.");
    else ArLog::log(ArLog::Normal, "PTZ cannot report real measured zoom positions, will report commanded positions instead.");
    testZoom(&ptz, ptz.getMaxZoom(), "max");
    testZoom(&ptz, ptz.getMinZoom(), "min");
    testZoom(&ptz, 0);
    ptz.reset();
    ArUtil::sleep(WAIT);
  }
  else
  {
    ArLog::log(ArLog::Normal, "PTU has no zoom feature.");
  }


  Aria::exit(0);
}

