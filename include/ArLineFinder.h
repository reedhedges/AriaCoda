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
#ifndef ARSICKLINEFINDER_H
#define ARSICKLINEFINDER_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ariaUtil.h"
#include <vector>

class ArLineFinderSegment;
class ArConfig;

/** This class finds lines out of any range device with raw readings (lasers for instance)
 @ingroup OptionalClasses
 @ingroup UtilityClasses
*/
class ArLineFinder
{
public:
  /// Constructor
  AREXPORT ArLineFinder(ArRangeDevice *dev);
  /// Destructor
  AREXPORT virtual ~ArLineFinder();

#ifndef SWIG
  /// Finds the lines and returns a pointer to ArLineFinder's map of them 
  /** @swigomit */
  AREXPORT std::map<int, ArLineFinderSegment *> *getLines(void);
  /// Finds the lines, but then returns a pointer to ArLineFinder's map of the points that AREN'T in lines
  /** @swigomit */
  AREXPORT std::map<int, ArPose> *getNonLinePoints(void);
#endif
  /// Finds the lines, then copies @b pointers to them them into a new set
  AREXPORT std::set<ArLineFinderSegment*> getLinesAsSet();
  /// Finds the lines, and then copies the points that AREN'T in the lines into a new set
  AREXPORT std::set<ArPose> getNonLinePointsAsSet();

  /// Gets the robot pose at which the data from the range device (provided in
  /// constructor) was received
  ArPose getLinesTakenPose(void) { return myPoseTaken; }
  /// Logs all the points and lines from the last getLines
  AREXPORT void saveLast(void);
  /// Gets the lines, then prints them
  AREXPORT void getLinesAndSaveThem(void);
  /// Whether to print verbose information about line decisions
  void setVerbose(bool verbose) { myPrinting = verbose; }
  /// Whether to print verbose information about line decisions
  bool getVerbose(void) { return myPrinting; }
  /// Sets some parameters for line creation
  void setLineCreationParams(int minLineLen = 40, int minLinePoints = 2)
    { myMakingMinLen = minLineLen; myMakingMinPoints = minLinePoints; }
  /// Sets some parameters for combining two possible lines into one line.
  void setLineCombiningParams(int angleTol = 30, int linesCloseEnough = 75) 
    { myCombiningAngleTol = angleTol; 
    myCombiningLinesCloseEnough = linesCloseEnough; }
  /// Filter out lines based on line length or number of points in the line.
  void setLineFilteringParams(int minPointsInLine = 3, int minLineLength = 75)
    { myFilteringMinPointsInLine = minPointsInLine; 
    myFilteringMinLineLength = minLineLength; }
  /// Don't let lines happen that have points not close to it
  void setLineValidParams(int maxDistFromLine = 30, 
			  int maxAveDistFromLine = 20)
    { myValidMaxDistFromLine = maxDistFromLine; 
    myValidMaxAveFromLine = maxAveDistFromLine; }

  /** Sets a maximum distance for points to be included in the same line. If
   points are greater than this distance apart, then they will not be
   considered in the same line.  If this value is 0, then there is no
   maximum-distance condition and points will be considered part of a line no
   matter how far apart.
  */
  void setMaxDistBetweenPoints(int maxDistBetweenPoints = 0)
    { myMaxDistBetweenPoints = maxDistBetweenPoints; }

  /// Add this ArLineFinder's parameters to the given ArConfig object.
  AREXPORT void addToConfig(ArConfig *config,
			    const char *section);
protected:
  // where the readings were taken
  ArPose myPoseTaken;
  // our points
  std::map<int, ArPose> *myPoints;
  std::map<int, ArLineFinderSegment *> *myLines;
  std::map<int, ArPose> *myNonLinePoints;
  // fills up the myPoints variable from sick laser
  AREXPORT void fillPointsFromLaser(void);
  // fills up the myLines variable from the myPoints
  AREXPORT void findLines(void);
  // cleans the lines and puts them into myLines 
  AREXPORT bool combineLines();
  // takes two segments and sees if it can average them
  AREXPORT ArLineFinderSegment *averageSegments(ArLineFinderSegment *line1, 
					  ArLineFinderSegment *line2);
  // removes lines that don't have enough points added in
  AREXPORT void filterLines();

  bool myFlippedFound;
  bool myFlipped;
  int myValidMaxDistFromLine;
  int myValidMaxAveFromLine;
  int myMakingMinLen;
  int myMakingMinPoints;
  int myCombiningAngleTol;
  int myCombiningLinesCloseEnough;
  int myFilteringMinPointsInLine;
  int myFilteringMinLineLength;
  int myMaxDistBetweenPoints;
  double mySinMultiplier;
  bool myPrinting;
  ArRangeDevice *myRangeDevice;
};

/// Class for ArLineFinder to hold more info than an ArLineSegment
class ArLineFinderSegment : public ArLineSegment
{
public:
  ArLineFinderSegment() {}
  ArLineFinderSegment(double x1, double y1, double x2, double y2, 
		      int numPoints = 0, int startPoint = 0, int endPoint = 0)
    { newEndPoints(x1, y1, x2, y2, numPoints, startPoint, endPoint); }
  virtual ~ArLineFinderSegment() {}
  void newEndPoints(double x1, double y1, double x2, double y2, 
		    int numPoints = 0, int startPoint = 0, int endPoint = 0)
    {
      ArLineSegment::newEndPoints(x1, y1, x2, y2);
      myLineAngle = ArMath::atan2(y2 - y1, x2 - x1);
      myLength = ArMath::distanceBetween(x1, y1, x2, y2);
      myNumPoints = numPoints;
      myStartPoint = startPoint;
      myEndPoint = endPoint;
      myAveDistFromLine = 0;
    }
  double getLineAngle(void) { return myLineAngle; }
  double getLength(void) { return myLength; }
  int getNumPoints(void) { return myNumPoints; }
  int getStartPoint(void) { return myStartPoint; }
  int getEndPoint(void) { return myEndPoint; }
  void setAveDistFromLine(double aveDistFromLine) 
    { myAveDistFromLine = aveDistFromLine; }
  double getAveDistFromLine(void) { return myAveDistFromLine; }
protected:
  double myLineAngle;
  double myLength;
  int myNumPoints;
  int myStartPoint;
  int myEndPoint;
  double myAveDistFromLine;
};

#endif // ARSICKLINEFINDER_H
