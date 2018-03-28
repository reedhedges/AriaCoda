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
#ifndef ARGPSCOORDS_H
#define ARGPSCOORDS_H

#include "ariaTypedefs.h"



class Ar3DPoint;
class ArLLACoords;
class ArECEFCoords;
class ArENUCoords;
class ArWGS84;

/** Base class for points in 3 dimensional cartesian space. 
  @ingroup UtilityClasses
*/
class Ar3DPoint 
{
  public:

  Ar3DPoint(void) : myX(0), myY(0), myZ(0) {}
  Ar3DPoint(double x, double y, double z) : myX(x), myY(y), myZ(z) {}
  /// Destructor.
  ~Ar3DPoint() {}
  /// Add
  Ar3DPoint operator+(Ar3DPoint c)
  {
    Ar3DPoint sum(myX + c.myX, 
		  myY + c.myY,
		  myZ + c.myZ);
    
    return sum;
  }
  /// Diff
  Ar3DPoint operator-(Ar3DPoint c)
  {
    Ar3DPoint dif(myX - c.myX, 
		  myY - c.myY,
		  myZ - c.myZ);
    return dif;
  }
  /// Diff
  Ar3DPoint operator*(double c)
  {
    Ar3DPoint pro(myX*c, myY*c, myZ*c);
    return pro;
  }
  /// Dot product
  double dot(Ar3DPoint c)
  {
    double dotP(myX * c.myX + myY * c.myY + myZ * c.myZ);
    return dotP;
  }
  /// Cross product
  Ar3DPoint cross(Ar3DPoint c)
  {
    Ar3DPoint crossP(myY * c.myZ - myZ * c.myY, 
		     myZ * c.myX - myX * c.myZ, 
		     myX * c.myY - myY * c.myX);
    return crossP;
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

#ifdef WIN32
// Need to export some variables on Windows because they are used in inline methods (which is good), but they can't be exported if const.
#define ARGPSCOORDS_CONSTANT 
#else
#define ARGPSCOORDS_CONSTANT const
#endif


/**
 * All the constants defined by the World Geodetic System 1984.
 * @ingroup UtilityClasses
 */
class ArWGS84
{
  public:
  ArWGS84(void) {}
  
  static double getE()     {return mye;}
  static double getA()     {return mya;}
  static double getB()     {return myb;}
  static double getEP()    {return myep;}
  static double get1byf()  {return my1byf;}
  static double getOmega() {return myOmega;}
  static double getGM()    {return myGM;}
  

private:
  AREXPORT static ARGPSCOORDS_CONSTANT double mya;     // meters
  AREXPORT static ARGPSCOORDS_CONSTANT double myb;     // meters
  AREXPORT static ARGPSCOORDS_CONSTANT double myep; 
  AREXPORT static ARGPSCOORDS_CONSTANT double myc;     // m/sec
  AREXPORT static ARGPSCOORDS_CONSTANT double mye;
  AREXPORT static ARGPSCOORDS_CONSTANT double my1byf;
  AREXPORT static ARGPSCOORDS_CONSTANT double myOmega; // rad/sec
  AREXPORT static ARGPSCOORDS_CONSTANT double myGM;    // m^3/sec^2
  AREXPORT static ARGPSCOORDS_CONSTANT double myg;     // m/sec^2. Ave g.
  AREXPORT static ARGPSCOORDS_CONSTANT double myM;     // kg. Mass of earth.
};


/**
 * Earth Centered Earth Fixed Coordinates.
   @ingroup UtilityClasses
 */
class ArECEFCoords : public Ar3DPoint
{
  public:
  ArECEFCoords(double x, double y, double z) : Ar3DPoint(x, y, z) {}
  AREXPORT ArLLACoords ECEF2LLA(void);
  AREXPORT ArENUCoords ECEF2ENU(ArECEFCoords ref);
};

/**
 * Latitude, Longitude and Altitude Coordinates.
 * @ingroup UtilityClasses
 */
class ArLLACoords : public Ar3DPoint
{
  public:
  ArLLACoords(void) : Ar3DPoint(0, 0, 0) {}
  ArLLACoords(double x, double y, double z) : Ar3DPoint(x, y, z) {}
  ArLLACoords(ArPose pos, double alt) : Ar3DPoint(pos.getX(), pos.getY(), alt) {}
  AREXPORT ArECEFCoords LLA2ECEF(void);
  double getLatitude(void) const {return getX();}
  double getLongitude(void) const {return getY();}
  double getAltitude(void) const {return getZ();}
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
  AREXPORT ArECEFCoords ENU2ECEF(ArLLACoords ref);
  double getEast(void) const {return getX();}
  double getNorth(void) const {return getY();}
  double getUp(void) const {return getZ();}
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
  ArMapGPSCoords(ArLLACoords org) : ArENUCoords(0.0, 0.0, 0.0), myOriginECEF(0), myOriginLLA(0), myOriginSet(false) 
  {
    setOrigin(org);
  }
  ArMapGPSCoords() : ArENUCoords(0, 0, 0), myOriginECEF(0), myOriginLLA(0), myOriginSet(false)
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
  void setOrigin(ArLLACoords org) {
    if(myOriginLLA)
      delete myOriginLLA;
    if(myOriginECEF)
      delete myOriginECEF;
    myOriginSet = true;
    myOriginLLA = new ArLLACoords(org);
    myOriginECEF = new ArECEFCoords(myOriginLLA->LLA2ECEF());
  }
     
  ArECEFCoords* myOriginECEF;
  ArLLACoords* myOriginLLA;
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
  virtual void invoke(char *buf) {
    // can't do anything
  }
};
  
  



#endif // ARGPSCOORDS_H
