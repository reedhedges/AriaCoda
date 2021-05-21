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
#ifndef ARGPSCOORDS_H
#define ARGPSCOORDS_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"


class ArLLACoords;
class ArECEFCoords;
class ArENUCoords;
struct ArWGS84;

/** Base class for points in 3 dimensional cartesian space. 
  @ingroup UtilityClasses
*/
class Ar3DPoint 
{
  public:

  Ar3DPoint() : myX(0), myY(0), myZ(0) {}
  Ar3DPoint(double x, double y, double z) : myX(x), myY(y), myZ(z) {}

  /// Add
  Ar3DPoint operator+(const Ar3DPoint& c)
  {
    return Ar3DPoint(myX + c.myX, 
		  myY + c.myY,
		  myZ + c.myZ);
  }
  /// Diff
  Ar3DPoint operator-(const Ar3DPoint& c)
  {
    return Ar3DPoint(myX - c.myX, 
		  myY - c.myY,
		  myZ - c.myZ);
  }
  /// Multiply
  Ar3DPoint operator*(double c)
  {
    return Ar3DPoint(myX*c, myY*c, myZ*c);
  }
  /// Dot product
  double dot(const Ar3DPoint& c)
  {
    return (myX * c.myX + myY * c.myY + myZ * c.myZ);
  }
  /// Cross product
  Ar3DPoint cross(const Ar3DPoint& c)
  {
    return Ar3DPoint(myY * c.myZ - myZ * c.myY, 
		     myZ * c.myX - myX * c.myZ, 
		     myX * c.myY - myY * c.myX);
  }
  /// Print.
  /** @swignote Use 'printPoint' instead */
  AREXPORT void print(const char* head=NULL);

  double getX() const {return myX;}
  double getY() const {return myY;}
  double getZ() const {return myZ;}
  void setX(double x) { myX = x; }
  void setY(double y) { myY = y; }
  void setZ(double z) { myZ = z; }

protected:

  double myX;
  double myY;
  double myZ;

};



/**
 * All the constants defined by the World Geodetic System 1984.
 * @ingroup UtilityClasses
 */
struct ArWGS84
{
  static const double A; ///< m
  static const double B; ///< m
  static const double E;
  static const double C; ///< m/s
  static const double EP;
  static const double Finv; ///< inverse of flattening factor (1/f)
  static const double Omega; ///< rad/s
  static const double GM; ///< m^3/sec^2
  static const double G; ///< m/s^2 average gravity accel
  static const double M; ///< kg mass of earth


};


/**
 * Earth Centered Earth Fixed Coordinates.
   @ingroup UtilityClasses
 */
class ArECEFCoords : public Ar3DPoint
{
  public:
  ArECEFCoords(const double x, const double y, const double z) : Ar3DPoint(x, y, z) {}
  ArECEFCoords() : ArECEFCoords(0,0,0) {}
  AREXPORT ArLLACoords ECEF2LLA() const;
  AREXPORT ArENUCoords ECEF2ENU(const ArECEFCoords& ref) const;
};

/**
 * Latitude, Longitude and Altitude Coordinates.
 * @ingroup UtilityClasses
 */
class ArLLACoords : public Ar3DPoint
{
  public:
  ArLLACoords() : Ar3DPoint(0, 0, 0) {}
  ArLLACoords(double x, double y, double z) : Ar3DPoint(x, y, z) {}
  ArLLACoords(const ArPose& pos, double alt) : Ar3DPoint(pos.getX(), pos.getY(), alt) {}
  AREXPORT ArECEFCoords LLA2ECEF() const;
  double getLatitude() const {return getX();}
  double getLongitude() const {return getY();}
  double getAltitude() const {return getZ();}
  void setLatitude(double l) { setX(l); }
  void setLongitude(double l) { setY(l); }
  void setAltitude(double a) { setZ(a); }
};

/**
 * East North Up coordinates.
   @ingroup UtilityClasses
 */
class ArENUCoords : public Ar3DPoint
{
  public:
  ArENUCoords(double x, double y, double z) : Ar3DPoint(x, y, z) {}
  AREXPORT ArECEFCoords ENU2ECEF(const ArLLACoords& ref) const;
  double getEast() const {return getX();}
  double getNorth() const {return getY();}
  double getUp() const {return getZ();}
  void setEast(double e) { setX(e); }
  void setNorth(double n) { setY(n); }
  void setUp(double u) { setZ(u); }
};

/**
 * Coordinates based on a map with origin in LLA coords with conversion
 * methods from LLA to ENU and from ENU to LLA coordinates.
 * @ingroup UtilityClasses
 */
class ArMapGPSCoords : public ArENUCoords
{
public:
  ArMapGPSCoords(const ArLLACoords& org) : ArMapGPSCoords()
  {
    setOrigin(org);
  }

  ArMapGPSCoords() : ArENUCoords(0, 0, 0), myOriginSet(false)
  {
  }

  AREXPORT bool convertMap2LLACoords(const double ea, const double no, const double up,
			    double& lat, double& lon, double& alt) const;
  AREXPORT bool convertLLA2MapCoords(const double lat, const double lon, const double alt,
			    double& ea, double& no, double& up) const;
  bool convertLLA2MapCoords(const ArLLACoords& lla, double& ea, double& no, double& up)
{
    return convertLLA2MapCoords(lla.getLatitude(), lla.getLongitude(), lla.getAltitude(), ea, no, up);
  }
  void setOrigin(const ArLLACoords& org) {
    myOriginSet = true;
    myOriginLLA = org;
    myOriginECEF = myOriginLLA.LLA2ECEF();
  }
private:
  ArECEFCoords myOriginECEF;
  ArLLACoords myOriginLLA;
  bool myOriginSet;
};



/** Specialization of ArStringFormatBufFunctor for Ar3DPoint. 
    The format string should include three values: X, Y and Z.
    @see ArStringFormatBufFunctor
    @see Ar3DPoint
 */
template <typename SizeType>
class ArStringFormatBufFunctor <Ar3DPoint, SizeType> : public virtual ArFunctor2<char *, SizeType>
{
  ArRetFunctor<Ar3DPoint> *myAccessor;
  const char *myFormat;
public:
  ArStringFormatBufFunctor() {}
  ArStringFormatBufFunctor(ArRetFunctor<Ar3DPoint> *func, const char *format) :
    myAccessor(func),
    myFormat(format)
  {}
  virtual void invoke(char *buf, SizeType maxlen) {
    Ar3DPoint p = myAccessor->invokeR();
    snprintf(buf, maxlen, myFormat, p.getX(), p.getY(), p.getZ());
  }
  virtual void invoke() {
    // can't do anything
  }
  virtual void invoke(char *) {
    // can't do anything
  }
};
  
  



#endif // ARGPSCOORDS_H
