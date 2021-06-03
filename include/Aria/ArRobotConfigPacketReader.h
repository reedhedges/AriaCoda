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

#ifndef ARROBOTCONFIGPACKETREADER_H
#define ARROBOTCONFIGPACKETREADER_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArFunctor.h"
#include "Aria/ArRobotPacket.h"

class ArRobot;


/// This class will read a config packet from the robot
/// @internal
class ArRobotConfigPacketReader
{
public:
  /// Constructor
  AREXPORT explicit ArRobotConfigPacketReader(ArRobot *robot, 
				     bool onlyOneRequest = false,
				     ArFunctor *packetedArrivedCB = NULL);
  /// Destructor
  AREXPORT ~ArRobotConfigPacketReader();
  /// Request a packet.. true if we could, false if onlyOneRequest already done
  AREXPORT bool requestPacket();
  /// See if we've requested a packet yet
  bool hasPacketBeenRequested() const { return myPacketRequested; }
  /// See if we've gotten the data
  bool hasPacketArrived() const { return myPacketArrived; }
  /// Gets a pointer to the packet that we built the config packet from
  const ArRobotPacket *getRobotPacket() const { return &myPacket; } 
  /// Log the config
  AREXPORT void log() const;
  /// Log the movement part of the config config
  AREXPORT void logMovement() const;
  /// Builds a string of the info
  AREXPORT std::string buildString() const;
  /// Builds a string of the movement info
  AREXPORT std::string buildStringMovement() const;
  /// Gets the type of robot
  const char *getType() const { return myType.c_str(); }
  /// Gets the subtype of robot
  const char *getSubType() const { return mySubType.c_str(); }
  /// Gets the serial number of the robot
  const char *getSerialNumber() const { return mySerialNumber.c_str(); }
  /// Gets the absolute maximum rotational velocity in deg/sec (cannot be set above this in firmware or through software)
  int getRotVelTop() const { return myRotVelTop; }
  /// Gets the absolute maximum translational velocity in mm/sec (cannot be set above this in firmware or through software)
  int getTransVelTop() const { return myTransVelTop; }
  /// Gets the absolute maximum rotational acceleration in deg/sec/sec (cannot be set above this in firmware or through software)
  int getRotAccelTop() const { return myRotAccelTop; }
  /// Gets the absolute maximum translational acceleration in mm/sec/sec (cannot be set above this in firmware or through software)
  int getTransAccelTop() const { return myTransAccelTop; }
  /// Gets the maximum PWM the robot will have (stallval cannot be above this)
  int getPwmMax() const { return myPwmMax; }
  /// Gets the name of the robot
  const char *getName() const { return myName.c_str(); }
  /// Gets the cycle time in ms of the motor packets
  int getSipCycleTime() const { return mySipCycleTime; }
  /// Gets the host baud number, look at the manual for what these mean
  int getHostBaud() const { return myHostBaud; }
  /// Gets the host baud number, look at the manual for what these mean
  int getAux1Baud() const { return myAux1Baud; }
  /// Gets the gripper value (whether or not the robot has a gripper)
  bool getHasGripper() const { return myHasGripper; }
  /// Gets whether or not the robot has front sonar
  bool getFrontSonar() const { return myFrontSonar; }
  /// Gets whether or not the robot has rear sonar
  bool getRearSonar() const { return myRearSonar; }
  /// Gets the low battery beeping indicating voltage times 10
  int getLowBattery() const { return myLowBattery; }
  /// Gets the revcount
  int getRevCount() const { return myRevCount; }
  /// Gets the watchdog (how many ms after command robot stops)
  int getWatchdog() const { return myWatchdog; }
  /// Returns if the robot is using normal packets or new style packets
  bool getNormalMPacs() const { return myNormalMPacs; }
  /// Returns the stallval (pwms at which robot stalls)
  int getStallVal() const { return myStallVal; }
  /// Returns the stallcount (how many 10ms increments robot stops after stall)
  int getStallCount() const { return myStallCount; }
  /// Returns the joystick translational velocity
  int getJoyVel() const { return myJoyVel; }
  /// Returns the joystick rotational velocity
  int getJoyRotVel() const { return myJoyRotVel; }
  /// Returns the current maximum rotational velocity (deg/sec) (can be set)
  int getRotVelMax() const { return myRotVelMax; } 
  /// Returns the current maximum translational velocity (mm/sec) (can be set)
  int getTransVelMax() const { return myTransVelMax; } 
  /// Returns the rotational acceleration
  int getRotAccel() const { return myRotAccel; }
  /// Returns the rotational deceleration
  int getRotDecel() const { return myRotDecel; }
  /// Returns the rotational KP value (look at the manual)
  int getRotKP() const { return myRotKP; }
  /// Returns the rotational KV value (look at the manual)
  int getRotKV() const { return myRotKV; }
  /// Returns the rotational KI value (look at the manual)
  int getRotKI() const { return myRotKI; }
  /// Returns the translational acceleration
  int getTransAccel() const { return myTransAccel; }
  /// Returns the translational deceleration
  int getTransDecel() const { return myTransDecel; }
  /// Returns the translational KP value (look at the manual)
  int getTransKP() const { return myTransKP; }
  /// Returns the translational KV value (look at the manual)
  int getTransKV() const { return myTransKV; }
  /// Returns the translational KI value (look at the manual)
  int getTransKI() const { return myTransKI; }
  /// Returns the number of front bumpers
  int getFrontBumps() const { return myFrontBumps; }
  /// Returns the number of rear bumpers
  int getRearBumps() const { return myRearBumps; }
  /// Returns whether the robot has a charger
  int getHasCharger() const { return myHasCharger; }
  /// Returns the number of ms the sonar cycle is (default is 40)
  int getSonarCycle() const { return mySonarCycle; }
  /// Returns the baud rate
  bool getResetBaud() const { return myResetBaud; }
  /// Returns if the robot has a gyro or not
  bool getHasGyro() const { return myHasGyro; }
  /// Returns if the robot has a gyro or not
  int getGyroType() const { return myGyroType; }
  /// Returns the DriftFactor value (see the manual)
  int getDriftFactor() const { return myDriftFactor; }
  /// Returns the Aux2 baud number, look at the manual for what these mean
  int getAux2Baud() const { return myAux2Baud; }
  /// Returns the Aux3 baud number, look at the manual for what these mean
  int getAux3Baud() const { return myAux3Baud; }
  /// Returns the Ticks/MM for the robot
  int getTicksMM() const { return myTicksMM; }
  /// Returns the voltage (x10) that the robot will shut down the computer at
  int getShutdownVoltage() const { return myShutdownVoltage; }
  /// Gets the firmware version
  const char *getFirmwareVersion() const 
    { return myFirmwareVersion.c_str(); }
  /// Gets the gyro CW value
  int getGyroCW() const { return myGyroCW; }
  /// Gets the gyro CCW value
  int getGyroCCW() const { return myGyroCCW; }
  /// Gets the kinematics delay
  int getKinematicsDelay() const { return myKinematicsDelay; }
  /// Gets the absolute maximum lateral velocity in mm/sec (cannot be set above this in firmware or through software)
  int getLatVelTop() const { return myLatVelTop; }
  /// Gets the absolute maximum lateral acceleration in mm/sec/sec (cannot be set above this in firmware or through software)
  int getLatAccelTop() const { return myLatAccelTop; }
  /// Returns the current maximum lateral velocity (mm/sec) (can be set)
  int getLatVelMax() const { return myLatVelMax; } 
  /// Returns the lateral acceleration
  int getLatAccel() const { return myLatAccel; }
  /// Returns the lateral deceleration
  int getLatDecel() const { return myLatDecel; }
  /// Gets the powerbot charge threshold
  int getPowerbotChargeThreshold() const 
    { return myPowerbotChargeThreshold; }
  /// Gets the port the PDB is on
  unsigned char getPDBPort() const 
    { return myPDBPort; }
  /// Gets the port the PDB is on
  int getGyroRateLimit() const 
    { return myGyroRateLimit; }
  /// Gets the high temperature threshold
  char getHighTemperatureShutdown() const 
    { return myHighTemperatureShutdown; }
  /// Gets the power bits
  int getPowerBits() const
    { return myPowerBits; }
  /// Gets the battery type 
  unsigned char getBatteryType() const
    { return myBatteryType; }
  /// Gets the warning state of charge
  int getStateOfChargeLow() const
    { return myStateOfChargeLow; }
  /// Gets the shutdown state of charge
  int getStateOfChargeShutdown() const
    { return myStateOfChargeShutdown; }
  const char *getFirmwareBootloaderVersion() const 
    { return myFirmwareBootloaderVersion.c_str(); }
  unsigned int getConfigFlags() const 
    { return myConfigFlags; }
  int getGyroFWVersion() const 
    { return myGyroFWVersion; }
  /// internal, packet handler
  AREXPORT bool packetHandler(ArRobotPacket *packet);
  /// internal, connection callback
  AREXPORT void connected();
protected:
  // the different parameters from the robot
  std::string myType;
  std::string mySubType;
  std::string mySerialNumber;
  int myRotVelTop;
  int myTransVelTop;
  int myRotAccelTop;
  int myTransAccelTop;
  int myPwmMax;
  std::string myName;
  int mySipCycleTime;
  int myHostBaud;
  int myAux1Baud;
  bool myHasGripper;
  bool myFrontSonar;
  bool myRearSonar;
  int myLowBattery;
  int myRevCount;
  int myWatchdog;
  bool myNormalMPacs;
  int myStallVal;
  int myStallCount;
  int myJoyVel;
  int myJoyRotVel;
  int myRotVelMax;
  int myTransVelMax;
  int myRotAccel;
  int myRotDecel;
  int myRotKP;
  int myRotKV;
  int myRotKI;
  int myTransAccel;
  int myTransDecel;
  int myTransKP;
  int myTransKV;
  int myTransKI;
  int myFrontBumps;
  int myRearBumps;
  int myHasCharger;
  int mySonarCycle;
  bool myResetBaud;
  bool myHasGyro;
  int myGyroType;
  int myDriftFactor;
  int myAux2Baud;
  int myAux3Baud;
  int myTicksMM;
  int myShutdownVoltage;
  std::string myFirmwareVersion;
  int myGyroCW;
  int myGyroCCW;
  int myKinematicsDelay;
  int myLatVelTop;
  int myLatAccelTop;
  int myLatVelMax;
  int myLatAccel;
  int myLatDecel;
  int myPowerbotChargeThreshold;
  unsigned char myPDBPort;
  int myGyroRateLimit;
  char myHighTemperatureShutdown;
  int myPowerBits;
  unsigned char myBatteryType;
  int myStateOfChargeLow;
  int myStateOfChargeShutdown;
  std::string myFirmwareBootloaderVersion;
  unsigned int myConfigFlags;
  int myGyroFWVersion;

  // the robot
  ArRobot *myRobot;
  // whether only one request can be done or not
  bool myOnlyOneRequest;
  // whether a request has been made or not
  bool myPacketRequested;
  // whether our data has been received or not
  bool myPacketArrived;
  // last time we requested a packet (we'll only ask every 200 ms)
  ArTime myLastPacketRequest;
  // a copy of the packet
  ArRobotPacket myPacket;
  
  // the callback
  ArRetFunctor1C<bool, ArRobotConfigPacketReader, ArRobotPacket *> myPacketHandlerCB;
  // callback for connections in case we need to request a config packet
  ArFunctorC<ArRobotConfigPacketReader> myConnectedCB;
  // callback for when we get a packet
  ArFunctor *myPacketArrivedCB;
  
};

#endif // ifndef ARROBOTCONFIGPACKETREADER_H

#endif // ifndef ARIA_WRAPPER