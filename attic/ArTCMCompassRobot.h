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
#ifndef ARTCMCOMPASSROBOT_H
#define ARTCMCOMPASSROBOT_H

#include "ariaUtil.h"
#include "ArCommands.h"
#include "ArFunctor.h"
#include "ArRobot.h"
#include "ArTCM2.h"

/** Interface to a TCM 2/2.5/2.6 3-axis compass through the robot microcontroller.
 *  When a Pioneer robot is equipped with a TCM compass, it is typically connected
 *  to the robot microcontroller, which returns compass information in 
 *  compass data packets upon request (set compass mode/type to 2 or 3 in
 *  firmware configuration; mode 1 (data in SIP) is not supported by ARIA).  This class communicates
 *  with the robot microcontroller to configure the compass and recieve data
 *  from it. 
 *
*/
class ArTCMCompassRobot : public virtual ArTCM2
{
public:

  AREXPORT ArTCMCompassRobot(ArRobot *robot);
  AREXPORT virtual ~ArTCMCompassRobot();

  virtual void commandOff(void) { myRobot->comInt(ArCommands::TCM2, 0); }
  virtual void commandJustCompass(void) { myRobot->comInt(ArCommands::TCM2, 1); }
  virtual void commandOnePacket(void) { myRobot->comInt(ArCommands::TCM2, 2); }
  virtual void commandContinuousPackets(void) { myRobot->comInt(ArCommands::TCM2, 3); }
  virtual void commandUserCalibration(void) { myRobot->comInt(ArCommands::TCM2, 4); }
  virtual void commandAutoCalibration(void) { myRobot->comInt(ArCommands::TCM2, 5); }
  virtual void commandStopCalibration(void) { myRobot->comInt(ArCommands::TCM2, 6); }
  virtual void commandSoftReset(void) { myRobot->comInt(ArCommands::TCM2, 7); }

private:  
  ArRobot *myRobot;
  ArRetFunctor1C<bool, ArTCMCompassRobot, ArRobotPacket*> myPacketHandlerCB;
  bool packetHandler(ArRobotPacket *packet);
};


#endif 
