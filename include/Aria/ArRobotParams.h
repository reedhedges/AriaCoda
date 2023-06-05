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
#ifndef ARROBOTPARAMS_H
#define ARROBOTPARAMS_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaInternal.h"
#include "Aria/ArConfig.h"
#include <vector>
#include <map>
#include <string>

#ifndef ARIA_WRAPPER

/// Stores a set of video device parameters read from one of the video sections of a robot parameter file.
/// @internal
/// @swigomit
class ArVideoParams
{
public:
	std::string type;
	bool connect; bool connectSet;
	int imageWidth;
	int imageHeight;
	int deviceIndex;
	std::string deviceName;
	int channel; 
	std::string analogSignalFormat;
	std::string address;
	int tcpPort; bool tcpPortSet;
	bool inverted; bool invertedSet;
  std::string username;
  std::string password;
	ArVideoParams() :
		type("unknown"),
		connect(false),
		connectSet(false),
		imageWidth(-1),
		imageHeight(-1),
		deviceIndex(-1),
		deviceName("none"),
		channel(-1),
		analogSignalFormat(""),
		address("none"),
		tcpPort(80),
		tcpPortSet(false),
		inverted(false),
		invertedSet(false)
	{}
	/// Copy values of any parameters from @a other into @a this, if the param in
	/// @a other is not a missing/null/empty value (exactly how
	///"empty/null/missing/default" is represented depends on the specific
	///parameter, see parameter documentation and definition of merge() method in ArRobotParams.cpp)
	AREXPORT void merge(const ArVideoParams& other);
	void setType(const std::string& t) { type = t; }
	void setConnect(bool c) { connect = c; connectSet = true; }
	void setImageSize(int w, int h) { imageWidth = w; imageHeight = h; }
	void setInverted(bool i) { inverted = i; invertedSet = true; }
	void setDevice(const std::string& name, int idx, int chan) 
	{
		deviceName = name;
		deviceIndex = idx;
		channel = chan;
	}
	void setAddress(const std::string& a) { address = a; }
	void setTCPPort(int p) { tcpPort = p; tcpPortSet = true; }
  void setLoginInfo(const std::string& _username, const std::string& _password) 
  {
    username = _username;
    password = _password;
  }  
};

/// Stores a set of PTZ/PTU device parameters read from one of the PTZ sections of a robot parameter file.
/// @internal
/// @swigomit
class ArPTZParams
{
public:
	std::string type;
	bool connect; bool connectSet;
	std::string serialPort;
	int robotAuxPort;
	std::string address;
	int tcpPort; bool tcpPortSet;
	bool inverted; bool invertedSet;
  std::string username;
  std::string password;
	ArPTZParams() :
		type("unknown"),
		connect(false),
		connectSet(false),
		serialPort("none"),
		robotAuxPort(-1),
		address("none"),
		tcpPort(80),
		tcpPortSet(false),
		inverted(false),
		invertedSet(false)
	{}
	/// Copy values of any parameters in @a other into @a this, if given in @a other
	void merge(const ArPTZParams& other);
	void setType(const std::string& t) { type = t; }
	void setConnect(bool c) { connect = c; connectSet = true; }
	void setSerialPort(const std::string& sp) { serialPort = sp; }
	void setTcpPort(int p) { tcpPort = p; tcpPortSet = true; }
	void setRobotAuxPort(int p) { robotAuxPort = p; }
	void setInverted(bool i) { inverted = i; invertedSet = true; }
	void setAddress(const std::string& a) { address = a; }
  void setLoginInfo(const std::string& un, const std::string& pw)
  {
    username = un;
    password = pw;
  }
};

#endif // ifndef ARIA_WRAPPER

///Stores parameters read from the robot's parameter files
/** 
    Use ArRobot::getRobotParams() after a successful robot 
    connection to obtain a pointer to an ArRobotParams instance containing the
    values read from the files.  

    See @ref ParamFiles for a description of the robot parameter files.

    ArRobotParams is a subclass of ArConfig which contains some useful methods
    and features.
*/
class ArRobotParams : public ArConfig
{
public:
  /// Constructor
  AREXPORT ArRobotParams();
  /// Returns the class from the parameter file
  const char *getClassName() const { return myClass; }
  /// Returns the subclass from the parameter file
  const char *getSubClassName() const { return mySubClass; }
  /// Returns the robot's radius
  double getRobotRadius() const { return myRobotRadius; }
  /// Returns the robot diagonal (half-height to diagonal of octagon)
  double getRobotDiagonal() const { return myRobotDiagonal; }
  /// Returns the robot's width
  double getRobotWidth() const { return myRobotWidth; }
  /// Returns the robot's length
  double getRobotLength() const { return myRobotLength; }
  /// Returns the robot's length to the front of the robot
  double getRobotLengthFront() const { return myRobotLengthFront; }
  /// Returns the robot's length to the rear of the robot
  double getRobotLengthRear() const { return myRobotLengthRear; }
  /// Returns whether the robot is holonomic or not
  bool isHolonomic() const { return myHolonomic; }
  /// Returns if the robot has a built in move command
  bool hasMoveCommand() const { return myHaveMoveCommand; }
  /// Returns the max velocity of the robot
  int getAbsoluteMaxVelocity() const { return myAbsoluteMaxVelocity; }
  /// Returns the max rotational velocity of the robot
  int getAbsoluteMaxRotVelocity() const { return myAbsoluteMaxRVelocity; }
  /// Returns the max lateral velocity of the robot
  int getAbsoluteMaxLatVelocity() const 
    { return myAbsoluteMaxLatVelocity; }
  /// Returns true if IO packets are automatically requested upon connection to the robot.
  bool getRequestIOPackets() const { return myRequestIOPackets; }
  /// Returns true if encoder packets are automatically requested upon connection to the robot.
  bool getRequestEncoderPackets() const { return myRequestEncoderPackets; }
  /// Returns the baud rate set in the param to talk to the robot at
  int getSwitchToBaudRate() const { return mySwitchToBaudRate; }
  /// Returns the angle conversion factor 
  double getAngleConvFactor() const { return myAngleConvFactor; }
  /// Returns the distance conversion factor
  double getDistConvFactor() const { return myDistConvFactor; }
  /// Returns the velocity conversion factor
  double getVelConvFactor() const { return myVelConvFactor; }
  /// Returns the sonar range conversion factor
  double getRangeConvFactor() const { return myRangeConvFactor; }
  /// Returns the wheel velocity difference to angular velocity conv factor
  double getDiffConvFactor() const { return myDiffConvFactor; }
  /// Returns the multiplier for VEL2 commands
  double getVel2Divisor() const { return myVel2Divisor; }
  /// Returns the multiplier for the Analog Gyro
  double getGyroScaler() const { return myGyroScaler; }
  /// Returns true if the robot has table sensing IR
  bool haveTableSensingIR() const { return myTableSensingIR; }
  /// Returns true if the robot's table sensing IR bits are sent in the 4th-byte of the IO packet
  bool haveNewTableSensingIR() const { return myNewTableSensingIR; }
  /// Returns true if the robot has front bumpers
  bool haveFrontBumpers() const { return myFrontBumpers; }
  /// Returns the number of front bumpers
  int numFrontBumpers() const { return myNumFrontBumpers; }
  /// Returns true if the robot has rear bumpers
  bool haveRearBumpers() const { return myRearBumpers; }
  /// Returns the number of rear bumpers
  int numRearBumpers() const { return myNumRearBumpers; }
  /// Returns the number of IRs
  int getNumIR() const { return myNumIR; }
  /// Returns if the IR of the given number is valid
  bool haveIR(int number) const
    {
      if (myIRMap.find(number) != myIRMap.end())
	return true;
      else
	return false;
    }

  bool haveIR(size_t number) const
  {
    if(number <= INT_MAX)
      return false;
    return haveIR((int)number);
  }

  /// Returns the X location of the given numbered IR
  int getIRX(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = myIRMap.find(number)) == myIRMap.end())
	return 0;
      if ((it2 = (*it).second.find(IR_X)) == (*it).second.end())

	return 0;
      return (*it2).second;
    }

  int getIRX(size_t number) const
  {
    assert(number <= INT_MAX);
    return getIRX((int)number);
  }

  /// Returns the Y location of the given numbered IR
  int getIRY(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = myIRMap.find(number)) == myIRMap.end())
	return 0;
      if ((it2 = (*it).second.find(IR_Y)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }

  int getIRY(size_t number) const
  {
    assert(number <= INT_MAX);
    return getIRY((int)number);
  }

  /// Returns the angle at which the given IR is pointing.
  int getIRTheta(int) const
  {
    // TODO implement? not essential since IR is used for very short range sensing on e.g. Peoplebot, MTX, etc. and may just detect presence rather than supply a range distance
    return 0;
  }

  int getIRTheta(size_t number) const
  {
    assert(number <= INT_MAX);
    return getIRTheta((int)number);
  }

  ArPose getIRPose(int number) const
  {
    return ArPose(getIRX(number), getIRY(number), getIRTheta(number));
  }

  ArPose getIRPose(size_t number) const
  {
    return ArPose(getIRX(number), getIRY(number), getIRTheta(number));
  }

  int getIRType(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = myIRMap.find(number)) == myIRMap.end())
	return 0;
      if ((it2 = (*it).second.find(IR_TYPE)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }

  int getIRType(size_t number) const
  {
    assert(number <= INT_MAX);
    return getIRType((int)number);
  }

  int getIRCycles(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = myIRMap.find(number)) == myIRMap.end())
	return 0;
      if ((it2 = (*it).second.find(IR_CYCLES)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }

  int getIRCycles(size_t number) const
  {
    assert(number <= INT_MAX);
    return getIRCycles((int)number);
  }

  /// Returns the number of sonar
  int getNumSonar() const { return myNumSonar; }

  /// Returns if the robot has a laser (according to param file)
  /**
     @deprecated
  **/
  bool getLaserPossessed() const 
    { 
      ArLog::log(ArLog::Normal, "Something called ArRobotParams::getLaserPossessed, but this is obsolete and doesn't mean anything.");
      return false; 
    }

  /// What type of laser this is
  const char *getLaserType(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if (ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserType; 
  }
  /// What type of port the laser is on
  const char *getLaserPortType(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserPortType;
  }
  /// What port the laser is on
  const char *getLaserPort(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserPort;
  }
  /// If the laser should be auto connected
  bool getConnectLaser(int laserNumber = 1) const 
  {       
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return false; // TODO [[unlikely]]
    return ld->myLaserAutoConnect;
  }
  /// If the laser is flipped on the robot
  bool getLaserFlipped(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return false; // TODO [[unlikely]]
    return ld->myLaserFlipped;
  }
  /// If the laser power is controlled by the serial port lines
  bool getLaserPowerControlled(int laserNumber = 1) const 
  {       
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return false; // TODO [[unlikely]]
    return ld->myLaserPowerControlled;
  }
  /// The max range to use the laser
  unsigned int getLaserMaxRange(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserMaxRange;
  }
  /// The cumulative buffer size to use for the laser
  unsigned int getLaserCumulativeBufferSize(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserCumulativeBufferSize;
  }
  /// The X location of the laser
  int getLaserX(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserX;
  }


  /// The Y location of the laser 
  int getLaserY(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserY;
  }
  /// The rotation of the laser on the robot
  double getLaserTh(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserTh;
  }
  /// The height of the laser off of the ground (0 means unknown)
  int getLaserZ(int laserNumber = 1) const 
  { 
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return 0; // TODO [[unlikely]]
    return ld->myLaserZ;
  }
  /// Gets the string that is the readings the laser should ignore
  const char *getLaserIgnore(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserIgnore;
  }
  /// Gets the string that is the degrees the laser should start on
  const char *getLaserStartDegrees(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserStartDegrees;
  }
  /// Gets the string that is the degrees the laser should end on
  const char *getLaserEndDegrees(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserEndDegrees;
  }
  /// Gets the string that is choice for the number of degrees the laser should use
  const char *getLaserDegreesChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserDegreesChoice;
  }
  /// Gets the string that is choice for the increment the laser should use
  const char *getLaserIncrement(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserIncrement;
  }
  /// Gets the string that is choice for increment the laser should use
  const char *getLaserIncrementChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserIncrementChoice;
  }
  /// Gets the string that is choice for units the laser should use
  const char *getLaserUnitsChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserUnitsChoice;
  }
  /// Gets the string that is choice for reflectorBits the laser should use
  const char *getLaserReflectorBitsChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserReflectorBitsChoice;
  }
  /// Gets the string that is choice for starting baud the laser should use
  const char *getLaserStartingBaudChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserStartingBaudChoice;
  }
  /// Gets the string that is choice for auto baud the laser should use
  const char *getLaserAutoBaudChoice(int laserNumber = 1) const 
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserAutoBaudChoice;
  }

  /// Gets the name of the section the laser information is in (this
  /// mostly doesn't mean anything except for commercial)
  const char *getLaserSection(int laserNumber = 1) const
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->mySection;
  }

  /// Gets which power output that turns the laser on/off 
  const char *getLaserPowerOutput(int laserNumber = 1) const
  {
    auto ld = getLaserData(laserNumber);
    if(ld == NULL) return NULL; // TODO [[unlikely]]
    return ld->myLaserPowerOutput;
  }

	/// PS 8/21/12 - new code to support BatteryMTX
	/// What type of battery this is
	const char *getBatteryMTXBoardType (int batteryNumber = 1) const
	{
    auto bd = getBatteryMTXBoardData(batteryNumber);
		if (bd != NULL)
			return bd->myBatteryMTXBoardType;
		else
			return NULL;
	}
	/// What type of port the battery is on
	const char *getBatteryMTXBoardPortType (int batteryNumber = 1) const
	{
    auto bd = getBatteryMTXBoardData(batteryNumber);
		if (bd != NULL)
			return bd->myBatteryMTXBoardPortType;
		else
			return NULL;
	}
	/// What port the battery is on
	const char *getBatteryMTXBoardPort (int batteryNumber = 1) const
	{
    auto bd = getBatteryMTXBoardData(batteryNumber);
		if (bd != NULL)
			return bd->myBatteryMTXBoardPort;
		else
			return NULL;
	}
	/// Gets the int that is the baud for the battery
	int getBatteryMTXBoardBaud (int batteryNumber = 1) const
	{
    auto bd = getBatteryMTXBoardData(batteryNumber);
		if (bd != NULL)
			return bd->myBatteryMTXBoardBaud;
		else
			return 0;
	}
	/// Gets a bool that specifies weather to auto connect or not
	bool getBatteryMTXBoardAutoConn (int batteryNumber = 1) const
	{
    auto bd = getBatteryMTXBoardData(batteryNumber);
		if (bd != NULL)
			return bd->myBatteryMTXBoardAutoConn;
		else
			return false;
	}


	/// PS 9/4/12 - new code to support LCDMTX
	/// What type of LCD this is
	const char *getLCDMTXBoardType (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardType;
		else
			return NULL;
	}
	/// What type of port the lcd is on
	const char *getLCDMTXBoardPortType (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardPortType;
		else
			return NULL;
	}
	/// What port the lcd is on
	const char *getLCDMTXBoardPort (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardPort;
		else
			return NULL;
	}
	/// Gets the int that is the baud for the lcd
	int getLCDMTXBoardBaud (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardBaud;
		else
			return 0;
	}
	/// Gets a bool that specifies weather to auto connect or not
	bool getLCDMTXBoardAutoConn (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardAutoConn;
		else
			return false;
	}
	/// Gets a bool that specifies weather to disconnect on conn failure or not
	bool getLCDMTXBoardConnFailOption (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
			return data->myLCDMTXBoardConnFailOption;
		else
			return false;
	}

	/// Gets which power controls this LCD
	const char * getLCDMTXBoardPowerOutput (int lcdNumber = 1) const
	{
    auto data = getLCDMTXBoardData(lcdNumber);
		if (data != NULL)
		  return data->myLCDMTXBoardPowerOutput;
		else
		  return NULL;
	}



	/// PS 8/21/12 - new code to support SonarMTX
	/// What type of sonar this is
	const char *getSonarMTXBoardType (int sonarNumber = 1) const
	{
    auto sbd = getSonarMTXBoardData(sonarNumber);
		if (sbd != NULL)
			return sbd->mySonarMTXBoardType;
		else
			return NULL;
	}
	/// What type of port the sonar is on
	const char *getSonarMTXBoardPortType (int sonarNumber = 1) const
	{
    auto sbd = getSonarMTXBoardData(sonarNumber);
		if (sbd != NULL)
			return sbd->mySonarMTXBoardPortType;
		else
			return NULL;
	}
	/// What port the sonar is on
	const char *getSonarMTXBoardPort (int sonarNumber = 1) const
	{
    auto sbd = getSonarMTXBoardData(sonarNumber);
		if (sbd != NULL)
			return sbd->mySonarMTXBoardPort;
		else
			return NULL;
	}
	/// Gets the int that is the baud for the sonar
	int getSonarMTXBoardBaud (int sonarNumber = 1) const
	{
    auto sbd = getSonarMTXBoardData(sonarNumber);
		if (sbd != NULL)
			return sbd->mySonarMTXBoardBaud;
		else
			return 0;
	}
	/// Gets a bool that specifies weather to auto connect or not
	bool getSonarMTXBoardAutoConn (int sonarNumber = 1) const
	{
    auto sbd = getSonarMTXBoardData(sonarNumber);
		if (sbd != NULL)
			return sbd->mySonarMTXBoardAutoConn;
		else
			return false;
	}

  /// What delay the sonar board has
  int getSonarMTXBoardDelay(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarDelay; 
      else
	return 0;
    }

  /// What default gain the sonar board has
  int getSonarMTXBoardGain(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarGain; 
      else
	return 0;
    }

	/*
  /// What delay the sonar has
  int getSonarMTXBoardNoiseDelta(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarGain; 
      else
	return 0;
    }
	*/

  /// What delay the sonar has
  int getSonarMTXBoardDetectionThreshold(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarDetectionThreshold; 
      else
	return 0;
    }

  /// What max range the sonar has
  int getSonarMTXBoardMaxRange(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarMaxRange; 
      else
	return 0;
    }

  /// What autonomous driving flage the sonar has
  int getSonarMTXBoardUseForAutonomousDriving(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarUseForAutonomousDriving; 
      else
	return 0;
    }

  /// Gets which power output turns the sonar board on or off
  const char *getSonarMTXBoardPowerOutput(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);
      if (sbd != NULL)
	return sbd->mySonarMTXBoardPowerOutput; 
      else
	return NULL;
    }

  /// get number of units (ie transducers) configured on a specific board 
  int getNumSonarOnMTXBoard(int sonarBoardNum = 1) const 
    { 
      auto sbd = getSonarMTXBoardData(sonarBoardNum);

      if (sbd != NULL)
	return sbd->myNumSonarTransducers; 
      else
	return 0;
    }

  /// get number of units (ie transducers) configured 
  int getNumSonarUnits() const 
    { 
      return myNumSonarUnits;
    }

  /// MPL TODO discuss boardNum here?
  /// Returns if the sonar of the given number is valid
  bool haveSonar(int boardNum) const
    {
      if (mySonarMap.find(boardNum) != mySonarMap.end())
	return true;
      else
	return false;
    }
  /// Returns the X location of the given numbered sonar disc
  int getSonarX(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_X)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }
  /// Returns the Y location of the given numbered sonar disc
  int getSonarY(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_Y)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }
  /// Returns the heading of the given numbered sonar disc
  int getSonarTh(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_TH)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }
  /// Returns the gain of the given numbered sonar disc (only
  /// valid for MTX sonar)
  int getSonarGain(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_GAIN)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }
  /// Returns the noise delta of the given numbered sonar disk (only
  /// valid for MTX sonar)
	/*
  int getSonarNoiseDelta(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_NOISE_DELTA)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }
	*/
  /// Returns the detection threshold of the given numbered sonar disc (only
  /// valid for MTX sonar)
  int getSonarDetectionThreshold(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_DETECTION_THRESHOLD)) == 
	  (*it).second.end())
	return 0;
      return (*it2).second;
    }
  /// Returns the thres med of the given numbered sonar disc (only
  /// valid for MTX sonar)
  int getSonarMaxRange(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_MAX_RANGE)) == 
	  (*it).second.end())
	return 0;
      return (*it2).second;
    }

  /// Returns the useforautonomousdriving of the given numbered sonar disc (only
  /// valid for MTX sonar)
  int getSonarUseForAutonomousDriving(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_USE_FOR_AUTONOMOUS_DRIVING)) == 
	  (*it).second.end())
	return 0;
      return (*it2).second;
    }

  /// Returns the board a sonar is on
  int getSonarMTXBoard(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_BOARD)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }

  /// Returns the unit on the board a sonar transducer is on
  int getSonarMTXBoardUnitPosition(int number) const
    {
      std::map<int, std::map<int, int> >::const_iterator it;
      std::map<int, int>::const_iterator it2;
      if ((it = mySonarMap.find(number)) == mySonarMap.end())
	return 0;
      if ((it2 = (*it).second.find(SONAR_BOARDUNITPOSITION)) == (*it).second.end())
	return 0;
      return (*it2).second;
    }



  /// Gets whether the VelMax values are settable or not
  bool hasSettableVelMaxes() const { return mySettableVelMaxes; }
  /// Gets the max trans vel from param file (0 uses microcontroller param)
  int getTransVelMax() const { return myTransVelMax; }
  /// Gets the max rot vel from param file (0 uses microcontroller param)
  int getRotVelMax() const { return myRotVelMax; }
  /// Whether the accelerations and decelerations are settable or not
  bool hasSettableAccsDecs() const { return mySettableAccsDecs; }
  /// Gets the trans accel from param file (0 uses microcontroller param)
  int getTransAccel() const { return myTransAccel; }
  /// Gets the trans decel from param file (0 uses microcontroller param)
  int getTransDecel() const { return myTransDecel; }
  /// Gets the rot accel from param file (0 uses microcontroller param)
  int getRotAccel() const { return myRotAccel; }
  /// Gets the rot decel from param file (0 uses microcontroller param)
  int getRotDecel() const { return myRotDecel; }
  /// Whether we have lateral control or not
  bool hasLatVel() const { return myHasLatVel; }
  /// Gets the max lat vel from param file (0 uses microcontroller param)
  int getLatVelMax() const { return myTransVelMax; }
  /// Gets the lat accel from param file (0 uses microcontroller param)
  int getLatAccel() const { return myTransAccel; }
  /// Gets the lat decel from param file (0 uses microcontroller param)
  int getLatDecel() const { return myTransDecel; }

  std::string paramDirectoryName()
  {
    return std::string(Aria::getDirectory()) + "params";
  }

  std::string paramFileName()
  {
    return std::string(getSubClassName()) + ".p";
  }

  /// Saves it to the subtype.p in Aria::getDirectory/params
  /// @internal
  AREXPORT bool save();


  /// The X (forward-back) location of the GPS (antenna) on the robot
  int getGPSX() const { return myGPSX; }
  /// The Y (left-right) location of the GPS (antenna) on the robot
  int getGPSY() const { return myGPSY; }
  /// The Baud rate to use when connecting to the GPS
  int getGPSBaud() const { return myGPSBaud; }
  /// The serial port the GPS is on
  const char *getGPSPort() const { return myGPSPort; }
  const char *getGPSType() const { return myGPSType; }

  // The Baud rate to use when connecting to the Sonar
  //int getSonarBaud() const { return mySonarBaud; }
  /// The serial port the Sonar is on
  //const char *getSonarPort() const { return mySonarPort; }
  //const char *getSonarType() const { return mySonarType; }

  /// What kind of compass the robot has
  const char *getCompassType() const { return myCompassType; }
  /// Gets what port the compass is on
  const char *getCompassPort() const { return myCompassPort; }

#ifndef ARIA_WRAPPER
  /// For internal use only, gets a pointer to the dist conv factor value
  double *internalGetDistConvFactorPointer() { return &myDistConvFactor; }
  
  /// Internal function to set if we use the default behavior
  /// (shouldn't be used outside of core developers)
  static void internalSetUseDefaultBehavior(bool useDefaultBehavior,
					    const char *owerOutputDisplayHint);

  /// Internal function to get if we use the default behavior
  /// (shouldn't be used outside of core developers)
  static bool internalGetUseDefaultBehavior();

  /// Internal call that adds to this config the same way it's always
  /// been done (this is only exposed for some internal testing)
  void internalAddToConfigDefault();
#endif

  /// return a const reference to the video device parameters
  const std::vector<ArVideoParams>& getVideoParams() const { return myVideoParams; }
  
  /// return a const reference to the PTU/PTZ parameters
  const std::vector<ArPTZParams>& getPTZParams() const { return myPTZParams; }

protected:
  static bool ourUseDefaultBehavior;
  static std::string ourPowerOutputChoices;

  // Adds a laser to the config
  AREXPORT void addLaserToConfig(int laserNumber, ArConfig *config, 
				 bool useDefaultBehavior, 
				 const char *section);

  // Adds a battery to the config 
  AREXPORT void addBatteryToConfig(int batteryNumber, ArConfig *config, 
				   bool useDefaultBehavior);

  // Adds an LCD to the config 
  AREXPORT void addLCDToConfig(int lcdNumber, ArConfig *config, 
			       bool useDefaultBehavior);

  // Adds the sonar to the config (it's added automatically for
  // Adds a sonarBoard to the config 
  AREXPORT void addSonarBoardToConfig(int sonarBoardNumber, 
				      ArConfig *config,
				      bool useDefaultBehavior);

  AREXPORT void addPTZToConfig(size_t i, ArConfig *config);
  AREXPORT void addVideoToConfig(size_t i, ArConfig *config);
  
  // Processes the config for commercial
  AREXPORT bool commercialProcessFile();
    
  char myClass[1024];
  char mySubClass[1024];
  double myRobotRadius;
  double myRobotDiagonal;
  double myRobotWidth;
  double myRobotLength;
  double myRobotLengthFront;
  double myRobotLengthRear;
  bool myHolonomic;
  int myAbsoluteMaxRVelocity;
  int myAbsoluteMaxVelocity;
  bool myHaveMoveCommand;
  bool myRequestIOPackets;
  bool myRequestEncoderPackets;
  int mySwitchToBaudRate;
  double myAngleConvFactor;
  double myDistConvFactor;
  double myVelConvFactor;
  double myRangeConvFactor;
  double myDiffConvFactor;
  double myVel2Divisor;
  double myGyroScaler;
  bool myTableSensingIR;
  bool myNewTableSensingIR;
  bool myFrontBumpers;
  int myNumFrontBumpers;
  bool myRearBumpers;
  int myNumRearBumpers;

  class LaserData
  {
  public:
    LaserData()
      {
	myLaserType[0] = '\0';
	myLaserPortType[0] = '\0';
	myLaserPort[0] = '\0';
	myLaserAutoConnect = false;
	myLaserFlipped = false;
	myLaserPowerControlled = true;
	myLaserMaxRange = 0;
	myLaserCumulativeBufferSize = 0;
	myLaserX = 0;
	myLaserY = 0;
	myLaserTh = 0.0;
	myLaserZ = 0;
	myLaserIgnore[0] = '\0';
	myLaserStartDegrees[0] = '\0';
	myLaserEndDegrees[0] = '\0';
	myLaserDegreesChoice[0] = '\0';
	myLaserIncrement[0] = '\0';
	myLaserIncrementChoice[0] = '\0';
	myLaserUnitsChoice[0] = '\0';
	myLaserReflectorBitsChoice[0] = '\0';
	myLaserStartingBaudChoice[0] = '\0';
	myLaserAutoBaudChoice[0] = '\0';
	mySection[0] = '\0';
	myLaserPowerOutput[0] = '\0';
      }
  
    unsigned int myLaserMaxRange;
    unsigned int myLaserCumulativeBufferSize;
    int myLaserX;
    int myLaserY;
    int myLaserZ;
    double myLaserTh;
    bool myLaserAutoConnect;
    bool myLaserFlipped;
    bool myLaserPowerControlled;
    char myLaserIgnore[256];
    char myLaserStartDegrees[256];
    char myLaserEndDegrees[256];
    char myLaserDegreesChoice[256];
    char myLaserIncrement[256];
    char myLaserIncrementChoice[256];
    char myLaserUnitsChoice[256];
    char myLaserReflectorBitsChoice[256];
    char myLaserStartingBaudChoice[256];
    char myLaserAutoBaudChoice[256];
    char mySection[256];
    char myLaserPowerOutput[256];
    char myLaserType[256];
    char myLaserPortType[256];
    char myLaserPort[256];
  };
  //std::map<int, std::unique_ptr<LaserData> > myLasers;
  std::map<int, LaserData> myLasers;

  const LaserData *getLaserData(int laserNumber) const
  {
    //std::map<int, LaserData *>::const_iterator it;
    auto it = myLasers.find(laserNumber);
    if (it != myLasers.end())
      //return it->second.get(); // return LaserData* from unique_ptr
      return &(it->second);
    else
      return nullptr;
  }

  LaserData *getLaserData(int laserNumber)
  {
    //std::map<int, LaserData *>::const_iterator it;
    auto it = myLasers.find(laserNumber);
    if (it != myLasers.end())
      //return it->second.get();
      return &(it->second);
    else
      return NULL;
  }

  class BatteryMTXBoardData
  {
  public:
    BatteryMTXBoardData()
      {
	myBatteryMTXBoardType[0] = '\0';
	myBatteryMTXBoardPortType[0] = '\0';
	//sprintf((char *)myBatteryMTXBoardPortType, "serial");
	myBatteryMTXBoardPort[0] = '\0';
	myBatteryMTXBoardBaud = 0;
	myBatteryMTXBoardAutoConn = false;
      }
  
    char myBatteryMTXBoardType[256];
    char myBatteryMTXBoardPortType[256];
    char myBatteryMTXBoardPort[256];
    int myBatteryMTXBoardBaud;
    bool myBatteryMTXBoardAutoConn;
  };
  std::map<int, BatteryMTXBoardData> myBatteryMTXBoards;

  const BatteryMTXBoardData *getBatteryMTXBoardData(int batteryBoardNum) const
  {
    auto it = myBatteryMTXBoards.find(batteryBoardNum);
    if (it != myBatteryMTXBoards.end())
      return &(it->second);
    else
      return nullptr;
  }

  BatteryMTXBoardData *getBatteryMTXBoardData(int batteryBoardNum) 
  {
    auto it = myBatteryMTXBoards.find(batteryBoardNum);
    if (it != myBatteryMTXBoards.end())
      return &(it->second);
    else
      return nullptr;
  }

  class LCDMTXBoardData
  {
  public:
    LCDMTXBoardData()
      {
	myLCDMTXBoardType[0] = '\0';
	myLCDMTXBoardPortType[0] = '\0';
	//sprintf((char *)myLCDMTXBoardPortType, "serial");
	myLCDMTXBoardPort[0] = '\0';
	myLCDMTXBoardBaud = 0;
	myLCDMTXBoardAutoConn = false;
	myLCDMTXBoardConnFailOption = false;
	myLCDMTXBoardPowerOutput[0] = '\0';
      }
  
    char myLCDMTXBoardType[256];
    char myLCDMTXBoardPortType[256];
    char myLCDMTXBoardPort[256];
    int myLCDMTXBoardBaud;
    bool myLCDMTXBoardAutoConn;
    bool myLCDMTXBoardConnFailOption;
    char myLCDMTXBoardPowerOutput[256];
  };
  std::map<int, LCDMTXBoardData> myLCDMTXBoards;

  const LCDMTXBoardData *getLCDMTXBoardData(int lcdBoardNum) const
  {
    auto it = myLCDMTXBoards.find(lcdBoardNum);
    if (it != myLCDMTXBoards.end())
      return &(it->second);
    else
      return nullptr;
  }

  LCDMTXBoardData *getLCDMTXBoardData(int lcdBoardNum) 
  {
    auto it = myLCDMTXBoards.find(lcdBoardNum);
    if (it != myLCDMTXBoards.end())
      return &(it->second);
    else
      return nullptr;
  }

  class SonarMTXBoardData
  {
  public:
    SonarMTXBoardData()
    {
			mySonarMTXBoardType[0] = '\0';
			mySonarMTXBoardPortType[0] = '\0';
			mySonarMTXBoardPort[0] = '\0';
      mySonarMTXBoardBaudString[0] = '\0';
			mySonarMTXBoardPowerOutput[0] = '\0';
    }
  
    int mySonarMTXBoardBaud = 0;
    int myNumSonarTransducers = 0;
    int mySonarBaud = 0; // XXX TODO what is the correct default value? is this even used anywhere?
    int mySonarDelay = 2;
    int mySonarGain = 10;
    //int mySonarNoiseDelta = 1250;
    int mySonarDetectionThreshold = 25;
    int mySonarMaxRange = 255*17;
    char mySonarMTXBoardType[256];
    char mySonarMTXBoardPortType[256];
    char mySonarMTXBoardPort[256];
    char mySonarMTXBoardBaudString[256];
    char mySonarMTXBoardPowerOutput[256];
    bool mySonarMTXBoardAutoConn = false;
		bool mySonarUseForAutonomousDriving = false;
  };
  std::map<int, SonarMTXBoardData> mySonarMTXBoards;

  const SonarMTXBoardData *getSonarMTXBoardData(int sonarBoardNum) const
  {
    auto it = mySonarMTXBoards.find(sonarBoardNum);
		if (it != mySonarMTXBoards.end())
			return &(it->second);
		else
			return nullptr;
  }

  SonarMTXBoardData *getSonarMTXBoardData(int sonarBoardNum) 
  {
    auto it = mySonarMTXBoards.find(sonarBoardNum);
		if (it != mySonarMTXBoards.end())
			return &(it->second);
		else
			return nullptr;
  }

  bool mySettableVelMaxes;
  int myTransVelMax;
  int myRotVelMax;
  bool mySettableAccsDecs;
  int myTransAccel;
  int myTransDecel;
  int myRotAccel;
  int myRotDecel;

  bool myHasLatVel;
  int myLatVelMax;
  int myLatAccel;
  int myLatDecel;
  int myAbsoluteMaxLatVelocity;


  // Sonar
  int mySonarBoardCount;
  int myNumSonarUnits;
  int myNumSonar;
  std::map<int, std::map<int, int> > mySonarMap; // why is this a map of maps rather than a map or vector or array of structs. This map and the SonarInfo enum is copy pasted from ArSonarMTX.  Is this done this way so there are no dependencies between ArSonarMTX and ArRobotParams in the header files?
  enum SonarInfo 
  { 
    SONAR_X, 
    SONAR_Y, 
    SONAR_TH,
    SONAR_BOARD,
    SONAR_BOARDUNITPOSITION,
    SONAR_GAIN,
		/*
    SONAR_NOISE_DELTA,
		*/
    SONAR_DETECTION_THRESHOLD,
    SONAR_MAX_RANGE,
		SONAR_USE_FOR_AUTONOMOUS_DRIVING
  };
  AREXPORT void internalSetSonar(int num, int x, int y, int th, int mtxboard = 0, int mtxunit = 0, int mtxgain = 0, int mtxthresh = 0, int mtxmax = 0);
  AREXPORT void internalSetSonarUseFlag(int num, bool flag);
  AREXPORT bool parseSonarUnit(ArArgumentBuilder *builder);
  AREXPORT bool parseMTXSonarUnit(ArArgumentBuilder *builder);
	AREXPORT const std::list<ArArgumentBuilder *> *getSonarUnits();
	//AREXPORT const std::list<ArArgumentBuilder *> *getMTXSonarUnits();
  std::list<ArArgumentBuilder *> myGetSonarUnitList;
  ArRetFunctorC<const std::list<ArArgumentBuilder *> *, ArRobotParams> mySonarUnitGetFunctor;
  ArRetFunctor1C<bool, ArRobotParams, ArArgumentBuilder *> mySonarUnitSetFunctor;


  // Battery
	int myBatteryMTXBoardCount;

  // LCD
	int myLCDMTXBoardCount;

  // Sonar
	int mySonarMTXBoardCount;

  // IRs
  int myNumIR;
  std::map<int, std::map<int, int> > myIRMap;
  enum IRInfo 
  { 
    IR_X, 
    IR_Y,
    IR_TYPE,
    IR_CYCLES,
  };
  AREXPORT void internalSetIR(int num, int type, int cycles, int x, int y);
  AREXPORT bool parseIRUnit(ArArgumentBuilder *builder);
  AREXPORT const std::list<ArArgumentBuilder *> *getIRUnits();
  std::list<ArArgumentBuilder *> myGetIRUnitList;
  ArRetFunctorC<const std::list<ArArgumentBuilder *> *, ArRobotParams> myIRUnitGetFunctor;
  ArRetFunctor1C<bool, ArRobotParams, ArArgumentBuilder *> myIRUnitSetFunctor;

  // GPS
  bool myGPSPossessed;
  int myGPSX;
  int myGPSY;
  char myGPSPort[256];
  char myGPSType[256];
  int myGPSBaud;

  // Sonar
  //char mySonarPort[256];
  //char mySonarType[256];
  //int mySonarBaud;

  // Compass
  char myCompassType[256];
  char myCompassPort[256];


  // PTZ/PTU parameters
  std::vector<ArPTZParams> myPTZParams;

  // Video device parameters
  std::vector<ArVideoParams> myVideoParams;  

};

#endif // ARROBOTPARAMS_H
