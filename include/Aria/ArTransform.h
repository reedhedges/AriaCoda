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
#ifndef ARTRANSFORM_H
#define ARTRANSFORM_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"

/// Perform transforms between different coordinates
/** @ingroup UtilityClasses
*/
class ArTransform
{
public:
  /// Constructor
  ArTransform() : myX(0), myY(0), myTh(0)
    { 
      myCos = ArMath::cos(myTh);
      mySin = ArMath::sin(myTh);
    }
  /// Constructor, Sets the transform so points in this coord system
  /// transform to abs world coords

  ArTransform(const ArPose& pose) 
    { 
      setTransform(pose);
    }
  /// Constructor, sets the transform so that pose1 will be
  /// transformed to pose2
  ArTransform(const ArPose& pose1, const ArPose& pose2)
    {
      setTransform(pose1, pose2);
    }
  /// Destructor
  //virtual ~ArTransform() {}

  /// Take the source pose and run the transform on it to put it into abs 
  /// coordinates
  /** 
      @param source the parameter to transform
      @return the source transformed into absolute coordinates
  */
  ArPose doTransform(const ArPose& source) const
  {
    return ArPose(
      (myX + myCos * source.getX() + mySin * source.getY()),
      (myY + myCos * source.getY() - mySin * source.getX()),
      (ArMath::addAngle(source.getTh(),myTh)) 
    );
  }

  ArPose operator()(const ArPose& source) const
  {
    return doTransform(source);
  }

  ArPose operator()(ArPose& source) const
  {
    return doTransform(source);
  }


  /// Take the source pose and run the transform on it to put it into abs 
  /// coordinates
  /** 
      @param source the parameter to transform
      @return the source transformed into absolute coordinates
  */
  ArPoseWithTime doTransform(const ArPoseWithTime& source) const
  {
    return ArPoseWithTime(
      (myX + myCos * source.getX() + mySin * source.getY()),
      (myY + myCos * source.getY() - mySin * source.getX()),
      (ArMath::addAngle(source.getTh(),myTh)), 
      source.getTime()
    );
  }

  ArPoseWithTime operator()(const ArPoseWithTime& source) const
  {
    return doTransform(source);
  }

  
  ArPoseWithTime operator()(ArPoseWithTime& source) const
  {
    return doTransform(source);
  }

  /// Take the source pose and run the inverse transform on it, taking it from
  /// abs coords to local
  /** 
      The source and result can be the same
      @param source the parameter to transform
      @return the source transformed from absolute into local coords
  */
  ArPose doInvTransform(const ArPose& source) const
    {
      const double tx = source.getX() - myX;
      const double ty = source.getY() - myY;
      return ArPose(
        (myCos * tx - mySin * ty),
        (myCos * ty + mySin * tx),
        ArMath::subAngle(source.getTh(),myTh)
      );        
    }

  /// Take the source pose and run the inverse transform on it, taking it from
  /// abs coords to local
  /** 
      The source and result can be the same
      @param source the parameter to transform
      @return the source transformed from absolute into local coords
  */
  ArPoseWithTime doInvTransform(const ArPoseWithTime& source) const
    {
      const double tx = source.getX() - myX;
      const double ty = source.getY() - myY;
      return ArPoseWithTime(
        (myCos * tx - mySin * ty),
        (myCos * ty + mySin * tx),
        ArMath::subAngle(source.getTh(),myTh),     
        source.getTime()
      );
    }


  /// Take a std::list of sensor readings and do the transform on it
  /// @deprecated use std::transform(poseList, t)
  AREXPORT void doTransform(std::list<ArPose *> *poseList);
  /// @deprecated use std::transform(poseList, t)
  /// Take a std::list of sensor readings and do the transform on it
  AREXPORT void doTransform(std::list<ArPoseWithTime *> *poseList);
  /// Sets the transform so points in this coord system transform to abs world coords
  AREXPORT void setTransform(const ArPose& pose);
  /// Sets the transform so that pose1 will be transformed to pose2
  AREXPORT void setTransform(const ArPose& pose1, const ArPose& pose2);
  /// Gets the transform x value (mm)
  double getX() const { return myX; } 
  /// Gets the transform y value (mm)
  double getY() const { return myY; }
  /// Gets the transform angle value (degrees)
  double getTh()const  { return myTh; }
  /// Internal function for setting the transform from low level data not poses
  AREXPORT void setTransformLowLevel(double x, double y, double th);
protected:
  double myX;
  double myY;
  double myTh;
  double myCos;
  double mySin;
};



#endif // ARTRANSFORM_H
