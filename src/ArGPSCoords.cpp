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

#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArLog.h"
#include "Aria/ArGPSCoords.h"

#include <math.h>

const double ArWGS84::A = 6378137; // meters
const double ArWGS84::B = 6356752.3142; // meters
const double ArWGS84::E = 8.2094437949696e-2; 
const double ArWGS84::C = 299792458; // m/sec
const double ArWGS84::EP = 8.1819190842622e-2;
const double ArWGS84::Finv = 298.257223563;
const double ArWGS84::Omega = 7292115e-11; // rad/sec
const double ArWGS84::GM = 3986004.418e8; // m^3/sec^2
const double ArWGS84::G = 9.7976432222; // m/sec^2. Ave g.
const double ArWGS84::M = 5.9733328e24; // kg. Mass of earth.



AREXPORT void Ar3DPoint::print(const char *head)
{
    ArLog::log(ArLog::Normal, "%s%f, %f, %f", head?head:"", myX, myY, myZ);
}

/*!
 * Converts Earth Centered Earth Fixed coordinates into Latitude Longitude
 * Altitude coordinates.
 *
 */
AREXPORT ArLLACoords
ArECEFCoords::ECEF2LLA() const
{
  // ECEF2LLA - convert earth-centered earth-fixed (ECEF)
  // cartesian coordinates to latitude, longitude,
  // and altitude
  //
  // USAGE:
  // [lat,lon,alt] = ecef2lla(x,y,z)
  //
  // lat = geodetic latitude (radians)
  // lon = longitude (radians)
  // alt = height above WGS84 ellipsoid (m)
  // x = ECEF X-coordinate (m)
  // y = ECEF Y-coordinate (m)
  // z = ECEF Z-coordinate (m)
  const double x = myX;//(*this)(0);
  const double y = myY;//(*this)(1);
  const double z = myZ;//(*this)(2);
  //
  // Notes: (1) This function assumes the WGS84 model.
  // (2) Latitude is customary geodetic (not geocentric).
  // (3) Inputs may be scalars, vectors, or matrices of the same
  // size and shape. Outputs will have that same size and shape.
  // (4) Tested but no warranty; use at your own risk.
  // (5) Michael Kleder, April 2006

  // function [lat,lon,alt] = ecef2lla(x,y,z)

  // WGS84 ellipsoid constants:
  const double a = ArWGS84::A;
  const double e = ArWGS84::E;
  const double b = ArWGS84::B;
  const double ep = ArWGS84::EP;
  // Calculations.
  const double p = sqrt(x*x + y*y);
  const double th = atan2(a*z, b*p);
  double lon = atan2(y, x);
  double lat = atan2((z + ep*ep*b*pow(sin(th), 3)),
		     (p - e*e*a*pow(cos(th), 3)));
  const double N = a / sqrt(1 - e*e*pow(sin(lat), 2));
  double alt = p / cos(lat) - N;

  // return lon in range [0,2*pi)
  if(lon < -M_PI)
    lon += 2*M_PI;

  // convert to degrees
  lat *= 180.0/M_PI;
  lon *= 180.0/M_PI;

  // correct for numerical instability in altitude near exact poles:
  // (after this correction, error is about 2 millimeters, which isabout
  // the same as the numerical precision of the overall function)
  if(fabs(x) < 1 && fabs(y) < 1)
    alt = -b;

  return ArLLACoords(lat, lon, alt);
}
/*!
 * Converts Earth Centered Earth Fixed coordinates into the tangential 
 * East North Up coordinates with ref being the origin.
 *
 * @param ref: The ECEF coords of the origin.
 *
 */
AREXPORT ArENUCoords
ArECEFCoords::ECEF2ENU(const ArECEFCoords& ref) const
{
  //    function [e,n,u] = xyz2enuTest(Xr, Yr, Zr, X, Y, Z)

  const double Xr = ref.myX;//ref(0);
  const double Yr = ref.myY;//ref(1);
  const double Zr = ref.myZ;//ref(2);
  const double dx = myX - Xr;//(*this)(0) - Xr;
  const double dy = myY - Yr;//(*this)(1) - Yr;
  const double dz = myZ - Zr;//(*this)(2) - Zr;

  // convert ECEF coordinates to local east, north, up

  const double phiP = atan2(Zr, sqrt(Xr*Xr + Yr*Yr));

  const double lambda = atan2(Yr, Xr);

  const double e = -sin(lambda)*(dx) + cos(lambda)*(dy);

  const double n = (-sin(phiP)*cos(lambda)*(dx) - 
	      sin(phiP)*sin(lambda)*(dy) + cos(phiP)*(dz));

  const double u = (cos(phiP)*cos(lambda)*(dx) + 
	      cos(phiP)*sin(lambda)*(dy) + sin(phiP)*(dz)); 

  // Return in mm.
  return ArENUCoords(e*1000.0, n*1000.0, u*1000.0);
}
/*!
 * Converts Latitude Longitude Altitude coordinates into Earth Centered
 * Earth Fixed coordinates.
 *
 */
AREXPORT ArECEFCoords
ArLLACoords::LLA2ECEF() const
{
  // LLA2ECEF - convert latitude, longitude, and altitude to
  // earth-centered, earth-fixed (ECEF) cartesian
  //
  // USAGE:
  // [x,y,z] = lla2ecef(lat,lon,alt)
  //
  // x = ECEF X-coordinate (m)
  // y = ECEF Y-coordinate (m)
  // z = ECEF Z-coordinate (m)
  // lat = geodetic latitude (radians)
  // lon = longitude (radians)
  // alt = height above WGS84 ellipsoid (m)
  const double lat = myX*M_PI/180.0;//(*this)(0)*M_PI/180.0;
  const double lon = myY*M_PI/180.0;//(*this)(1)*M_PI/180.0;
  const double alt = myZ;//(*this)(2);
  //
  // Notes: This function assumes the WGS84 model.
  // Latitude is customary geodetic (not geocentric).
  //
  // Source: "Department of Defense World Geodetic System 1984"
  // Page 4-4
  // National Imagery and Mapping Agency
  // Last updated June, 2004
  // NIMA TR8350.2
  //
  // Michael Kleder, July 2005

  //function [x,y,z]=lla2ecef(lat,lon,alt)

  // WGS84 ellipsoid constants:
  const double a = ArWGS84::A;
  const double e = ArWGS84::E;

  // intermediate calculation
  // (prime vertical radius of curvature)
  const double N = a/ sqrt(1 - e*e * pow(sin(lat),2));

  // results:
  const double x = (N + alt) * cos(lat) * cos(lon);
  const double y = (N + alt) * cos(lat) * sin(lon);
  const double z = ((1 - e*e) * N + alt) * sin(lat);

  return ArECEFCoords(x, y, z);
}
/*!
 * Converts Tangential East North Up coordinates with ref as origin into
 * Earth Centered Earth Fixed Coordinates.
 *
 * @param ref: The coordinates of the origin.
 *
 */
AREXPORT ArECEFCoords
ArENUCoords::ENU2ECEF(const ArLLACoords& ref) const
{

  // function [X, Y, Z] = enu2xyz(refLat, refLong, refH, e, n, u)

  // Convert east, north, up coordinates (labelled e, n, u) to ECEF

  // coordinates. The reference point (phi, lambda, h) must be given. 
  // All distances are in metres

//  double refLat = ref(0)*M_PI/180.0;
//  double refLon = ref(1)*M_PI/180.0;
//  double refH = ref(2);
  const double refLon = ref.getY()*M_PI/180.0;
  
  const ArECEFCoords refECEF = ref.LLA2ECEF();
  const double Xr = refECEF.getX();//refECEF(0);
  const double Yr = refECEF.getY();//refECEF(1);
  const double Zr = refECEF.getZ();//refECEF(2);

  const double phiP = atan2(Zr, sqrt(Xr*Xr + Yr*Yr)); // Geocentric latitude

  const double e = myX/1000.0;//(*this)(0)/1000.0;
  const double n = myY/1000.0;//(*this)(1)/1000.0;
  const double u = myZ/1000.0;//(*this)(2)/1000.0;
  
  const double X = (-sin(refLon)*e - cos(refLon)*sin(phiP)*n + 
	      cos(refLon)*cos(phiP)*u + Xr);

  const double Y = (cos(refLon)*e - sin(refLon)*sin(phiP)*n + 
	      cos(phiP)*sin(refLon)*u + Yr);

  const double Z = cos(phiP)*n + sin(phiP)*u + Zr; 

  return ArECEFCoords(X, Y, Z);
}

/*!
 *  Actual function which does the conversion from LLA to Map Coords.
 *
 * @param lat: Latitude coords.
 * @param lon: Longitude coords.
 * @param alt: Altitude coords.
 * @param ea: East coords.
 * @param no: North coords.
 * @param up: Up coords.
 *
 * @return true if conversion is possible else false.
 *
 */
AREXPORT bool
ArMapGPSCoords::convertLLA2MapCoords(const double lat, const double lon, const double alt,
				     double& ea, double& no, double& up) const
{						    
  if(!myOriginSet)
    return false;

  const ArLLACoords lla(lat, lon, alt);
  const ArECEFCoords ecef = lla.LLA2ECEF();
  const ArENUCoords enu = ecef.ECEF2ENU(myOriginECEF);
//  ArLog::log(ArLog::Normal, "GPSLocaLog: convertLLA2MapCoords: ENU %g %g %g",
//	     enu.getX(), enu.getY(), enu.getZ());
  ea = enu.getX();//enu(0);
  no = enu.getY();//enu(1);
  up = enu.getZ();//enu(2);
//  ArLog::log(ArLog::Normal, 
//	     "GPSLocaLog: convertLLA2MapCoords: ENU + offset %g %g %g",
//	     ea, no, up);
  return true;
}
/*!
 *  Actual function which does the conversion from ENU coords to LLA coords.
 *
 * @param ea: East coords.
 * @param no: North coords.
 * @param up: Up coords.
 * @param lat: Is set to Latitude 
 * @param lon: Is set to Longitude 
 * @param alt: Is set to Altitude 
 *
 * @return true if conversion is possible else false.
 */
AREXPORT bool
ArMapGPSCoords::convertMap2LLACoords(const double ea, const double no, const double up,
				     double& lat, double& lon, double& alt) const
{						    
  if(!myOriginSet)
    return false;

  const ArENUCoords enu(ea, no, up);
  const ArECEFCoords ecef = enu.ENU2ECEF(myOriginLLA);
  const ArLLACoords lla = ecef.ECEF2LLA();

//  ArLog::log(ArLog::Normal, "GPSLocaLog: convertMap2LLACoords: ENU %g %g %g",
//	     enu.getX(), enu.getY(), enu.getZ());  
  lat = lla.getX();//lla(0);
  lon = lla.getY();//lla(1);
  alt = lla.getZ();//lla(2);
  
//  ArLog::log(ArLog::Normal, "GPSLocaLog: convertMap2LLACoords: %g %g %g",
//	     lat, lon, alt);
  return true;
}
