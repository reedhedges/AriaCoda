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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArVCC4.h"
#include "ArCommands.h"
#include "ArRobot.h"


AREXPORT ArVCC4Packet::ArVCC4Packet(ArTypes::UByte2 bufferSize) :
  ArBasePacket(bufferSize, 0)
{
}

AREXPORT ArVCC4Packet::~ArVCC4Packet()
{

}


AREXPORT void ArVCC4Packet::byte2ToBuf(ArTypes::Byte4 val)
{
  int i;
  char buf[5];
  if (myLength + 4 > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArVCC4Packet::uByte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }

  sprintf(buf, "%X", val);
  for (i=0;i<(int)strlen(buf);i++)
  {
    myBuf[myLength] = buf[i];
    ++myLength;
  }
}


/* Automatically tacks on footer char */
AREXPORT void ArVCC4Packet::finalizePacket(void)
{
  uByteToBuf(ArVCC4Commands::FOOTER);
}


/*
Creates new packet with default header, device id, and delimeter - FE 30 30 00
*/
AREXPORT void ArVCC4::preparePacket(ArVCC4Packet *myPacket)		
{
  myPacket->uByteToBuf(ArVCC4Commands::HEADER);
  myPacket->uByteToBuf(ArVCC4Commands::DEVICEID);
  myPacket->uByteToBuf(ArVCC4Commands::DEVICEID);
  myPacket->uByteToBuf(ArVCC4Commands::DELIM);

  myPacketTime.setToNow();
  
  if (myAutoUpdate)
    myIdleTime.setToNow();
}


/**
   @param robot the robot this camera is attached to

   @param inverted if this camera is inverted or not, the only time a
   camera will normally be inverted is on a robot where it's mounted
   on the underside of something, ie like in a peoplebot

   @param commDirection this is the type of communications that the camera should 
   use.  It can be unidirectional, bidirectional, or unknown.  If unidirectional
   it sends packets without knowing if the camera has received them or not.  This
   results in necessary 300 ms delays between packets, otherwise the packets
   will get dropped.  In bidirectional mode, responses are received from the
   camera and evaluated for success of receipt of the previous command.  In
   unknown mode, it will use bidirectional communication if a response is 
   received, otherwise it will be unidirectional.

   @param autoUpdate this will cause the usertask to periodically query the
   camera for actual positional information (pan, tilt, zoom).  This will
   happen every 1 sec idle time, and will switch between pan/tilt info
   and zoom info.

   @param disableLED if set to true this will force the LED, on the front
   of the camera, to turn off.  Otherwise it will function normally

   @param cameraType used to discriminate between VC-C4 and C50i
**/
AREXPORT ArVCC4::ArVCC4(ArRobot *robot, bool inverted, CommState commDirection, bool autoUpdate, bool disableLED, CameraType cameraType) :
  ArPTZ(robot),
  myTaskCB(this, &ArVCC4::camTask)
{
  myConn = NULL;
//  myInverted = inverted;
  ArPTZ::setInverted(inverted);
  myRobot = robot;
  myError = CAM_ERROR_NONE;
  myCommType = commDirection;

  myCameraType = cameraType;

  // the spec is 300ms, but give 400ms because of processing time for the loop
  myPacketTimeout = 400;

  myIdleTime.setToNow();

  myAutoUpdate = autoUpdate;
  myAutoUpdateCycle = 1;
  myWasError = false;
  myWaitingOnStop = false;
  myBytesLeft = 0;

  // set the state timeout based on the type of communication
  if (myCommType == COMM_BIDIRECTIONAL || myCommType == COMM_UNKNOWN)
  {
    myStateTimeout = BIDIRECTIONAL_TIMEOUT;
    ArLog::log(ArLog::Verbose,"ArVCC4::ArVCC4: Using bidirectional communication.");
  }
  else
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::ArVCC4: Using unidirectional communication.");
    myStateTimeout = UNIDIRECTIONAL_TIMEOUT;
  }

  // Set these to TOLERANCE +1 and desired's to 0, so it will be automatically
  // zero out during the first first passes through the state machine
  myPan = TOLERANCE + 1;
  myTilt = TOLERANCE + 1;
  myZoom = TOLERANCE + 1;
  myPanResponse = 0;
  myTiltResponse = 0;
  myZoomResponse = 0;
  myPanSlew = 0;
  myTiltSlew = 0;
  myFocusMode = -1;

  if (myCameraType == CAMERA_C50I)
    myDigitalZoom = -1;
  else
    myDigitalZoom = 0;

  
  myFOVAtMinZoom = 45;
  myFOVAtMaxZoom = 1.97;
/*
  myFOVAtMinZoom = 41.26;
  myFOVAtMaxZoom = 1.97;
*/
  /*myFOVAtMinZoom = 38;
  myFOVAtMaxZoom = 0.1;
  */
  ArPTZ::setLimits(MAX_PAN, MIN_PAN, MAX_TILT, MIN_TILT, MAX_ZOOM_OPTIC, MIN_ZOOM);

  if (disableLED)
    myDesiredLEDControlMode = 2;
  else
    myDesiredLEDControlMode = -1;

  myIRLEDsEnabled = false;
  myDesiredIRLEDsMode = false;
  // if C50I then force it to turn off the filter
  if (myCameraType == CAMERA_C50I)
    myIRFilterModeEnabled = true;
  else
    myIRFilterModeEnabled = false;
  myDesiredIRFilterMode = false;
  myFocusModeDesired = 0;

  myPanDesired = 0;
  myTiltDesired = 0;
  myZoomDesired = 0;
  myDigitalZoomDesired = 0;
  myPanSlewDesired = getMaxPanSlew();
  myTiltSlewDesired = getMaxTiltSlew();

  myRequestProductName = false;

  myPacketBufLen = 0;

  // initialize the state variables
  myState = UNINITIALIZED;
  myPreviousState = UNINITIALIZED;

  // Initialize flags to false
  myResponseReceived = false;
  myCameraHasBeenInitted = false;
  myInitRequested = false;
  myCameraIsInitted = false;
  myRealPanTiltRequested = false;
  myRealZoomRequested = false;

  // add the user task if we have a valid robot
  if (myRobot != NULL)
    myRobot->addUserTask("vcc4", 50, &myTaskCB);
}

AREXPORT ArVCC4::~ArVCC4()
{
  if (myRobot != NULL)
    myRobot->remUserTask(&myTaskCB);
}

void ArVCC4::connectHandler(void)
{
}

/*
  This will send a request for a certain number of bytes if it's not using
  a computer serial port.  You can also send 0, which will flush the buffer.

  This will let you either request 0 bytes (flush the buffer), 6 bytes, or
  more than 6 bytes by first requesting 6, then requesting all the rest at
  one time.  This will not work with more than two requests for one packet
  of data (i.e. 6 bytes, then 4 bytes, then 2 bytes to get 12 bytes total)
*/
void ArVCC4::requestBytes(int num)
{
  // only send a request if we're not using a computer serial port
  if (myUsingAuxPort && myCommType != COMM_UNIDIRECTIONAL)
  {
    // send a request to the robot, because we're using an Aux port
    if (myBytesLeft == 0)
    {
      // We're not waiting for more bytes from the camera, so assume
      // that this is the first request for a response.  Responses start
      // as 6 bytes, then more if needed.  Start by asking for 6
      
      // sending 0 will flush the buffer
      if (num == 0)
      {
	myRobot->comInt(myAuxRxCmd,0);
	return;
      }

      // don't ask for fewer than 6 bytes, because we don't know
      // how to handle it
      if (num < 6)
      {
	ArLog::log(ArLog::Terse, "ArVCC4::requestBytes: Requested fewer than 6 bytes total.  Not sending request.");
	return;
      }

      // we're not waiting for any bytes to come in, so request a full packet
      myRobot->comInt(myAuxRxCmd,6);

      // set the number of bytes left to 6 less than the request
      myBytesLeft = num - 6;
    }
    else
    {
      // request the rest of the bytes.  asumess num=myBytesLeft
      myRobot->comInt(myAuxRxCmd,num);
      myBytesLeft = 0;
    }
  }

  myWaitingOnPacket = true;

}


/*
This is the user task for the camera.  It controls the state that the camera is in and responds accordingly.

The POWERED_ON state will send commands as needed, and then switch the state into a state of waiting for a response.  If that state waits for too long without a response, it will timeout.  The states wait for a responseReceived flag, which says that a valid response packet was received back from the camera.  Based on that, it uses the myError variable to determine what the packet said.  If there is not responseReceived, or if operating in undirectional mode, the state will wait for a timeout, at which point it will fail if in bidirectional, or assume success in unidirectional mode.
*/
void ArVCC4::camTask(void)
{

  switch(myState)
  {
    // this is the case the camera starts in before being initialized
    case UNINITIALIZED:
      if (myConn == NULL)
	myConn = getDeviceConnection();

      // if myConn is still NULL then its an auxport
      if (myConn == NULL)
	myUsingAuxPort = true;
      else
        myUsingAuxPort = false;

      if (myInitRequested)
	switchState(STATE_UNKNOWN);

      break;
    // this case is the starting case, and fallback in case of error
    case STATE_UNKNOWN:
      ArLog::log(ArLog::Verbose,"ArVCC4::camTask: Attempting to power on and initialize.");
      myPowerStateDesired = true;
      myPowerState = false;
      myResponseReceived = false;

      // flush the buffer
      requestBytes(0);
      myBytesLeft = 0;
      myPacketBufLen = 0;
      switchState(AWAITING_INITIAL_POWERON);
      sendPower();
      break;
    // waiting for the camera to power on for the first time
    case AWAITING_INITIAL_POWERON:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(SETTING_CONTROL_MODE);
	  myPowerState = true;
	  myPowerStateDesired = true;
	  setControlMode();
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendPower();
	}
	else if (myError == CAM_ERROR_MODE)
	{
	  switchState(SETTING_CONTROL_MODE);
	  setControlMode();
	  myPowerState = false;
	  myPowerStateDesired = true;
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while executing a power command.");
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout(4000))
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: No response from the camera.  Using unidirectional communication.");
	  myCommType = COMM_UNIDIRECTIONAL;
	  myStateTimeout = UNIDIRECTIONAL_TIMEOUT;
	  myAutoUpdate = false;
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  if (myCommType == COMM_UNKNOWN)
	  {
	    ArLog::log(ArLog::Terse,"ArVCC4::camTask: No response from the camera.  Assuming unidirectional communications.");
	    myCommType = COMM_UNIDIRECTIONAL;
	    myStateTimeout = UNIDIRECTIONAL_TIMEOUT;
	    myAutoUpdate = false;
	  }
	  switchState(SETTING_CONTROL_MODE);
	  myPowerState = true;
	  myPowerStateDesired = true;
	  setControlMode();
	}
      }
      break;
    // waiting for the camera to initialize for the first time
    case AWAITING_INITIAL_INIT:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(SETTING_INIT_PAN_RATE);
	  sendPanSlew();
	}
	// in the event of busy or mode error, keep trying, camera may be 
	// powering on
	else if (myError == CAM_ERROR_BUSY || myError == CAM_ERROR_MODE)
	{
	  sendInit();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while executing an init command.");
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout(2500))
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out while executing an init command.");
	  switchState(STATE_UNKNOWN);
	}
	else 
	{
	  switchState(SETTING_INIT_PAN_RATE);
	  sendPanSlew();
	}
      }
      break;
    // set the control mode to host mode, otherwise commands will be ignored
    case SETTING_CONTROL_MODE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  sendInit();
	  switchState(AWAITING_INITIAL_INIT);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  setControlMode();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the control mode.");
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  sendInit();
	  switchState(AWAITING_INITIAL_INIT);
	}
      }
      break;
    // setting the initial pan rate
    case SETTING_INIT_PAN_RATE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myPanSlew = myPanSlewSent;
	  sendTiltSlew();
	  switchState(SETTING_INIT_TILT_RATE);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendPanSlew();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the pan rates.");
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding to an initialize pan slew command.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  myPanSlew = myPanSlewSent;
	  sendTiltSlew();
	  switchState(SETTING_INIT_TILT_RATE);
	}
      }
      break;
    // setting the initial tilt rate
    case SETTING_INIT_TILT_RATE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myTiltSlew = myTiltSlewSent;
	  setDefaultRange();
	  switchState(SETTING_INIT_RANGE);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendTiltSlew();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the tilt rate.");
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  myTiltSlew = myTiltSlewSent;
	  setDefaultRange();
	  switchState(SETTING_INIT_RANGE);
	}
      }
      break;
    // setting the initial range so the camera will tilt the full amount
    case SETTING_INIT_RANGE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(POWERED_ON);
	  myCameraHasBeenInitted = true;
	  myCameraIsInitted = true;
	  ArLog::log(ArLog::Verbose,"ArVCC4::camTask: Camera initialized and ready.");
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  setDefaultRange();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the default range.");
	  // try to power off and see if we can recover
	  myPowerStateDesired = false;
	  sendPower();
	  switchState(POWERING_OFF);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  // The camera sometimes responds with an error to this, or times out
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding to a set range command.  Power cycle the camera.");
	  myCameraHasBeenInitted = false;
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  ArLog::log(ArLog::Verbose,"ArVCC4::camTask: Camera initialized and ready.");
	  myCameraHasBeenInitted = true;
	  myCameraIsInitted = true;
	  switchState(POWERED_ON);
	}
      }
      break;
    // initializing the camera
    case INITIALIZING:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myCameraIsInitted = true;
	  myInitRequested = false;
	  // delay the state transition by 2500ms to allow init to take place
	  switchState(POWERED_ON, 5000);
	  ArLog::log(ArLog::Verbose,"ArVCC4::camTask: Camera initialized.");
	}
	else if (myError == CAM_ERROR_MODE)
	{
	  setControlMode();
	  switchState(SETTING_CONTROL_MODE);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendInit();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while initializing the camera.");
	  myCameraIsInitted = false;
	  myInitRequested = false;
	  switchState(STATE_UNKNOWN);
	}
	myResponseReceived = false;
      }
      else if (timeout(2000))
      {
	myInitRequested = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding to an initialization request.");
	  myCameraIsInitted = false;
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  myCameraIsInitted = true;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting for the camera to power on
    case POWERING_ON:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myPowerState = true;
	  myPowerStateDesired = true;
	  if (myCameraHasBeenInitted == false)
	  {
	    myPowerState = true;
	    myPowerStateDesired = true;
	    setControlMode();
	    switchState(SETTING_CONTROL_MODE);
	  }
	  else
	    switchState(POWERED_ON, 4500); // use a delay
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendPower();
	}
	else
	{
	  ArLog::log(ArLog::Terse, "ArVCC4::camTask: Error while executing power command.");
	  switchState(POWERED_OFF);
	}
	myResponseReceived = false;
      }
      else if (timeout(4500))
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding to a power on command.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  myPowerState = true;
	  myPowerStateDesired = true;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting for the camera to power on
    case POWERING_OFF:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myPowerState = false;
	  myPowerStateDesired = false;
	  switchState(POWERED_OFF);
	  ArLog::log(ArLog::Verbose, "ArVCC4::camTask: Camera powered off.");
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendPower();
	}
	else
	{
	  ArLog::log(ArLog::Terse, "ArVCC4::camTask: Error while executing power command.");
	  switchState(POWERED_ON);
	}
	myResponseReceived = false;
      }
      else if (timeout(2000))
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  myPowerState = false;
	  myPowerStateDesired = false;
	  switchState(POWERED_OFF);
	}
      }
      break;
    // idle state.  This is the main state.  This will call other processes
    // in the event that a desired state doesn't match the current ones
    case POWERED_ON:
      if (myCameraHasBeenInitted == false || isInitted() == false)
      {
	ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera not initialized.");
	switchState(STATE_UNKNOWN);
      }
      else if (myPowerStateDesired == false)
      {
	sendPower();
	switchState(POWERING_OFF);
      }
      else if (myInitRequested == true)
      {
	sendInit();
	switchState(INITIALIZING);
      }
      else if (myRequestProductName == true)
      {
	sendProductNameRequest();
	switchState(AWAITING_PRODUCTNAME_REQUEST);
      }
      else if (myRealPanTiltRequested == true)
      {
	sendRealPanTiltRequest();
	switchState(AWAITING_POS_REQUEST);
      }
      else if (myRealZoomRequested == true)
      {
	sendRealZoomRequest();
	switchState(AWAITING_ZOOM_REQUEST);
      }
      else if (myHaltZoomRequested == true)
      {
	sendHaltZoom();
	switchState(AWAITING_STOP_ZOOM_RESPONSE);
      }
      else if (myHaltPanTiltRequested == true)
      {
	sendHaltPanTilt();
	switchState(AWAITING_STOP_PAN_TILT_RESPONSE);
      }
      else if (fabs(myPan - myPanDesired) > TOLERANCE || 
	       fabs(myTilt - myTiltDesired) > TOLERANCE)
      {
	// pan tilt sets its own state because it might stop first
	sendPanTilt();
      }
      else if (myZoom != myZoomDesired)
      {
	// zoom sets its own state because it might stop first
	sendZoom();
      }
      else if (myDigitalZoom != myDigitalZoomDesired)
      {
	if (myCameraType == CAMERA_C50I)
	{
	  sendDigitalZoom();
	  switchState(AWAITING_DIGITAL_ZOOM_RESPONSE);
	}
	else
	{
	  ArLog::log(ArLog::Terse, "ArVCC4::camTask: Camera type does not support digital zoom.");
	  myDigitalZoom = 0;
	  myDigitalZoomDesired = 0;
	}
      }
      else if (myFocusMode != myFocusModeDesired)
      {
	sendFocus();
	switchState(AWAITING_FOCUS_RESPONSE);
      }
      else if (myPanSlewDesired != myPanSlew)
      {
	sendPanSlew();
	switchState(AWAITING_PAN_SLEW_RESPONSE);
      }
      else if (myTiltSlewDesired != myTiltSlew)
      {
	sendTiltSlew();
	switchState(AWAITING_TILT_SLEW_RESPONSE);
      }
      else if (myDesiredLEDControlMode != -1)
      {
	sendLEDControlMode();
	switchState(AWAITING_LED_CONTROL_RESPONSE);
      }
      else if (myDesiredIRFilterMode != myIRFilterModeEnabled)
      {
	if (myCameraType == CAMERA_C50I)
	{
	  sendIRFilterControl();
	  switchState(AWAITING_IRFILTER_RESPONSE);
	}
	else
	{
	  ArLog::log(ArLog::Terse, "ArVCC4::camTask: Camera type does not support IR filtering.");
	  myDesiredIRFilterMode = false;
	}
      }
      else if (myDesiredIRLEDsMode != myIRLEDsEnabled)
      {
	if (myCameraType == CAMERA_C50I)
	{
	  if (myDesiredIRLEDsMode && !myIRFilterModeEnabled)
	  {
	    ArLog::log(ArLog::Terse, "ArVCC4::camTask: Need to first enable IR-filter before turning on InfraRed LEDs.");
	    myDesiredIRLEDsMode = false;
	  }
	  else
	  {
	    sendIRLEDControl();
	    switchState(AWAITING_IRLEDS_RESPONSE);
	  }
	}
	else
	{
	  ArLog::log(ArLog::Terse, "ArVCC4::camTask: Camera model does not support IR LED functions.");
	  myDesiredIRLEDsMode = false;
	}
      }
      else if (myAutoUpdate &&
	  myCommType == COMM_BIDIRECTIONAL &&
	  myIdleTime.mSecSince() > AUTO_UPDATE_TIME)
      {
	switch (myAutoUpdateCycle)
	{
	  case 1:
	    sendRealPanTiltRequest();
	    switchState(AWAITING_POS_REQUEST);
	    break;
	  case 2:
	    sendRealZoomRequest();
	    switchState(AWAITING_ZOOM_REQUEST);
	    break;
	  default:
	    myAutoUpdateCycle = 0;
	    break;
	}
	myAutoUpdateCycle++;
      }
      break;
    // camera is powered off
    case POWERED_OFF:
      if (myPowerStateDesired == true)
      {
	sendPower();
	switchState(POWERING_ON);
      }
      break;
    // waiting to hear back from the camera after a zoom command was sent
    case AWAITING_ZOOM_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myZoom = myZoomDesired;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendHaltZoom();
	  // switch states, but don't reset the timer
	  switchState(AWAITING_STOP_ZOOM_RESPONSE, false);
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while executing a zoom command.");
	  myZoomDesired = myZoom;
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  myZoomDesired = myZoom;
	  switchState(myPreviousState);
	}
	else
	{
	  myZoom = myZoomDesired;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting to hear back for verification from digital zoom command
    case AWAITING_DIGITAL_ZOOM_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myDigitalZoom = myDigitalZoomDesired;
	  switchState(POWERED_ON);
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while executing a digital zoom command.");
	  myDigitalZoomDesired = myDigitalZoom;
	  switchState(myPreviousState);
	}
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  myDigitalZoomDesired = myDigitalZoom;
	  switchState(myPreviousState);
	}
	else
	{
	  myDigitalZoom = myDigitalZoomDesired;
	  switchState(POWERED_ON);
	}
      }
      break;
      // waiting to hear back for verification of a pan/tilt command
    case AWAITING_PAN_TILT_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myPan = myPanSent;
	  myTilt = myTiltSent;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendHaltPanTilt();
	  // switch states, but don't reset the timer
	  switchState(AWAITING_STOP_PAN_TILT_RESPONSE, false);
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while executing a panTilt command.");
	  myPanDesired = myPan;
	  myTiltDesired = myTilt;
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  myTiltDesired = myTilt;
	  myPanDesired = myPan;
	  if (myPowerState)
	    switchState(POWERED_ON);
	  else
	    switchState(POWERED_OFF);
	}
	else
	{
	  myTilt = myTiltSent;
	  myPan = myPanSent;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting to hear back after requesting to stop pan/tilt movements
    case AWAITING_STOP_PAN_TILT_RESPONSE:
      if (myResponseReceived == true)
      {
	myResponseReceived = false;
	if (myError == CAM_ERROR_NONE)
	{
	  myHaltPanTiltRequested = false;
	  switchState(POWERED_ON);
	  camTask();
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  // A busy error is only generated when initializing. so just 
	  // return to the previous state
	  myHaltPanTiltRequested = false;
	  switchState(myPreviousState);
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while stopping panTilt motion.");
	  myHaltPanTiltRequested = false;
	  switchState(myPreviousState);
	}
      }
      else if (timeout())
      {
	myHaltPanTiltRequested = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  if (myWaitingOnStop == true)
	  {
	    sendPanTilt();
	    myWaitingOnStop = false;
	    switchState(AWAITING_PAN_TILT_RESPONSE);
	  }
	  else
	    switchState(myPreviousState);
	}
      }
      break;
    // waiting to hear back about a stop zooming command
    case AWAITING_STOP_ZOOM_RESPONSE:
      if (myResponseReceived == true)
      {
	myResponseReceived = false;
	if (myError == CAM_ERROR_NONE)
	{
	  myHaltZoomRequested = false;
	  switchState(POWERED_ON);
	  camTask();
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  // A busy error is only generated when initializing. so just
	  // return to the previous state
	  myHaltZoomRequested = false;
	  switchState(myPreviousState);
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while the zoom.");
	  myHaltZoomRequested = false;
	  switchState(myPreviousState);
	}
      }
      else if (timeout())
      {
	myHaltZoomRequested = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  if (myWaitingOnStop == true)
	  {
	    sendZoom();
	    myWaitingOnStop = false;
	    switchState(AWAITING_ZOOM_RESPONSE);
	  }
	  else
	    switchState(myPreviousState);
	}
      }
      break;
    // waiting to hear back about a pan speed setting
    case AWAITING_PAN_SLEW_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myPanSlew = myPanSlewSent;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendPanSlew();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting pan slew.");
	  myPanSlewDesired = myPanSlew;
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  myPanSlewDesired = myPanSlew;
	  switchState(myPreviousState);
	}
	else
	{
	  myPanSlew = myPanSlewSent;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting to hear back about a tilt speed setting
    case AWAITING_TILT_SLEW_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myTiltSlew = myTiltSlewSent;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendTiltSlew();
	}
	else
	{                                               
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting tilt slew.");
	  myTiltSlewDesired = myTiltSlew;
	  switchState(myPreviousState);
	} 
	myResponseReceived = false;
      }                                                     
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  myTiltSlewDesired = myTiltSlew;
	  switchState(myPreviousState);
	}
	else
	{
	  myTiltSlew = myTiltSlewSent;
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting to hear back with pan/tilt information
    case AWAITING_POS_REQUEST:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(POWERED_ON);
	  myPan = myPanResponse;
	  myTilt = myTiltResponse;
	  myRealPanTiltRequested = false;
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendRealPanTiltRequest();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendRealPanTiltRequest: Camera responded with an error while requesting pan/tilt information.");
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	myRealPanTiltRequested = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendRealPanTiltRequest: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  switchState(POWERED_ON);
	}
      }
      break;
    // waiting to hear back with zoom information
    case AWAITING_ZOOM_REQUEST:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(POWERED_ON);
	  myZoom = myZoomResponse;
	  myRealZoomRequested = false;
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendRealZoomRequest();
	}
	else
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendRealZoomRequest: Camera responded with an error while requesting zoom position.");
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	myRealZoomRequested = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendRealZoomRequest: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  switchState(POWERED_ON);
	}
      }
      break;
    // sit in this state until the specified delay has occurred
    case AWAITING_LED_CONTROL_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myDesiredLEDControlMode = -1;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendLEDControlMode();
	}
	else
	{
	  myDesiredLEDControlMode = -1;
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the LED control mode.");
	  switchState(POWERED_ON);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  switchState(myPreviousState);
	}
      }
      break;
    case AWAITING_IRFILTER_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myIRFilterModeEnabled = myDesiredIRFilterMode;

	  // the camera automatically shuts off the IR LEDs when removing
	  // the filter
	  if (!myIRFilterModeEnabled)
	    myIRLEDsEnabled = false;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendIRFilterControl();
	}
	else
	{
	  myIRFilterModeEnabled = myDesiredIRFilterMode;
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the infrared cutoff control.");
	  switchState(POWERED_ON);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	myDesiredIRFilterMode = myIRFilterModeEnabled;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  switchState(myPreviousState);
	}
      }
      break;
    case AWAITING_IRLEDS_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  myIRLEDsEnabled = myDesiredIRLEDsMode;
	  switchState(POWERED_ON);
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendIRLEDControl();
	}
	else
	{
	  myDesiredIRLEDsMode = myIRLEDsEnabled;
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera responded with an error while setting the infrared light.");
	  switchState(POWERED_ON);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	myDesiredIRLEDsMode = myIRLEDsEnabled;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::camTask: Camera timed out without responding.");
	  switchState(STATE_UNKNOWN);
	}
	else
	{
	  switchState(myPreviousState);
	}
      }
      break;
    case AWAITING_PRODUCTNAME_REQUEST:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(POWERED_ON);
	  myProductName = myProductNameResponse;
	  myRequestProductName = false;
	}
	else if (myError == CAM_ERROR_BUSY)
	{
	  sendProductNameRequest();
	}
	else
	{
	  myRequestProductName = false;
	  ArLog::log(ArLog::Terse,"ArVCC4::sendProductNameRequest: Camera responded with an error while requesting product name.");
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	myRequestProductName = false;
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendProductNameRequest: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  switchState(POWERED_ON);
	}
      }
      break;
    case AWAITING_FOCUS_RESPONSE:
      if (myResponseReceived == true)
      {
	if (myError == CAM_ERROR_NONE)
	{
	  switchState(POWERED_ON);
	  myFocusMode = myFocusModeDesired;
	}
	else
	{
	  myFocusModeDesired = myFocusMode;
	  ArLog::log(ArLog::Terse,"ArVCC4::sendFocus: Camera responded with an error while setting the focus.");
	  switchState(myPreviousState);
	}
	myResponseReceived = false;
      }
      else if (timeout())
      {
	if (myCommType == COMM_BIDIRECTIONAL)
	{
	  ArLog::log(ArLog::Terse,"ArVCC4::sendFocus: Camera timed out without responding.");
	  switchState(myPreviousState);
	}
	else
	{
	  switchState(POWERED_ON);
	}
      }
      break;
    case STATE_DELAYED_SWITCH:
      if (timeout(myStateDelayTime))
      {
	myState = myPreviousState;
	switchState(myNextState);
      }
      break;
    case STATE_ERROR:
    default:
      ArLog::log(ArLog::Terse,"ArVCC4::camTask: Unknown case in usertask.");
      break;
  }

  // if there were bad parameters, a control mode error, or an unknown error, 
  if (myWasError == false &&
      (myError == CAM_ERROR_PARAM || myError == CAM_ERROR_UNKNOWN ||
       myError == CAM_ERROR_MODE))
  {
    myWasError = true;
    throwError();
  }
  // turn off error flag if we're out of the error condition now
  else if (myWasError == true &&
      (myError == CAM_ERROR_NONE || myError == CAM_ERROR_BUSY))
    myWasError = false;

}


/*
   This performs the actual state switch.  In most cases it just switches
   from one state to another, and sets the state timer to keep track of
   how long it has been in a state.

   When you're requesting a pan/tilt movement, and the camera is busy,
   it will switch states to send a stop command, then send another pan/tilt
   command.  In doing so, you want the state timeout to be from the time the
   first pan/tilt request was sent, to the time that it succeeds, including
   any time spent telling the camera to stop it's movement.  In that case
   you want to send a false for the 'useTimer' parameter.

   The delayTime is useful for certain commands, like initialization, that
   even when the command is complete, the camera will still respond with
   busys for a certain period of time.  Use the delayTime to delay the state
   switch to the next state.
 */
void ArVCC4::switchState(State state, int delayTime)
{
  // if we're switching to a different state, then store the previous one
  if (state != myState)
    myPreviousState = myState;

  if (delayTime != 0)
  {
    myNextState = state;
    myStateDelayTime = delayTime;
    myState = STATE_DELAYED_SWITCH;
  }
  else
  {
    myStateTime.setToNow();
    myState = state;
  }
}

/*
   This checks for either a packet timeout or a state timeout.  A packet
  timeout is when the camera hasn't sent back a packet within the
  allotted amount of time.  The state timeout is when it takes too long 
  to transition from state to state, despite how many packets have or 
  haven't been received.  Passing an argument will check for a statetimeout
  greater than the argument.  The packet timeout is always the same, but
  does not exist for unidirecitonal communication
*/
bool ArVCC4::timeout(int mSec)
{
  bool stateTimeout;
  bool packetTimeout = false;

  if (mSec == 0)
    stateTimeout = (myStateTime.mSecSince() > myStateTimeout);
  else
    stateTimeout = (myStateTime.mSecSince() > mSec);

  if (myCommType != COMM_UNIDIRECTIONAL && myWaitingOnPacket)
    packetTimeout = myPacketTime.mSecSince() > myPacketTimeout;

  return (stateTimeout || packetTimeout);
}

/*
  This will read bytes from the computer serial port.  It will read until
  it finds a RESPONSE byte for the header, and then read until it doesn't
  get anymore, up to a max of the longest possible response packet.

  If it reads and gets a good header and footer, then it puts the data
  in a packet, so that packetHandler will be called.
*/
ArBasePacket* ArVCC4::readPacket(void)
{
  unsigned char data[MAX_RESPONSE_BYTES];
  unsigned char byte;
  int num;
  
  myPacketBufLen = 0;

  myWaitingOnPacket = false;

  // Check if the connection is NULL and exit if it is
  if (myConn == NULL)
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::readPacket:  Error reading packet from serial port.  May not be open yet.");
    return NULL;
  }

  // Check for good header character -
  // Loop MAX_RESPONSE_BYTES times, then exit on the next loop
  // if we haven't gotten a RESPONSE header byte, yet
  for (num=0;num<=MAX_RESPONSE_BYTES + 1;num++)
  {
    // if we don't get any bytes, or if we've just exceeded the limit
    // then return null
    if (myConn->read((char *)&byte,1,1) <= 0 ||
	num == MAX_RESPONSE_BYTES + 1)
      return NULL;
    else if (byte == ArVCC4Commands::RESPONSE)
    {
      data[0] = byte;
      break;
    }
  }

  // we got the header character so keep reading bytes for MAX_RESPONSE_BYTES more
  for(num=1;num<=MAX_RESPONSE_BYTES;num++)
  {
    if (myConn->read((char *)&byte, 1, 1) <= 0)
    {
      // there are no more bytes, so check the last byte for the footer
      if (data[num-1] != ArVCC4Commands::FOOTER)
      {
        ArLog::log(ArLog::Terse, "ArVCC4::readPacket: Discarding bad packet.");
        return NULL;
      }
      else
	break;
    }
    else
      // add the byte to the array
      data[num] = byte;
    
  }

  // now put the data into a new packet so that it can be called by 
  // packet handlers
  myPacket.empty();
  myPacket.dataToBuf((char *)data, num);
  myPacket.resetRead();
  
  return &myPacket;
}

/*
  This will read packets from the VCC4 camera and determine whether or
  not we're going to need more bytes from the camera.  Trying to get
  pan/tilt or zoom position data from the camera generates a 6-byte 
  packet in event of an error, or a longer packet if no error.  This
  means we have to receive the first 6-bytes and see if there's a footer
  character.  If there is, then set myError to the error.  Otherwise
  request more bytes, put them all in a buffer, and call a function
  to process the buffer, depending on what type of response we were 
  waiting for.
*/
AREXPORT bool ArVCC4::packetHandler(ArBasePacket *packet)
{
  unsigned int errorCode;

  myWaitingOnPacket = false;

  // If we received a packet, then we can use bidirectional communications,
  // so enable if unless the user specified unidirectional
  if (myCommType == COMM_UNIDIRECTIONAL)
    return true;
  else if (myCommType == COMM_UNKNOWN)
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::packetHandler: Using bidirectional communication.");
    myCommType = COMM_BIDIRECTIONAL;
    myStateTimeout = BIDIRECTIONAL_TIMEOUT;
  }

  // reset the packet time since we got a response from the packet
  myPacketTime.setToNow();

  if (myBytesLeft == 0)
  {
    // We've received all the expected bytes from the camera, so it's
    // either a 6-byte packet from an aux port, the second half of another
    // response packet from an aux port, or a complete packet from a
    // computer serial port

    if (myPacketBufLen == 0 && myUsingAuxPort)
    {
      // we don't have any data, so it's supposed to be a 6-byte packet
      // from an aux port
      myPacketBufLen = packet->getDataLength(); 

      if (myPacketBufLen != 6)
      {
	// we don't know what this is, so scrap it and exit
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Incorrect number of bytes in response packet.");
	myPacketBufLen = 0;
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
	return true;
      }

      packet->bufToData((char *)myPacketBuf, myPacketBufLen);

      // check the header and footer
      if (myPacketBuf[0] != ArVCC4Commands::RESPONSE ||
	  myPacketBuf[5] != ArVCC4Commands::FOOTER)
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Bad header or footer character in response packet.");
	myPacketBufLen = 0;
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
	return true;
      }

      // so far so good.  Set myError to the error byte
      errorCode = myPacketBuf[3];

      if (errorCode == CAM_ERROR_NONE ||
	  errorCode == CAM_ERROR_BUSY ||
	  errorCode == CAM_ERROR_PARAM ||
	  errorCode == CAM_ERROR_MODE)
	myError = errorCode;
      else
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Unrecognized error code sent from camera.");
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
	return true;
      }

      // Set the flag that says we had a valid response
      myResponseReceived = true;

      myPacketBufLen = 0;
      camTask();
    }
    else
    {
      // we already have some data, or it came in on the computer serial
      // port as one big packet, so add it to the rest of the buffer

      // only add up to the max number of bytes
      if ((myPacketBufLen + packet->getDataLength()) > MAX_RESPONSE_BYTES)
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Too many bytes in response packet.  Truncating to maximum of %d.", MAX_RESPONSE_BYTES);
	requestBytes(0);
	packet->bufToData((char *)&myPacketBuf[myPacketBufLen],MAX_RESPONSE_BYTES - myPacketBufLen);
	myPacketBufLen = MAX_RESPONSE_BYTES;
      }
      else
      {
	packet->bufToData((char *)&myPacketBuf[myPacketBufLen],packet->getDataLength());
	myPacketBufLen += packet->getDataLength();
      }


      // now check the header and footer
      if (myPacketBuf[0] != ArVCC4Commands::RESPONSE ||
	  myPacketBuf[myPacketBufLen-1] != ArVCC4Commands::FOOTER)
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Bad header or footer character in long response packet.");
	myPacketBufLen = 0;
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
	return true;
      }

      // set the error to the error byte
      errorCode = myPacketBuf[3];

      if (errorCode == CAM_ERROR_NONE ||
	  errorCode == CAM_ERROR_BUSY ||
	  errorCode == CAM_ERROR_PARAM ||
	  errorCode == CAM_ERROR_MODE)
	myError = errorCode;
      else
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Unrecognized error code sent from camera.");
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
	return true;
      }	

      // We now have all the data for a long response packet, so process
      // if based on myState, and what type of response we're waiting for
      switch (myState)
      {
	case AWAITING_POS_REQUEST:
	  processGetPanTiltResponse();
	  break;
	case AWAITING_ZOOM_REQUEST:
	  processGetZoomResponse();
	  break;
	//case AWAITING_PRODUCTNAME_REQUEST:
	//  processGetProductNameResponse();
	//  break;
	default:
	  myPacketBufLen = 0;
	  break;
      }

      // Set the flag for a valid response
      myResponseReceived = true;
      camTask();
    }
  }
  else
  {
    // We're waiting for more bytes.  Check the header and error.
    // If they're okay, then ask for more bytes

    myPacketBufLen = packet->getDataLength();

    if (myPacketBufLen != 6)
    {
      // there should have been 6 bytes.  Scrap it and exit.
      ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Incorrect number of bytes in first part of long response packet.");
      myPacketBufLen = 0;
      myBytesLeft = 0;
      myError = CAM_ERROR_UNKNOWN;
      requestBytes(0);
      return true;
    }

    myPacketBufLen = packet->getDataLength();
    packet->bufToData((char *)myPacketBuf, myPacketBufLen);

    // check the header character
    if (myPacketBuf[0] != ArVCC4Commands::RESPONSE)
    {
      ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Bad header character in long response packet.");
      myPacketBufLen = 0;
      myBytesLeft = 0;
      myError = CAM_ERROR_UNKNOWN;
      requestBytes(0);
      return true;
    }

    if (myPacketBuf[5] != ArVCC4Commands::FOOTER)
    {
      // this means there is no error, and to expect more data
      requestBytes(myBytesLeft);
      myError = CAM_ERROR_NONE;
      return true;
    }
    else
    {
      // there was an error, so set the flags and exit
      errorCode = myPacketBuf[3];
      if (errorCode == CAM_ERROR_NONE ||
	  errorCode == CAM_ERROR_BUSY ||
	  errorCode == CAM_ERROR_PARAM ||
	  errorCode == CAM_ERROR_MODE)
	myError = errorCode;
      else
      {
	ArLog::log(ArLog::Terse, "ArVCC4::packetHandler: Unrecognized error code sent from camera when expecting long response packet.");
	myError = CAM_ERROR_UNKNOWN;
	requestBytes(0);
      }
      myPacketBufLen = 0;
      myBytesLeft = 0;

      return true;
    }
  }

  return true;
}

/* This needs to eventually use digital zooming, too */
AREXPORT int ArVCC4::getMaxZoom(void) const
{
  if (myCameraType == CAMERA_C50I)
    return MAX_ZOOM_OPTIC;
  else
    return MAX_ZOOM_OPTIC;
}

AREXPORT bool ArVCC4::panTilt_i(double pdeg, double tdeg)
{
  if (pdeg > getMaxPosPan_i())
    myPanDesired = getMaxPosPan_i();
  else if (pdeg < getMaxNegPan_i())
    myPanDesired = getMaxNegPan_i();
  else
    myPanDesired = pdeg;

  if (tdeg > getMaxPosTilt_i())
    myTiltDesired = getMaxPosTilt_i();
  else if (tdeg < getMaxNegTilt_i())
    myTiltDesired = getMaxNegTilt_i();
  else
    myTiltDesired = tdeg;

  return true;
}

AREXPORT bool ArVCC4::zoom(int deg)
{
  if (deg > getMaxZoom())
    myZoomDesired = getMaxZoom();
  else if (deg < getMinZoom())
    myZoomDesired = getMinZoom();
  else
    myZoomDesired = deg;

  return true;
}

/* Is supposed to accept 1x, 2x, 4x, 8x, and 12x, but doesn't seem to work
 for 12x*/
AREXPORT bool ArVCC4::digitalZoom(int deg)
{
  if (deg < 0)
    myDigitalZoomDesired = 0;
  else if (deg > 3)
    myDigitalZoomDesired = 3;
  /* uncomment this if the 12x mode ever works */
  //else if (deg > 4)
  //  myDigitalZoomDesired = 4;
  else
    myDigitalZoomDesired = deg;

  return true;
}

/*
   This will process a response from the camera for where it thinks it
  is panned and tilted to.
*/
void ArVCC4::processGetPanTiltResponse(void)
{
  unsigned char buf[4];
  char byte;
  unsigned int valU;
  double val;
  int i;

  // remove the ascii encoding, and put into 4-byte array
  for (i=0;i<4;i++)
  {
    byte = myPacketBuf[i+5];
    if (byte < 0x40)
      byte = byte - 0x30;
    else
      byte = byte - 'A' + 10;
    buf[i] = byte;
  }

  // convert the 4-bytes into a number
  valU = buf[0]*0x1000 + buf[1]*0x100 + buf[2]*0x10 + buf[3];

  // convert the number to a value that's meaningful, based on camera specs
  val = (((int)valU - (int)0x8000)*0.1125);

  // now set myPan to the response received for where the camera thinks it is
  myPanResponse = val;

  // repeat the steps for the tilt value
  for (i=0;i<4;i++)
  {
    byte = myPacketBuf[i+9];
    if (byte < 0x40)
      byte = byte - 0x30;
    else
      byte = byte - 'A' + 10;
    buf[i] = byte;
  }
  valU = buf[0]*0x1000 + buf[1]*0x100 + buf[2]*0x10 + buf[3];
  val =(((int)valU  - (int)0x8000)*0.1125);
  myTiltResponse = val;

  myPacketBufLen = 0;
}

/*
  This will process the response from the camera when requesting to find
  out where it thinks it is zoomed to.
*/
void ArVCC4::processGetZoomResponse(void)
{
  unsigned char buf[4];
  char byte;
  unsigned int valU;
  int i;

  // Make sure we have the correct number of bytes
  if (myPacketBufLen != 10)
  {
    myPacketBufLen = 0;
    return;
  }

  // remove the ascii encoding, and put into 2 bytes
  for (i=0;i<4;i++)
  {
    byte = myPacketBuf[i+5];
    if (byte < 0x40)
      byte = byte - 0x30;
    else
      byte = byte - 'A' + 10;
    buf[i] = byte;
  }

  // convert the 2 bytes into a number
  valU = 0;
  for (i=0;i<4;i++)
    valU += buf[i]*(unsigned int)pow(16.0,(double)(3-i));

  myZoomResponse = (int)valU;
  
  myPacketBufLen = 0;
}

/* This is not implemented yet, but should eventually determine the product name
 */
/*
void ArVCC4::processGetProductNameResponse(void)
{
  char byte;
  int i;

  if (myPacketBufLen != 10)
  {
    myPacketBufLen = 0;
    return;
  }
}*/

bool ArVCC4::sendPower(void)
{
  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::POWER);
  if (myPowerStateDesired)
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::sendPower: sending power on packet\n");
    myPacket.uByteToBuf(ArVCC4Commands::DEVICEID + 1);
  }
  else
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::sendPower: sending power off packet\n");
    myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);
  }


  // The camera will return 6 bytes.  If busy, then resend
  requestBytes();
  return sendPacket(&myPacket);
}

bool ArVCC4::setControlMode(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::setControlMode: sending control mode packet\n");
  myPacket.empty();             //Send Control command
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::CONTROL);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);

  // The camera will return 6 bytes, which fail if menu is operational
  // If failure, then power-off and power-on
  requestBytes();
  return sendPacket(&myPacket);
}

AREXPORT void ArVCC4::addErrorCB(ArFunctor *functor, ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    myErrorCBList.push_front(functor);
  else if (position == ArListPos::LAST)
    myErrorCBList.push_back(functor);
  else
    ArLog::log(ArLog::Terse, "ArVCC4::addErrorCB: Invalid position.");
}

AREXPORT void ArVCC4::remErrorCB(ArFunctor *functor)
{
  myErrorCBList.remove(functor);
}

void ArVCC4::throwError(void)
{
  std::list<ArFunctor *>::iterator it;

  for (it = myErrorCBList.begin();
      it != myErrorCBList.end();
      it++)
    (*it)->invoke();

}

bool ArVCC4::sendInit(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendInit: sending init packet\n");


  myPacket.empty();		// Send Init command
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::INIT);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);

  // The camera will return 6 bytes.  If busy, resend
  // If error, then not initted or not in control mode

  requestBytes();
  return sendPacket(&myPacket);
}


/*
It's necessary to set the default ranges, because the camera defaults to a max tilt range of +30, instead of +90.
*/
bool ArVCC4::setDefaultRange(void)
{

  ArLog::log(ArLog::Verbose,"ArVCC4::setDefaultRange: setting default range for camera movements");

  myPacket.empty();
  preparePacket(&myPacket);

  myPacket.uByteToBuf(ArVCC4Commands::SETRANGE);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID + 1);

  // Note the conversion from degrees to camera units:
  //    units = degrees / 0.1125

  // Set min tilt range
  myPacket.byte2ToBuf(ArMath::roundInt(MIN_TILT/.1125) + 0x8000);

  // Set max tilt range
  myPacket.byte2ToBuf(ArMath::roundInt(MAX_TILT/.1125) + 0x8000);

  requestBytes();
  return sendPacket(&myPacket);
}

bool ArVCC4::sendPanTilt()
{
  if (myCommType != COMM_BIDIRECTIONAL && myWaitingOnStop == false)
  {
    sendHaltPanTilt();
    myWaitingOnStop = true;
    switchState(AWAITING_STOP_PAN_TILT_RESPONSE);
    return true;
  }

  ArLog::log(ArLog::Verbose,"ArVCC4::sendPanTilt: sending panTilt packet");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::PANTILT);
  myPacket.byte2ToBuf(ArMath::roundInt( (myPanDesired)/.1125 ) + 0x8000);
  myPacket.byte2ToBuf(ArMath::roundInt( (myTiltDesired)/.1125 ) + 0x8000);

  // set these so that we know what was sent if the command is successful
  myPanSent = myPanDesired;
  myTiltSent = myTiltDesired;

  switchState(AWAITING_PAN_TILT_RESPONSE);

  requestBytes();
  return sendPacket(&myPacket);
}

bool ArVCC4::sendZoom()
{
  int i;
  if (myCommType != COMM_BIDIRECTIONAL && myWaitingOnStop == false)
  {
    sendHaltZoom();
    myWaitingOnStop = true;
    switchState(AWAITING_STOP_ZOOM_RESPONSE);
    return true;
  }

  ArLog::log(ArLog::Verbose,"ArVCC4::sendZoom: sending zoom packet");

  char buf[5];

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::ZOOM);
  sprintf(buf, "%4X", myZoomDesired);

  for (i=0;i<3;i++)
    if (buf[i] == ' ')
      buf[i] = '0';

  for (i=0;i<4;i++)
    myPacket.byteToBuf(buf[i]);

  // remember what value was sent
  myZoomSent = myZoomDesired;

  switchState(AWAITING_ZOOM_RESPONSE);

  requestBytes();
  return sendPacket(&myPacket);
}

/* This sends-
  0x30, 0x31 to turn off digital zooming
  0x30, 0x32 for 2x
  0x30, 0x34 for 4x
  0x30, 0x38 for 8x
  0x30, 0x3C for 12x
 */
bool ArVCC4::sendDigitalZoom()
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendDigitalZoom: sending digital zoom packet");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::DIGITALZOOM);

  if (myDigitalZoomDesired < 4)
  {
    myPacket.uByteToBuf(0x30);
    myPacket.uByteToBuf(0x30 + (0x1 << myDigitalZoomDesired));
  }
  else
  {
    /* this currently never gets called because myDigitalZoomDesired is
     * always < 4.  The manual says 0x3C should work, but it doesn't */
    myPacket.uByteToBuf(0x30);
    myPacket.uByteToBuf(0x3C);
  }

  requestBytes();
  return sendPacket(&myPacket);
}





bool ArVCC4::sendHaltPanTilt(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendHaltPanTilt: sending halt pantilt packet");
  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::STOP);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);

  requestBytes();
  return sendPacket(&myPacket);
}


bool ArVCC4::sendHaltZoom(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendHaltZoom: sending halt zoom packet");
  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::ZOOMSTOP);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);

  requestBytes();
  return sendPacket(&myPacket);
}


bool ArVCC4::sendPanSlew(void)
{
  char buf[4];

  if (myPanSlewDesired > getMaxPanSlew())
    myPanSlewDesired = getMaxPanSlew();
  if (myPanSlewDesired < getMinPanSlew())
    myPanSlewDesired = getMinPanSlew();

  if (myPanSlewDesired != myPanSlew)
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::sendPanSlew: sending panSlew packet");

    myPacket.empty();
    preparePacket(&myPacket);
    myPacket.uByteToBuf(ArVCC4Commands::PANSLEW);

    sprintf(buf,"%3X", ArMath::roundInt(myPanSlewDesired/.1125));

    if (buf[0] < '0')
      buf[0] = '0';
    if (buf[1] < '0')
      buf[1] = '0';
    if (buf[2] < '0')
      buf[2] = '0';

    myPacket.byteToBuf(buf[0]);
    myPacket.byteToBuf(buf[1]);
    myPacket.byteToBuf(buf[2]);

    requestBytes();
    myPanSlewSent = myPanSlewDesired;

    return sendPacket(&myPacket);
  }
  return true;
}


bool ArVCC4::sendTiltSlew(void)
{
  char buf[4];

  if (myTiltSlewDesired > getMaxTiltSlew())
    myTiltSlewDesired = getMaxTiltSlew();
  if (myTiltSlewDesired < getMinTiltSlew())
    myTiltSlewDesired = getMinTiltSlew();

  if (myTiltSlewDesired != myTiltSlew)
  {
    ArLog::log(ArLog::Verbose,"ArVCC4::sendTiltSlew: sending tiltSlew packet");

    myPacket.empty();
    preparePacket(&myPacket);
    myPacket.uByteToBuf(ArVCC4Commands::TILTSLEW);

    sprintf(buf,"%3X", ArMath::roundInt(myTiltSlewDesired/.1125));

    if (buf[0] == ' ')
      buf[0] = '0';
    if (buf[1] == ' ')
      buf[1] = '0';

    myPacket.byteToBuf(buf[0]);
    myPacket.byteToBuf(buf[1]);
    myPacket.byteToBuf(buf[2]);

    requestBytes();
    myTiltSlewSent = myTiltSlewDesired;

    return sendPacket(&myPacket);
  }
  return true;
}

bool ArVCC4::sendRealPanTiltRequest(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendRealPanTiltRequest: sending request for real pan/tilt information");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::PANTILTREQ);

  // The camera will return 6 bytes in case of error
  // 9 bytes otherwise.
  requestBytes(14);
  return sendPacket(&myPacket);
}


bool ArVCC4::sendRealZoomRequest(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendRealZoomRequest: sending request for real zoom position.");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::ZOOMREQ);
  myPacket.uByteToBuf(ArVCC4Commands::DEVICEID);

  // The camera will return 6 bytes in case of error
  // bytes othewise
  requestBytes(10);
  return sendPacket(&myPacket);
}

/**
This controls the status of the LED.
0 - disable forced control
1 - green forced on
2 - all forced off
3 - red forced on
4 - orange forced on
**/

bool ArVCC4::sendLEDControlMode(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendLEDControlMode: sending LED control packet.");

  if (myDesiredLEDControlMode < 0 || myDesiredLEDControlMode > 4)
  {
    ArLog::log(ArLog::Terse,"ArVCC4::sendLEDControlMode: incorrect parameter.  Not sending packet.");
    myDesiredLEDControlMode = -1;
    return false;
  }

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::LEDCONTROL);
  myPacket.uByteToBuf(0x30 + (unsigned char) myDesiredLEDControlMode);

  requestBytes(6);

  return sendPacket(&myPacket);
}

bool ArVCC4::sendIRFilterControl(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendIRFilterControl: sending IR cut filter control packet.");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::IRCUTFILTER);

  if (myDesiredIRFilterMode)
    myPacket.uByteToBuf(0x30);
  else
    myPacket.uByteToBuf(0x31);

  requestBytes(6);

  return sendPacket(&myPacket);
}
                                                              
/* The camera automatically shuts off the IR after some specified period of time.
 This command tells the camera to leave them on for the max of 6 hours.
 */
bool ArVCC4::sendIRLEDControl(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendIRLEDControl: sending IR-LED control packet.");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::INFRARED);
  if (myDesiredIRLEDsMode)
    myPacket.uByteToBuf(0x36);
  else
    myPacket.uByteToBuf(0x30);

  requestBytes(6);

  return sendPacket(&myPacket);
}

bool ArVCC4::sendProductNameRequest(void)
{
  ArLog::log(ArLog::Verbose,"ArVCC4::sendProductNameRequest: sending request for product name.");

  myPacket.empty();
  preparePacket(&myPacket);
  myPacket.uByteToBuf(ArVCC4Commands::PRODUCTNAME);

  return sendPacket(&myPacket);
}

bool ArVCC4::sendFocus(void)
{
  ArLog::log(ArLog::Verbose,
    "ArVCC4::sendFocus: sending focus control packet with mode %d (value 0x%X in packet).", 
    myFocusModeDesired, 0x30+(unsigned char)myFocusModeDesired );

  myPacket.empty();
  preparePacket(&myPacket);
  
  myPacket.uByteToBuf(ArVCC4Commands::AUTOFOCUS);
  myPacket.uByteToBuf(0x30 + (unsigned char) myFocusModeDesired);

  requestBytes(6);

  return sendPacket(&myPacket);
}

ArPTZConnector::GlobalPTZCreateFunc ArVCC4::ourCreateFunc(&ArVCC4::create);

ArPTZ* ArVCC4::create(size_t index, ArPTZParams params, ArArgumentParser *parser, ArRobot *robot)
{
  CameraType vccType = CAMERA_VCC4;
  if(params.type == "vcc50i")
    vccType = CAMERA_C50I;
  return new ArVCC4(robot, params.inverted, COMM_UNKNOWN, true, false, vccType);
  // ArPTZConnector takes care of choosing serial port or aux robot port
}

void ArVCC4::registerPTZType()
{
  ArPTZConnector::registerPTZType("vcc4", &ourCreateFunc);
  ArPTZConnector::registerPTZType("vcc50i", &ourCreateFunc);
  ArPTZConnector::registerPTZType("vcc", &ourCreateFunc);
}
