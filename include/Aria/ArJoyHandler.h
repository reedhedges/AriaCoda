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
#ifndef ARJOYHANDLER_H
#define ARJOYHANDLER_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"

#ifdef WIN32
#include <mmsystem.h> // for JOYINFO
#else // if not win32
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif
#ifdef __linux__
#include <linux/joystick.h>  // for JS_DATA_TYPE
#endif



/// Interfaces to a joystick or game controller attached to the computer.
/** 
  Use an object of this class is to read data from a joystick or game controller device attached to the computer
  (usually via USB).  Any device supported by the operating system should work. 
  Generic PC game controllers, as well as Playstation, XBox, and other USB game controllers
  usually work.  
  On Linux, `/dev/input/js0` is used.  On Windows, the first detected device is used.
  The joystick handler keeps track of the minimum and maximums for both
  axes, updating them to constantly be better calibrated.  Scaling values set 
  with setSpeed() influence what is returned by getAdjusted() or getDoubles().

  The number of usable buttons is provided by getNumButtons().  To get whether a
  button is pressed at any moment, use getButton().   

  The position of the
  joystick or game controller thumb stick is available by calling getAdjusted() or
  getDoubles().  If the joystick or controller has a
  separate one-axis sliding "throttle" or Z control in addition to X and Y, this will be the Z
  component.  The
  presence of this axis is indicated by hasZAxis().  Note that On some devices, 
  the Z axis or other additional axis control may also trigger a specific button
  press as well.

  The number of joystick axes
  is provided by getNumAxes().  Get the value of a specific axis by calling
  getAxis(). If a device contains more than one thumbstick or 
  joystick, then the first stick will provide the axes 1 (X) and 2 (Y)  with
  values also available by calling getAdjusted() or getDoubles(). If an
  additional throttle or Z control is provided, it will be axis 3.
  The second thumb/joy stick inputs will provide axes 3 and 4. Any additional controls would be 5, 6, 7,
  etc.
    
  The joystick device is not opened until init() is called.  If there was
	an error connecting to the joystick device, init() will return false, and
	haveJoystick() will return false. After calling
	init(), use getAdjusted() or getDoubles()
	to get values, and getButton() to check whether a button is pressed. setSpeed() may be
	called at any time to configure or reconfigure the range of the values returned by
	getAdjusted() and getDoubles().

	To get the raw data instead, use getUnfiltered() or getAxis().

	For example, if you want the X axis output to range from -1000 to 1000, and the Y axis
	output to range from -100 to 100, call <code>setSpeed(1000, 100);</code>.

  The X joystick axis is usually the left-right axis, and Y is forward-back.
  If a joystick has a Z axis, it is usually a "throttle" slider or dial on the
  joystick. The usual way to 
	drive a robot with a joystick is to use the X joystick axis for rotational velocity,
	and Y for translational velocity (note the robot coordinate system, this is its local
	X axis), and use the Z axis to adjust the robot's maximum driving speed.

  See joyHandlerExample.cpp or joytest.cpp for an example and run it to test out
  your game controller or joystick.

  @ingroup OptionalClasses
*/
class ArJoyHandler
{
 public:
  /// Constructor
  AREXPORT ArJoyHandler(bool useOSCal = true, bool useOldJoystick = false);

  /// Initializes the joystick, returns true if successful
  AREXPORT bool init();
  /// Returns if the joystick was successfully initialized or not
  bool haveJoystick() { return myInitialized; }
  /// Gets the adjusted reading, as floats, between -1.0 and 1.0
  AREXPORT void getDoubles(double *x, double *y, double *z = NULL);
  /// Gets the button 
  AREXPORT bool getButton(unsigned int button);
  /// Returns true if we definitely have a Z axis (we don't know in windows unless it moves)
  bool haveZAxis() { return myHaveZ; }

  /// Sets the maximums for the x, y and optionally, z axes.
  void setSpeeds(int x, int y, int z = 0) 
    { myTopX = x; myTopY = y; myTopZ = z; }
  /// Gets the adjusted reading, as integers, based on the setSpeed
  AREXPORT void getAdjusted(int *x, int *y, int *z = NULL);

  /// Gets the number of axes the joystick has
  AREXPORT unsigned int getNumAxes();
  /// Gets the floating (-1 to 1) location of the given joystick axis
  AREXPORT double getAxis(unsigned int axis);
  /// Gets the number of buttons the joystick has
  AREXPORT unsigned int getNumButtons();

  /// Sets whether to just use OS calibration or not
  AREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether to just use OS calibration or not
  AREXPORT bool getUseOSCal();
  /// Starts the calibration process
  AREXPORT void startCal();
  /// Ends the calibration process
  AREXPORT void endCal();
  /// Gets the unfiltered reading, mostly for internal use, maybe
  /// useful for Calibration
  AREXPORT void getUnfiltered(int *x, int *y, int *z = NULL);
  /// Gets the stats for the joystick, useful after calibrating to save values
  AREXPORT void getStats(int *maxX, int *minX, int *maxY, int *minY, 
		 int *cenX, int *cenY);
  /// Sets the stats for the joystick, useful for restoring calibrated settings
  AREXPORT void setStats(int maxX, int minX, int maxY, int minY, 
		int cenX, int cenY);
  /// Gets the maximums for each axis.
  AREXPORT void getSpeeds(int *x, int *y, int *z);

 protected:
  // function to get the data for OS dependent part
  void getData();
  int myMaxX, myMinX, myMaxY, myMinY, myCenX, myCenY, myTopX, myTopY, myTopZ;
  bool myHaveZ;

  std::map<unsigned int, int> myAxes;
  std::map<unsigned int, bool> myButtons;

  int myPhysMax;
  bool myInitialized;
  bool myUseOSCal;
  bool myUseOld;
  bool myFirstData;
  ArTime myLastDataGathered;
#ifdef WIN32
  unsigned int myJoyID;
  int myLastZ;
  JOYINFO myJoyInfo;
  JOYCAPS myJoyCaps;
#else // if not win32
  int myJoyNumber;
  char myJoyNameTemp[512];
  ArTime myLastOpenTry;
  void getOldData();
  void getNewData();
  #ifdef __linux__ 
  struct JS_DATA_TYPE myJoyData; // structure for the buttons and x,y coords
  #else
  int myJoyData;
  #endif
  FILE * myOldJoyDesc;
  int myJoyDesc;
#endif // linux
};


#endif // ARJOYHANDLER_H

