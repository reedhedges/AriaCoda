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
#ifndef ARROBOT_H
#define ARROBOT_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRobotPacketSender.h"
#include "Aria/ArRobotPacketReceiver.h"
#include "Aria/ArFunctor.h"
#include "Aria/ArSyncTask.h"
#include "Aria/ArSensorReading.h"
#include "Aria/ArMutex.h"
#include "Aria/ArCondition.h"
#include "Aria/ArSyncLoop.h"
#include "Aria/ArRobotPacketReaderThread.h"
#include "Aria/ArRobotParams.h"
#include "Aria/ArActionDesired.h"
#include "Aria/ArResolver.h"
#include "Aria/ArTransform.h"
#include "Aria/ArInterpolation.h"
#include "Aria/ArKeyHandler.h"
#include <list>
#include <vector>

class ArAction;
class ArRobotConfigPacketReader;
class ArRobotBatteryPacketReader;
class ArDeviceConnection;
class ArRangeDevice;
class ArRobotPacket;
class ArPTZ;
class ArLaser;
class ArBatteryMTX;
class ArSonarMTX;
class ArLCDMTX;

/// Central class for communicating with and operating the robot
/** 
    This is the most important class. It is used to communicate with
    the robot by sending commands and retrieving data (including
    wheel odometry, digital and analog inputs, sonar data, and more). 
    It is also used
    to provide access to objects for controlling attached accessories, 
    ArRangeDevice objects, ArAction objects, and others.  For details
    on usage, and how the task cycle and robot state synchronization works,
    see the @ref robot "ArRobot section" and the
    @ref ClientCommands "Commands and Actions section" of the ARIA overview.

    @note In Windows you cannot make an ArRobot object a global variable, 
    it will crash because the compiler initializes the constructors in
    the wrong order. You can, however, make a pointer to an ArRobot and then
    allocate it with 'new' at program start.

    @see ArRobotConnector

    @ingroup ImportantClasses
   @ingroup DeviceClasses
**/
class ArRobot
{
public:

  typedef enum {
    WAIT_CONNECTED, ///< The robot has connected
    WAIT_FAILED_CONN, ///< The robot failed to connect
    WAIT_RUN_EXIT, ///< The run loop has exited
    WAIT_TIMEDOUT, ///< The wait reached the timeout specified
    WAIT_INTR, ///< The wait was interupted by a signal
    WAIT_FAIL ///< The wait failed due to an error
  } WaitState;

  enum ChargeState : uint8_t {
    CHARGING_UNKNOWN = 255,
    CHARGING_NOT = 0,
    CHARGING_BULK = 1,
    CHARGING_OVERCHARGE = 2,
    CHARGING_FLOAT = 3,
    CHARGING_BALANCE = 4
  };
  /// Constructor.
  /// @param unused an old flag no longer used
  AREXPORT ArRobot(const char * name = NULL, UNUSED bool unused = true,
		   bool doSigHandle=true, 
		   bool normalInit = true, bool addAriaExitCallback = true);
		   
  /// Destructor
  AREXPORT ~ArRobot();

  /// Starts the instance to do processing in this thread
  AREXPORT void run(bool stopRunIfNotConnected, 
		    bool runNonThreaded = false);
  /// Starts the instance to do processing in its own new thread
  AREXPORT void runAsync(bool stopRunIfNotConnected, 
			 bool runNonThreadedPacketReader = false);

  /// @copydoc run(bool, bool)
  /// @ingroup easy
  void run() { run(true); }

  /// @copydoc runAsync(bool, bool)
  /// @ingroup easy
  void runAsync() { runAsync(true); }

  /// Returns whether the robot is currently running or not
  /// @ingroup easy
  AREXPORT bool isRunning() const;

  /// Stops the robot from doing any more processing
  /// @ingroup easy
  AREXPORT void stopRunning(bool doDisconnect=true); 

  /// Sets the connection this instance uses
  AREXPORT void setDeviceConnection(ArDeviceConnection *connection);
  /// Gets the connection this instance uses
  AREXPORT ArDeviceConnection *getDeviceConnection() const;
  
  /// Questions whether the robot is connected or not
  /** 
      @return true if connected to a robot, false if not
      @ingroup easy
  */
  bool isConnected() const { return myIsConnected; }
  /// Connects to a robot, not returning until connection made or failed
  AREXPORT bool blockingConnect(bool tryHarderToConnect = true);
  /// Connects to a robot, from the robots own thread
  AREXPORT bool asyncConnect();
  /// Disconnects from a robot
  AREXPORT bool disconnect();

  /// Clears what direct motion commands have been given, so actions work
  AREXPORT void clearDirectMotion();
  /// Returns true if direct motion commands are blocking actions
  AREXPORT bool isDirectMotion() const;

  /// Sets the state reflection to be inactive (until motion or clearDirectMotion)
  /// @see clearDirectMotion
  AREXPORT void stopStateReflection();
  

  /// Enables the motors on the robot
  /// @ingroup easy
  AREXPORT void enableMotors();
  /// Disables the motors on the robot
  /// @ingroup easy
  AREXPORT void disableMotors();

  /// Enables the sonar on the robot
  /// @ingroup easy
  AREXPORT void enableSonar();
  /// Enables some of the sonar on the robot (the ones for autonomous driving)
  AREXPORT void enableAutonomousDrivingSonar();
  /// Disables the sonar on the robot
  /// @ingroup easy
  AREXPORT void disableSonar();

  /// Stops the robot
  /// @see clearDirectMotion
  /// @ingroup easy
  AREXPORT void stop();
  /// Sets the velocity
  /// @see clearDirectMotion
  /// @ingroup easy
  AREXPORT void setVel(double velocity);
  /// Sets differential movement if implemented by robot
  AREXPORT void setVel2(double leftVelocity, double rightVelocity);
  /// Move the given distance forward/backwards
  /// @ingroup easy
  AREXPORT void move(double distance);
  /// Sees if the robot is done moving the previously given move
  /// @ingroup easy
  AREXPORT bool isMoveDone(double delta = 0.0);
  /// Sets the difference required for being done with a move
  void setMoveDoneDist(double dist) { myMoveDoneDist = dist; }
  /// Gets the difference required for being done with a move
  double getMoveDoneDist() { return myMoveDoneDist; }
  /// Sets the heading
  /// @ingroup easy
  AREXPORT void setHeading(double heading);
  /// Sets the rotational velocity
  /// @ingroup easy
  AREXPORT void setRotVel(double velocity);
  /// Sets the delta heading
  /// @ingroup easy
  AREXPORT void setDeltaHeading(double deltaHeading);
  /// Sees if the robot is done changing to the previously given setHeading
  /// @ingroup easy
  AREXPORT bool isHeadingDone(double delta = 0.0) const;
  /// sets the difference required for being done with a heading change (e.g. used in isHeadingDone())
  void setHeadingDoneDiff(double degrees) 
    { myHeadingDoneDiff = degrees; }
  /// Gets the difference required for being done with a heading change (e.g. used in isHeadingDone())
  double getHeadingDoneDiff() const { return myHeadingDoneDiff; }
  /// Sets the lateral velocity
  /// @see clearDirectMotion
  AREXPORT void setLatVel(double latVelocity);


  /// sees if we're stopped
  /// @ingroup easy
  AREXPORT bool isStopped(double stoppedVel = 0.0, double stoppedRotVel = 0.0,
			  double stoppedLatVel = 0.0);

  /// Sets the vels required to be stopped
  AREXPORT void setStoppedVels(double stoppedVel, double stoppedRotVel,
			       double stoppedLatVel);

  /// Sets the length of time a direct motion command will take precedence 
  /// over actions, in milliseconds
  AREXPORT void setDirectMotionPrecedenceTime(int mSec);

  /// Gets the length of time a direct motion command will take precedence 
  /// over actions, in milliseconds
  AREXPORT unsigned int getDirectMotionPrecedenceTime() const;

  /// Sends a command to the robot with no arguments
  AREXPORT bool com(unsigned char command);
  /// Sends a command to the robot with an int for argument
  AREXPORT bool comInt(unsigned char command, short int argument);
  /// Sends a command to the robot with two bytes for argument
  AREXPORT bool com2Bytes(unsigned char command, char high, char low);
  /// Sends a command to the robot with a length-prefixed string for argument
  AREXPORT bool comStr(unsigned char command, const char *argument);
  /// Sends a command to the robot with a length-prefixed string for argument
  AREXPORT bool comStrN(unsigned char command, const char *str, size_t size);
  /// Sends a command containing exactly the data in the given buffer as argument
  AREXPORT bool comDataN(unsigned char command, const char *data, size_t size);

  /// Returns the robot's name that is set in its onboard firmware configuration
  const char * getRobotName() const { return myRobotName.c_str();}
  /// Returns the type of the robot we are currently connected to
  const char * getRobotType() const { return myRobotType.c_str();}
  /// Returns the subtype of the robot we are currently connected to
  const char * getRobotSubType() const 
    { return myRobotSubType.c_str(); }
  /// Returns robot serial number, as provided by the config packet. Returns NULL if no config packet was received or there is no serial number.
  const char * getRobotSerialNumber();

  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransVel() const 
    { return myAbsoluteMaxTransVel; }
  /// Sets the robot's absolute maximum translational velocity
  AREXPORT bool setAbsoluteMaxTransVel(double maxVel);

  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransNegVel() const 
    { return myAbsoluteMaxTransNegVel; }
  /// Sets the robot's absolute maximum translational velocity
  AREXPORT bool setAbsoluteMaxTransNegVel(double maxVel);

  /// Gets the robot's absolute maximum translational acceleration
  double getAbsoluteMaxTransAccel() const 
    { return myAbsoluteMaxTransAccel; }
  /// Sets the robot's absolute maximum translational acceleration
  AREXPORT bool setAbsoluteMaxTransAccel(double maxAccel);

  /// Gets the robot's absolute maximum translational deceleration
  double getAbsoluteMaxTransDecel() const 
    { return myAbsoluteMaxTransDecel; }
  /// Sets the robot's absolute maximum translational deceleration
  AREXPORT bool setAbsoluteMaxTransDecel(double maxDecel);

  /// Gets the robot's absolute maximum rotational velocity
  double getAbsoluteMaxRotVel() const 
    { return myAbsoluteMaxRotVel; }
  /// Sets the robot's absolute maximum rotational velocity
  AREXPORT bool setAbsoluteMaxRotVel(double maxVel);
  
  /// Gets the robot's absolute maximum rotational acceleration
  double getAbsoluteMaxRotAccel() const 
    { return myAbsoluteMaxRotAccel; }
  /// Sets the robot's absolute maximum rotational acceleration
  AREXPORT bool setAbsoluteMaxRotAccel(double maxAccel);

  /// Gets the robot's absolute maximum rotational deceleration
  double getAbsoluteMaxRotDecel() const 
    { return myAbsoluteMaxRotDecel; }
  /// Sets the robot's absolute maximum rotational deceleration
  AREXPORT bool setAbsoluteMaxRotDecel(double maxDecel);

  /// Gets the robot's absolute maximum lateral velocity
  double getAbsoluteMaxLatVel() const 
    { return myAbsoluteMaxLatVel; }
  /// Sets the robot's absolute maximum lateral velocity
  AREXPORT bool setAbsoluteMaxLatVel(double maxVel);

  /// Gets the robot's absolute maximum lateral acceleration
  double getAbsoluteMaxLatAccel() const 
    { return myAbsoluteMaxLatAccel; }
  /// Sets the robot's absolute maximum lateral acceleration
  AREXPORT bool setAbsoluteMaxLatAccel(double maxAccel);

  /// Gets the robot's absolute maximum lateral deceleration
  double getAbsoluteMaxLatDecel() const 
    { return myAbsoluteMaxLatDecel; }
  /// Sets the robot's absolute maximum lateral deceleration
  AREXPORT bool setAbsoluteMaxLatDecel(double maxDecel);


  // Accessors

  /** @brief Get the current stored global position of the robot.
   *
   *  This position is updated by data reported by the robot as it
   *  moves, and may also be changed by other program components,
   *  such as a localization process (see moveTo()).
   *
   *  This position is also referred to as the robot's 
   *  "odometry" or "odometric" pose, since the robot
   *  uses its odometry data to determine this pose; but it
   *  may also incorporate additional data sources such as 
   *  an onboard gyro.  The term "odometric pose" also 
   *  distinguishes this position by the fact that its
   *  coordinate system may be arbitrary, and separate
   *  from any external coordinate system.
   *
   *  @sa getEncoderPose()
   *  @sa moveTo()
   *  @ingroup easy
   *
   */
  ArPose getPose() const { return myGlobalPose; }
  /// Gets the global X position of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getX() const { return  myGlobalPose.getX(); }
  /// Gets the global Y position of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getY() const { return myGlobalPose.getY(); }
  /// Gets the global angular position ("theta") of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getTh() const { return myGlobalPose.getTh(); }
  /// Gets the distance to a point from the robot's current position
  /// @ingroup easy
  double findDistanceTo(const ArPose& pose) const
    { return myGlobalPose.findDistanceTo(pose); }
  /// Gets the angle to a point from the robot's current position and orientation
  /// @ingroup easy
  double findAngleTo(const ArPose& pose) const
    { return myGlobalPose.findAngleTo(pose); }
  /// Gets the difference between the angle to a point from the robot's current heading
  /// @ingroup easy
  double findDeltaHeadingTo(const ArPose& pose) const
    { return ArMath::subAngle(myGlobalPose.findAngleTo(pose),
			      myGlobalPose.getTh()); }


  /// Gets the current translational velocity of the robot
  /// @ingroup easy
  double getVel() const { return myVel; }
  /// Gets the current rotational velocity of the robot
  /**
     Note that with new firmware versions (ARCOS as of April 2006 or so)
     this is the velocity reported by the robot.  With older firmware
     this number is calculated using the difference between the robot's reported wheel
     velocities multiplied by diffConvFactor from the .p (robot parameter) files.
      @ingroup easy
  **/
  double getRotVel() const { return myRotVel; }
  /// Gets the current lateral velocity of the robot
  /**
     Note that this will only be valid if hasLatVel() returns true
     @since 2.6.0
   **/
  double getLatVel() const { return myLatVel; }
  /** Sees if the robot supports lateral velocities (e.g. Seekur(TM))
      @since 2.6.0
  */
  bool hasLatVel() const { return myParams->hasLatVel(); }
  /// Gets the robot radius (in mm)
  /// @ingroup easy
  double getRobotRadius() const { return myParams->getRobotRadius(); }
  /// Gets the robot width (in mm)
  /// @ingroup easy
  double getRobotWidth() const { return myParams->getRobotWidth(); }
  /// Gets the robot length (in mm)
  /// @ingroup easy
  double getRobotLength() const { return myParams->getRobotLength(); }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthFront() const { return myRobotLengthFront; }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthRear() const { return myRobotLengthRear; }
  /// Gets distance from center of robot to first vertex of octagon approximating the shape of robot. (mm) 
  double getRobotDiagonal() const { return myParams->getRobotDiagonal(); }
  /// Gets the battery voltage of the robot (normalized to 12 volt system)
  /**
     This value is averaged over a number of readings, use getBatteryVoltageNow() to
     get the value most recently received.  (Access the number of readings
     used in the running average with getBatteryVoltageAverageOfNum() and
     setBatteryVoltageAverageOfNum().)

     This is a value normalized to 12 volts, if you want what the
     actual voltage of the robot is use getRealBatteryVoltage().

     @sa getRealBatteryVoltage()
     @sa getBatteryVoltageNow()
   **/
  double getBatteryVoltage() const {return myBatteryAverager.getAverage();}
  /// Gets the instantaneous battery voltage
  /**
     This is a value normalized to 12 volts, if you want what the
     actual voltage of the robot is use getRealBatteryVoltage().
     
     @sa getBatteryVoltage()
     @sa getRealBatteryVoltage()
   **/
  double getBatteryVoltageNow() const { return myBatteryVoltage; }
  /// Gets the real battery voltage of the robot 
  /**
     This value is averaged over a number of readings, use getRealBatteryVoltageNow() to
     get the value most recently received.  (Access the number of readings
     used in the running average with getRealBatteryVoltageAverageOfNum() and
     setRealBatteryVoltageAverageOfNum().)

     This is whatever the actual voltage of the robot is, if you want
     a value normalized to common 12 volts for all robot types use getBatteryVoltage(). 

     If the robot doesn't support a "real" battery voltage, then
     this method will just return the normal battery voltage (normalized to 12
     volt scale). (Most older robots that don't support a real battery voltage
     have 12 volts batteries anyway.)
   **/
  double getRealBatteryVoltage() const 
    { return myRealBatteryAverager.getAverage(); }
  /// Gets the instantaneous battery voltage
  /**
     This is whatever the actual voltage of the robot is, if you want
     a value normalized to a common 12 volts for all robot types use getBatteryVoltage().  If the
     robot doesn't support this number the voltage will be less than 0
     and you should use getBatteryVoltageNow().
  **/
  double getRealBatteryVoltageNow() const { return myRealBatteryVoltage; }
  /// Gets if the state of charge value is in use
  bool haveStateOfCharge() const { return myHaveStateOfCharge; }
  /// @copydoc haveStateOfCharge()
  bool hasStateOfCharge() const { return haveStateOfCharge(); }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfCharge() const 
    { if (!myHaveStateOfCharge) return 0; else return myStateOfCharge; }
  /// Gets the last time the state of charge was set
  ArTime getStateOfChargeSetTime() const { return myStateOfChargeSetTime; }
  /// Gets the state of charge that is considered low
  double getStateOfChargeLow() const 
    { return myStateOfChargeLow; }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfChargeShutdown() const 
    { return myStateOfChargeShutdown; }
  /// Gets the velocity of the left side of the robot in mm/s (use of getVel() and getRotVel() is preferred instead)
  double getLeftVel() const { return myLeftVel; }
  /// Gets the velocity of the right side of the robot in mm/s (use of getVel() and getRotVel() is preferred instead)
  double getRightVel() const { return myRightVel; }
  /// Gets the 2 bytes of stall and bumper flags from the robot 
  /** See robot operations manual SIP packet documentation for details.
   *  @sa isLeftMotorStalled()
   *  @sa isRightMotorStalled()
   *  @sa isFrontBumperTriggered()
   *  @sa isRearBumperTriggered()
   *  @sa ArBumpers
   */
  int getStallValue() const { return myStallValue; }
  /// Returns true if the left motor is stalled
  bool isLeftMotorStalled() const 
    { return (myStallValue & 0xff) & ArUtil::BIT0; }
  /// Returns true if the left motor is stalled
  bool isRightMotorStalled() const
    { return ((myStallValue & 0xff00) >> 8) & ArUtil::BIT0; }
  /// Returns true if the front bumper is triggered.   @sa ArBumpers
  bool isFrontBumperTriggered() const
  { return hasFrontBumpers() && ((((myStallValue & 0xff00) >> 8) & ~ArUtil::BIT0) != 0); }
  /// Returns true if the rear bumper is triggered. @sa ArBumpers
  bool isRearBumperTriggered() const
  { return hasRearBumpers() && (((myStallValue & 0xff) & ~ArUtil::BIT0) !=0 ); }

  /// Gets the legacy control heading
  /**
    Gets the control heading as an offset from the current heading.
    Only used in older Pioneer 2 and P2 robots with PSOS, P2OS, AROS.
    @see getTh
  */
  double getControl() const { return myControl; }
  /// Sets whether to keep the control value raw or not
  void setKeepControlRaw(bool keepControlRaw) 
    { myKeepControlRaw = keepControlRaw; }
  /// Gets whether the control value is kept raw 
  bool getKeepControlRaw() 
    { return myKeepControlRaw; }
  /// Gets the flags values
  int getFlags() const { return myFlags; }
  /// Gets the fault flags values
  int getFaultFlags() const { return myFaultFlags; }
  /// Gets whether or not we're getting the fault flags values
  bool hasFaultFlags() const { return myHasFaultFlags; }
  /// Gets the flags3 values
  int getFlags3() const { return myFlags3; }
  /// Gets whether or not we're getting the fault flags values
  bool hasFlags3() const { return myHasFlags3; }
  /// returns true if the motors are enabled
  bool areMotorsEnabled() const { return (myFlags & ArUtil::BIT0); }
  /// returns true if the sonars are enabled
  /** This used to just check the low level firmware values, but
   * that's now done by areSonarsEnabledLegacy.
   */ 
  bool areSonarsEnabled() const 
    { return mySonarEnabled; }
  /// returns true if the sonars are enabled for autonomous driving 
  bool areAutonomousDrivingSonarsEnabled() const 
    { return myAutonomousDrivingSonarEnabled; }
  /// returns true if the sonars are enabled on legacy platforms (by checking the low level stuff)
  bool areSonarsEnabledLegacy() const { 
    return (myFlags & 
	    (ArUtil::BIT1 | ArUtil::BIT2 | ArUtil::BIT3 | ArUtil::BIT4)); }
  /// returns true if the estop is pressed (or unrelieved)
  bool isEStopPressed() const { return (myFlags & ArUtil::BIT5); }

  /// Returns true if the E-Stop button is pressed
  bool getEstop() { return isEStopPressed(); }

  /// Gets the compass heading from the robot
  double getCompass() const { return myCompass; }
  /// Gets which analog port is selected
  int getAnalogPortSelected() const { return myAnalogPortSelected; }
  /// Gets the analog value
  unsigned char getAnalog() const { return myAnalog; }
  /// Gets the byte representing digital input status
  unsigned char getDigIn() const { return myDigIn; }
  /// Gets the byte representing digital output status
  unsigned char getDigOut() const { return myDigOut; }
  /// Gets the charge state of the robot (see long docs)
  AREXPORT ChargeState getChargeState() const;
  /// Gets the name of the charge state
  const char *getChargeStateName() const
  {
    switch(getChargeState())
    {
      case CHARGING_NOT:
        return "not charging";
      case CHARGING_BULK:
        return "bulk";
      case CHARGING_OVERCHARGE:
        return "overcharge";
      case CHARGING_FLOAT:
        return "float";
      case CHARGING_BALANCE:
        return "balance";
      case CHARGING_UNKNOWN:
      default:
        return "unknown";
    }
    return "unknown";
  }
  /// Gets if the robot is on a charger
  AREXPORT bool isChargerPowerGood() const;

  /// Gets the number of bytes in the analog IO buffer
  int getIOAnalogSize() const { return myIOAnalogSize; }
  /// Gets the number of bytes in the digital input IO buffer
  int getIODigInSize() const { return myIODigInSize; }
  /// Gets the number of bytes in the digital output IO buffer
  int getIODigOutSize() const { return myIODigOutSize; }

  /// Gets the n'th byte from the analog input data from the IO packet
  AREXPORT int getIOAnalog(int num) const;
  /// Gets the n'th byte from the analog input data from the IO packet
  AREXPORT double getIOAnalogVoltage(int num) const;
  /// Gets the n'th byte from the digital input data from the IO packet
  AREXPORT unsigned char getIODigIn(int num) const;
  /// Gets the n'th byte from the digital output data from the IO packet
  AREXPORT unsigned char getIODigOut(int num) const;

  /// Gets whether the robot has table sensing IR or not (see params in docs)
  bool hasTableSensingIR() const { return myParams->haveTableSensingIR(); }
  /// Returns true if the left table sensing IR is triggered
  AREXPORT bool isLeftTableSensingIRTriggered() const;
  /// Returns true if the right table sensing IR is triggered
  AREXPORT bool isRightTableSensingIRTriggered() const;
  /// Returns true if the left break beam IR is triggered
  AREXPORT bool isLeftBreakBeamTriggered() const;
  /// Returns true if the right break beam IR is triggered
  AREXPORT bool isRightBreakBeamTriggered() const;
  /// Returns the time received of the last IO packet
  ArTime getIOPacketTime() const { return myLastIOPacketReceivedTime; }

  /// Gets whether the robot has front bumpers (see ARCOS parameters in the robot manual)
  bool hasFrontBumpers() const { return myParams->haveFrontBumpers(); }
  /// Get the number of the front bumper switches
  unsigned int getNumFrontBumpers() const 
  {
    assert(myParams->numFrontBumpers() > 0);
    return (unsigned int)myParams->numFrontBumpers(); 
  }
  /// Gets whether the robot has rear bumpers (see ARCOS parameters in the robot manual)
  bool hasRearBumpers() const { return myParams->haveRearBumpers(); }
  /// Gets the number of  rear bumper switches
  unsigned int getNumRearBumpers() const 
  {
    assert(myParams->numRearBumpers() > 0);
    return (unsigned int) myParams->numRearBumpers(); 
  }

  /** @brief Get the position of the robot according to the last robot SIP,
   * possibly with gyro correction if installed and enabled, but without any
   * transformations applied.
   * @sa getPose()
   * @sa getRawEncoderPose()
   */
  ArPose getEncoderPose() const { return myEncoderPose; }
  /// Gets the encoder X position of the robot
  /** @sa getEncoderPose() */
  double getEncoderX() const { return  myEncoderPose.getX(); }
  /// Gets the encoder Y position of the robot
  /** @sa getEncoderPose() */
  double getEncoderY() const { return myEncoderPose.getY(); }
  /// Gets the encoder  angular position ("theta") of the robot
  /** @sa getEncoderPose() */
  double getEncoderTh() const { return myEncoderPose.getTh(); }

  /// Gets if the robot is trying to move or not 
  /**
     "Trying" to move means that some action or other command has
     requested motion, but another action or factor has cancelled
     that request. 
   
     This is so that if the robot is trying to move, but is prevented
     (mainly by an action) there will still be some indication that the
     robot is trying to move (e.g. we can prevent the sonar from automatically being
     turned off). Note that this flag
     doesn't have anything to do with if the robot is really moving or
     not, to check that, check the current velocities.
     @sa forceTryingToMove() to force this state on
   **/
  bool isTryingToMove() { return myTryingToMove; }

  /// Manually sets the flag that says the robot is trying to move for one cycle
  /**
     This is so that things that might move the robot at any time
     can can make the robot look
     like it is otherwise in the TryingToMove state, even if no motion
     is currently being requested.  For example, ArNetworking's teleoperation
     mode forces TryingToMove at all times while active, not just when
     requesting motion.  This method must be called in each task cycle,
     it is reset at the end of the task cycle (in state reflection stage)
     to its natural (non-forced) value.

     @sa isTryingToMove()
   **/
  void forceTryingToMove() { myTryingToMove = true; }
  

  /// Gets the number of motor packets received in the last second
  AREXPORT int getMotorPacCount() const;
  /// Gets the number of sonar returns received in the last second
  AREXPORT int getSonarPacCount() const;

  /// Gets the range of the last sonar reading for the given sonar
  /// @ingroup easy
  AREXPORT int getSonarRange(int num) const;
  /// Find out if the given sonar reading was newly refreshed by the last incoming SIP received.
  AREXPORT bool isSonarNew(int num) const;
  /// Find the number of sonar sensors (that the robot has yet returned values for)
  /// @ingroup easy
  int getNumSonar() const { return myNumSonar; }
  /// Returns the sonar reading for the given sonar
  /// @ingroup easy
  AREXPORT ArSensorReading *getSonarReading(int num) const;
  /// Returns the closest of the current sonar reading in the given range
  /// @ingroup easy
  AREXPORT int getClosestSonarRange(double startAngle, double endAngle) const;
  /// Returns the number of the sonar that has the closest current reading in the given range
  /// @ingroup easy
  AREXPORT int getClosestSonarNumber(double startAngle, double endAngle) const;

  /// Gets the robots name in ARIAs list
  AREXPORT const char *getName() const;
  /// Sets the robots name in ARIAs list
  AREXPORT void setName(const char *name);

  /// Change stored pose (i.e. the value returned by getPose())
  AREXPORT void moveTo(ArPose pose, bool doCumulative = true);
  /// Change stored pose (i.e. the value returned by getPose()) 
  AREXPORT void moveTo(ArPose to, ArPose from, bool doCumulative = true);
  /// Manually sets the current percentage that the robot is charged (argument is percentage, as a number between 0 and 100)
  AREXPORT void setStateOfCharge(double stateOfCharge);
  /// Sets the state of charge (percentage) that is considered to be low
  void setStateOfChargeLow(double stateOfChargeLow) 
    { myStateOfChargeLow = stateOfChargeLow; }
  /// Sets the state of charge that will cause a shutdown
  void setStateOfChargeShutdown(double stateOfChargeShutdown) 
    { myStateOfChargeShutdown = stateOfChargeShutdown; }
  /// Sets the charge state (for use with setting the state of charge)
  AREXPORT void setChargeState(ArRobot::ChargeState chargeState);
  /// Sets if we're on the charger (for use with setting the state of charge)
  AREXPORT void setIsChargerPowerGood(bool onCharger);

  /// Sets things so that the battery info from the microcontroller
  /// will be ignored
  AREXPORT void setIgnoreMicroControllerBatteryInfo(
	  bool ignoreMicroControllerBatteryInfo);
  /// Sets the battery info
  AREXPORT void setBatteryInfo(double realBatteryVoltage, 
			       double normalizedBatteryVoltage,
			       bool haveStateOfCharge,
			       double stateOfCharge);

  /// Gets the number of readings the battery voltage is the average of
  size_t getBatteryVoltageAverageOfNum() 
    { return myBatteryAverager.getNumToAverage(); }

  /// Sets the number of readings the battery voltage is the average of (default 20)
  void setBatteryVoltageAverageOfNum(size_t numToAverage)
    { myBatteryAverager.setNumToAverage(numToAverage); }

  /// Gets the number of readings the battery voltage is the average of
  size_t getRealBatteryVoltageAverageOfNum() 
    { return myRealBatteryAverager.getNumToAverage(); }

  /// Sets the number of readings the real battery voltage is the average of (default 20)
  void setRealBatteryVoltageAverageOfNum(size_t numToAverage)
    { myRealBatteryAverager.setNumToAverage(numToAverage); }

  /// Returns true if we have a temperature, false otherwise
  bool hasTemperature() 
    { if (myTemperature != -128) return true; else return false; }
  
  /// Gets the temperature of the robot, -128 if not available, -127 to 127 otherwise
  int getTemperature() const { return myTemperature; }

  /// Starts a continuous stream of encoder packets
  AREXPORT void requestEncoderPackets();

  /// Starts a continuous stream of IO packets
  AREXPORT void requestIOPackets();

  /// Stops a continuous stream of encoder packets
  AREXPORT void stopEncoderPackets();

  /// Stops a continuous stream of IO packets
  AREXPORT void stopIOPackets();

  /// Sees if we've explicitly requested encoder packets
  AREXPORT bool haveRequestedEncoderPackets();

  /// Sees if we've explicitly requested IO packets
  AREXPORT bool haveRequestedIOPackets();

  /// Gets packet data from the left encoder
  AREXPORT long int getLeftEncoder() const;

  /// Gets packet data from the right encoder
  AREXPORT long int getRightEncoder() const;

  /// Changes the transform
  AREXPORT void setEncoderTransform(const ArPose& deadReconPos,
				    const ArPose& globalPos);

  /// Changes the transform directly
  AREXPORT void setEncoderTransform(const ArPose& transformPos);

  /// Changes the transform directly
  AREXPORT void setEncoderTransform(const ArTransform &transform);

  /// Sets the encoder pose, for internal use
  void setEncoderPose(const ArPose& encoderPose) 
    { myEncoderPose = encoderPose; }

  /// Sets the raw encoder pose, for internal use
  void setRawEncoderPose(const ArPose& rawEncoderPose) 
    { myRawEncoderPose = rawEncoderPose; }
  
  /// Adds a callback for when the encoder transform is changed
  void addSetEncoderTransformCB(ArFunctor *functor, int position = 50)
    { mySetEncoderTransformCBList.addCallback(functor, position); }

  /// Removes a callback for when the encoder transform is changed
  void remSetEncoderTransformCB(ArFunctor *functor)
    { mySetEncoderTransformCBList.remCallback(functor); }

  /// Adds a callback that'll be used to see the time on the computer side (for an MTX)
  void setMTXTimeUSecCB(ArRetFunctor1<bool, 
					       uint32_t *> *functor)
    { myMTXTimeUSecCB = functor; }

  /// Gets the encoder transform
  ArTransform getEncoderTransform() const {
    return myEncoderTransform;
  }

  /// This gets the transform from local coords to global coords
  ArTransform getToGlobalTransform() const
  {
    return ArTransform(ArPose(0, 0, 0), getPose());
  }


  /// This gets the transform for going from global coords to local coords
  ArTransform getToLocalTransform() const
  {
    return ArTransform(getPose(), ArPose(0,0,0));
  }

  /// This applies a transform to all the robot range devices and to the sonar
  AREXPORT void applyTransform(const ArTransform& trans, bool doCumulative = true);

  /// Sets the dead recon position of the robot (normally set automatically from robot data)
  AREXPORT void setDeadReconPose(const ArPose& pose, const ArTime t = ArTime());

  /// This gets the distance the robot has travelled since the last time resetTripOdometer() was called (mm)
  /// This is a virtual odometer that measures the total linear distance the
  /// robot has travelled since first connected, or resetTripOdometer() was
  /// called, approximated by adding linear
  /// distance between current pose and previous pose in each ArRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  AREXPORT double getTripOdometerDistance() 
    { return myTripOdometerDistance; }

  /// This gets the number of degrees the robot has turned since the last time resetTripOdometer() was called (deg)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since first connected, or resetTripOdometer() was last
  /// called, approximated by adding linear
  /// distance between current pose and previous pose in each ArRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  /// @see getTripOdometerDistance()
  double getTripOdometerDegrees() 
    { return myTripOdometerDegrees; }

  /// This gets the time since the "Trip Odometer" was reset (sec)
  /// @see getTripOdometerDistance()
  long getTripOdometerTime() 
    { return myTripOdometerStart.secSince(); }

  /// Resets the "Trip Odometer"
  /// @see getTripOdometerDistance()
  AREXPORT void resetTripOdometer();

  /// This gets the total cumulative distance the robot has travelled (mm)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since ARIA connected, approximated by adding linear
  /// distance between current pose and previous pose in each ArRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  double getOdometerDistance() 
    { return myOdometerDistance; }
  /// @see getOdometerDistance
  double getOdometerDistanceMeters()
    { return myOdometerDistance/1000.0; }

  /// This gets the total cumulative number of degrees the robot has turned (deg)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since ARIA connected, approximated by adding linear
  /// distance between current pose and previous pose in each ArRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  double getOdometerDegrees() 
    { return myOdometerDegrees; }

  /// This gets the time since the robot started (sec)
  long getOdometerTime() 
    { return myOdometerStart.secSince(); }
  /// This gets the time since the robot started (mins)
  double getOdometerTimeMinutes()
    { return (double)myOdometerStart.secSince() / 60.0; }
  


  /// Adds a rangeDevice to the robot's list of them, and set the ArRangeDevice
  /// object's robot pointer to this ArRobot object.
  AREXPORT void addRangeDevice(ArRangeDevice *device);
  /// Remove a range device from the robot's list, by name
  AREXPORT void remRangeDevice(const char *name);
  /// Remove a range device from the robot's list, by instance
  AREXPORT void remRangeDevice(ArRangeDevice *device);

  /// Finds a rangeDevice in the robot's list
  AREXPORT const ArRangeDevice *findRangeDevice(const char *name, 
						bool ignoreCase = false) const;

  /// Finds a rangeDevice in the robot's list
  AREXPORT ArRangeDevice *findRangeDevice(const char *name, 
					  bool ignoreCase = false);

  [[deprecated]] std::list<ArRangeDevice *> *getRangeDeviceList() { return &myRangeDeviceList; }
  std::vector<ArRangeDevice*>& getRangeDevices() { return myRangeDeviceVector; }
  std::vector<ArRangeDevice*>::const_iterator getRangeDevicesBegin() { return myRangeDeviceVector.cbegin(); }
  std::vector<ArRangeDevice*>::const_iterator getRangeDevicesEnd() { return myRangeDeviceVector.cend(); }

  /// Finds whether a particular range device is attached to this robot or not
  AREXPORT bool hasRangeDevice(ArRangeDevice *device) const;

  /// Goes through all the range devices and checks them
  AREXPORT double checkRangeDevicesCurrentPolar(
	  double startAngle, double endAngle, double *angle = NULL,
	  const ArRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  /// Goes through all the range devices and checks them
  AREXPORT double checkRangeDevicesCumulativePolar(
	  double startAngle, double endAngle, double *angle = NULL,
	  const ArRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;


  // Goes through all the range devices and checks them
  AREXPORT double checkRangeDevicesCurrentBox(
	  double x1, double y1, double x2, double y2,
	  ArPose *readingPos = NULL,
	  const ArRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  // Goes through all the range devices and checks them
  AREXPORT double checkRangeDevicesCumulativeBox(
	  double x1, double y1, double x2, double y2,
	  ArPose *readingPos = NULL, 
	  const ArRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  /// Adds a laser to the robot's map of them
  AREXPORT bool addLaser(ArLaser *laser, int laserNumber, 
			 bool addAsRangeDevice = true);
  /// Remove a range device from the robot's list, by instance
  AREXPORT bool remLaser(ArLaser *laser, bool removeAsRangeDevice = true);
  /// Remove a range device from the robot's list, by number
  AREXPORT bool remLaser(int laserNumber, bool removeAsRangeDevice = true);

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  AREXPORT const ArLaser *findLaser(int laserNumber) const;

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  const ArLaser *findLaser(size_t laserNumber) const {
    assert(laserNumber <= INT_MAX);
    return findLaser(static_cast<int>(laserNumber));
  }

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  AREXPORT ArLaser *findLaser(int laserNumber);

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  ArLaser *findLaser(size_t laserNumber) {
    assert(laserNumber <= INT_MAX);
    return findLaser(static_cast<int>(laserNumber));
  }

  /// Gets the range device list
  AREXPORT const std::map<int, ArLaser *> *getLaserMap() const;

  /// Gets the range device list
  AREXPORT std::map<int, ArLaser *> *getLaserMap();

  /// Finds whether a particular range device is attached to this robot or not
  AREXPORT bool hasLaser(ArLaser *device) const;

  int getNumLasers() {
    const size_t size = myLaserMap.size();
    assert(size <= INT_MAX);
    return (int)size;
  }

#ifndef ARIA_WRAPPER
  /// Adds a battery to the robot's map of them
  /// @internal
  /// (ArBatteryConnector/ArRobotConnector will automatically add ArBatteryMTX
  /// object(s) if connected.)
  AREXPORT bool addBattery(ArBatteryMTX *battery, int batteryNumber);
  /// Remove a battery from the robot's list, by instance
  /// (MTX robots only)
  /// Primarily for ARIA internal use only.
  /// @internal
  AREXPORT bool remBattery(ArBatteryMTX *battery);
  /// Remove a battery from the robot's list, by number
  /// (MTX robots only)
  /// Primarily for ARIA internal use only.
  /// @internal
  AREXPORT bool remBattery(int batteryNumber);
#endif

  /// Finds a battery in the robot's list (@a batteryNumber indices start at 1)
  /// (MTX robots only)
  AREXPORT const ArBatteryMTX *findBattery(int batteryNumber) const;

  /// Finds a battery in the robot's list (@a batteryNumber indices start at 1)
  /// (MTX robots only)
  AREXPORT ArBatteryMTX *findBattery(int batteryNumber);

  /// Gets the battery list
  AREXPORT const std::map<int, ArBatteryMTX *> *getBatteryMap() const;

  /// Gets the battery list
  AREXPORT std::map<int, ArBatteryMTX *> *getBatteryMap();

  /// Finds whether a particular battery is attached to this robot or not
  AREXPORT bool hasBattery(ArBatteryMTX *device) const;

#ifndef ARIA_WRAPPER
  /// Adds a lcd to the robot's map of them
  /// @internal
  /// (ArLCDConnector/ArRobotConnector will automatically add LCD interfaces if
  /// connected.)
  AREXPORT bool addLCD(ArLCDMTX *lcd, int lcdNumber);
  /// Remove a lcd from the robot's list, by instance
  /// @internal
  /// (ArLCDConnector/ArRobotConnector will automatically add LCD interfaces if
  /// connected.)
  AREXPORT bool remLCD(ArLCDMTX *lcd);
  /// Remove a lcd from the robot's list, by number
  /// @internal
  /// (ArLCDConnector/ArRobotConnector will automatically add LCD interfaces if
  /// connected.)
  AREXPORT bool remLCD(int lcdNumber);
#endif

  /// Finds a lcd in the robot's list (@a lcdNumber indices start at 1)
  /// (MTX robots only) 
  AREXPORT const ArLCDMTX *findLCD(int lcdNumber = 1) const;

  /// Finds a lcd in the robot's list (@a lcdNumber indices start at 1)
  /// (MTX robots only) 
  AREXPORT ArLCDMTX *findLCD(int lcdNumber = 1);

  /// Gets the lcd list
  /// (MTX robots only) 
  AREXPORT const std::map<int, ArLCDMTX *> *getLCDMap() const;

  /// Gets the lcd list
  /// (MTX robots only) 
  AREXPORT std::map<int, ArLCDMTX *> *getLCDMap();

  /// Finds whether a particular lcd is attached to this robot or not
  /// (MTX robots only) 
  AREXPORT bool hasLCD(ArLCDMTX *device) const;


#ifndef ARIA_WRAPPER
  /// Adds a sonar to the robot's map of them
  /// @internal (ArSonarConnector/ArRobotConnector will automatically and MTX
  /// sonar interfaces upon connection)
  AREXPORT bool addSonar(ArSonarMTX *sonar, int sonarNumber);
  /// Remove a sonar from the robot's list, by instance
  /// @internal
  AREXPORT bool remSonar(ArSonarMTX *sonar);
  /// Remove a sonar from the robot's list, by number
  /// @internal
  AREXPORT bool remSonar(int sonarNumber);
#endif

  /// Finds a sonar in the robot's list (@a sonarNumber indices start at 1)
  /// (MTX robots only)
  AREXPORT const ArSonarMTX *findSonar(int sonarNumber) const;

  /// Finds a sonar in the robot's list (@a sonarNumber indices start at 1)
  /// (MTX robots only)
  AREXPORT ArSonarMTX *findSonar(int sonarNumber);

  /// Gets the sonar list
  /// (MTX robots only)
  AREXPORT const std::map<int, ArSonarMTX *> *getSonarMap() const;

  /// Gets the sonar list
  /// (MTX robots only)
  AREXPORT std::map<int, ArSonarMTX *> *getSonarMap();

  /// Finds whether a particular sonar is attached to this robot or not
  /// (MTX robots only)
  AREXPORT bool hasSonar(ArSonarMTX *device) const;

  /// Associates an ArPTZ object with this robot (see getPTZ())
  void setPTZ(ArPTZ *ptz) { myPtz = ptz; }
  /// Get PTZ interface associated with this robot.
  ArPTZ *getPTZ() { return myPtz; }

  /// Sets the number of milliseconds between state reflection refreshes
  /// if the state has not changed 
  AREXPORT void setStateReflectionRefreshTime(int msec);

  /// Gets the number of milliseconds between state reflection refreshes
  /// if the state has not changed 
  AREXPORT int getStateReflectionRefreshTime() const;

  /// Adds a packet handler to the list of packet handlers
  AREXPORT void addPacketHandler(
	  ArRetFunctor1<bool, ArRobotPacket *> *functor, 
	  ArListPos::Pos position = ArListPos::LAST);
  
  /// Removes a packet handler from the list of packet handlers
  AREXPORT void remPacketHandler(
	  ArRetFunctor1<bool, ArRobotPacket *> *functor);

  /// Adds a connect callback
  AREXPORT void addConnectCB(ArFunctor *functor, 
			     ArListPos::Pos position = ArListPos::LAST);
  /// Removes a connect callback
  AREXPORT void remConnectCB(ArFunctor *functor);

  /// Adds a callback for when a connection to the robot is failed
  AREXPORT void addFailedConnectCB(ArFunctor *functor, 
				   ArListPos::Pos position = ArListPos::LAST);
  /// Removes a callback for when a connection to the robot is failed
  AREXPORT void remFailedConnectCB(ArFunctor *functor);

  /// Adds a callback for when disconnect is called while connected
  AREXPORT void addDisconnectNormallyCB(ArFunctor *functor, 
				ArListPos::Pos position = ArListPos::LAST);
  /// Removes a callback for when disconnect is called while connected
  AREXPORT void remDisconnectNormallyCB(ArFunctor *functor);
  
  /// Adds a callback for when disconnection happens because of an error
  AREXPORT void addDisconnectOnErrorCB(ArFunctor *functor, 
				   ArListPos::Pos position = ArListPos::LAST);
  /// Removes a callback for when disconnection happens because of an error
  AREXPORT void remDisconnectOnErrorCB(ArFunctor *functor);

  /// Adds a callback for when the run loop exits for what ever reason
  AREXPORT void addRunExitCB(ArFunctor *functor, 
			     ArListPos::Pos position = ArListPos::LAST);
  /// Removes a callback for when the run loop exits for what ever reason
  AREXPORT void remRunExitCB(ArFunctor *functor);

  /// Suspend calling thread until the ArRobot is connected
  /// @ingroup easy
  AREXPORT WaitState waitForConnect(unsigned int msecs=0);
  /// Suspend calling thread until the ArRobot is connected or fails to connect
  AREXPORT WaitState waitForConnectOrConnFail(unsigned int msecs=0);
  /// Suspend calling thread until the ArRobot run loop has exited
  /// @ingroup easy
  AREXPORT WaitState waitForRunExit(unsigned int msecs=0);

  /// Wake up all threads waiting on this robot
  AREXPORT void wakeAllWaitingThreads();
  /// Wake up all threads waiting for connection
  AREXPORT void wakeAllConnWaitingThreads();
  /// Wake up all threads waiting for connection or connection failure
  AREXPORT void wakeAllConnOrFailWaitingThreads();
  /// Wake up all threads waiting for the run loop to exit
  AREXPORT void wakeAllRunExitWaitingThreads();

  /// Adds a user task to the list of synchronous tasks
  AREXPORT bool addUserTask(const char *name, int position, 
			       ArFunctor *functor,
			       ArTaskState::State *state = NULL);

  bool addUserTask(ArFunctor *functor, const char *name = NULL, int position = ArListPos::LAST)
  {
    return addUserTask(name, position, functor);
  }

  
  bool addUserTask(ArFunctor& functor, const char *name = NULL, int position = ArListPos::LAST)
  {
    return addUserTask(name, position, &functor);
  }

  /// Removes a user task from the list of synchronous tasks by name
  AREXPORT void remUserTask(const char *name);
  /// Removes a user task from the list of synchronous tasks by functor
  AREXPORT void remUserTask(ArFunctor *functor);

  /// Finds a user task by name
  AREXPORT ArSyncTask *findUserTask(const char *name);
  /// Finds a user task by functor
  AREXPORT ArSyncTask *findUserTask(ArFunctor *functor);
  
  /// Logs the list of user tasks, strictly for your viewing pleasure
  AREXPORT void logUserTasks() const;
  /// Logs the list of all tasks, strictly for your viewing pleasure
  AREXPORT void logAllTasks() const;

  /// Adds a sensor interpretation task. These are called during the ArRobot
  /// task synchronous cycle after robot data has been received (from the SIP
  /// and other robot packets).
  AREXPORT bool addSensorInterpTask(const char *name, int position, 
				       ArFunctor *functor,
	       			       ArTaskState::State *state = NULL);

  bool addSensorInterpTask(ArFunctor *functor, const char *name = NULL, int position = ArListPos::LAST)
  {
    return addSensorInterpTask(name, position, functor);
  }

  
  bool addSensorInterpTask(ArFunctor& functor, const char *name = NULL, int position = ArListPos::LAST)
  {
    return addSensorInterpTask(name, position, &functor);
  }

  /// Removes a sensor interp tasks by name
  AREXPORT void remSensorInterpTask(const char *name);
  /// Removes a sensor interp tasks by functor
  AREXPORT void remSensorInterpTask(ArFunctor *functor);

  /// Finds a task by name
  AREXPORT ArSyncTask *findTask(const char *name);
  /// Finds a task by functor
  AREXPORT ArSyncTask *findTask(ArFunctor *functor);

  /// Adds an action to the list with the given priority 
  /// @ingroup easy
  AREXPORT bool addAction(ArAction *action, int priority);
  /// Removes an action from the list, by pointer
  /// @ingroup easy
  AREXPORT bool remAction(ArAction *action);
  /// Removes an action from the list, by name
  /// @ingroup easy
  AREXPORT bool remAction(const char *actionName);
  /// Returns the first (highest priority) action with the given name (or NULL)
  AREXPORT ArAction *findAction(const char *actionName);
  /// Returns the map of actions... don't do this unless you really
  /// know what you're doing
  AREXPORT ArResolver::ActionMap *getActionMap();
  /// Deactivates all the actions
  AREXPORT void deactivateActions();

  /// Logs out the actions and their priorities
  /// @ingroup easy
  AREXPORT void logActions(bool logDeactivated = false) const;

  /// Gets the resolver the robot is using
  [[deprecated]]
  AREXPORT ArResolver *getResolver();

  /// Sets the resolver the robot is using
  [[deprecated]]
  AREXPORT void setResolver(ArResolver *resolver);

  /// Get last ArActionDesired obtained from running the action resolver
  const ArActionDesired *getLastActionResolverOutput() { return &myActionDesired; }

  /// Sets the encoderCorrectionCallback
  AREXPORT void setEncoderCorrectionCallback(
	  ArRetFunctor1<double, ArPoseWithTime> *functor);
  /// Gets the encoderCorrectionCallback
  AREXPORT ArRetFunctor1<double, ArPoseWithTime> *
          getEncoderCorrectionCallback() const;
  
  /// Gets the time the connection to the robot was made
  ArTime getConnectionOpenedTime() const 
    { return myConnectionOpenedTime; }

  // set up some of the internals of how the ArRobot class works
  /// Sets the number of ms between cycles
  AREXPORT void setCycleTime(unsigned int ms);
  /// Gets the number of ms between cycles
  AREXPORT unsigned int getCycleTime() const;
  /// Sets the number of ms between cycles to warn over
  AREXPORT void setCycleWarningTime(unsigned int ms);
  /// Gets the number of ms between cycles to warn over
  AREXPORT unsigned int getCycleWarningTime() const;
  /// Gets the number of ms between cycles to warn over
  AREXPORT unsigned int getCycleWarningTime();
  /// Sets the multiplier for how many cycles ArRobot waits when connecting
  AREXPORT void setConnectionCycleMultiplier(unsigned int multiplier);
  /// Gets the multiplier for how many cycles ArRobot waits when connecting
  AREXPORT unsigned int getConnectionCycleMultiplier() const;
  void setAsyncConnectSyncPacketRetries(int t) { myAsyncConnectSyncPacketRetries = t; }
  
  /// Sets whether to chain the robot cycle to when we get in SIP packets
  void setCycleChained(bool cycleChained) { myCycleChained = cycleChained; }
  /// Gets whether we chain the robot cycle to when we get in SIP packets
  bool isCycleChained() const { return myCycleChained; }
  /// Sets the time without a response until connection assumed lost (threadsafe)
  AREXPORT void setConnectionTimeoutTime(int mSecs);
  /// Gets the time without a response until connection assumed lost (threadsafe)
  AREXPORT int getConnectionTimeoutTime();
  /// Gets the time the last packet was received (threadsafe)
  AREXPORT ArTime getLastPacketTime();
  /// Gets the time the last odometry was received (threadsafe)
  AREXPORT ArTime getLastOdometryTime();

  /// Sets the number of packets back in time the ArInterpolation goes
  void setPoseInterpNumReadings(size_t numReadings) 
    { myInterpolation.setNumberOfReadings(numReadings); }

  /// Sets the number of packets back in time the position interpol goes
  size_t getPoseInterpNumReadings() const
    { return myInterpolation.getNumberOfReadings(); }
  
  
  /// Gets the position the robot was at at the given timestamp
  /** @see ArInterpolation::getPose 
   */
  int getPoseInterpPosition(ArTime timeStamp, ArPose *position,
				     ArPoseWithTime *mostRecent = NULL)
    { return myInterpolation.getPose(timeStamp, position, mostRecent); }

  /// Gets the pose interpolation object, this should only really used internally
  ArInterpolation *getPoseInterpolation()
    { return &myInterpolation; }

  /// Gets the callback that will call getPoseInterpPosition
  ArRetFunctor3<int, ArTime, ArPose *, ArPoseWithTime *> *
  getPoseInterpPositionCallback()
  { 
    return &myPoseInterpPositionCB; 
  }

  /// Sets the number of packets back in time the ArInterpolation goes for encoder readings
  void setEncoderPoseInterpNumReadings(size_t numReadings) 
    { myEncoderInterpolation.setNumberOfReadings(numReadings); }

  /// Sets the number of packets back in time the encoder position interpolation goes
  size_t getEncoderPoseInterpNumReadings() const
    { return myEncoderInterpolation.getNumberOfReadings(); }

  /// Gets the encoder position the robot was at at the given timestamp
  /** @see ArInterpolation::getPose 
   */
  int getEncoderPoseInterpPosition(ArTime timeStamp, ArPose *position, 
					    ArPoseWithTime *mostRecent = NULL)
    { return myEncoderInterpolation.getPose(timeStamp, position, mostRecent); }

  ArInterpolation *getEncoderPoseInterpolation()
    { return &myEncoderInterpolation; }

  /// Gets the callback that will call getPoseInterpPosition
  ArRetFunctor3<int, ArTime, ArPose *, ArPoseWithTime *> *
  getEncoderPoseInterpPositionCallback()
  { 
    return &myEncoderPoseInterpPositionCB; 
  }

  /// Applies the encoder motion from the passed in ArPoseWithTime to the passed in ArTime
  AREXPORT int applyEncoderOffset(ArPoseWithTime from, ArTime to, 
				  ArPose *result);

  /// Gets the Counter for the time through the loop
  unsigned int getCounter() const { return myCounter; }

  /// Gets the parameters the robot is using
  AREXPORT const ArRobotParams *getRobotParams() const;

  /// Sets if a config packet is required or not... 
  /** By default it is not required, since some ancient robots have no
      config packets... some of the modern software may die hideously
      if there is no config packet, so you should set this to true if
      you're using modern robots (basically anything other than a
      pioneer 1)
  **/
  void setRequireConfigPacket(bool requireConfigPacket)
    { myRequireConfigPacket = requireConfigPacket; }

  /// Gets the original robot config packet information
  AREXPORT const ArRobotConfigPacketReader *getOrigRobotConfig() const;

  /// Gets the battery packet reader
  ArRobotBatteryPacketReader *getBatteryPacketReader() 
    { return myBatteryPacketReader; }

  /// Sets the maximum translational velocity
  AREXPORT void setTransVelMax(double vel);
  /// Sets the maximum translational velocity
  AREXPORT void setTransNegVelMax(double negVel);
  /// Sets the translational acceleration
  AREXPORT void setTransAccel(double acc);
  /// Sets the translational acceleration
  AREXPORT void setTransDecel(double decel);
  /// Sets the maximum rotational velocity
  AREXPORT void setRotVelMax(double vel);
  /// Sets the rotational acceleration
  AREXPORT void setRotAccel(double acc);
  /// Sets the rotational acceleration
  AREXPORT void setRotDecel(double decel);
  /// Sets the maximum lateral velocity
  AREXPORT void setLatVelMax(double vel);
  /// Sets the lateral acceleration
  AREXPORT void setLatAccel(double acc);
  /// Sets the lateral acceleration
  AREXPORT void setLatDecel(double decel);

  /// If the robot has settable maximum velocities
  bool hasSettableVelMaxes() const 
    { return myParams->hasSettableVelMaxes(); }
  /// Gets the maximum translational velocity
  AREXPORT double getTransVelMax() const;
  /// Gets the maximum translational velocity
  AREXPORT double getTransNegVelMax() const;
  /// Gets the maximum rotational velocity
  AREXPORT double getRotVelMax() const;
  /// If the robot has settable accels and decels
  bool hasSettableAccsDecs()
      const { return myParams->hasSettableAccsDecs(); }
  /// Gets the translational acceleration
  AREXPORT double getTransAccel() const;
  /// Gets the translational acceleration
  AREXPORT double getTransDecel() const;
  /// Gets the rotational acceleration
  AREXPORT double getRotAccel() const;
  /// Gets the rotational acceleration
  AREXPORT double getRotDecel() const;

  /// Gets the maximum lateral velocity
  AREXPORT double getLatVelMax() const;
  /// Gets the lateral acceleration
  AREXPORT double getLatAccel() const;
  /// Gets the lateral acceleration
  AREXPORT double getLatDecel() const;

  /// Loads a parameter file (replacing all other params)
  AREXPORT bool loadParamFile(const char *file);
  /// Sets the robot to use a passed in set of params (passes ownership)
  AREXPORT void setRobotParams(ArRobotParams *params);

  /// Attach a key handler
  AREXPORT void attachKeyHandler(ArKeyHandler *keyHandler,
				 bool exitOnEscape = true,
				 bool useExitNotShutdown = true);
  /// Gets the key handler attached to this robot
  AREXPORT ArKeyHandler *getKeyHandler() const;

  /// Lock the robot instance
  int lock() {return(myMutex.lock());}
  /// Try to lock the robot instance without blocking
  int tryLock() {return(myMutex.tryLock());}
  /// Unlock the robot instance
  int unlock() {return(myMutex.unlock());}
  /// Turn on verbose locking of robot mutex
  void setMutexLogging(bool v) { myMutex.setLog(v); }
  /// Set robot lock warning time (see ArMutex::setLockWarningTime())
  void setMutexLockWarningTime(double sec) { myMutex.setLockWarningTime(sec); }
  /// Set robot lock-unlock warning time (see ArMutex::setUnlockWarningTime())
  void setMutexUnlockWarningTime(double sec) { myMutex.setUnlockWarningTime(sec); }

  /// This tells us if we're in the preconnection state
  bool isStabilizing() { return myIsStabilizing; }

  /// How long we should stabilize for in ms (0 disables stabilizing)
  AREXPORT void setStabilizingTime(int mSecs);

  /// How long we stabilize for in ms (0 means no stabilizing)
  AREXPORT int getStabilizingTime() const;


  /// Adds a callback called when the robot starts stabilizing before declaring connection
  AREXPORT void addStabilizingCB(ArFunctor *functor, 
			     ArListPos::Pos position = ArListPos::LAST);
  /// Removes stabilizing callback
  AREXPORT void remStabilizingCB(ArFunctor *functor);
  
  /// This gets the root of the synchronous task tree, only serious 
  /// developers should use it
  PUBLICDEPRECATED("")
  AREXPORT ArSyncTask *getSyncTaskRoot();

  /// This function loops once...  only serious developers should use it
  AREXPORT void loopOnce();

  /// Sets the delay in the odometry readings
  /**
     Note that this doesn't cause a delay, its informational so that
     the delay can be adjusted for and causes nothing to happen in
     this class.
  **/
  void setOdometryDelay(int msec) { myOdometryDelay = msec; }

  /// Gets the delay in odometry readings
  /**
     This gets the odometry delay, not that this is just information
     about what the delay is it doesn't cause anything to happen in
     this class.
  **/
  int getOdometryDelay() { return myOdometryDelay; }

  /// Gets if we're logging all the movement commands sent down
  bool getLogMovementSent() { return myLogMovementSent; }
  /// Sets if we're logging all the movement commands sent down
  void setLogMovementSent(bool logMovementSent)
    { myLogMovementSent = logMovementSent; }

  /// Gets if we're logging all the positions received from the robot
  bool getLogMovementReceived() { return myLogMovementReceived; }
  /// Sets if we're logging all the positions received from the robot
  void setLogMovementReceived(bool logMovementReceived)
    { myLogMovementReceived = logMovementReceived; }

  /// Gets if we're logging all the velocities (and heading) received
  bool getLogVelocitiesReceived() 
    { return myLogVelocitiesReceived; }
  /// Sets if we're logging all the velocities (and heading) received
  void setLogVelocitiesReceived(bool logVelocitiesReceived) 
    { myLogVelocitiesReceived = logVelocitiesReceived; }

  /// Gets if we're logging all the packets received (just times and types)
  bool getPacketsReceivedTracking()
    { return myPacketsReceivedTracking; }
  /// Sets if we're logging all the packets received (just times and types)
  AREXPORT void setPacketsReceivedTracking(bool packetsReceivedTracking);

  /// Gets if we're logging the contents of the standard SIP (motors packet)
  bool getLogSIPContents() { return myLogSIPContents; }
  /// Set if we're logging the contents of the standard SIP (motor packets)
  void setLogSIPContents(bool logSIP)  { myLogSIPContents = logSIP; }

  /// Gets if we're logging all the packets sent and their payload
  bool getPacketsSentTracking()
    { return myPacketsSentTracking; }
  /// Sets if we're logging all the packets sent and their payloads
  void setPacketsSentTracking(bool packetsSentTracking)
		{ 
		if (packetsSentTracking)
			ArLog::log(ArLog::Normal, "ArRobot: tracking packets sent");
		else
			ArLog::log(ArLog::Normal, "ArRobot: not tracking packets sent");
		myPacketsSentTracking = packetsSentTracking; 
		}

  /// Gets if we're logging all the actions as they happen
  bool getLogActions() { return myLogActions; }
  /// Sets if we're logging all the actions as they happen
  void setLogActions(bool logActions)
  { myLogActions = logActions; }


    

  /// Sets the flag that controls if the robot will switch baud rates
  void setDoNotSwitchBaud(bool doNotSwitchBaud = true)
    { myDoNotSwitchBaud = doNotSwitchBaud; } 

  /// Gets the flag that controls if the robot won't switch baud rates
  bool getDoNotSwitchBaud() { return myDoNotSwitchBaud; }
  
#ifndef ARIA_WRAPPER
  /// This is only for use by syncLoop
  /// @internal
  void incCounter() { myCounter++; }

  /// Packet Handler, internal (calls old or new as appropriate)
  /// @internal
  void packetHandler();
  /// Action Handler, internal
  /// @internal
  void actionHandler();
  /// State Reflector, internal
  /// @internal
  void stateReflector();

  /// Packet handler, internal, for use in the syncloop when there's no threading
  /// @internal
  void packetHandlerNonThreaded();
  /// Packet handler, internal, for use in the syncloop to handle the
  /// actual packets
  /// @internal
  void packetHandlerThreadedProcessor();
  /// Packet handler, internal, for using from a thread to actually
  /// read all the packets
  /// @internal
  void packetHandlerThreadedReader();

  /// For the key handler, escape calls this to exit, internal
  /// @internal
  void keyHandlerExit();

  /// Processes a motor packet, internal 
  /// @internal 
  bool processMotorPacket(ArRobotPacket *packet);
  /// Processes a new sonar reading, internal
  /// @internal
  void processNewSonar(int number, unsigned int range, ArTime timeReceived);
  /// Processes a new encoder packet, internal
  /// @internal
  bool processEncoderPacket(ArRobotPacket *packet);
  /// Processes a new IO packet, internal
  /// @internal
  bool processIOPacket(ArRobotPacket *packet);
  
  /// Internal function, shouldn't be used
  /// @internal
  void init();

  /// Internal function, shouldn't be used, sets up the default sync list
  /// @internal
  void setUpSyncList();
  /// Internal function, shouldn't be used, sets up the default packet handlers
  /// @internal
  void setUpPacketHandlers();
#endif

  ArRetFunctor1C<bool, ArRobot, ArRobotPacket *> myMotorPacketCB;
  ArRetFunctor1C<bool, ArRobot, ArRobotPacket *> myEncoderPacketCB;
  ArRetFunctor1C<bool, ArRobot, ArRobotPacket *> myIOPacketCB;
  ArFunctorC<ArRobot> myPacketHandlerCB;
  ArFunctorC<ArRobot> myActionHandlerCB;
  ArFunctorC<ArRobot> myStateReflectorCB;
  ArRetFunctorC<int, ArRobot> myRobotLockerCB;
  ArRetFunctorC<int, ArRobot> myRobotUnlockerCB;
  ArFunctorC<ArRobot> myKeyHandlerExitCB;
  ArFunctorC<ArKeyHandler> *myKeyHandlerCB;

  // These four are internal... only people monkeying deeply should mess
  // with them, so they aren't documented... these process the cblists
  // and such

#ifndef ARIA_WRAPPER

  // Internal function, shouldn't be used, does a single run of connecting
  int asyncConnectHandler(bool tryHarderToConnect);

  /// Internal function, shouldn't be used, drops the conn because of error
  /// @internal
  void dropConnection(const char *technicalReason = "Lost connection to the microcontroller because of undefined error.",
			       const char *userReason = NULL);
  /// Internal function that gets the reason the connection dropped
  /// @internal
  const char *getDropConnectionReason() { return myDropConnectionReason.c_str(); }
  /// Internal function that gets the reason the connection dropped
  /// @internal
  const char *getDropConnectionUserReason() { return myDropConnectionUserReason.c_str(); }
  /// Internal function, shouldn't be used, denotes the conn failed
  /// @internal
  void failedConnect();
  /// Internal function, shouldn't be used, does the initial conn stuff
  /// @internal
  bool madeConnection(bool resetConnectionTime = true);
  /// Internal function, shouldn't be used, calls the preconnected stuff
  /// @internal
  void startStabilization();
  /// Internal function, shouldn't be used, does the after conn stuff
  /// @internal
  void finishedConnection();
  /// Internal function, shouldn't be used, cancels the connection quietly
  /// @internal
  void cancelConnection();

  /// Internal function, takes a packet and passes it to the packet handlers,
  /// returns true if handled, false otherwise
  /// @internal
  bool handlePacket(ArRobotPacket *packet);

#endif // SWIG

  std::vector<ArFunctor*> getRunExitListCopy() { return myRunExitCBList; }
  friend class ArSyncLoop; ///< Needs to call getRunExitListCopy()

#ifndef ARIA_WRAPPER
  // Internal function, processes a parameter file
  void processParamFile();
#endif

public:
  /** @brief Get the position of the robot according to the last robot SIP only,
   *  with no correction by the gyro, other devices or software proceses.
   *
   * @note For the most accurate pose, use getPose() or getEncoderPose(); 
   * only use this method if you must have raw encoder pose with no correction.
   * @sa getPose()
   * @sa getEncoderPose()
   */
  ArPose getRawEncoderPose() const { return myRawEncoderPose; }

  /// Internal function for sync loop and sync task to see if we should warn this cycle or not
  bool getNoTimeWarningThisCycle()  
    { return myNoTimeWarningThisCycle; }
  /// Internal function for sync loop and sync task to say if we should warn this cycle or not
  void setNoTimeWarningThisCycle(bool noTimeWarningThisCycle)  
    { myNoTimeWarningThisCycle = noTimeWarningThisCycle; }

protected:
  // callbacks for warning time and if we should warn now to pass to sync tasks
  ArRetFunctorC<unsigned int, ArRobot> myGetCycleWarningTimeCB;
  ArRetFunctorC<bool, ArRobot> myGetNoTimeWarningThisCycleCB;
  /// internal function called when Aria::exit is called
  /// @internal
  void ariaExitCallback();
public:
  /// internal call that will let the robot connect even if it can't find params
  /// @internal
  void setConnectWithNoParams(bool connectWithNoParams) 
    { myConnectWithNoParams = connectWithNoParams; }
  /// Internal call that will get the thread the robot is running in
  /// @internal
  ArThread::ThreadType getOSThread() { return mySyncLoop.getOSThread(); }
  /// Internal call that will get the async connect state
  /// @internal
  int getAsyncConnectState() { return myAsyncConnectState; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  /// @internal
  /// Internal object used by ArRobot to send packets to the robot.
  /// @internal
  ArRobotPacketSender *getPacketSender()
    { return &mySender; }
  /// Internal object used by ArRobot to handle incoming packets.
  /// @internal
  ArRobotPacketReceiver *getPacketReceiver()
    { return &myReceiver; }
protected:

  // Gets a pointer to the robot parameters in an internal way so they can be modified (only for internal use)
  ArRobotParams *getRobotParamsInternal();

  // Sets if we've received the first encoder pose for very internal usage
  void internalSetFakeFirstEncoderPose(bool fakeFirstEncoderPose)
    { myFakeFirstEncoderPose = fakeFirstEncoderPose; }
  // Sets a flag to ignore the next packet handled, for very internal usage
  void internalIgnoreNextPacket();
protected:
  enum RotDesired { 
    ROT_NONE,
    ROT_IGNORE,
    ROT_HEADING,
    ROT_VEL
  };
  enum TransDesired {
    TRANS_NONE,
    TRANS_IGNORE,
    TRANS_VEL,
    TRANS_VEL2,
    TRANS_DIST,
    TRANS_DIST_NEW
  };
  enum LatDesired {
    LAT_NONE,
    LAT_IGNORE,
    LAT_VEL
  };
  void reset();
 

  // the config the robot had at connection
  ArRobotConfigPacketReader *myOrigRobotConfig;
  // the reader for the battery packet
  ArRobotBatteryPacketReader *myBatteryPacketReader;
  // the values we'll maintain for the different motion parameters
  double myRotVelMax;
  double myRotAccel;
  double myRotDecel;
  double myTransVelMax;
  double myTransNegVelMax;
  double myTransAccel;
  double myTransDecel;
  double myLatVelMax;
  double myLatAccel;
  double myLatDecel;

  ArPTZ *myPtz;
  bool myNoTimeWarningThisCycle;

  long int myLeftEncoder;
  long int myRightEncoder;
  bool myFirstEncoderPose;
  bool myFakeFirstEncoderPose;
  ArPoseWithTime myRawEncoderPose;

  ArTransform myEncoderTransform;
  ArCallbackList mySetEncoderTransformCBList;

  ArRetFunctor1<bool, uint32_t *> *myMTXTimeUSecCB;

  bool myIgnoreNextPacket;

  bool myLogMovementSent;
  bool myLogMovementReceived;
  bool myPacketsReceivedTracking;
  bool myLogActions;
  bool myLogVelocitiesReceived;
  bool myLogSIPContents;

  double myLastVel;
  double myLastRotVel;
  double myLastHeading;
  double myLastDeltaHeading;
  double myLastLatVel;

  bool myTryingToMove;

  long myPacketsReceivedTrackingCount;
  ArTime myPacketsReceivedTrackingStarted;
  bool myPacketsSentTracking;
  ArMutex myMutex;
  ArSyncTask *mySyncTaskRoot;
  std::vector<ArRetFunctor1<bool, ArRobotPacket *>*> myPacketHandlerList; 

  ArSyncLoop mySyncLoop;
  ArRobotPacketReaderThread myPacketReader;

  // the data items for reading packets in one thread and processing them in another
  ArMutex myPacketMutex;
  std::list<ArRobotPacket *> myPacketList;
  ArCondition myPacketReceivedCondition;
  bool myRunningNonThreaded;


  // TODO replace these with ArCallbackList (or simplified ArCallbackList with no indices)
  std::vector<ArFunctor*> myStabilizingCBList;
  std::vector<ArFunctor*> myConnectCBList;
  std::vector<ArFunctor*> myFailedConnectCBList;
  std::vector<ArFunctor*> myDisconnectNormallyCBList;
  std::vector<ArFunctor*> myDisconnectOnErrorCBList;
  std::vector<ArFunctor*> myRunExitCBList;

  ArRetFunctor1<double, ArPoseWithTime> *myEncoderCorrectionCB;
  std::list<ArRangeDevice *> myRangeDeviceList; // to support old getRangeDeviceList() accessor
  std::vector<ArRangeDevice *> myRangeDeviceVector;
  std::map<int, ArLaser *> myLaserMap;

  std::map<int, ArBatteryMTX *> myBatteryMap;

  std::map<int, ArLCDMTX *> myLCDMap;

  std::map<int, ArSonarMTX *> mySonarMap;

  ArCondition myConnectCond;
  ArCondition myConnOrFailCond;
  ArCondition myRunExitCond;

  ArResolver::ActionMap myActions;
  bool myOwnTheResolver;
  ArResolver *myResolver;

  std::map<int, ArSensorReading *> mySonars;
  int myNumSonar;
  
  unsigned int myCounter;
  bool myIsConnected;
  bool myIsStabilizing;
  bool myRequireConfigPacket;

  bool mySonarEnabled;
  bool myAutonomousDrivingSonarEnabled;

  bool myBlockingConnectRun;
  bool myAsyncConnectFlag;
  int myAsyncConnectState;
  int myAsyncConnectNoPacketCount;
  int myAsyncConnectTimesTried;
  int myAsyncConnectSyncPacketRetries;
  ArTime myAsyncStartedConnection;
  int myAsyncConnectStartBaud;
  ArTime myAsyncConnectStartedChangeBaud;
  bool myAsyncConnectSentChangeBaud;
  ArTime myStartedStabilizing;

  ArTime myConnectionOpenedTime;

  int myStabilizingTime;

  bool mySentPulse;

  double myTransVal;
  double myTransVal2;
  int myLastTransVal;
  int myLastTransVal2;
  TransDesired myTransType;
  TransDesired myLastTransType;
  ArTime myTransSetTime;
  ArTime myLastTransSent;
  int myLastActionTransVal;
  bool myActionTransSet;
  ArPose myTransDistStart;
  double myMoveDoneDist;

  double myRotVal;
  int myLastRotVal;
  RotDesired myRotType;
  RotDesired myLastRotType;
  ArTime myRotSetTime;
  ArTime myLastRotSent;
  int myLastActionRotVal;
  bool myLastActionRotHeading;
  bool myLastActionRotStopped;
  bool myActionRotSet;
  double myHeadingDoneDiff;

  double myLatVal;
  int myLastLatVal;
  LatDesired myLatType;
  LatDesired myLastLatType;
  ArTime myLatSetTime;
  ArTime myLastLatSent;
  int myLastActionLatVal;
  bool myActionLatSet;

  double myStoppedVel;
  double myStoppedRotVel;
  double myStoppedLatVel;

  double myLastSentTransVelMax;
  double myLastSentTransAccel;
  double myLastSentTransDecel;
  double myLastSentRotVelMax;
  double myLastSentRotVelPosMax;
  double myLastSentRotVelNegMax;
  double myLastSentRotAccel;
  double myLastSentRotDecel;
  double myLastSentLatVelMax;
  double myLastSentLatAccel;
  double myLastSentLatDecel;

  ArTime myLastPulseSent;

  unsigned int myDirectPrecedenceTime;
  
  int myStateReflectionRefreshTime;

  ArActionDesired myActionDesired;

  std::string myName;
  std::string myRobotName;
  std::string myRobotType;
  std::string myRobotSubType;

  double myAbsoluteMaxTransVel;
  double myAbsoluteMaxTransNegVel;
  double myAbsoluteMaxTransAccel;
  double myAbsoluteMaxTransDecel;
  double myAbsoluteMaxRotVel;
  double myAbsoluteMaxRotAccel;
  double myAbsoluteMaxRotDecel;
  double myAbsoluteMaxLatVel;
  double myAbsoluteMaxLatAccel;
  double myAbsoluteMaxLatDecel;

  ArDeviceConnection *myConn;

  ArRobotPacketSender mySender;
  ArRobotPacketReceiver myReceiver;

  std::string myDropConnectionReason;
  std::string myDropConnectionUserReason;

  ArRobotParams *myParams;
  double myRobotLengthFront;
  double myRobotLengthRear;

  ArInterpolation myInterpolation;
  ArInterpolation myEncoderInterpolation;

  ArKeyHandler *myKeyHandler;
  bool myKeyHandlerUseExitNotShutdown;

  bool myConnectWithNoParams;
  bool myWarnedAboutExtraSonar;

  // variables for tracking the data stream
  time_t myTimeLastMotorPacket;
  int myMotorPacCurrentCount;
  int myMotorPacCount;
  time_t myTimeLastSonarPacket;
  int mySonarPacCurrentCount;
  int mySonarPacCount;
  unsigned int myCycleTime;
  unsigned int myCycleWarningTime;
  unsigned int myConnectionCycleMultiplier;
  bool myCycleChained;
  ArTime myLastPacketReceivedTime;
  ArTime myLastOdometryReceivedTime;
  int myTimeoutTime;
  ArMutex myConnectionTimeoutMutex;

  bool myRequestedIOPackets;
  bool myRequestedEncoderPackets;

  // all the state reflecing variables
  ArPoseWithTime myEncoderPose;
  ArTime myEncoderPoseTaken;
  ArPose myGlobalPose;
  ArPose myLastGlobalPose;
  // um, this myEncoderGlobalTrans doesn't do anything
  ArTransform myEncoderGlobalTrans;
  double myLeftVel;
  double myRightVel;
  bool myIgnoreMicroControllerBatteryInfo;
  double myBatteryVoltage;
  ArRunningAverage myBatteryAverager;
  double myRealBatteryVoltage;
  ArRunningAverage myRealBatteryAverager;
  bool myHaveStateOfCharge;
  double myStateOfCharge;
  ArTime myStateOfChargeSetTime;
  double myStateOfChargeLow;
  double myStateOfChargeShutdown;

  int myStallValue;
  double myControl;
  bool myKeepControlRaw;
  int myFlags;
  int myFaultFlags;
  bool myHasFaultFlags;
  int myFlags3;
  bool myHasFlags3;
  double myCompass;
  int myAnalogPortSelected;
  unsigned char myAnalog;
  unsigned char myDigIn;
  unsigned char myDigOut;
  int myIOAnalog[128];
  unsigned char myIODigIn[255];
  unsigned char myIODigOut[255];
  int myIOAnalogSize;
  int myIODigInSize;
  int myIODigOutSize;
  ArTime myLastIOPacketReceivedTime;
  double myVel;
  double myRotVel;
  double myLatVel;
  int myLastX;
  int myLastY;
  int myLastTh;
  ChargeState myChargeState;
  bool myOverriddenChargeState;
  bool myIsChargerPowerGood;
  bool myOverriddenIsChargerPowerGood;
  
  char myTemperature;

  double myTripOdometerDistance;
  double myTripOdometerDegrees;
  ArTime myTripOdometerStart;

  double myOdometerDistance;
  double myOdometerDegrees;
  ArTime myOdometerStart;
  
  int myOdometryDelay;

  bool myDoNotSwitchBaud;
  
  bool myAddedAriaExitCB;
  ArFunctorC<ArRobot> myAriaExitCB;

  ArRetFunctor3C<int, ArRobot, ArTime, ArPose *, 
		 ArPoseWithTime *> myPoseInterpPositionCB;
  ArRetFunctor3C<int, ArRobot, ArTime, ArPose *, 
		 ArPoseWithTime *> myEncoderPoseInterpPositionCB;

};


#endif // ARROBOT_H
