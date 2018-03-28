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
//#include "ArVideo.h"

/** @example cameraPTZExample.cpp This is an example showing how to connect to
 * and use a camera's PTZ control (or DPPTU)
 *
 * This program can control PTZ cameras with support built in to the ARIA
 * library: Canon VCC 4/5, Sony PTZ, RVision, and DPPTU.    To control the Axis
 * network cameras, see example programs in the ArVideo library instead.
 *
 * PTZ type and connection parameters are read from robot parameter file if
 * available, or use command-line arguments to override the robot parameter
 * file. Run with --help for a complete list of command-line options available.
 *
 * For example, to connect to Canon VCC on robot auxilliary serial port, use:
 *  ./cameraPTZExample -ptzType vcc4 
 *
 * To connect to Canon VCC on serial port COM4, use:
 *  ./cameraPTZExample -ptzType vcc4 -ptzSerialPort COM4 
 * or on Linux this works too:
 *  ./cameraPTZExample -ptzType vcc4 -ptzSerialPort /dev/ttyS3
 *
 * To connect to RVision camera, use:
 *  ./cameraPTZExample -ptzType rvision
 * 
 * To connect to DPPTU use:
 *  ./cameraPTZExample -ptzType dpptu
 *
 * Canon VCC defaults to use the microcontroller port Aux1.  Use
 * -ptzRobotAuxSerialPort or -ptzSerialPort arguments to change.
 */


int main(int argc, char **argv)
{
  Aria::init();
	//ArVideo::init();
//  ArLog::init(ArLog::StdOut, ArLog::Verbose);
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);

  // This is used to create and configure the PTZ interface object based on the
  // robot's parameter file and this program's command line options (run with
  // -help for list). Must be created before calling Aria::parseArgs().
  ArPTZConnector ptzConnector(&parser, &robot);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "cameraPTZExample: Warning, Could not connect to the robot. Won't use robot parameter file for defaults");
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "cameraPTZExample: Connected.");

  robot.runAsync(true);

  // Create and connect to all PTZs given in robot parameter file and command
  // line.
  const int PauseTime = 3500; // ms
  ptzConnector.connect();
  ArUtil::sleep(PauseTime*2);
  for(size_t i = 0; i < ptzConnector.getNumPTZs(); ++i)
  {
    ArLog::log(ArLog::Normal, "cameraPTZExample: [getPTZ(%d)..]", i);

    ArPTZ *ptz = ptzConnector.getPTZ(i);
    if(!ptz)
    {
      ArLog::log(ArLog::Normal, "cameraPTZExample: there is no PTZ %d.", i+1);     
      continue;
    }

    ArLog::log(ArLog::Normal, "cameraPTZExample: ===== PTZ #%d is a %s =====", i+1, ptz->getTypeName());

    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan left -90..");
    ptz->pan(-90);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan position is now %f (%s).", ptz->getPan(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getPan(), -90, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: pan position is more than one degree different from command -90.");
      
    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan right 90..");
    ptz->pan(90);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan position is now %f. (%s)", ptz->getPan(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getPan(), 90, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: pan position is more than one degree different from command 90.");

    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan center 0..");
    ptz->pan(0);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan position is now %f (%s).", ptz->getPan(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getPan(), 0, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: pan position is more than one degree different from command 0.");

    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt up 90..");
    ptz->tilt(90);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt position is now %f(%s).", ptz->getTilt(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getTilt(), 90, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: tilt position is more than one degree different from command 90.");

    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt down -20..");
    ptz->tilt(-20);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt position is now %f(%s).", ptz->getTilt(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getTilt(), -20, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: tilt position is more than one degree different from command -20.");

    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt center 0..");
    ptz->tilt(0);
    ArUtil::sleep(PauseTime);
    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt position is now %f (%s).", ptz->getTilt(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    if(!ArMath::compareFloats(ptz->getTilt(), 0, 1))
      ArLog::log(ArLog::Normal, "cameraPTZExample: Warning: tilt position is more than one degree different from command 0.");

    ArLog::log(ArLog::Normal, "cameraPTZExample: Reset..");
    ptz->reset();
    ArLog::log(ArLog::Normal, "cameraPTZExample: Pan position is now %f (%s).", ptz->getPan(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    ArLog::log(ArLog::Normal, "cameraPTZExample: Tilt position is now %f (%s).", ptz->getTilt(), (ptz->canGetRealPanTilt()?"real position from camera":"last command sent"));
    ArUtil::sleep(PauseTime);
  }

  ArLog::log(ArLog::Normal, "cameraPTZExample: Ending robot thread..");
  robot.stopRunning();

  // wait for the thread to stop
  robot.waitForRunExit();

  // exit
  ArLog::log(ArLog::Normal, "cameraPTZExample: Exiting.");
  Aria::exit(0);
  return 0;
}

