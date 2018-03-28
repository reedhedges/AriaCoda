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
#ifndef ARMODES_H
#define ARMODES_H

#include "ariaTypedefs.h"
#include "ArMode.h"
#include "ArActionGroups.h"
#include "ArGripper.h"
#include "ArTcpConnection.h"
#include "ArSerialConnection.h"
#include "ArPTZ.h"
#include "ArRobotConfigPacketReader.h"

class ArTCMCompassRobot;
class ArACTS_1_2;
class ArRobotPacket;
class ArAnalogGyro;

/// Mode for teleoping the robot with joystick + keyboard
class ArModeTeleop : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeTeleop(ArRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeTeleop();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  //ArActionGroupTeleop myGroup;
  // use our new ratio drive instead
  ArActionGroupRatioDrive myGroup;
  ArFunctorC<ArRobot> myEnableMotorsCB;
};

/// Mode for teleoping the robot with joystick + keyboard
class ArModeUnguardedTeleop : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeUnguardedTeleop(ArRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeUnguardedTeleop();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  //ArActionGroupUnguardedTeleop myGroup;
  // use our new ratio drive instead
  ArActionGroupRatioDriveUnsafe myGroup;
  ArFunctorC<ArRobot> myEnableMotorsCB;
};

/// Mode for wandering around
class ArModeWander : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeWander(ArRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeWander();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  ArActionGroupWander myGroup;
};

/// Mode for controlling the gripper
class ArModeGripper : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeGripper(ArRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  AREXPORT virtual ~ArModeGripper();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void open(void);
  AREXPORT void close(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void stop(void);
  AREXPORT void exercise(void);
protected:
  enum ExerState {
    UP_OPEN,
    UP_CLOSE,
    DOWN_CLOSE,
    DOWN_OPEN
  };
  ArGripper myGripper;
  bool myExercising;
  ExerState myExerState;
  ArTime myLastExer;
  ArFunctorC<ArModeGripper> myOpenCB;
  ArFunctorC<ArModeGripper> myCloseCB;
  ArFunctorC<ArModeGripper> myUpCB;
  ArFunctorC<ArModeGripper> myDownCB;
  ArFunctorC<ArModeGripper> myStopCB;
  ArFunctorC<ArModeGripper> myExerciseCB;
  
};

/// Mode for controlling the camera
class ArModeCamera : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeCamera(ArRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  AREXPORT virtual ~ArModeCamera();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void left(void);
  AREXPORT void right(void);
  AREXPORT void center(void);
  AREXPORT void zoomIn(void);
  AREXPORT void zoomOut(void);
  AREXPORT void exercise(void);
  AREXPORT void toggleAutoFocus();
  AREXPORT void sony(void);
  AREXPORT void canon(void);
  AREXPORT void dpptu(void);
  AREXPORT void amptu(void);
  AREXPORT void canonInverted(void);
  AREXPORT void sonySerial(void);
  AREXPORT void canonSerial(void);
  AREXPORT void dpptuSerial(void);
  AREXPORT void amptuSerial(void);
  AREXPORT void canonInvertedSerial(void);
  AREXPORT void rvisionSerial(void);
  AREXPORT void com1(void);
  AREXPORT void com2(void);
  AREXPORT void com3(void);
  AREXPORT void com4(void);
  AREXPORT void usb0(void);
  AREXPORT void usb9(void);
  AREXPORT void aux1(void);
  AREXPORT void aux2(void);
protected:
  void takeCameraKeys(void);
  void giveUpCameraKeys(void);
  void helpCameraKeys(void);
  void takePortKeys(void);
  void giveUpPortKeys(void);
  void helpPortKeys(void);
  void takeAuxKeys(void);
  void giveUpAuxKeys(void);
  void helpAuxKeys(void);
  void takeMovementKeys(void);
  void giveUpMovementKeys(void);
  void helpMovementKeys(void);
  enum State {
    STATE_CAMERA,
    STATE_PORT,
    STATE_MOVEMENT
  };
  void cameraToMovement(void);
  void cameraToPort(void);
  void cameraToAux(void);
  void portToMovement(void);
  void auxToMovement(void);
  enum ExerState {
    CENTER,
    UP_LEFT,
    UP_RIGHT,
    DOWN_RIGHT,
    DOWN_LEFT
  };

  bool myExercising;
  State myState;
  ExerState myExerState;
  ArTime myLastExer;
  bool myExerZoomedIn;
  ArTime myLastExerZoomed;
  ArSerialConnection myConn;
  ArPTZ *myCam;
  ArFunctorC<ArModeCamera> myUpCB;
  ArFunctorC<ArModeCamera> myDownCB;
  ArFunctorC<ArModeCamera> myLeftCB;
  ArFunctorC<ArModeCamera> myRightCB;
  ArFunctorC<ArModeCamera> myCenterCB;
  ArFunctorC<ArModeCamera> myZoomInCB;
  ArFunctorC<ArModeCamera> myZoomOutCB;
  ArFunctorC<ArModeCamera> myExerciseCB;
  ArFunctorC<ArModeCamera> mySonyCB;
  ArFunctorC<ArModeCamera> myCanonCB;
  ArFunctorC<ArModeCamera> myDpptuCB;
  ArFunctorC<ArModeCamera> myAmptuCB;
  ArFunctorC<ArModeCamera> myCanonInvertedCB;
  ArFunctorC<ArModeCamera> mySonySerialCB;
  ArFunctorC<ArModeCamera> myCanonSerialCB;
  ArFunctorC<ArModeCamera> myDpptuSerialCB;
  ArFunctorC<ArModeCamera> myAmptuSerialCB;
  ArFunctorC<ArModeCamera> myCanonInvertedSerialCB;
  ArFunctorC<ArModeCamera> myRVisionSerialCB;
  ArFunctorC<ArModeCamera> myCom1CB;
  ArFunctorC<ArModeCamera> myCom2CB;
  ArFunctorC<ArModeCamera> myCom3CB;
  ArFunctorC<ArModeCamera> myCom4CB;
  ArFunctorC<ArModeCamera> myUSBCom0CB;
  ArFunctorC<ArModeCamera> myUSBCom9CB;
  ArFunctorC<ArModeCamera> myAux1CB;
  ArFunctorC<ArModeCamera> myAux2CB;
  const int myPanAmount;
  const int myTiltAmount;
  bool myAutoFocusOn;
  ArFunctorC<ArModeCamera> myToggleAutoFocusCB;
};

/// Mode for displaying the sonar
class ArModeSonar : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeSonar(ArRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeSonar();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void allSonar(void);
  AREXPORT void firstSonar(void);
  AREXPORT void secondSonar(void);
  AREXPORT void thirdSonar(void);
  AREXPORT void fourthSonar(void);
protected:
  enum State 
  {
    STATE_ALL,
    STATE_FIRST,
    STATE_SECOND,
    STATE_THIRD,
    STATE_FOURTH
  };
  State myState;
  ArFunctorC<ArModeSonar> myAllSonarCB;
  ArFunctorC<ArModeSonar> myFirstSonarCB;
  ArFunctorC<ArModeSonar> mySecondSonarCB;
  ArFunctorC<ArModeSonar> myThirdSonarCB;
  ArFunctorC<ArModeSonar> myFourthSonarCB;
};

class ArModeBumps : public ArMode
{
public:
  AREXPORT ArModeBumps(ArRobot *robot, const char *name, char key, char key2);
  AREXPORT ~ArModeBumps();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
};

class ArModePosition : public ArMode
{
public:
  AREXPORT ArModePosition(ArRobot *robot, const char *name, char key,
			  char key2, ArAnalogGyro *gyro = NULL);
  AREXPORT ~ArModePosition();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void left(void);
  AREXPORT void right(void);
  AREXPORT void stop(void);
  AREXPORT void reset(void);
  AREXPORT void mode(void);
  AREXPORT void gyro(void);
  AREXPORT void incDistance(void);
  AREXPORT void decDistance(void);
protected:
  enum Mode { MODE_BOTH, MODE_EITHER };
  ArAnalogGyro *myGyro;
  double myGyroZero;
  double myRobotZero;
  Mode myMode;
  std::string myModeString;
  bool myInHeadingMode;
  double myHeading;
  double myDistance;
  ArFunctorC<ArModePosition> myUpCB;
  ArFunctorC<ArModePosition> myDownCB;
  ArFunctorC<ArModePosition> myLeftCB;
  ArFunctorC<ArModePosition> myRightCB;
  ArFunctorC<ArModePosition> myStopCB;  
  ArFunctorC<ArModePosition> myResetCB;  
  ArFunctorC<ArModePosition> myModeCB;
  ArFunctorC<ArModePosition> myGyroCB;
  ArFunctorC<ArModePosition> myIncDistCB;
  ArFunctorC<ArModePosition> myDecDistCB;
};

class ArModeIO : public ArMode
{
public:
  AREXPORT ArModeIO(ArRobot *robot, const char *name, char key,
			  char key2);
  AREXPORT ~ArModeIO();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
protected:
  bool myExplanationReady;
  bool myExplained;
  ArTime myLastPacketTime;
  char myExplanation[1024];
  char myOutput[1024];
  ArFunctorC<ArModeIO> myProcessIOCB;
  void toggleOutput(int output);
  ArFunctor1C<ArModeIO, int> myTog1CB;
  ArFunctor1C<ArModeIO, int> myTog2CB;
  ArFunctor1C<ArModeIO, int> myTog3CB;
  ArFunctor1C<ArModeIO, int> myTog4CB;
  ArFunctor1C<ArModeIO, int> myTog5CB;
  ArFunctor1C<ArModeIO, int> myTog6CB;
  ArFunctor1C<ArModeIO, int> myTog7CB;
  ArFunctor1C<ArModeIO, int> myTog8CB;
};

class ArModeLaser : public ArMode
{
public:
  AREXPORT ArModeLaser(ArRobot *robot, const char *name, char key, char key2);
  AREXPORT ~ArModeLaser();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void switchToLaser(int laserNumber);

protected:
  void togMiddle(void);
  void togConnect(void);

  enum State {
    STATE_UNINITED,
    STATE_CONNECTING,
    STATE_CONNECTED
  };
  
  State myState;
  ArLaser *myLaser;
  int myLaserNumber;

  bool myPrintMiddle;

  ArFunctorC<ArModeLaser> myTogMiddleCB;

  std::map<int, ArLaser *> myLasers;
  std::map<int, ArFunctor1C<ArModeLaser, int> *> myLaserCallbacks;
};

/// Mode for following a color blob using ACTS
class ArModeActs : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeActs(ArRobot *robot, const char *name, char key, char key2,
		      ArACTS_1_2 *acts = NULL);
  /// Destructor
  AREXPORT virtual ~ArModeActs();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void channel1(void);
  AREXPORT virtual void channel2(void);
  AREXPORT virtual void channel3(void);
  AREXPORT virtual void channel4(void);
  AREXPORT virtual void channel5(void);
  AREXPORT virtual void channel6(void);
  AREXPORT virtual void channel7(void);
  AREXPORT virtual void channel8(void);
  AREXPORT virtual void stop(void);
  AREXPORT virtual void start(void);
  AREXPORT virtual void toggleAcquire(void);
  
protected:
  ArActionGroupColorFollow *myGroup;
  ArPTZ *camera;
  ArACTS_1_2 *myActs;
  ArRobot *myRobot;

  ArFunctorC<ArModeActs> myChannel1CB;
  ArFunctorC<ArModeActs> myChannel2CB;
  ArFunctorC<ArModeActs> myChannel3CB;
  ArFunctorC<ArModeActs> myChannel4CB;
  ArFunctorC<ArModeActs> myChannel5CB;
  ArFunctorC<ArModeActs> myChannel6CB;
  ArFunctorC<ArModeActs> myChannel7CB;
  ArFunctorC<ArModeActs> myChannel8CB;
  ArFunctorC<ArModeActs> myStopCB;
  ArFunctorC<ArModeActs> myStartCB;
  ArFunctorC<ArModeActs> myToggleAcquireCB;
};

class ArModeCommand : public ArMode
{
public:
  AREXPORT ArModeCommand(ArRobot *robot, const char *name, char key,
			  char key2);
  AREXPORT ~ArModeCommand();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
protected:
  void takeKeys(void);
  void giveUpKeys(void);
  void addChar(int ch);
  void finishParsing(void);
  void reset(bool print = true);
  char myCommandString[70];
  ArFunctor1C<ArModeCommand, int> my0CB;
  ArFunctor1C<ArModeCommand, int> my1CB;
  ArFunctor1C<ArModeCommand, int> my2CB;
  ArFunctor1C<ArModeCommand, int> my3CB;
  ArFunctor1C<ArModeCommand, int> my4CB;
  ArFunctor1C<ArModeCommand, int> my5CB;
  ArFunctor1C<ArModeCommand, int> my6CB;
  ArFunctor1C<ArModeCommand, int> my7CB;
  ArFunctor1C<ArModeCommand, int> my8CB;
  ArFunctor1C<ArModeCommand, int> my9CB;
  ArFunctor1C<ArModeCommand, int> myMinusCB;
  ArFunctor1C<ArModeCommand, int> myBackspaceCB;
  ArFunctor1C<ArModeCommand, int> mySpaceCB;
  ArFunctorC<ArModeCommand> myEnterCB;

};

/// Mode for following a color blob using ACTS
class ArModeTCM2 : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeTCM2(ArRobot *robot, const char *name, char key, char key2, ArTCM2 *tcm2 = NULL);
  /// Destructor
  AREXPORT virtual ~ArModeTCM2();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
  
protected:
  void init();
  ArTCM2 *myTCM2;
  ArCompassConnector *connector;
  ArRobot *myRobot;
  ArFunctorC<ArTCM2> *myOffCB;
  ArFunctorC<ArTCM2> *myCompassCB;
  ArFunctorC<ArTCM2> *myOnePacketCB;
  ArFunctorC<ArTCM2> *myContinuousPacketsCB;
  ArFunctorC<ArTCM2> *myUserCalibrationCB;
  ArFunctorC<ArTCM2> *myAutoCalibrationCB;
  ArFunctorC<ArTCM2> *myStopCalibrationCB;
  ArFunctorC<ArTCM2> *myResetCB;

};


/// Mode for requesting config packet
class ArModeConfig : public ArMode
{
public:
  /// Constructor
  AREXPORT ArModeConfig(ArRobot *robot, const char *name, char key, char key2);
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  
protected:
  ArRobot *myRobot;
  ArRobotConfigPacketReader myConfigPacketReader;
  ArFunctorC<ArModeConfig> myGotConfigPacketCB;

  void gotConfigPacket();
};



/// Mode for displaying status and diagnostic info
class ArModeRobotStatus : public ArMode
{
public:
  AREXPORT ArModeRobotStatus(ArRobot *robot, const char *name, char key, char key2);
  AREXPORT virtual void activate();
  AREXPORT virtual void deactivate();
  AREXPORT virtual void help();
  AREXPORT virtual void userTask();
  
protected:
  ArRobot *myRobot;
  ArRetFunctor1C<bool, ArModeRobotStatus, ArRobotPacket*> myDebugMessageCB;
  ArRetFunctor1C<bool, ArModeRobotStatus, ArRobotPacket*> mySafetyStateCB;
  ArRetFunctor1C<bool, ArModeRobotStatus, ArRobotPacket*> mySafetyWarningCB;

  void printFlags();
  void printFlagsHeader();

  //std::string byte_as_bitstring(unsigned char byte);
  //std::string int16_as_bitstring(ArTypes::Byte2 n);
  //std::string int32_as_bitstring(ArTypes::Byte4 n);

  bool handleDebugMessage(ArRobotPacket *p);
  bool handleSafetyStatePacket(ArRobotPacket *p);
  const char *safetyStateName(int state);
  bool handleSafetyWarningPacket(ArRobotPacket *p);

  bool myBatteryShutdown;
  void toggleShutdown();
  ArFunctorC<ArModeRobotStatus> myToggleShutdownCB;
};


#endif // ARMODES_H
