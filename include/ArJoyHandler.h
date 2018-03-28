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
#ifndef ARJOYHANDLER_H
#define ARJOYHANDLER_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"

#ifdef WIN32
#include <mmsystem.h> // for JOYINFO
#else // if not win32
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif
#ifdef linux
#include <linux/joystick.h>  // for JS_DATA_TYPE
#endif



/// Interfaces to a computer joystick
/** 
  This class is used to read data from a joystick device attached to the computer
  (usually via USB).
  The joystick handler keeps track of the minimum and maximums for both
  axes, updating them to constantly be better calibrated.  The speeds set 
  with setSpeed() influence what is returned by getAdjusted() or getDoubles().
    
  The joystick device is not opened until init() is called.  If there was
	an error connecting to the joystick device, it will return false, and
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

  @ingroup OptionalClasses
*/
class ArJoyHandler
{
 public:
  /// Constructor
  AREXPORT ArJoyHandler(bool useOSCal = true, bool useOldJoystick = false);
  /// Destructor
  AREXPORT ~ArJoyHandler();
  /// Intializes the joystick, returns true if successful
  AREXPORT bool init(void);
  /// Returns if the joystick was successfully initialized or not
  bool haveJoystick(void) { return myInitialized; }
  /// Gets the adjusted reading, as floats, between -1.0 and 1.0
  AREXPORT void getDoubles(double *x, double *y, double *z = NULL);
  /// Gets the button 
  AREXPORT bool getButton(unsigned int button);
  /// Returns true if we definitely have a Z axis (we don't know in windows unless it moves)
  bool haveZAxis(void) { return myHaveZ; }

  /// Sets the maximums for the x, y and optionally, z axes.
  void setSpeeds(int x, int y, int z = 0) 
    { myTopX = x; myTopY = y; myTopZ = z; }
  /// Gets the adjusted reading, as integers, based on the setSpeed
  AREXPORT void getAdjusted(int *x, int *y, int *z = NULL);

  /// Gets the number of axes the joystick has
  AREXPORT unsigned int getNumAxes(void);
  /// Gets the floating (-1 to 1) location of the given joystick axis
  AREXPORT double getAxis(unsigned int axis);
  /// Gets the number of buttons the joystick has
  AREXPORT unsigned int getNumButtons(void);

  /// Sets whether to just use OS calibration or not
  AREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether to just use OS calibration or not
  AREXPORT bool getUseOSCal(void);
  /// Starts the calibration process
  AREXPORT void startCal(void);
  /// Ends the calibration process
  AREXPORT void endCal(void);
  /// Gets the unfilitered reading, mostly for internal use, maybe
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
  void getData(void);
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
  void getOldData(void);
  void getNewData(void);
  #ifdef linux 
  struct JS_DATA_TYPE myJoyData; // structure for the buttons and x,y coords
  #else
  int myJoyData;
  #endif
  FILE * myOldJoyDesc;
  int myJoyDesc;
#endif // linux
};


#endif // ARJOYHANDLER_H

