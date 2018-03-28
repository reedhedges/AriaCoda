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
/** @example gpsCoordsTest.cpp : works the LLA2ENU and ENU2LLA coord transformations */

#include "Aria.h"
#include "ArGPSCoords.h"

int main(int argc, char **argv)
{
  // Initialize Aria and Arnl global information
  Aria::init();
  ArLLACoords originLatLon(42.805464,  -71.574738, 64.0);
  ArMapGPSCoords mapGPSCoords(originLatLon);
  ArLog::log(ArLog::Normal, "Origin LLA is %.6f %.6f %g",
	     originLatLon.getLatitude(),
	     originLatLon.getLongitude(),
	     originLatLon.getAltitude());
  // Line of points along longitude.
  for(int i = 0; i < 10; i++)
  {
    double di = (double) i * 1e-6;
    ArLLACoords lla(originLatLon.getLatitude(),
		    originLatLon.getLongitude() + di,
		    originLatLon.getAltitude());

    double ea, no, up;
    mapGPSCoords.convertLLA2MapCoords(lla.getLatitude(),
				      lla.getLongitude(),
				      lla.getAltitude(), 
				      ea, no, up);

    ArLog::log(ArLog::Normal, 
	       "LLA %.6f %.6f %g (deg deg m)-> ENU %.2f %.2f %.2f (mm)",
	       lla.getLatitude(),
	       lla.getLongitude(),
	       lla.getAltitude(), 
	       ea, no, up);
  }
  // Line of points along latitude.
  for(int i = 0; i < 10; i++)
  {
    double di = (double) i * 1e-6;
    ArLLACoords lla(originLatLon.getLatitude() + di,
		    originLatLon.getLongitude(),
		    originLatLon.getAltitude());

    double ea, no, up;
    mapGPSCoords.convertLLA2MapCoords(lla.getLatitude(),
				      lla.getLongitude(),
				      lla.getAltitude(), 
				      ea, no, up);

    ArLog::log(ArLog::Normal, 
	       "LLA %.6f %.6f %g (deg deg m)-> ENU %.2f %.2f %.2f (mm)",
	       lla.getLatitude(),
	       lla.getLongitude(),
	       lla.getAltitude(), 
	       ea, no, up);
  }
  // Line of points along east.
  for(int i = 0; i < 10; i++)
  {
    double di = (double) i * 1000.0;
    double lat, lon, alt;
    double ea=di, no=0.0, up=0.0;
    mapGPSCoords.convertMap2LLACoords(ea, no, up,
				      lat, lon, alt);

    ArLog::log(ArLog::Normal, 
	       "ENU %g %g %g (mm) -> LLA %.6f %.6f %g (deg deg m)",
	       ea, no, up, lat, lon, alt);
  }
  // Line of points along north.
  for(int i = 0; i < 10; i++)
  {
    double di = (double) i * 1000.0;
    double lat, lon, alt;
    double ea=0.0, no=di, up=0.0;
    mapGPSCoords.convertMap2LLACoords(ea, no, up,
				      lat, lon, alt);

    ArLog::log(ArLog::Normal, 
	       "ENU %g %g %g (mm) -> LLA %.6f %.6f %g (deg deg m)",
	       ea, no, up, lat, lon, alt);
  }
    
  Aria::exit(0);
}


