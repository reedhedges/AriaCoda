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


*/

#ifndef ARIA_WRAPPER

#ifndef ARROBOTBATTERYPACKETREADER_H
#define ARROBOTBATTERYPACKETREADER_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArFunctor.h"

class ArRobot;
class ArRobotPacket;


/// This class will read a config packet from the robot
/// @internal
class ArRobotBatteryPacketReader
{
public:
  /// Constructor
  AREXPORT explicit ArRobotBatteryPacketReader(ArRobot *robot);
  /// Destructor
  AREXPORT ~ArRobotBatteryPacketReader();
  /// Request a single packet.. 
  AREXPORT void requestSinglePacket();
  /// Request a continuous stream of packets
  AREXPORT void requestContinuousPackets();
  /// Stop the stream of packets
  AREXPORT void stopPackets();
  /// See if we've requested packets
  AREXPORT bool haveRequestedPackets();
  /// See if we've gotten the data
  bool hasPacketArrived() const { return myPacketArrived; }
  /// Gets the number of batteries
  int getNumBatteries() const { return myNumBatteries; }
  /// Gets the number of bytes per battery
  int getNumBytesPerBattery() const { return myNumBytesPerBattery; }
  /// Gets the flags1 for a particular battery
  int getFlags1(int battery) { return myFlags1[battery]; }
  /// Gets the flags2 for a particular battery
  int getFlags2(int battery) { return myFlags2[battery]; }
  /// Gets the flags3 for a particular battery
  int getFlags3(int battery) { return myFlags3[battery]; }
  /// Gets the relative state of charge for a particular battery
  int getRelSOC(int battery) { return myRelSOC[battery]; }
  /// Gets the absolute state of charge for a particular battery
  int getAbsSOC(int battery) { return myAbsSOC[battery]; }


protected:
  /// internal, packet handler
  AREXPORT bool packetHandler(ArRobotPacket *packet);
  /// internal, packet handler
  AREXPORT void connectCallback();

  // the robot
  ArRobot *myRobot;

  int myNumBatteries;
  int myNumBytesPerBattery;
  std::map<int, int> myFlags1;
  std::map<int, int> myFlags2;
  std::map<int, int> myFlags3;
  std::map<int, int> myRelSOC;
  std::map<int, int> myAbsSOC;


  // whether our data has been received or not
  bool myPacketArrived;
  // last time we requested a packet (we'll only ask every 200 ms)
  ArTime myLastPacketRequest;

  bool myRequestedBatteryPackets;

  // the callback
  ArRetFunctor1C<bool, ArRobotBatteryPacketReader, ArRobotPacket *> myPacketHandlerCB;
  ArFunctorC<ArRobotBatteryPacketReader> myConnectCB;
};

#endif // ARROBOTBATTERYPACKETREADER_H

#endif // not ARIA_WRAPPER