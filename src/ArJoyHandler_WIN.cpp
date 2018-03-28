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
#include "ArJoyHandler.h"

AREXPORT bool ArJoyHandler::init(void)
{

  myPhysMax = 1;
  myLastZ = 0;

  // first see if we can talk to the first joystick
  if (joyGetDevCaps(JOYSTICKID1,&myJoyCaps,sizeof(myJoyCaps)) == 
      JOYERR_NOERROR &&
      joyGetPos(JOYSTICKID1,&myJoyInfo) != JOYERR_UNPLUGGED) 
  {
    myJoyID = JOYSTICKID1;

    // So far, it seems that the x range is the same as that of y and
    // z, so only one is used
    myPhysMax  = myJoyCaps.wXmax - myJoyCaps.wXmin;
	/*
	ArLog::log(ArLog::Normal, "caps 0x%x numAxes %d maxAxes %d %s",
			myJoyCaps.wCaps, myJoyCaps.wNumAxes, myJoyCaps.wMaxAxes, myJoyCaps.szPname);
	ArLog::log(ArLog::Normal, 
		"xMin %d xMax %d yMin %d yMax %d zMin %d zMax %d rMin %d rMax %d uMin %d uMax %d",
			myJoyCaps.wXmin, myJoyCaps.wXmax, myJoyCaps.wYmin, myJoyCaps.wYmax, 
			myJoyCaps.wRmin, myJoyCaps.wRmax, myJoyCaps.wZmin, 
			myJoyCaps.wZmax, myJoyCaps.wUmin, myJoyCaps.wUmax); 
	*/
    myInitialized = true;
    startCal();
    endCal();
    return true;
  } 
  // we couldn't talk to the first one so try the second one
  else if (joyGetDevCaps(JOYSTICKID2,&myJoyCaps,sizeof(myJoyCaps)) == 
      JOYERR_NOERROR &&
      joyGetPos(JOYSTICKID2,&myJoyInfo) != JOYERR_UNPLUGGED) 
  {
    myJoyID = JOYSTICKID2;

    // So far, it seems that the x range is the same as that of y and
    // z, so only one is used
    myPhysMax = myJoyCaps.wXmax - myJoyCaps.wXmin;
	/*
	ArLog::log(ArLog::Normal, "2 caps 0x%x numAxes %d maxAxes %d %s",
			myJoyCaps.wCaps, myJoyCaps.wNumAxes, myJoyCaps.wMaxAxes, myJoyCaps.szPname);
	ArLog::log(ArLog::Normal, 
		"2 xMin %d xMax %d yMin %d yMax %d zMin %d zMax %d rMin %d rMax %d uMin %d uMax %d",
			myJoyCaps.wXmin, myJoyCaps.wXmax, myJoyCaps.wYmin, myJoyCaps.wYmax, 
			myJoyCaps.wRmin, myJoyCaps.wRmax, myJoyCaps.wZmin, 
			myJoyCaps.wZmax, myJoyCaps.wUmin, myJoyCaps.wUmax); 
    */
    myInitialized = true;
    startCal();
    endCal();
    return true;
  } 
  // we couldn't talk to either one
  else
  {
    myInitialized = false;
    return false;
  }

  // Just to prevent any possible divide-by-zeros...
  if (myPhysMax == 0) {
    myPhysMax = 1;
  }

  getData();
}


void ArJoyHandler::getData(void)
{
  int x, y, z, r, u, v;
  if (!myFirstData && myLastDataGathered.mSecSince() < 5)
    return;

  myLastDataGathered.setToNow();
  JOYINFOEX joyInfoEx;
  joyInfoEx.dwFlags = JOY_RETURNALL;
  joyInfoEx.dwSize = sizeof(joyInfoEx);

  MMRESULT joyResult = joyGetPosEx(myJoyID,&joyInfoEx);

  std::string axes;

  /*
  if (myFirstData)
  {
	if (joyInfoEx.dwFlags & JOY_RETURNR)
		axes += " r ";
	if (joyInfoEx.dwFlags & JOY_RETURNU)
		axes += " u ";
	if (joyInfoEx.dwFlags & JOY_RETURNV)
		axes += " v ";
	if (joyInfoEx.dwFlags & JOY_RETURNX)
		axes += " x ";
	if (joyInfoEx.dwFlags & JOY_RETURNY)
		axes += " y ";
	if (joyInfoEx.dwFlags & JOY_RETURNZ)
		axes += " z ";
	ArLog::log(ArLog::Normal, "Axes %s\n", axes.c_str());
  }
  */

  myFirstData = false;

  if (joyResult == JOYERR_NOERROR) 
  {
    // KMC: I don't understand this logic... The spec says that 
    // getAxis returns a number between -1 and 1; the getAxis method
    // multiplies the contents of myAxes by 128.  The logic below
    // however seems to double everything... 
/**/
    x = (int)(joyInfoEx.dwXpos*256.0/myPhysMax)-128;
    y = (int)-((joyInfoEx.dwYpos*256.0/myPhysMax)-128);
    z = (int)-((joyInfoEx.dwZpos*256.0/myPhysMax)-128);
    r = (int)-((joyInfoEx.dwRpos*256.0/myPhysMax)-128);
    u = (int)-((joyInfoEx.dwUpos*256.0/myPhysMax)-128);
    v = (int)-((joyInfoEx.dwVpos*256.0/myPhysMax)-128);
	
/***/
/***
    x = (int) 128 * ((2.0 * (double) myJoyInfo.wXpos / (double) myPhysMax) - 1);
    y = (int)-128 * ((2.0 * (double) myJoyInfo.wYpos / (double) myPhysMax) - 1);
    z = (int)-128 * ((2.0 * (double) myJoyInfo.wZpos / (double) myPhysMax) - 1);
**/

    if (myLastZ != z)
      myHaveZ = true;
    if (x > myMaxX)
      myMaxX = x;
    if (x < myMinX)
      myMinX = x;
    if (y > myMaxY)
      myMaxY = y;
    if (y < myMinY)
      myMinY = y;

    myAxes[1] = x;
    myAxes[2] = y;
    myAxes[3] = z;
    myAxes[4] = r;
    myAxes[5] = u;
    myAxes[6] = v;
	
    myLastZ = z;
	
	/* This seems off by one on the dual controller 
    myButtons[1] = (joyInfoEx.dwButtons & 0x1);
    myButtons[2] = (joyInfoEx.dwButtons & 0x2);
    myButtons[3] = (joyInfoEx.dwButtons & 0x4);
    myButtons[4] = (joyInfoEx.dwButtons & 0x8);
    myButtons[5] = (joyInfoEx.dwButtons & 0xf);
    myButtons[6] = (joyInfoEx.dwButtons & 0x10);
    myButtons[7] = (joyInfoEx.dwButtons & 0x20);
    myButtons[8] = (joyInfoEx.dwButtons & 0x40);
    myButtons[9] = (joyInfoEx.dwButtons & 0x80);
    myButtons[10] = (joyInfoEx.dwButtons & 0xf0);
    myButtons[11] = (joyInfoEx.dwButtons & 0x100);
	myButtons[12] = (joyInfoEx.dwButtons & 0x200);
*/
	myButtons[1] = (joyInfoEx.dwButtons & JOY_BUTTON1);
	myButtons[2] = (joyInfoEx.dwButtons & JOY_BUTTON2);
    myButtons[3] = (joyInfoEx.dwButtons & JOY_BUTTON3);
    myButtons[4] = (joyInfoEx.dwButtons & JOY_BUTTON4);
    myButtons[5] = (joyInfoEx.dwButtons & JOY_BUTTON5);
    myButtons[6] = (joyInfoEx.dwButtons & JOY_BUTTON6);
    myButtons[7] = (joyInfoEx.dwButtons & JOY_BUTTON7);
    myButtons[8] = (joyInfoEx.dwButtons & JOY_BUTTON8);
    myButtons[9] = (joyInfoEx.dwButtons & JOY_BUTTON9);
    myButtons[10] = (joyInfoEx.dwButtons & JOY_BUTTON10);
    myButtons[11] = (joyInfoEx.dwButtons & JOY_BUTTON11);
	myButtons[12] = (joyInfoEx.dwButtons & JOY_BUTTON12);

  }
  else //(joyResult == JOYERR_UNPLUGGED) 
  {
    myAxes[1] = 0;
    myAxes[2] = 0;
    myAxes[3] = 0;
    myAxes[4] = 0;
    myAxes[5] = 0;
    myAxes[6] = 0;
    myButtons[1] = 0;
    myButtons[2] = 0;
    myButtons[3] = 0;
    myButtons[4] = 0;
    myButtons[5] = 0;
    myButtons[6] = 0;
    myButtons[7] = 0;
    myButtons[8] = 0;
    myButtons[9] = 0;
    myButtons[10] = 0;
    myButtons[11] = 0;
    myButtons[12] = 0;
	
	// Reset the initialized flag so that the joystick button in the GUI
	// will be disabled.
    myInitialized = false;
  } 
}

#if 0

void ArJoyHandler::getData(void)
{
  int x, y, z;
  if (!myFirstData && myLastDataGathered.mSecSince() < 5)
    return;

  myFirstData = false;
  myLastDataGathered.setToNow();
  MMRESULT joyResult = joyGetPos(myJoyID,&myJoyInfo);

  if (joyResult == JOYERR_NOERROR) 
  {
    // KMC: I don't understand this logic... The spec says that 
    // getAxis returns a number between -1 and 1; the getAxis method
    // multiplies the contents of myAxes by 128.  The logic below
    // however seems to double everything... 
/**/
    x = (int)(myJoyInfo.wXpos*256.0/myPhysMax)-128;
    y = (int)-((myJoyInfo.wYpos*256.0/myPhysMax)-128);
    z = (int)-((myJoyInfo.wZpos*256.0/myPhysMax)-128);

/***/
/***
    x = (int) 128 * ((2.0 * (double) myJoyInfo.wXpos / (double) myPhysMax) - 1);
    y = (int)-128 * ((2.0 * (double) myJoyInfo.wYpos / (double) myPhysMax) - 1);
    z = (int)-128 * ((2.0 * (double) myJoyInfo.wZpos / (double) myPhysMax) - 1);
**/

    if (myLastZ != z)
      myHaveZ = true;
    if (x > myMaxX)
      myMaxX = x;
    if (x < myMinX)
      myMinX = x;
    if (y > myMaxY)
      myMaxY = y;
    if (y < myMinY)
      myMinY = y;

    myAxes[1] = x;
    myAxes[2] = y;
    myAxes[3] = z;
	
    myLastZ = z;
	
    myButtons[1] = (myJoyInfo.wButtons & 1);
    myButtons[2] = (myJoyInfo.wButtons & 2);
    myButtons[3] = (myJoyInfo.wButtons & 4);
    myButtons[4] = (myJoyInfo.wButtons & 8);
  }
  else //(joyResult == JOYERR_UNPLUGGED) 
  {
    myAxes[1] = 0;
    myAxes[2] = 0;
    myAxes[3] = 0;
    myButtons[1] = 0;
    myButtons[2] = 0;
    myButtons[3] = 0;
    myButtons[4] = 0;
	
	// Reset the initialized flag so that the joystick button in the GUI
	// will be disabled.
    myInitialized = false;
  } 
}

#endif 
