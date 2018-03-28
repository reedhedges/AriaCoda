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
#include "ArMapComponents.h"

#include <algorithm>
#include <iterator>
#ifdef WIN32
#include <process.h>
#endif 
#include <ctype.h>

#include "ArFileParser.h"
#include "ArMapUtils.h"
#include "ArMD5Calculator.h"

//#define ARDEBUG_MAP_COMPONENTS
#ifdef ARDEBUG_MAP_COMPONENTS
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

// ---------------------------------------------------------------------------- 
// ArMapScan
// ---------------------------------------------------------------------------- 

const char *ArMapScan::EOL_CHARS = "";

AREXPORT ArMapScan::ArMapScan(const char *scanType) :

  myScanType(!ArUtil::isStrEmpty(scanType) ? scanType : ""),
  myIsSummaryScan(isSummaryScanType(scanType)),
  myLogPrefix(),
  myKeywordPrefix(),
  myPointsKeyword(),
  myLinesKeyword(),
  myTimeChanged(),
  myDisplayString(!ArUtil::isStrEmpty(scanType) ? scanType : ""),
  myNumPoints(0),
  myNumLines(0),
  myResolution(0),
  myMax(),
  myMin(),
  myLineMax(),
  myLineMin(),

  myIsSortedPoints(false),
  myIsSortedLines(false),

  myPoints(),
  myLines(),

  myMinPosCB(this, &ArMapScan::handleMinPos),
  myMaxPosCB(this, &ArMapScan::handleMaxPos),
  myIsSortedPointsCB(this, &ArMapScan::handleIsSortedPoints),
  myNumPointsCB(this, &ArMapScan::handleNumPoints),

  myLineMinPosCB(this, &ArMapScan::handleLineMinPos),
  myLineMaxPosCB(this, &ArMapScan::handleLineMaxPos),
  myIsSortedLinesCB(this, &ArMapScan::handleIsSortedLines),
  myNumLinesCB(this, &ArMapScan::handleNumLines),

  myResolutionCB(this, &ArMapScan::handleResolution),
  myDisplayStringCB(this, &ArMapScan::handleDisplayString),
  //myDataCB(this, &ArMapScan::handleData),
  //myLinesCB(this, &ArMapScan::handleLines),
  myPointCB(this, &ArMapScan::handlePoint),
  myLineCB(this, &ArMapScan::handleLine)
{

  if (isDefaultScanType(myScanType.c_str()) ||
      (ArUtil::strcasecmp(myScanType.c_str(), "SickLaser") == 0)) {
    myKeywordPrefix = "";
    myPointsKeyword = "DATA";
    myLinesKeyword = "LINES";
  }
  else {
    myKeywordPrefix = myScanType;
    // TODO Any way to do an upper?
    myPointsKeyword = myScanType + "_DATA";
    myLinesKeyword = myScanType + "_LINES";
  }

  myLogPrefix = myScanType;
  if (!myLogPrefix.empty()) {
    myLogPrefix += " ";
  }

} // end constructor


AREXPORT ArMapScan::ArMapScan(const ArMapScan &other) :
  myScanType(other.myScanType),
  myIsSummaryScan(other.myIsSummaryScan),
  myLogPrefix(other.myLogPrefix),
  myKeywordPrefix(other.myKeywordPrefix),
  myPointsKeyword(other.myPointsKeyword),
  myLinesKeyword(other.myLinesKeyword),
  myTimeChanged(other.myTimeChanged),
  myDisplayString(other.myDisplayString),
  myNumPoints(0),  // Set later
  myNumLines(0),   // Set later
  myResolution(other.myResolution),
  myMax(other.myMax),
  myMin(other.myMin),
  myLineMax(other.myLineMax),
  myLineMin(other.myLineMin),
  myIsSortedPoints(other.myIsSortedPoints),
  myIsSortedLines(other.myIsSortedLines),
  myPoints(other.myPoints),
  myLines(other.myLines),

  // Not entirely sure what to do with these in a copy ctor situation...
  // but this seems safest
  myMinPosCB(this, &ArMapScan::handleMinPos),
  myMaxPosCB(this, &ArMapScan::handleMaxPos),
  myIsSortedPointsCB(this, &ArMapScan::handleIsSortedPoints),
  myNumPointsCB(this, &ArMapScan::handleNumPoints),

  myLineMinPosCB(this, &ArMapScan::handleLineMinPos),
  myLineMaxPosCB(this, &ArMapScan::handleLineMaxPos),
  myIsSortedLinesCB(this, &ArMapScan::handleIsSortedLines),
  myNumLinesCB(this, &ArMapScan::handleNumLines),

  myResolutionCB(this, &ArMapScan::handleResolution),
  myDisplayStringCB(this, &ArMapScan::handleDisplayString),
  //myDataCB(this, &ArMapScan::handleData),
  //myLinesCB(this, &ArMapScan::handleLines),
  myPointCB(this, &ArMapScan::handlePoint),
  myLineCB(this, &ArMapScan::handleLine)
{

  if (!myIsSummaryScan) {
    myNumLines = other.myLines.size();
  }
  else {
    myNumLines = other.myNumLines;
  }
  if (myNumLines != other.myNumLines) {
    ArLog::log(ArLog::Normal,
              "%sArMapScan copy constructor adjusted numLines from %i to %i",
              myLogPrefix.c_str(),
              other.myNumLines,
              myNumLines);
  }

  if (!myIsSummaryScan) {
    myNumPoints = other.myPoints.size();
  }
  else {
    myNumPoints = other.myNumPoints;
  }
  if (myNumPoints != other.myNumPoints) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan copy constructor adjusted numPoints from %i to %i",
               myLogPrefix.c_str(),
               other.myNumPoints,
               myNumPoints);
  }

} // end copy constructor


AREXPORT ArMapScan &ArMapScan::operator=(const ArMapScan &other) 
{
  if (&other != this) {
  
    myScanType = other.myScanType;
    myIsSummaryScan = other.myIsSummaryScan;

    myLogPrefix     = other.myLogPrefix;
    myKeywordPrefix = other.myKeywordPrefix;
    myPointsKeyword = other.myPointsKeyword;
    myLinesKeyword  = other.myLinesKeyword;

    myTimeChanged = other.myTimeChanged;
    myDisplayString = other.myDisplayString;

    //myNumPoints   = other.myNumPoints;
    //myNumLines = other.myNumLines;
    if (!myIsSummaryScan) {
      myNumLines = other.myLines.size();
    }
    else {
      myNumLines = other.myNumLines;
    }
    if (myNumLines != other.myNumLines) {
      ArLog::log(ArLog::Normal,
                "%sArMapScan operator= adjusted numLines from %i to %i",
                myLogPrefix.c_str(),
                other.myNumLines,
                myNumLines);
    }

    if (!myIsSummaryScan) {
      myNumPoints = other.myPoints.size();
    }
    else {
      myNumPoints = other.myNumPoints;
    }
    if (myNumPoints != other.myNumPoints) {
      ArLog::log(ArLog::Normal,
                "%sArMapScan operator= adjusted numPoints from %i to %i",
                myLogPrefix.c_str(),
                other.myNumPoints,
                myNumPoints);
    }

    myResolution = other.myResolution;
    myMax = other.myMax;
    myMin = other.myMin;
    myLineMax = other.myLineMax;
    myLineMin = other.myLineMin;
    myIsSortedPoints = other.myIsSortedPoints;
    myIsSortedLines = other.myIsSortedLines;
    myPoints = other.myPoints;
    myLines = other.myLines;
  }
  return *this;
}


AREXPORT ArMapScan::~ArMapScan()
{
}

AREXPORT bool ArMapScan::addToFileParser(ArFileParser *fileParser)
{
   ArLog::log(ArLog::Verbose, 
              "%sArMapScan::addToFileParser: called parser = %p",
              myLogPrefix.c_str(), fileParser);
 

  if (fileParser == NULL) {
    ArLog::log(ArLog::Terse, 
                 "%sArMapScan::addToFileParser: could not add handlers to NULL fileParser",
                 myLogPrefix.c_str());
    return false;
  }
  if (!addHandlerToFileParser(fileParser, "MinPos:", &myMinPosCB) ||
      !addHandlerToFileParser(fileParser, "MaxPos:", &myMaxPosCB) ||
      !addHandlerToFileParser(fileParser, "NumPoints:", &myNumPointsCB) ||
      !addHandlerToFileParser(fileParser, "PointsAreSorted:", &myIsSortedPointsCB) ||
      !addHandlerToFileParser(fileParser, "LineMinPos:", &myLineMinPosCB) ||
      !addHandlerToFileParser(fileParser, "LineMaxPos:", &myLineMaxPosCB) ||
      !addHandlerToFileParser(fileParser, "NumLines:", &myNumLinesCB) ||
      !addHandlerToFileParser(fileParser, "LinesAreSorted:", &myIsSortedLinesCB) ||
      !addHandlerToFileParser(fileParser, "Resolution:", &myResolutionCB) ||
      !addHandlerToFileParser(fileParser, "Display:", &myDisplayStringCB))
       {
    ArLog::log(ArLog::Terse, 
               "%sArMapScan::addToFileParser: could not add handlers",
               myLogPrefix.c_str());
    return false;
  }  
  ArLog::log(ArLog::Verbose,
             "%sArMapScan::addToFileParser() successfully added handlers",
             myLogPrefix.c_str());

  return true;

} // end method addToFileParser


AREXPORT bool ArMapScan::remFromFileParser(ArFileParser *fileParser)
{
 if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myMinPosCB);
  fileParser->remHandler(&myMaxPosCB);
  fileParser->remHandler(&myNumPointsCB);
  fileParser->remHandler(&myIsSortedPointsCB);

  fileParser->remHandler(&myLineMinPosCB);
  fileParser->remHandler(&myLineMaxPosCB);
  fileParser->remHandler(&myNumLinesCB);
  fileParser->remHandler(&myIsSortedLinesCB);

  fileParser->remHandler(&myResolutionCB);
  fileParser->remHandler(&myDisplayStringCB);
 
  return true;

} // end method remFromFileParser


AREXPORT bool ArMapScan::addExtraToFileParser(ArFileParser *fileParser,
                                              bool isAddLineHandler)
{
  ArLog::log(ArLog::Verbose,
             "ArMapScan::addExtraToFileParser called");

  if (fileParser == NULL) {
    ArLog::log(ArLog::Verbose,
               "ArMapScan::addExtraToFileParser cannot add to NULL fileParser");
    return false;
  }
  if (isAddLineHandler) {
    if (!addHandlerToFileParser(fileParser, NULL, &myLineCB)) {
       ArLog::log(ArLog::Verbose,
                 "ArMapScan::addExtraToFileParser error adding line handler");
       return false;
    }
  }
  else {
    if (!addHandlerToFileParser(fileParser, NULL, &myPointCB)) {
       ArLog::log(ArLog::Verbose,
                 "ArMapScan::addExtraToFileParser error adding point handler");
      return false;
    }
  }
  return true;

} // end method addExtraToFileParser


AREXPORT bool ArMapScan::remExtraFromFileParser(ArFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myLineCB);
  fileParser->remHandler(&myPointCB);

  return true;

} // end method remExtraFromFileParser



AREXPORT ArTime ArMapScan::getTimeChanged() const
{
  return myTimeChanged;
}
  

AREXPORT void ArMapScan::clear()
{
  myTimeChanged.setToNow();
  myNumPoints = 0;
  myNumLines  = 0;
  myResolution = 0;
  myMax.setPose(0, 0);
  myMin.setPose(0, 0);
  myLineMax.setPose(0, 0);
  myLineMin.setPose(0, 0);
  myIsSortedPoints = false;
  myIsSortedLines = false;

  myPoints.clear();
  myLines.clear();

} // end method clear

AREXPORT const char *ArMapScan::getDisplayString(const char *scanType)
{
  return myDisplayString.c_str();
}

AREXPORT std::vector<ArPose> *ArMapScan::getPoints(const char *scanType)
{
  return &myPoints;
}

AREXPORT std::vector<ArLineSegment> *ArMapScan::getLines(const char *scanType)
{
  return &myLines;
}

AREXPORT ArPose ArMapScan::getMinPose(const char *scanType)
{
  return myMin;
}

AREXPORT ArPose ArMapScan::getMaxPose(const char *scanType)
{
  return myMax;
}

AREXPORT int ArMapScan::getNumPoints(const char *scanType)
{
  return myNumPoints;
}

AREXPORT ArPose ArMapScan::getLineMinPose(const char *scanType)
{
  return myLineMin;
}

AREXPORT ArPose ArMapScan::getLineMaxPose(const char *scanType)
{
  return myLineMax;
}

AREXPORT int ArMapScan::getNumLines(const char *scanType)
{ 
  return myNumLines;
}

AREXPORT int ArMapScan::getResolution(const char *scanType)
{
  return myResolution;
}

AREXPORT bool ArMapScan::isSortedPoints(const char *scanType) const
{
  return myIsSortedPoints;
}

AREXPORT bool ArMapScan::isSortedLines(const char *scanType) const
{
  return myIsSortedLines;
}


AREXPORT void ArMapScan::setPoints(const std::vector<ArPose> *points,
                                   const char *scanType,
                                   bool isSorted,
                                   ArMapChangeDetails *changeDetails)
{
  if (!myIsSortedPoints) {
	  std::sort(myPoints.begin(), myPoints.end());
    myIsSortedPoints = true;
  }

  const std::vector<ArPose> *newPoints = points;
  std::vector<ArPose> *pointsCopy = NULL;

  if (!isSorted && (points != NULL)) {
	  pointsCopy = new std::vector<ArPose>(*points);
	  std::sort(pointsCopy->begin(), pointsCopy->end());
    newPoints = pointsCopy;
  }

  if (changeDetails != NULL) {
    
    if (newPoints != NULL) {
    
      ArTime timeToDiff;

      set_difference(myPoints.begin(), myPoints.end(), 
                     newPoints->begin(), newPoints->end(),
                     std::inserter(*(changeDetails->getChangedPoints
                                    (ArMapChangeDetails::DELETIONS, scanType)), 
                              changeDetails->getChangedPoints
                                    (ArMapChangeDetails::DELETIONS, scanType)->begin()));
      set_difference(newPoints->begin(), newPoints->end(),
                     myPoints.begin(), myPoints.end(), 
                     std::inserter(*(changeDetails->getChangedPoints
                                    (ArMapChangeDetails::ADDITIONS, scanType)), 
                              changeDetails->getChangedPoints
                                    (ArMapChangeDetails::ADDITIONS, scanType)->begin()));

      ArLog::log(ArLog::Normal,
                 "%sArMapScan::setPoints() %i points were deleted, %i added",
                 myLogPrefix.c_str(),
                 changeDetails->getChangedPoints
                                    (ArMapChangeDetails::DELETIONS, scanType)->size(),
                 changeDetails->getChangedPoints
                                    (ArMapChangeDetails::ADDITIONS, scanType)->size());

      long int elapsed = timeToDiff.mSecSince();

      ArLog::log(ArLog::Normal,
                "%sArMapScan::setPoints() took %i msecs to find changes in %i points for %s",
                myLogPrefix.c_str(),
                elapsed,
                myNumPoints,
                scanType);

    }
    else { // null points means none added and all deleted

      *(changeDetails->getChangedPoints(ArMapChangeDetails::DELETIONS, scanType)) = myPoints;
    }
  } // end if track changes

  int origNumPoints = myNumPoints;
  ArPose origMin = myMin;
  ArPose origMax = myMax;

  myTimeChanged.setToNow();

  if ((newPoints != NULL) && (!newPoints->empty())) {

    ArTime timeToCopy;

    double maxX = INT_MIN;
    double maxY = INT_MIN;
    double minX = INT_MAX;
    double minY = INT_MAX;

    for (std::vector<ArPose>::const_iterator it = newPoints->begin(); 
         it != newPoints->end(); 
         it++)
    {
      const ArPose &pose = (*it);

      if (pose.getX() > maxX)
        maxX = pose.getX();
      if (pose.getX() < minX)
        minX = pose.getX();

      if (pose.getY() > maxY)
        maxY = pose.getY();
      if (pose.getY() < minY)
        minY = pose.getY();
     
    } // end for each point

    myPoints = *newPoints;  
    if (myNumPoints != (int) myPoints.size()) {

      ArLog::log(ArLog::Normal,
                 "%sArMapScan::setPoints() point count changed from %i to %i",
                 myLogPrefix.c_str(),
                 myNumPoints,
                 myPoints.size());

      myNumPoints = myPoints.size();
    } 
    myMax.setPose(maxX, maxY);
    myMin.setPose(minX, minY);

    long int elapsed = timeToCopy.mSecSince();

    ArLog::log(ArLog::Normal,
               "%sArMapScan::setPoints() took %i msecs to find min/max of %i points",
               myLogPrefix.c_str(),
               elapsed,
               myNumPoints);

  } // end if new points
  else { // no new points

    myPoints.clear();
    myMax.setX(INT_MIN);
    myMax.setY(INT_MIN);
    myMin.setX(INT_MAX);
    myMin.setY(INT_MAX);
    myNumPoints = 0;

  } // end else no new points


  if (changeDetails != NULL) {

    ArMapFileLineSetWriter deletionWriter(changeDetails->getChangedSummaryLines
                                          (ArMapChangeDetails::DELETIONS, scanType));
    ArMapFileLineSetWriter additionWriter(changeDetails->getChangedSummaryLines
                                          (ArMapChangeDetails::ADDITIONS, scanType));

    if (origNumPoints != myNumPoints) {
      ArUtil::functorPrintf(&deletionWriter, "%sNumPoints: %d%s",
                            getKeywordPrefix(),
			                      origNumPoints, EOL_CHARS);
      ArUtil::functorPrintf(&additionWriter, "%sNumPoints: %d%s", 
                            getKeywordPrefix(),
			                      myNumPoints, EOL_CHARS);
    }

    if (origMin != myMin) {
      if (origNumPoints != 0) {
        ArUtil::functorPrintf(&deletionWriter, "%sMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origMin.getX(), origMin.getY(),  EOL_CHARS);
      }
      if (myNumPoints != 0) {
        ArUtil::functorPrintf(&additionWriter, "%sMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myMin.getX(), myMin.getY(),  EOL_CHARS);
      }
    } // end if min changed
    if (origMax != myMax) {
      if (origNumPoints != 0) {
        ArUtil::functorPrintf(&deletionWriter, "%sMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origMax.getX(), origMax.getY(),  EOL_CHARS);
      }
      if (myNumPoints != 0) {
        ArUtil::functorPrintf(&additionWriter, "%sMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myMax.getX(), myMax.getY(),  EOL_CHARS);
      }
    } // end if min changed

  } // end if track changes

  if (pointsCopy != NULL) {
    delete pointsCopy;
  }

} // end method setPoints


AREXPORT void ArMapScan::setLines(const std::vector<ArLineSegment> *lines,
                                  const char *scanType,
                                  bool isSorted,
                                  ArMapChangeDetails *changeDetails)
{
  if (!myIsSortedLines) {
	  std::sort(myLines.begin(), myLines.end());
    myIsSortedLines = true;
  }
 
  const std::vector<ArLineSegment> *newLines = lines;
  std::vector<ArLineSegment> *linesCopy = NULL;

  if (!isSorted && (lines != NULL)) {
	  linesCopy = new std::vector<ArLineSegment>(*lines);
	  std::sort(linesCopy->begin(), linesCopy->end());
    newLines = linesCopy;
  }


 if (changeDetails != NULL) {

    if (newLines != NULL) {
 
      set_difference(myLines.begin(), myLines.end(), 
                     newLines->begin(), newLines->end(),
                     std::inserter(*(changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::DELETIONS, scanType)), 
                              changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::DELETIONS, scanType)->begin()));
      set_difference(newLines->begin(), newLines->end(),
                     myLines.begin(), myLines.end(), 
                     std::inserter(*(changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::ADDITIONS, scanType)), 
                              changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::ADDITIONS, scanType)->begin()));

      ArLog::log(ArLog::Normal,
                 "%sArMapScan::setLines() %i lines were deleted, %i added",
                 myLogPrefix.c_str(),
                 changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::DELETIONS, scanType)->size(),
                 changeDetails->getChangedLineSegments
                                    (ArMapChangeDetails::ADDITIONS, scanType)->size());

    }
    else { // null lines means none added and all deleted

      *(changeDetails->getChangedLineSegments(ArMapChangeDetails::DELETIONS, scanType)) 
                  = myLines;
    }
  } // end if track changes
 

  int origNumLines = myNumLines;
  ArPose origLineMin = myLineMin;
  ArPose origLineMax = myLineMax;

  myTimeChanged.setToNow();

  if ((newLines != NULL) && (!newLines->empty())) {

    ArTime timeToCopy;

    double maxX = INT_MIN;
    double maxY = INT_MIN;
    double minX = INT_MAX;
    double minY = INT_MAX;

    for (std::vector<ArLineSegment>::const_iterator it = newLines->begin(); 
         it != newLines->end(); 
         it++)
    {
      const ArLineSegment &line = (*it);

      if (line.getX1() > maxX)
        maxX = line.getX1();
      if (line.getX1() < minX)
        minX = line.getX1();

      if (line.getY1() > maxY)
        maxY = line.getY1();
      if (line.getY1() < minY)
        minY = line.getY1();
      
      if (line.getX2() > maxX)
        maxX = line.getX2();
      if (line.getX2() < minX)
        minX = line.getX2();

      if (line.getY2() > maxY)
        maxY = line.getY2();
      if (line.getY2() < minY)
        minY = line.getY2();
 
    } // end for each line

    myLines = *newLines;  
   
    if (myNumLines != (int) myLines.size()) {
      ArLog::log(ArLog::Normal,
                 "%sArMapScan::setLines() line count changed from %i to %i",
                 myLogPrefix.c_str(),
                 myNumLines,
                 myLines.size());
      myNumLines = myLines.size();
    }

    myLineMax.setPose(maxX, maxY);
    myLineMin.setPose(minX, minY);

    long int elapsed = timeToCopy.mSecSince();

    ArLog::log(ArLog::Normal,
               "%sArMapScan::setLines() took %i msecs to find min/max of %i lines",
               myLogPrefix.c_str(),
               elapsed,
               myNumLines);

  } // end if new lines
  else { // no new lines

    myLines.clear();
    myLineMax.setX(INT_MIN);
    myLineMax.setY(INT_MIN);
    myLineMin.setX(INT_MAX);
    myLineMin.setY(INT_MAX);
    myNumLines = 0;

  } // end else no new lines


  if (changeDetails != NULL) {

    ArMapFileLineSetWriter deletionWriter(changeDetails->getChangedSummaryLines
                                          (ArMapChangeDetails::DELETIONS, scanType));
    ArMapFileLineSetWriter additionWriter(changeDetails->getChangedSummaryLines
                                          (ArMapChangeDetails::ADDITIONS, scanType));

    if (origNumLines != myNumLines) {
      ArUtil::functorPrintf(&deletionWriter, "%sNumLines: %d%s", 
                            getKeywordPrefix(),
			                      origNumLines, EOL_CHARS);
      ArUtil::functorPrintf(&additionWriter, "%sNumLines: %d%s", 
                            getKeywordPrefix(),
			                      myNumLines, EOL_CHARS);
    }

    if (origLineMin != myLineMin) {
      if (origNumLines != 0) {
        ArUtil::functorPrintf(&deletionWriter, "%sLineMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origLineMin.getX(), origLineMin.getY(),  EOL_CHARS);
      }
      if (myNumLines != 0) {
        ArUtil::functorPrintf(&additionWriter, "%sLineMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myLineMin.getX(), myLineMin.getY(),  EOL_CHARS);
      }
    } // end if min changed

    if (origLineMax != myLineMax) {
      if (origNumLines != 0) {
        ArUtil::functorPrintf(&deletionWriter, "%sLineMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origLineMax.getX(), origLineMax.getY(),  EOL_CHARS);
      }
      if (myNumLines != 0) {
        ArUtil::functorPrintf(&additionWriter, "%sLineMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myLineMax.getX(), myLineMax.getY(),  EOL_CHARS);
      }
    } // end if max changed

  } // end if track changes

  if (linesCopy != NULL) {
    delete linesCopy;
  }

} // end method setLines


AREXPORT void ArMapScan::setResolution(int resolution,
                                       const char *scanType,
                                       ArMapChangeDetails *changeDetails)
{


  if (myResolution == resolution) {
    return;
  }

  ArMapFileLineSet origLines;

  if (changeDetails != NULL) {
    ArMapFileLineSetWriter origWriter(changeDetails->getChangedSummaryLines
                                  (ArMapChangeDetails::DELETIONS, scanType));

    ArUtil::functorPrintf(&origWriter, "%sResolution: %d%s", myResolution, 
                          getKeywordPrefix(),
			                    EOL_CHARS);
  }
	myResolution = resolution;

  if (changeDetails != NULL) {
    ArMapFileLineSetWriter newWriter(changeDetails->getChangedSummaryLines
                                  (ArMapChangeDetails::ADDITIONS, scanType));

    ArUtil::functorPrintf(&newWriter, "%sResolution: %d%s", myResolution, 
                          getKeywordPrefix(),
    			                EOL_CHARS);
  }
} // end method setResolution




AREXPORT void ArMapScan::writePointsToFunctor
		                         (ArFunctor2<int, std::vector<ArPose> *> *functor,
                              const char *scanType,
                              ArFunctor1<const char *> *keywordFunctor)
{
 
  if (keywordFunctor != NULL) {
    ArUtil::functorPrintf(keywordFunctor, "%s%s", 
                          getPointsKeyword(),
                          "");
  }
	functor->invoke(myNumPoints, &myPoints);

} // end method writePointsToFunctor


AREXPORT void ArMapScan::writeLinesToFunctor
	                           (ArFunctor2<int, std::vector<ArLineSegment> *> *functor,
                              const char *scanType,
                              ArFunctor1<const char *> *keywordFunctor)
{
  if (keywordFunctor != NULL) {
    ArUtil::functorPrintf(keywordFunctor, "%s%s", 
                          getLinesKeyword(),
                          "");
  }
	functor->invoke(myNumLines, &myLines);
} // end method writeLinesToFunctor


AREXPORT void ArMapScan::writeScanToFunctor
                             (ArFunctor1<const char *> *functor, 
			                        const char *endOfLineChars,
                              const char *scanType)
{
  if (!myDisplayString.empty()) {

    ArUtil::functorPrintf(functor, "%sDisplay: \"%s\"%s",
                          getKeywordPrefix(),
			                    myDisplayString.c_str(),  endOfLineChars);

  } // end if display specified

  if (myNumPoints != 0)
  {
    ArUtil::functorPrintf(functor, "%sMinPos: %.0f %.0f%s",
                          getKeywordPrefix(),
			                    myMin.getX(), myMin.getY(),  endOfLineChars);
    ArUtil::functorPrintf(functor, "%sMaxPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
			                    myMax.getX(), myMax.getY(),  endOfLineChars);
    ArUtil::functorPrintf(functor, "%sNumPoints: %d%s", 
                          getKeywordPrefix(),
			                    myNumPoints, endOfLineChars);
    ArUtil::functorPrintf(functor, "%sPointsAreSorted: %s%s", 
                          getKeywordPrefix(),
                          (myIsSortedPoints ? "true" : "false"), endOfLineChars);

  }

  if (myResolution != -1) {
    ArUtil::functorPrintf(functor, "%sResolution: %d%s", 
                          getKeywordPrefix(),
                          myResolution, endOfLineChars);
  }

  if (myNumLines != 0)
  {
    ArUtil::functorPrintf(functor, "%sLineMinPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
                          myLineMin.getX(), myLineMin.getY(),  endOfLineChars);
    ArUtil::functorPrintf(functor, "%sLineMaxPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
                          myLineMax.getX(), myLineMax.getY(),  endOfLineChars);
    ArUtil::functorPrintf(functor, "%sNumLines: %d%s", 
                          getKeywordPrefix(),
                          myNumLines, endOfLineChars);
    ArUtil::functorPrintf(functor, "%sLinesAreSorted: %s%s", 
                          getKeywordPrefix(),
                          (myIsSortedLines ? "true" : "false"), endOfLineChars);
  }


} // end method writeScanToFunctor


AREXPORT void ArMapScan::writePointsToFunctor
                                (ArFunctor1<const char *> *functor, 
			                           const char *endOfLineChars,
                                 const char *scanType)
{
  ArUtil::functorPrintf(functor, "%s%s", 
                        getPointsKeyword(),
                        endOfLineChars);

  if (myPoints.empty()) {
    return;
  }

  bool isFastWrite = 
    ((strcmp(endOfLineChars, "\n") == 0) &&
     ((myMin.getX() > INT_MIN) && (myMin.getX() < INT_MAX)) && 
     ((myMin.getY() > INT_MIN) && (myMin.getY() < INT_MAX)) && 
     ((myMax.getX() > INT_MIN) && (myMax.getX() < INT_MAX)) && 
     ((myMax.getY() > INT_MIN) && (myMax.getY() < INT_MAX)));

  if (isFastWrite) {

    // Write the map data points in text format....
    char buf[10000];
    
    for (std::vector<ArPose>::const_iterator pointIt = myPoints.begin(); 
         pointIt != myPoints.end();
         pointIt++)
    {
      // TODO Test the time of the long indicator...
      snprintf(buf, 10000, "%li %li\n", 
               (long int) (*pointIt).getX(), 
               (long int) (*pointIt).getY());
      functor->invoke(buf);
    } // end for each point
  } 
  else { // not fast write

    for (std::vector<ArPose>::const_iterator pointIt = myPoints.begin(); 
         pointIt != myPoints.end();
         pointIt++)
    {
      ArUtil::functorPrintf(functor, "%.0f %.0f%s", 
                                     (*pointIt).getX(), 
                                     (*pointIt).getY(), 
                                     endOfLineChars);

    }
  } // end else not fast write

} // end method writePointsToFunctor

AREXPORT void ArMapScan::writeLinesToFunctor
                                (ArFunctor1<const char *> *functor, 
                                 const char *endOfLineChars,
                                 const char *scanType)
{
  writeLinesToFunctor(functor, myLines, endOfLineChars, scanType);

} // end method writeLinesToFunctor


AREXPORT void ArMapScan::writeLinesToFunctor
                                (ArFunctor1<const char *> *functor, 
                                 const std::vector<ArLineSegment> &lines,
                                 const char *endOfLineChars,
                                 const char *scanType)
{
  if (lines.empty()) {
    return;
  }
    
  ArUtil::functorPrintf(functor, "%s%s", 
                        getLinesKeyword(), 
                        endOfLineChars);

  bool isFastWrite = 
    ((strcmp(endOfLineChars, "\n") == 0) &&
     ((myLineMin.getX() > INT_MIN) && (myLineMin.getX() < INT_MAX)) && 
     ((myLineMin.getY() > INT_MIN) && (myLineMin.getY() < INT_MAX)) && 
     ((myLineMax.getX() > INT_MIN) && (myLineMax.getX() < INT_MAX)) && 
     ((myLineMax.getY() > INT_MIN) && (myLineMax.getY() < INT_MAX)));

  if (isFastWrite) {

    // Write the map data points in text format....
    char buf[10000];

    for (std::vector<ArLineSegment>::const_iterator lineIt = lines.begin(); 
      lineIt != lines.end();
      lineIt++)
    {
      snprintf(buf, 10000, "%li %li %li %li\n", 
               (long int) (*lineIt).getX1(), 
               (long int) (*lineIt).getY1(),
               (long int) (*lineIt).getX2(), 
               (long int) (*lineIt).getY2());
      functor->invoke(buf);
    }
  }
  else { // slow write

    for (std::vector<ArLineSegment>::const_iterator lineIt = lines.begin(); 
         lineIt != lines.end();
         lineIt++)
    {
      ArUtil::functorPrintf(functor, "%.0f %.0f %.0f %.0f%s", 
                                    (*lineIt).getX1(), 
                                    (*lineIt).getY1(),
                                    (*lineIt).getX2(), 
                                    (*lineIt).getY2(),
                                    endOfLineChars);
    } // end for each line
  } // end else slow write

} // end method writeLinesToFunctor


bool ArMapScan::parseNumber(char *line, 
                            size_t lineLen, 
                            size_t *charCountOut,
                            int *numOut) const
{
  if (line == NULL) {
    return false;
  }

  bool isSuccess = true;
  size_t digitCount = 0;
  int num = 0;

  for (size_t i = 0; i < lineLen; i++) 
  {
    if ((isdigit(line[i])) || 
        ((i == 0) && (line[i] == '-'))) {
      digitCount++;
    }
    else {
      break;
    }
  } // end for each char in line

  // The less-than check should be okay since there should be a null-terminator
  if ((digitCount > 0) && (digitCount < (lineLen - 1))) {
    char origChar = line[digitCount];
    line[digitCount] = '\0';
  
    num = atoi(line);

    line[digitCount] = origChar;
  } 
  else { // no digits found
    isSuccess = false;
    digitCount = 0;
    num =  0;
  } // end else no digits found

  if (charCountOut != NULL) {
    *charCountOut = digitCount;
  }
  if (numOut != NULL) {
    *numOut = num;
  }

  return isSuccess;

} // end method parseNumber


bool ArMapScan::parseWhitespace(char *line,
                                size_t lineLen,
                                size_t *charCountOut) const
{
  if (line == NULL) {
    return false;
  }

  bool isSuccess = true;
  size_t wsCount = 0;

  for (size_t i = 0; i < lineLen; i++) 
  {
    if (isspace(line[i]) && (line[i] != '\0')) { 
      wsCount++;
    }
    else {
      break;
    }
  } // end for each char in line

  // The less-than check should be okay since there should be a null-terminator
  if ((wsCount > 0) && (wsCount < (lineLen - 1))) {
  } 
  else { // no digits found
    isSuccess = false;
    wsCount = 0;
  } // end else no digits found

  if (charCountOut != NULL) {
    *charCountOut = wsCount;
  }

  return isSuccess;
  return false;

} // end method parseWhitespace


AREXPORT bool ArMapScan::readDataPoint( char *line)
{
  if (line == NULL) {
    return false;
  }
  
  int x = 0;
  int y = 0;

  bool isSuccess  = true;
  size_t lineLen  = strlen(line) + 1;
  int startIndex  = 0;
  size_t parsedCount = 0;
 
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readDataPoint error parsing x (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readDataPoint error parsing first whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readDataPoint error parsing y (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  loadDataPoint(x, y);

  return true;

} // end method readDataPoint


AREXPORT bool ArMapScan::readLineSegment( char *line)
{
  if (line == NULL) {
    return false;
  }
  
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;

  bool isSuccess  = true;
  size_t lineLen     = strlen(line) + 1;
  int startIndex  = 0;
  size_t parsedCount = 0;
 
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x1);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing x1 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing first whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y1);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing y1 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing second whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x2);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing x2 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing third whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y2);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "%sArMapScan::readLineSegment error parsing y2 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  loadLineSegment(x1, y1, x2, y2);

  return true;

} // end method readLineSegment


AREXPORT void ArMapScan::loadDataPoint(double x, double y)
{
  if (x > myMax.getX())
    myMax.setX(x);
  if (y > myMax.getY())
    myMax.setY(y);
  
  if (x < myMin.getX())
    myMin.setX(x);
  if (y < myMin.getY())
    myMin.setY(y);
  
  myPoints.push_back(ArPose(x, y));
  
} // end method loadDataPoint


AREXPORT void ArMapScan::loadLineSegment(double x1, double y1, 
                                         double x2, double y2)
{
  if (x1 > myLineMax.getX())
    myLineMax.setX(x1);
  if (y1 > myLineMax.getY())
    myLineMax.setY(y1);
  
  if (x1 < myLineMin.getX())
    myLineMin.setX(x1);
  if (y1 < myLineMin.getY())
    myLineMin.setY(y1);

  if (x2 > myLineMax.getX())
    myLineMax.setX(x2);
  if (y2 > myLineMax.getY())
    myLineMax.setY(y2);
  
  if (x2 < myLineMin.getX())
    myLineMin.setX(x2);
  if (y2 < myLineMin.getY())
    myLineMin.setY(y2);
  
  myLines.push_back(ArLineSegment(x1, y1, x2, y2));

} // end method loadLineSegment


AREXPORT bool ArMapScan::unite(ArMapScan *other,
                               bool isIncludeDataPointsAndLines)
{
  if (other == NULL) {
    return false;
  }

  if ((myNumPoints > 0) || (myNumLines > 0)) {
    if (other->myTimeChanged.isAfter(myTimeChanged)) {
      myTimeChanged = other->myTimeChanged;
    }
    if (other->myResolution != -1) {
      // Not entirely sure whether it makes sense to use the largest 
      // resolution or the smallest...
      if (other->myResolution > myResolution) {
        myResolution = other->myResolution;
      }
    }
  }
  else {
    myTimeChanged = other->myTimeChanged;
    myResolution = other->myResolution;
  }

  if (myNumPoints > 0) {

    if (other->getMaxPose().getX() > myMax.getX()) {
      myMax.setX(other->getMaxPose().getX());
    }
    if (other->getMaxPose().getY() > myMax.getY()) {
      myMax.setY(other->getMaxPose().getY());
    }

    if (other->getMinPose().getX() < myMin.getX()) {
      myMin.setX(other->getMinPose().getX());
    }
    if (other->getMinPose().getY() < myMin.getY()) {
      myMin.setY(other->getMinPose().getY());
    }

    if (!other->isSortedPoints()) {
      myIsSortedPoints = false;
    }
  }
  else {
    myMax = other->getMaxPose();
    myMin = other->getMinPose();
  
    myIsSortedPoints = other->isSortedPoints();
  }
  myNumPoints += other->getNumPoints();

  if (myNumLines > 0) {

    if (other->getLineMaxPose().getX() > myLineMax.getX()) {
      myLineMax.setX(other->getLineMaxPose().getX());
    }
    if (other->getLineMaxPose().getY() > myLineMax.getY()) {
      myLineMax.setY(other->getLineMaxPose().getY());
    }

    if (other->getLineMinPose().getX() < myLineMin.getX()) {
      myLineMin.setX(other->getLineMinPose().getX());
    }
    if (other->getLineMinPose().getY() < myLineMin.getY()) {
      myLineMin.setY(other->getLineMinPose().getY());
    }
    if (!other->isSortedLines()) {
      myIsSortedLines = false;
    }
  }
  else {
    myLineMax = other->getLineMaxPose();
    myLineMin = other->getLineMinPose();
    
    myIsSortedLines = other->isSortedLines();
  }

  myNumLines += other->getNumLines();


  if (isIncludeDataPointsAndLines) {
   
    bool isPointsChanged = false;
    bool isLinesChanged = false;

    if (other->getPoints() != NULL) {
      myPoints.reserve(myNumPoints);
      for (std::vector<ArPose>::iterator iter = other->getPoints()->begin();
           iter != other->getPoints()->end();
           iter++) {
        myPoints.push_back(*iter);
        isPointsChanged = true;
      } // end for each other point
    } // end if other scan has points

    if (myIsSortedPoints) {
	    std::sort(myPoints.begin(), myPoints.end());
    }
    
    if (other->getLines() != NULL) {
      myLines.reserve(myNumLines);
      for (std::vector<ArLineSegment>::iterator iter = other->getLines()->begin();
           iter != other->getLines()->end();
           iter++) {
        myLines.push_back(*iter);
        isLinesChanged = true;
      } // end for each other line  
    } // end if other scan has lines 
    
    if (myIsSortedLines) {
	    std::sort(myLines.begin(), myLines.end());
    }

  } // end if include points and lines

  return true;

} // end method unite


bool ArMapScan::handleMinPos(ArArgumentBuilder *arg)
{
  return parsePose(arg, "MinPos:", &myMin);

} // end method handleMinPos


bool ArMapScan::handleMaxPos(ArArgumentBuilder *arg)
{
  return parsePose(arg, "MaxPos:", &myMax);
}

bool ArMapScan::handleNumPoints(ArArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {

    bool ok = false;
    int numPoints = arg->getArgInt(0, &ok);

    if (ok) {
      
      myNumPoints = numPoints;
      // myNumPoints = 0;

      ArLog::log(ArLog::Normal,
                 "%sArMapScan::handleNumPoints() set num points to %i",
                 myLogPrefix.c_str(),
                 numPoints);
  
      if (numPoints > 0) {
        // myPointText.reserve(myNumPoints);
        myPoints.reserve(numPoints);
      }
      else { // no points
        // Reset the min and max poses to have the same values that would result
        // from a call to setPoints with no points.  This is done primarily so 
        // that map change processing will behave correctly.
        myMax.setX(INT_MIN);
        myMax.setY(INT_MIN);
        myMin.setX(INT_MAX);
        myMin.setY(INT_MAX);
      } // end else no points

      return true;
    }
  } // end if enough args

  // If this is reached, then an error has occurred...  
  ArLog::log(ArLog::Terse, 
	           "%sArMapScan: '%sNumPoints:' bad argument, should be one integer (number of data points)",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  
  return false;

} // end method handleNumPoints


bool ArMapScan::handleIsSortedPoints(ArArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {
    bool ok = true;
    bool isSorted = arg->getArgBool(0, &ok);
    if (ok) {
      myIsSortedPoints = isSorted;
      return true;
    }
  } // end if correct arg count
    
  ArLog::log(ArLog::Terse, 
             "ArMapScan: 'PointsAreSorted:' bad arguments, should be a boolean");
  return false;

} // end method handleIsSortedPoints


bool ArMapScan::handleLineMinPos(ArArgumentBuilder *arg)
{
  return parsePose(arg, "LineMinPos:", &myLineMin);
}

bool ArMapScan::handleLineMaxPos(ArArgumentBuilder *arg)
{
  return parsePose(arg, "LineMaxPos:", &myLineMax);
}


bool ArMapScan::parsePose(ArArgumentBuilder *arg,
                          const char *keyword,
                          ArPose *poseOut) 
{
  if ((arg == NULL) || (keyword == NULL) || (poseOut == NULL)) {
    ArLog::log(ArLog::Normal,
               "ArMapScan::parsePose() invalid NULL parameters");
    return false;
  }

  if (arg->getArgc() == 2) {

    bool xOk = true;
    bool yOk = true;
    int x = arg->getArgInt(0, &xOk);
    int y = arg->getArgInt(1, &yOk);

    if (xOk && yOk) {
      poseOut->setPose(x, y);
      return true;
    }
  } // end if correct arg count

  ArLog::log(ArLog::Terse, 
 	           "%sArMapScan: '%s%s' bad arguments, should be two integers x y",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str(),
             keyword);

  return false;

} // end method parsePose


bool ArMapScan::handleNumLines(ArArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {

    bool ok = false;
    int numLines = arg->getArgInt(0, &ok);

    if (ok) {

      // Reset the myNumLines to 0, it is incremented as the actual lines are added
      myNumLines = numLines;

      if (numLines > 0) {
	      myLines.reserve(numLines);
      }
      else { // no lines
        // Reset the min and max poses to have the same values that would result
        // from a call to setLines with no lines.  This is done primarily so 
        // that map change processing will behave correctly.
        myLineMax.setX(INT_MIN);
        myLineMax.setY(INT_MIN);
        myLineMin.setX(INT_MAX);
        myLineMin.setY(INT_MAX);
      } // end else no lines

      return true;

    } // end if ok
  } // end if enough args

  // If this is reached, then an error has occurred...  
  ArLog::log(ArLog::Terse, 
	           "%sArMapScan: '%sNumLines:' bad argument, should be one integer (number of data points)",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  
  return false;
  
} // end method handleNumLines


bool ArMapScan::handleIsSortedLines(ArArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {
    bool ok = true;
    bool isSorted = arg->getArgBool(0, &ok);
    if (ok) {
      myIsSortedLines = isSorted;
      return true;
    }
  } // end if correct arg count
    
  ArLog::log(ArLog::Terse, 
 	           "%sArMapScan: '%sLinesAreSorted' bad arguments, should be a boolean",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  return false;

} // end method handleIsSortedLines



bool ArMapScan::handleResolution(ArArgumentBuilder *arg)
{
  if (arg->getArgc() == 1)
  {
    bool ok = true;
    int res = arg->getArgInt(0, &ok);
    if (ok) {
      myResolution = res;
      return true;
    }
  } // end if correct arg count

  ArLog::log(ArLog::Terse, 
	           "ArMapScan: 'Resolution:' bad argument, should be one integer (resolution in mm)");
  return false;

} // end method handleResolution


bool ArMapScan::handleDisplayString(ArArgumentBuilder *arg)
{
  arg->compressQuoted();

  if (arg->getArgc() >= 1) {

    const char *displayArg = arg->getArg(0);
    int displayBufferLen = strlen(displayArg) + 1;
    char *displayBuffer = new char[displayBufferLen];
  
     if (ArUtil::stripQuotes(displayBuffer, displayArg, displayBufferLen))
     {
        myDisplayString = displayBuffer;

        ArLog::log(ArLog::Normal, 
                  "%sArMapScan: '%sDisplay' setting display '%s'",
                  myLogPrefix.c_str(),
                  myKeywordPrefix.c_str(),
                  myDisplayString.c_str());
     }
     else {
      ArLog::log(ArLog::Terse, 
	                "%sArMapScan: '%sDisplay:' couldn't strip quotes from '%s'", 
                  myLogPrefix.c_str(),
                  myKeywordPrefix.c_str(),
	                displayArg);
     } // end if error stripping quotes

     delete [] displayBuffer;



    return true;
  }
  else {
    ArLog::log(ArLog::Terse, 
	            "%sArMapScan: '%sDisplay:' insufficient args '%s'", 
              myLogPrefix.c_str(),
              myKeywordPrefix.c_str(),
	            arg->getFullString());
    return false;
  
 }

} // end method handleDisplayString


bool ArMapScan::handlePoint(ArArgumentBuilder *arg)
{
  if (arg->getArgc() == 2) {

    bool xOk = true;
    bool yOk = true;

    int x = arg->getArgInt(0, &xOk);
    int y = arg->getArgInt(1, &yOk);

    if (xOk && yOk) {
      loadDataPoint(x, y);
      return true;
    }
  } // end if correct arg count

  ArLog::log(ArLog::Terse, 
	           "ArMapScan::handlePoint: map point wrong, should be x and y int coords (in mm) but is %s", 
             arg->getFullString());
    
  return false;

} // end method handlePoint


bool ArMapScan::handleLine(ArArgumentBuilder *arg)
{
 
  if (arg->getArgc() == 4) {

    bool x1Ok = true;
    bool y1Ok = true;
    bool x2Ok = true;
    bool y2Ok = true;

    int x1 = arg->getArgInt(0, &x1Ok);
    int y1 = arg->getArgInt(1, &y1Ok);
    int x2 = arg->getArgInt(2, &x2Ok);
    int y2 = arg->getArgInt(3, &y2Ok);

    if (x1Ok && y1Ok && x2Ok && y2Ok) {
      loadLineSegment(x1, y1, x2, y2);
      return true;
    }
  } // end if correct arg count

  ArLog::log(ArLog::Verbose, 
	            "ArMapScan::handleLine: line wrong, should be 2 x, y points (in mm) but is %s", 
              arg->getFullString());
  return false;

} // end method handleLine

  
bool ArMapScan::addHandlerToFileParser
                    (ArFileParser *fileParser,
                     const char *keyword,
                     ArRetFunctor1<bool, ArArgumentBuilder *> *handler)
{

  if ((fileParser == NULL) || (handler == NULL)) {
    ArLog::log(ArLog::Verbose,
               "ArMapScan::addHandlerToFileParser cannot add null parser / handler");
    return false;
  }
  bool isAdded = false;
 
  if (keyword != NULL) {
 
    std::string fullKeyword = getKeywordPrefix();
    fullKeyword += keyword;
    ArLog::log(ArLog::Verbose,
               "ArMapScan::addHandlerToFileParser adding handler for '%s' full = '%s'", keyword, fullKeyword.c_str());

    isAdded = fileParser->addHandler(fullKeyword.c_str(),
                                     handler);
  }
  else {
    ArLog::log(ArLog::Verbose,
               "ArMapScan::addHandlerToFileParser adding handler for %s", keyword);
    isAdded = fileParser->addHandler(NULL, handler);
  }
   ArLog::log(ArLog::Verbose,
               "ArMapScan::addHandlerToFileParser (%s) returns %i", myScanType.c_str(), isAdded);

  return isAdded;

} // end method addHandlerToFileParser
  
  
AREXPORT const char *ArMapScan::getScanType() const
{
  return myScanType.c_str();
}

AREXPORT const char *ArMapScan::getPointsKeyword() const
{
  return myPointsKeyword.c_str();
}

AREXPORT const char *ArMapScan::getLinesKeyword() const
{
  return myLinesKeyword.c_str();
}
  
const char *ArMapScan::getKeywordPrefix() const
{
  return myKeywordPrefix.c_str();
}


// ---------------------------------------------------------------------------- 
// ArMapObjects
// ---------------------------------------------------------------------------- 


const char *ArMapObjects::DEFAULT_KEYWORD = "Cairn:";


AREXPORT ArMapObjects::ArMapObjects(const char *keyword) :
  myTimeChanged(),
  myIsSortedObjects(false),
  myKeyword((keyword != NULL) ? keyword : DEFAULT_KEYWORD),
  myMapObjects(),
  myMapObjectCB(this, &ArMapObjects::handleMapObject)
{
}


AREXPORT ArMapObjects::ArMapObjects(const ArMapObjects &other) :
  myTimeChanged(other.myTimeChanged),
  myIsSortedObjects(other.myIsSortedObjects),
  myKeyword(other.myKeyword),
  myMapObjects(),
  myMapObjectCB(this, &ArMapObjects::handleMapObject)
{
  for (std::list<ArMapObject *>::const_iterator it = other.myMapObjects.begin(); 
       it != other.myMapObjects.end(); 
       it++)
  {
    myMapObjects.push_back(new ArMapObject(*(*it)));
  }

} // end copy ctor


AREXPORT ArMapObjects &ArMapObjects::operator=(const ArMapObjects &other)
{
  if (&other != this) {

    ArUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
    myMapObjects.clear();
  
    myTimeChanged = other.myTimeChanged;
    myIsSortedObjects = other.myIsSortedObjects;
  
    myKeyword = other.myKeyword;

    for (std::list<ArMapObject *>::const_iterator it = other.myMapObjects.begin(); 
         it != other.myMapObjects.end(); 
         it++)
    {
      myMapObjects.push_back(new ArMapObject(*(*it)));
    }
  }
  return *this;

} // end method operator=


AREXPORT ArMapObjects::~ArMapObjects()
{
  ArUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();
}


AREXPORT bool ArMapObjects::addToFileParser(ArFileParser *fileParser) 
{
  ArLog::log(ArLog::Verbose, "ArMapObjects::addToFileParser: called");

  if (fileParser == NULL) {
    ArLog::log(ArLog::Terse, "ArMapObjects::addToFileParser: could not add handlers to NULL parser");
    return false;
  }

  // make sure we can add all our handlers
  if (!fileParser->addHandler(myKeyword.c_str(), &myMapObjectCB))
  {
    ArLog::log(ArLog::Terse, "ArMapObjects::addToFileParser: could not add handlers");
    return false;
  }  

  return true;
  
} // end method addToFileParser


AREXPORT bool ArMapObjects::remFromFileParser(ArFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }

  fileParser->remHandler(&myMapObjectCB);

  return true;

} // end method remFromFileParser

  
AREXPORT ArTime ArMapObjects::getTimeChanged() const
{
  return myTimeChanged;
}


AREXPORT void ArMapObjects::clear() 
{
  myTimeChanged.setToNow();

  ArUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();

} // end method clear


AREXPORT ArMapObject *ArMapObjects::findFirstMapObject(const char *name, 
														                           const char *type,
                                                       bool isIncludeWithHeading)
{
  for (std::list<ArMapObject *>::iterator objIt = getMapObjects()->begin(); 
       objIt != getMapObjects()->end(); 
       objIt++)
  {
    ArMapObject* obj = (*objIt);
    if(obj == NULL)
      return NULL;
    // if we're searching any type or its the right type then check the name
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      if(name == NULL || strcasecmp(obj->getName(), name) == 0)
      {
        return obj;
      }
    }
  }

  // if we get down here we didn't find it
  return NULL;
} // end method findFirstMapObject


AREXPORT ArMapObject *ArMapObjects::findMapObject(const char *name, 
				                                          const char *type,
                                                  bool isIncludeWithHeading)
{
  std::list<ArMapObject *>::iterator objIt;
  ArMapObject* obj = NULL;

  for (objIt = getMapObjects()->begin(); 
       objIt != getMapObjects()->end(); 
       objIt++)
  {
    obj = (*objIt);
    if(obj == NULL)
      return NULL;
    // if we're searching any type or its the right type then check the name
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      if(name == NULL || strcasecmp(obj->getName(), name) == 0)
      {
	      return obj;
      }
    }
  }

  // if we get down here we didn't find it
  return NULL;
} // end method findMapObject


/**
   When the map changes the pointers will no longer be valid... this
   doesn't lock the map while finding, so if you are using it from
   somewhere other than mapChanged you need to lock it... its probably
   easiest to just use it from mapChanged though.

  @return Gets a list of pointers to all the map objects of a given *
 type... if none match the list will be empty.

   @param type The type of object to try to find... NULL means find
   any type
   @param isIncludeWithHeading also match "WithHeading" versions of @a type
   ("<i>type</i>WithHeading")
 **/
AREXPORT std::list<ArMapObject *> ArMapObjects::findMapObjectsOfType
                                                  (const char *type,
                                                   bool isIncludeWithHeading)
{
  std::list<ArMapObject *> ret;

  for (std::list<ArMapObject *>::iterator objIt = myMapObjects.begin(); 
       objIt != myMapObjects.end(); 
       objIt++)
  {
    ArMapObject* obj = (*objIt);
    if (obj == NULL)
      continue;
    // if we're searching any type or its the right type then add it
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      ret.push_back(obj);
    }
  }

  return ret;
} // end method findMapObjectsOfType

AREXPORT std::list<ArMapObject *> *ArMapObjects::getMapObjects(void)
{
  // Think this should be done in getMapObjects....
  if (!myIsSortedObjects) {
    sortMapObjects(&myMapObjects);
    myIsSortedObjects = true;
  }
  return &myMapObjects;

} // end method getMapObjects


void ArMapObjects::sortMapObjects(std::list<ArMapObject *> *mapObjects)
{
  ArMapObjectCompare compare;

  std::vector<ArMapObject *> tempObjects;
  for (std::list<ArMapObject *>::iterator iter1 = mapObjects->begin();
        iter1 != mapObjects->end();
        iter1++) {
    tempObjects.push_back(*iter1);    
  }
	std::sort(tempObjects.begin(), tempObjects.end(), compare);

  mapObjects->clear();
  for (std::vector<ArMapObject *>::iterator iter2 = tempObjects.begin();
        iter2 != tempObjects.end();
        iter2++) {
    mapObjects->push_back(*iter2);    
  }

} // end method sortMapObjects


AREXPORT void ArMapObjects::setMapObjects(const std::list<ArMapObject *> *mapObjects,
                                          bool isSortedObjects,
                                          ArMapChangeDetails *changeDetails) 
{

  myTimeChanged.setToNow();

  // Think this should be done in getMapObjects....
  if (!myIsSortedObjects) {
    sortMapObjects(&myMapObjects);
    myIsSortedObjects = true;
  }

  const std::list<ArMapObject *> *newMapObjects = mapObjects;
  std::list<ArMapObject *> *mapObjectsCopy = NULL;
  
  if ((mapObjects != NULL) &&
      (!isSortedObjects || (mapObjects == &myMapObjects))) {
    mapObjectsCopy =  new std::list<ArMapObject *>(*mapObjects);
	  sortMapObjects(mapObjectsCopy);
    newMapObjects = mapObjectsCopy;
  }
  

  ArMapFileLineSet origLines;

  if (changeDetails != NULL) {
    createMultiSet(&origLines);
  }


  std::list<ArMapObject *> origMapObjects = myMapObjects;
  //ArUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();
//  myMapObjectsChanged.setToNow();

  if (newMapObjects != NULL) {

    for (std::list<ArMapObject *>::const_iterator it = newMapObjects->begin(); 
        it != newMapObjects->end(); 
        it++)
    {
      ArMapObject *obj = *it;
      if (obj == NULL) {
        continue;
      }
      myMapObjects.push_back(new ArMapObject(*obj));
      origMapObjects.remove(obj);
    }
  }

  if (changeDetails != NULL) {

    ArMapFileLineSet newLines;
    createMultiSet(&newLines);

    bool isSuccess = ArMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedObjectLines
                                          (ArMapChangeDetails::DELETIONS),
                         changeDetails->getChangedObjectLines
                                          (ArMapChangeDetails::ADDITIONS),
                         false);

    if (!isSuccess) {
      ArLog::log(ArLog::Normal,
                 "ArMapObjects::setMapObjects() error calculating changes"); 
    }
  } // end if accumulate changes

  // Anything that is left in the original map objects list was not passed
  // in via the given map object list.  Delete all of the remaining objects.
  // The caller is responsible for deleting the objects in the given list.
  ArUtil::deleteSet(origMapObjects.begin(), origMapObjects.end());

  if (mapObjectsCopy != NULL) {
    delete mapObjectsCopy;
  }

} // end method setMapObjects


AREXPORT void ArMapObjects::writeObjectListToFunctor(ArFunctor1<const char *> *functor, 
		                                                 const char *endOfLineChars)
{
  // TODO: Ideally it would probably be nice to cache this string in the object...
  // Is this possible with the two different types of keywords?

  for (std::list<ArMapObject*>::iterator mapObjectIt = myMapObjects.begin(); 
       mapObjectIt != myMapObjects.end(); 
       mapObjectIt++)
  {
    ArMapObject *object = (*mapObjectIt);

    ArUtil::functorPrintf(functor, 
	                        "%s %s%s",
                          myKeyword.c_str(),
                          object->toString(),
	                        endOfLineChars);
 }
} // end method writeObjectListToFunctor


bool ArMapObjects::handleMapObject(ArArgumentBuilder *arg)
{
  ArMapObject *object = ArMapObject::createMapObject(arg);

  if (object == NULL) {
    return false;
  }
  myMapObjects.push_back(object);
//  object->log(myKeyword.c_str());
  //arg->log();
  return true;

} // end method handleMapObject




void ArMapObjects::createMultiSet(ArMapFileLineSet *multiSet)
{
  ArMapFileLineSetWriter origWriter(multiSet);

  writeObjectListToFunctor(&origWriter, "\n");

} // end method createMultiSet

void ArMapObjects::logMultiSet(const char *prefix,
                               ArMapFileLineSet *multiSet)
{
  if (prefix != NULL) {
    ArLog::log(ArLog::Normal,
               prefix);
  }
  if (multiSet == NULL) {
    ArLog::log(ArLog::Normal,
               "NULL");
    return;
  }

  for (ArMapFileLineSet::iterator mIter = multiSet->begin();
       mIter != multiSet->end();
       mIter++) {
    ArMapFileLineGroup &fileLine = *mIter;
    fileLine.log();
    /**
    ArLog::log(ArLog::Normal,
               "#%-3i : %s",
               fileLine.getLineNum(),
               fileLine.getLineText());
    **/
  }
} // end method logMultiSet


// ---------------------------------------------------------------------------- 
// ArMapInfo
// ---------------------------------------------------------------------------- 



void ArMapInfo::setDefaultInfoNames()
{
  myInfoTypeToNameMap[MAP_INFO]   = "MapInfo:";
  myInfoTypeToNameMap[MACRO_INFO] = "MacroInfo:";
  myInfoTypeToNameMap[META_INFO]  = "MetaInfo:";
  myInfoTypeToNameMap[TASK_INFO]  = "TaskInfo:";
  myInfoTypeToNameMap[ROUTE_INFO] = "RouteInfo:";
  myInfoTypeToNameMap[SCHED_TASK_INFO] = "SchedTaskInfo:";
  myInfoTypeToNameMap[SCHED_INFO] = "SchedInfo:";
  myInfoTypeToNameMap[CAIRN_INFO] = "CairnInfo:";
  myInfoTypeToNameMap[CUSTOM_INFO] = "CustomInfo:";

} // end method setDefaultInfoNames



ArMapInfo::ArMapInfoData::ArMapInfoData(ArMapInfo *parent,
                                        const char *keyword,
                                        int type) :
  myParent(parent),
  myType(type),
  myKeyword((keyword!= NULL) ? keyword : ""),
  myInfo(),
  myInfoCB(NULL)
{
  if (myParent != NULL) {
    myInfoCB = new ArRetFunctor1C<bool, ArMapInfo, ArArgumentBuilder *>
							                                (parent,
							                                 &ArMapInfo::handleInfo,
								                               NULL);
  }

}

ArMapInfo::ArMapInfoData::ArMapInfoData(ArMapInfo *parent,
                                        const ArMapInfoData &other) :
  myParent(parent),
  myType(other.myType),
  myKeyword(other.myKeyword),
  myInfo(),
  myInfoCB(NULL) // Don't copy callbacks
{
  if (myParent != NULL) {
    myInfoCB = new ArRetFunctor1C<bool, ArMapInfo, ArArgumentBuilder *>
							                                (parent,
							                                 &ArMapInfo::handleInfo,
								                               NULL);
  }
  for (std::list<ArArgumentBuilder *>::const_iterator iter = other.myInfo.begin();
        iter != other.myInfo.end();
        iter++) {
    ArArgumentBuilder *arg = *iter;
    if (arg == NULL) {
      continue;
    }
    myInfo.push_back(new ArArgumentBuilder(*arg));
  }
} // end pseudo-copy-constructor


ArMapInfo::ArMapInfoData &ArMapInfo::ArMapInfoData::operator=(const ArMapInfoData &other) 
{
  if (this != &other) {
    // Don't change parents
    myType = other.myType;
    myKeyword = other.myKeyword;

    ArUtil::deleteSet(myInfo.begin(), myInfo.end());
    myInfo.clear();

    for (std::list<ArArgumentBuilder *>::const_iterator iter = other.myInfo.begin();
         iter != other.myInfo.end();
         iter++) {
      ArArgumentBuilder *arg = *iter;
      if (arg == NULL) {
        continue;
      }
      myInfo.push_back(new ArArgumentBuilder(*arg));
    }
    // myInfoCB = other.myInfoCB;  // Don't copy callbacks
  }
  return *this;
}

ArMapInfo::ArMapInfoData::~ArMapInfoData()
{
  ArUtil::deleteSet(myInfo.begin(), myInfo.end());
  myInfo.clear();

  delete myInfoCB;
}




AREXPORT ArMapInfo::ArMapInfo(const char **infoNameList,
                              size_t infoNameCount,
                              const char *keywordPrefix) :
  ArMapInfoInterface(),
  myTimeChanged(),
  myNumInfos(0), 
  myPrefix((keywordPrefix != NULL) ? keywordPrefix : ""),
  myInfoTypeToNameMap(),
  myInfoNameToDataMap(),
  myKeywordToInfoNameMap()
{
  setDefaultInfoNames();

  if (infoNameList == NULL) {
   
    // TODO Someday the int types should disappear and this should just be a list of 
    // info names

    for (std::map<int, std::string>::iterator iter = myInfoTypeToNameMap.begin();
         iter != myInfoTypeToNameMap.end();
         iter++) {

      std::string dataName = myPrefix + iter->second.c_str();
     
      ArMapInfoData *data = new ArMapInfoData(this, dataName.c_str(), iter->first);
      myInfoNameToDataMap[iter->second] = data;
      myKeywordToInfoNameMap[dataName.c_str()] = iter->second;
      myNumInfos++;
    }
  }
  else { // info name list

    for (size_t i = 0; i < infoNameCount; i++) {
      const char *curName = infoNameList[i];
      if (ArUtil::isStrEmpty(curName)) {
        continue;
      }
      std::string dataName = myPrefix + curName;

      ArMapInfoData *data = new ArMapInfoData(this, dataName.c_str());
      myInfoNameToDataMap[curName] = data;
      myKeywordToInfoNameMap[dataName.c_str()] = curName;
      myNumInfos++;
    }
  } // end else info name list

} // end ctor



AREXPORT ArMapInfo::ArMapInfo(const ArMapInfo &other) :
  ArMapInfoInterface(),
  myTimeChanged(other.myTimeChanged),
  myNumInfos(other.myNumInfos), 
  myPrefix(other.myPrefix),
  myInfoTypeToNameMap(other.myInfoTypeToNameMap),
  myInfoNameToDataMap(),
  myKeywordToInfoNameMap(other.myKeywordToInfoNameMap)
{

  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::const_iterator iter = other.myInfoNameToDataMap.begin();
       iter != other.myInfoNameToDataMap.end();
       iter++) {
    const ArMapInfoData *otherData = iter->second;
    if (otherData == NULL) {
      continue;
    }
    myInfoNameToDataMap[iter->first] = new ArMapInfoData(this, *otherData);
  }
  
} // end copy ctor


AREXPORT ArMapInfo &ArMapInfo::operator=(const ArMapInfo &other)
{
  if (&other != this) {

    myTimeChanged = other.myTimeChanged;
    myNumInfos = other.myNumInfos;
    myPrefix = other.myPrefix;
    myInfoTypeToNameMap = other.myInfoTypeToNameMap;

    ArUtil::deleteSetPairs(myInfoNameToDataMap.begin(), myInfoNameToDataMap.end());
    myInfoNameToDataMap.clear();

    for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::const_iterator iter = other.myInfoNameToDataMap.begin();
         iter != other.myInfoNameToDataMap.end();
         iter++) {
      const ArMapInfoData *otherData = iter->second;
      if (otherData == NULL) {
        continue;
      }
      myInfoNameToDataMap[iter->first] = new ArMapInfoData(this, *otherData);
    }

    myKeywordToInfoNameMap = other.myKeywordToInfoNameMap;

  } // end if not this
  
  return *this;

} // end operator=


AREXPORT ArMapInfo::~ArMapInfo()
{
  ArUtil::deleteSetPairs(myInfoNameToDataMap.begin(), myInfoNameToDataMap.end());
  myInfoNameToDataMap.clear();

} // end dtor


AREXPORT bool ArMapInfo::addToFileParser(ArFileParser *fileParser)
{
 	ArLog::log(ArLog::Verbose, 
             "ArMapInfo::addToFileParser: called");

  if (fileParser == NULL) {
	  ArLog::log(ArLog::Terse, 
               "ArMapInfo::addToFileParser: could not add handlers to NULL fileParser");
    return false;
  }

  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {

    ArMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
	  if (!fileParser->addHandler(data->myKeyword.c_str(), 
									              data->myInfoCB)) {

 	    ArLog::log(ArLog::Terse, 
                 "ArMapInfo::addToFileParser: could not add handler for %s",
                 data->myKeyword.c_str());
                  
      return false;
   } // end if error adding handlers                              
  } // end for each info

  return true;

} // end method addToFileParser


AREXPORT bool ArMapInfo::remFromFileParser(ArFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }

  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {
    ArMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
	  fileParser->remHandler(data->myInfoCB);
  }
  return true;

} // end method remFromFileParser


AREXPORT ArTime ArMapInfo::getTimeChanged() const
{
  return myTimeChanged;
}

void ArMapInfo::setChanged()
{
  myTimeChanged.setToNow();
}

AREXPORT void ArMapInfo::clear()
{
  myTimeChanged.setToNow();

  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {
    ArMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
    ArUtil::deleteSet(data->myInfo.begin(), data->myInfo.end());
    data->myInfo.clear();

  } // end for each info

} // end method clear


AREXPORT std::list<ArArgumentBuilder *> *ArMapInfo::getInfo(int infoType)
{
  std::string infoName;
  std::map<int, std::string>::iterator iter1 = myInfoTypeToNameMap.find(infoType);
  if (iter1 != myInfoTypeToNameMap.end()) {
    infoName = iter1->second;
  }
  return getInfo(infoName.c_str());

}
AREXPORT std::list<ArArgumentBuilder *> *ArMapInfo::getInfo(const char *infoName)
{
  ArMapInfoData *data = findData(infoName);

  if (data != NULL) {
    return &data->myInfo;
  }
  else {
    return NULL;
  }

} // end method getInfo


AREXPORT std::list<ArArgumentBuilder *> *ArMapInfo::getMapInfo(void)
{
  return getInfo(MAP_INFO_NAME);
}

AREXPORT int ArMapInfo::getInfoCount() const
{
  return myNumInfos;
}

AREXPORT std::list<std::string> ArMapInfo::getInfoNames() const
{
  std::list<std::string> infoNames;
  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::const_iterator iter =
            myInfoNameToDataMap.begin();
       iter != myInfoNameToDataMap.end();
       iter++) {

    infoNames.push_back(iter->first);
  }
  return infoNames;
}


  
void ArMapInfo::createMultiSet(const char *infoName, 
                               ArMapFileLineSet *multiSet,
                               ArMapChangeDetails *changeDetails)
{
  ArMapFileLineSetWriter origWriter(multiSet);

  ArMapInfoData *data = findData(infoName);
  if (data == NULL) {
    return;
  }

  // TODO Will need to know position in order to propagate changes. ???
  for (std::list<ArArgumentBuilder *>::iterator infoIt = data->myInfo.begin();
       infoIt != data->myInfo.end();
       infoIt++) {

    ArArgumentBuilder *arg = *infoIt;
    if (arg == NULL) {
      continue;
    }
    bool isAddingChildren = false;
    if (changeDetails != NULL) {
      isAddingChildren = changeDetails->isChildArg(infoName, arg);
    }
    origWriter.setAddingChildren(isAddingChildren);

    ArUtil::functorPrintf(&origWriter, "%s %s%s", 
                          data->myKeyword.c_str(),
                          (*infoIt)->getFullString(), 
                          "");  // TODO: What to do about endOfLineChars

  } // end for each info in list

} // end method createMultiSet


AREXPORT ArMapInfo::ArMapInfoData *ArMapInfo::findData(const char *infoName)
{
  if (ArUtil::isStrEmpty(infoName)) {
    return NULL;
  }

  ArMapInfoData *data = NULL;
  std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::iterator iter1 = 
                                            myInfoNameToDataMap.find(infoName);

  if (iter1 != myInfoNameToDataMap.end()) {
    data = iter1->second;
  }

  return data;

} // end method findData

AREXPORT ArMapInfo::ArMapInfoData *ArMapInfo::findDataByKeyword(const char *keyword)
{
  if (ArUtil::isStrEmpty(keyword)) {
    return NULL;
  }
  std::map<std::string, std::string, ArStrCaseCmpOp>::iterator iter = myKeywordToInfoNameMap.find(keyword);
  if (iter != myKeywordToInfoNameMap.end()) {
    return findData(iter->second.c_str());
  }
  return NULL;

} // end method findDataByKeyword

AREXPORT bool ArMapInfo::setInfo(int infoType,
						                     const std::list<ArArgumentBuilder *> *infoList,
                                 ArMapChangeDetails *changeDetails)

{
  return setInfo(getInfoName(infoType), infoList, changeDetails);
}


AREXPORT bool ArMapInfo::setInfo(const char *infoName,
                                 const std::list<ArArgumentBuilder *> *infoList,
                                 ArMapChangeDetails *changeDetails)

{
  ArLog::log(ArLog::Verbose,
             "ArMapInfo::setInfo() %s", infoName);

  ArMapInfoData *data = findData(infoName);
  if (data == NULL) {
    return false;
  }

  
  ArMapFileLineSet origLines;

  if (changeDetails != NULL) {
    createMultiSet(infoName, &origLines, changeDetails);
  }

  // Make sure that the original list is not being passed back into this method.
  // TODO Fix this
  if (&(data->myInfo) == infoList) {
    ArLog::log(ArLog::Terse,
               "ArMapInfo::setInfo() cannot set to original list");
    return false;
  }

  ArUtil::deleteSet(data->myInfo.begin(), data->myInfo.end());
  data->myInfo.clear();

// TODO   myInfoChangedArray[infoType].setToNow();
  myTimeChanged.setToNow();

  if (infoList != NULL) {

    for (std::list<ArArgumentBuilder *>::const_iterator it = infoList->begin(); 
        it != infoList->end(); 
        it++)
    {
      data->myInfo.push_back(new ArArgumentBuilder(*(*it)));
    }
  }

  if (changeDetails != NULL) {

    ArMapFileLineSet newLines;
    createMultiSet(infoName, &newLines, changeDetails);

    bool isSuccess = ArMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedInfoLines
                                          (infoName,
                                           ArMapChangeDetails::DELETIONS),
                         changeDetails->getChangedInfoLines
                                          (infoName,
                                           ArMapChangeDetails::ADDITIONS));
    if (!isSuccess) {
      ArLog::log(ArLog::Normal,
                 "ArMapInfo::setInfo() error calculating changes");
    }
  } // end if changeDetails

  return true;

} // end method setInfo


AREXPORT bool ArMapInfo::setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                    ArMapChangeDetails *changeDetails)
{
  return setInfo(MAP_INFO_NAME, mapInfo, changeDetails);
}


AREXPORT void ArMapInfo::writeInfoToFunctor
				                     (ArFunctor1<const char *> *functor, 
			                        const char *endOfLineChars)
{

  // TODO: Write the info list?

  for (std::map<std::string, ArMapInfoData*, ArStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
       iter != myInfoNameToDataMap.end();
       iter++) {
    ArMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
    for (std::list<ArArgumentBuilder *>::iterator infoIt = data->myInfo.begin();
         infoIt != data->myInfo.end();
         infoIt++) {

      ArUtil::functorPrintf(functor, "%s %s%s", 
                            data->myKeyword.c_str(),
                            (*infoIt)->getFullString(), 
                            endOfLineChars);

      } // end for each info in list

  } // end for each info

} // end method writeInfoToFunctor


AREXPORT const char *ArMapInfo::getInfoName(int infoType)
{
  std::map<int, std::string>::iterator iter = myInfoTypeToNameMap.find(infoType);
  if (iter != myInfoTypeToNameMap.end()) {
    const std::string &name = iter->second;
    return name.c_str();
  }
  return NULL;

} // end method getInfoName


bool ArMapInfo::handleInfo(ArArgumentBuilder *arg)
{
  arg->compressQuoted();

  const char *keyword = arg->getExtraString();

  ArMapInfoData *data = findDataByKeyword(keyword);

  if (data == NULL) {
    ArLog::log(ArLog::Normal,
               "ArMapInfo::handleInfo cannot process %s",
               keyword);
    return false;
  }
  ArArgumentBuilder *infoBuilder = new ArArgumentBuilder(*arg);
  data->myInfo.push_back(infoBuilder);

  return true;

} // end method handleInfo


// -----------------------------------------------------------------------------

const char *ArMapSupplement::EOL_CHARS = "";

AREXPORT ArMapSupplement::ArMapSupplement() :
  myTimeChanged(),
  myHasOriginLatLongAlt(false),
  myOriginLatLong(),
  myOriginAltitude(0),
  myOriginLatLongAltCB(this, &ArMapSupplement::handleOriginLatLongAlt)
{
}

AREXPORT ArMapSupplement::ArMapSupplement(const ArMapSupplement &other) :
  myTimeChanged(other.myTimeChanged),
  myHasOriginLatLongAlt(other.myHasOriginLatLongAlt),
  myOriginLatLong(other.myOriginLatLong),
  myOriginAltitude(other.myOriginAltitude),

  myOriginLatLongAltCB(this, &ArMapSupplement::handleOriginLatLongAlt)
{
}


AREXPORT ArMapSupplement &ArMapSupplement::operator=(const ArMapSupplement &other) 
{
  if (&other != this) {
    myTimeChanged = other.myTimeChanged;
    myHasOriginLatLongAlt = other.myHasOriginLatLongAlt;
    myOriginLatLong = other.myOriginLatLong;
    myOriginAltitude = other.myOriginAltitude;
  }
  return *this;
}


AREXPORT ArMapSupplement::~ArMapSupplement()
{
}

AREXPORT bool ArMapSupplement::addToFileParser(ArFileParser *fileParser)
{
  ArLog::log(ArLog::Verbose, 
             "ArMapSupplement::addToFileParser: called");

  if (fileParser == NULL) {
    ArLog::log(ArLog::Verbose, 
               "ArMapSupplement::addToFileParser: could not add handlers to NULL fileParser");
    return false;
  }
  if (!fileParser->addHandler("OriginLatLongAlt:", &myOriginLatLongAltCB))
  {
    ArLog::log(ArLog::Terse, 
               "ArMapSupplement::addToFileParser: could not add handlers");
    return false;
  }  
  ArLog::log(ArLog::Verbose,
             "ArMapSupplement::addToFileParser() successfully added handlers");

  return true;

} // end method addToFileParser


AREXPORT bool ArMapSupplement::remFromFileParser(ArFileParser *fileParser)
{
 if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myOriginLatLongAltCB);
 
  return true;

} // end method remFromFileParser


AREXPORT ArTime ArMapSupplement::getTimeChanged() const
{
  return myTimeChanged;
}
  

AREXPORT void ArMapSupplement::clear()
{
  myTimeChanged.setToNow();

  myHasOriginLatLongAlt = false;
  myOriginLatLong.setPose(0, 0);
  myOriginAltitude = 0;

} // end method clear


/// Gets if this map has an OriginLatLong or not
AREXPORT bool ArMapSupplement::hasOriginLatLongAlt()
{
  return myHasOriginLatLongAlt;
}

/// Returns the latitude/longitude origin of the map; valid only if hasOriginLatLongAlt returns true
AREXPORT ArPose ArMapSupplement::getOriginLatLong()
{
  return myOriginLatLong;
}

/// Returns the altitude of the origin; valid only if hasOriginLatLongAlt returns true
AREXPORT double ArMapSupplement::getOriginAltitude()
{
  return myOriginAltitude;
}


AREXPORT void ArMapSupplement::setOriginLatLongAlt(bool hasOriginLatLongAlt,
                                             const ArPose &originLatLong,
                                             double altitude,
                                             ArMapChangeDetails *changeDetails)
{


  if ((myHasOriginLatLongAlt == hasOriginLatLongAlt) &&
      (myOriginLatLong == originLatLong) &&
      (fabs(myOriginAltitude - altitude) < ArMath::epsilon())) {
    return;
  }

  ArMapFileLineSet origLines;

  if (changeDetails != NULL) {
    ArMapFileLineSetWriter origWriter(changeDetails->getChangedSupplementLines
                                  (ArMapChangeDetails::DELETIONS));

    if (myHasOriginLatLongAlt) {

      char latLongBuf[512];
      // use printValue() to avoid localizing the floating point numbers (e.g. "," as decimal)
      ArUtil::printValue(latLongBuf, sizeof(latLongBuf),
                         "OriginLatLongAlt: %f %f %f",
                         myOriginLatLong.getX(), myOriginLatLong.getY(),
			                   myOriginAltitude);
      ArUtil::functorPrintf(&origWriter, "%s%s", 
			                      latLongBuf, EOL_CHARS);

         //ArUtil::functorPrintf(&origWriter, "OriginLatLongAlt: %f %f %f%s", 
			   //                   myOriginLatLong.getX(), myOriginLatLong.getY(),
			   //                   myOriginAltitude, EOL_CHARS);
    }
    else {
      ArUtil::functorPrintf(&origWriter, "OriginLatLongAlt:%s", 
			                      EOL_CHARS);
    }
  }
	myHasOriginLatLongAlt = hasOriginLatLongAlt;
  myOriginLatLong = originLatLong;
  myOriginAltitude = altitude;

  if (changeDetails != NULL) {
    ArMapFileLineSetWriter newWriter(changeDetails->getChangedSupplementLines
                                  (ArMapChangeDetails::ADDITIONS));

    if (myHasOriginLatLongAlt) {
      char latLongBuf[512];
      // use printValue() to avoid localizing the floating point numbers (e.g. "," as decimal)
      ArUtil::printValue(latLongBuf, sizeof(latLongBuf),
                         "OriginLatLongAlt: %f %f %f",
                         myOriginLatLong.getX(), myOriginLatLong.getY(),
			                   myOriginAltitude);
      ArUtil::functorPrintf(&newWriter, "%s%s", 
			                      latLongBuf, EOL_CHARS);


      //ArUtil::functorPrintf(&newWriter, "OriginLatLongAlt: %f %f %f%s", 
			   //                   myOriginLatLong.getX(), myOriginLatLong.getY(),
			   //                   myOriginAltitude, EOL_CHARS);
    }
    else { 
      ArUtil::functorPrintf(&newWriter, "OriginLatLongAlt:%s", 
			                      EOL_CHARS);

    }
  }
}


AREXPORT void ArMapSupplement::writeSupplementToFunctor
                              (ArFunctor1<const char *> *functor, 
			                         const char *endOfLineChars)
{
 
  if (myHasOriginLatLongAlt) {

    char latLongBuf[512];
    // use printValue() to avoid localizing the floating point numbers (e.g. "," as decimal)
    ArUtil::printValue(latLongBuf, sizeof(latLongBuf),
                        "OriginLatLongAlt: %f %f %f",
                        myOriginLatLong.getX(), myOriginLatLong.getY(),
			                  myOriginAltitude);
    ArUtil::functorPrintf(functor, "%s%s", 
			                    latLongBuf, endOfLineChars);

   /* ArUtil::functorPrintf(functor, "OriginLatLongAlt: %f %f %f%s", 
                          myOriginLatLong.getX(), myOriginLatLong.getY(),
                          myOriginAltitude, endOfLineChars);*/
  }

} // end method writeSupplementToFunctor


bool ArMapSupplement::handleOriginLatLongAlt(ArArgumentBuilder *arg)
{
  if (arg->getArgc() >= 3) {
 
    bool xOk   = true;
    bool yOk   = true;
    bool altOk = true;

    double x = arg->getArgDouble(0, &xOk);
    double y = arg->getArgDouble(1, &yOk);
    double alt = arg->getArgDouble(2, &altOk);

    if (xOk && yOk && altOk) {
      myHasOriginLatLongAlt = true;
      myOriginLatLong.setPose(x, y);
      myOriginAltitude = alt;
      return true;    
    }
  }
  else if (arg->getArgc() == 0) {

    // This is to handle the special and unlikely change case above -- where 
    // the map originally has an origin lat/long, but then is changed to not have
    // one
    myHasOriginLatLongAlt = false;
    myOriginLatLong.setPose(0,0);
    myOriginAltitude = 0;
    return true;

  }

  ArLog::log(ArLog::Verbose, 
	           "ArMap::handleOriginLatLongAlt: line wrong, should be x, y, altitude point (in lat long altitude) as doulbles but is %s", arg->getFullString());

  return false;

} // end method handleOriginLatLongAlt
  



// ---------------------------------------------------------------------------- 
// ---------------------------------------------------------------------------- 
// -----------------------------------------------------------------------------
// ArMapSimple
// -----------------------------------------------------------------------------

// TODO: Should these constants be somewhere else?
/*
const char *ArMapSimple::ourDefaultInactiveInfoNames[INFO_COUNT] =
{ 
  "_MapInfo:",
  "_MetaInfo:",
  "_TaskInfo:",
  "_RouteInfo:",
  "_SchedTaskInfo:",
  "_SchedInfo:",
  "_CairnInfo:",
  "_CustomInfo:"
};
*/

int ArMapSimple::ourTempFileNumber = 0;

ArMutex ArMapSimple::ourTempFileNumberMutex;

AREXPORT int ArMapSimple::getNextFileNumber()
{
  ourTempFileNumberMutex.lock();
  ourTempFileNumber++;
  int ret = ourTempFileNumber;
  ourTempFileNumberMutex.unlock();
  
  return ret;

} // end method getNextFileNumber
  
AREXPORT void ArMapSimple::invokeCallbackList(std::list<ArFunctor*> *cbList)
{
  if (cbList == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::invokeCallbackList failed because list is null");
    return;
  }
  for (std::list<ArFunctor*>::iterator iter = cbList->begin();
       iter != cbList->end();
       iter++) {
    ArFunctor *cb = *iter;
    if (cb == NULL) {
      continue;
    }
    cb->invoke();
  } 
} // end method invokeCallbackList

AREXPORT void ArMapSimple::invokeCallbackList(std::list<ArFunctor1<std::string>*> *cbList, std::string mapFormat)
{
  if (cbList == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::invokeCallbackList failed because list is null");
    return;
  }
  for (std::list<ArFunctor1<std::string>*>::iterator iter = cbList->begin();
       iter != cbList->end();
       iter++) {
    ArFunctor1<std::string> *cb = *iter;
    if (cb == NULL) {
      continue;
    }
    cb->invoke(mapFormat);
  }
} // end method invokeCallbackList

AREXPORT void ArMapSimple::addToCallbackList(ArFunctor *functor,
                                             ArListPos::Pos position,
                                             std::list<ArFunctor*> *cbList)
{
  if (functor == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::addToCallbackList cannot add null functor");
    return;
  }
  if (cbList == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::addToCallbackList cannot add functor to null list");
    return;
  }

  switch (position) {
  case ArListPos::FIRST:
    cbList->push_front(functor);
    break;
  case ArListPos::LAST:
    cbList->push_back(functor);
    break;
  default:
    ArLog::log(ArLog::Terse,
               "ArMapSimple::addToCallbackList invalid position (%i)",
               position);
  } // end switch
} // end method addToCallbackList

AREXPORT void ArMapSimple::remFromCallbackList(ArFunctor *functor,
                                               std::list<ArFunctor*> *cbList)
{
  if (functor == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::remFromCallbackList cannot remove null functor");
    return;
  }
  if (cbList == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::addToCallbackList cannot remove functor to null list");
    return;
  }
  cbList->remove(functor);

} // end method remFromCallbackList

AREXPORT ArMapSimple::ArMapSimple(const char *baseDirectory,
                                  const char *tempDirectory,
                                  const char *overrideMutexName)  :
  myMutex(),
  
  myMapCategoryList(),
  myMapCategory(),

  myChecksumCalculator(new ArMD5Calculator()),

  myBaseDirectory((baseDirectory != NULL) ? baseDirectory : ""),
  myFileName(),
  myReadFileStat(),

  myPreWriteCBList(),  
  myPostWriteCBList(),

  myIsWriteToTempFile(tempDirectory != NULL),
  myTempDirectory((tempDirectory != NULL) ? tempDirectory : ""),

  myMapId(),

  myLoadingParser(NULL),

  myIgnoreEmptyFileName(false), // ignoreEmptyFileName),
  myIgnoreCase(false),

  myMapChangedHelper(new ArMapChangedHelper()),

  myLoadingGotMapCategory(false),
  myLoadingDataStarted(false),
  myLoadingLinesAndDataStarted(false),

  myMapInfo(new ArMapInfo()),
  myMapObjects(new ArMapObjects()),
  myMapSupplement(new ArMapSupplement()),

  myScanTypeList(),
  myTypeToScanMap(),
  mySummaryScan(NULL),
  
  myLoadingDataTag(),
  myLoadingScan(NULL),

  // Use special keywords for the inactive elements.
  myInactiveInfo(new ArMapInfo(NULL, 0, "_")), 

  myInactiveObjects(new ArMapObjects("_Cairn:")), 

  myChildObjects(new ArMapObjects("ChildCairn:")), 

  myMapObjectNameToParamsMap(), 
  myRemainderList(),

  myTimeMapInfoChanged(),
  myTimeMapObjectsChanged(),
  myTimeMapScanChanged(),
  myTimeMapSupplementChanged(),

  myMapCategoryCB(this, &ArMapSimple::handleMapCategory),
  mySourcesCB(this, &ArMapSimple::handleSources),
  myDataIntroCB(this, &ArMapSimple::handleDataIntro),
  myRemCB(this, &ArMapSimple::handleRemainder),


  myIsQuiet(false),
  myIsReadInProgress(false),
  myIsCancelRead(false)

{
  if (overrideMutexName == NULL) {
    myMutex.setLogName("ArMapSimple::myMutex");
  }
  else {
    myMutex.setLogName(overrideMutexName);
    //myMutex.setLog(true);
  }

  ArUtil::appendSlash(myTempDirectory);
  ArUtil::fixSlashes(myTempDirectory);

  // Note that the categories must be listed in "increasing" order.
  myMapCategoryList.push_back(MAP_CATEGORY_2D);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_MULTI_SOURCES);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_EXTENDED);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_COMPOSITE);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_TEMPLATE);

  myMapCategory = MAP_CATEGORY_2D;

  // Create the default scan for the sick laser.  
  ArMapScan *mapScan = new ArMapScan(ARMAP_DEFAULT_SCAN_TYPE);
  // TODO This needs to be a constant!!
  myScanTypeList.push_back(ARMAP_DEFAULT_SCAN_TYPE);
  myTypeToScanMap[ARMAP_DEFAULT_SCAN_TYPE] = mapScan;

  /***
  std::list<std::string> inactiveInfoNames = createInactiveInfoNames
                                                (myMapInfo->getInfoNames());
  myInactiveInfo->resetInfoNames(inactiveInfoNames);
  ***/

  reset();
}  

AREXPORT ArMapSimple::ArMapSimple(const ArMapSimple &other) :
  myMutex(),
  myMapCategoryList(other.myMapCategoryList),
  myMapCategory(other.myMapCategory),
  myChecksumCalculator((other.myChecksumCalculator != NULL) ? 
                      new ArMD5Calculator() :
                      NULL),

  myBaseDirectory(other.myBaseDirectory),
  myFileName(other.myFileName),
  myReadFileStat(other.myReadFileStat),

  myPreWriteCBList(),  // Do not copy the callbacks...
  myPostWriteCBList(), 
  myIsWriteToTempFile(other.myIsWriteToTempFile),
  myTempDirectory(other.myTempDirectory),

  myMapId(other.myMapId),
  myLoadingParser(NULL), // TODO


  //myConfigParam(other.myConfigParam),
  myIgnoreEmptyFileName(other.myIgnoreEmptyFileName),
  myIgnoreCase(other.myIgnoreCase),

  myMapChangedHelper(new ArMapChangedHelper()), // Do not want to copy the other one

  // things for our config
  //myConfigProcessedBefore(other.myConfigProcessedBefore),
  //myConfigMapName(),

  myLoadingGotMapCategory(other.myLoadingGotMapCategory), 
  myLoadingDataStarted(other.myLoadingDataStarted), 
  myLoadingLinesAndDataStarted(other.myLoadingLinesAndDataStarted), 

  myMapInfo(new ArMapInfo(*other.myMapInfo)),
  myMapObjects(new ArMapObjects(*other.myMapObjects)),
  myMapSupplement(new ArMapSupplement(*other.myMapSupplement)),
  myScanTypeList(other.myScanTypeList),
  myTypeToScanMap(),
  mySummaryScan(NULL),

  // TODO Need to set this 
  myLoadingDataTag(),
  myLoadingScan(NULL),

  myInactiveInfo(new ArMapInfo(*other.myInactiveInfo)),
  myInactiveObjects(new ArMapObjects(*other.myInactiveObjects)),

  myChildObjects(new ArMapObjects(*other.myChildObjects)),

  myMapObjectNameToParamsMap(), // since this is a cache, ok not to copy
  myRemainderList(),

  myTimeMapInfoChanged(other.myTimeMapInfoChanged),  // TODO Or now??
  myTimeMapObjectsChanged(other.myTimeMapObjectsChanged), 
  myTimeMapScanChanged(other.myTimeMapScanChanged), 
  myTimeMapSupplementChanged(other.myTimeMapSupplementChanged),

  // callbacks
  myMapCategoryCB(this, &ArMapSimple::handleMapCategory),
  mySourcesCB(this, &ArMapSimple::handleSources),
  myDataIntroCB(this, &ArMapSimple::handleDataIntro),
  myRemCB(this, &ArMapSimple::handleRemainder),

  myIsQuiet(false),
  myIsReadInProgress(false),
  myIsCancelRead(false)
{
  myMapId.log("ArMapSimple::copy_ctor");

  myMutex.setLogName("ArMapSimple::myMutex");
  

  for (ArTypeToScanMap::const_iterator iter = 
            other.myTypeToScanMap.begin();
       iter != other.myTypeToScanMap.end();
       iter++) {
    myTypeToScanMap[iter->first] = new ArMapScan(*(iter->second));
  }

  if (other.mySummaryScan != NULL) {
    mySummaryScan = new ArMapScan(*other.mySummaryScan);
  }  // end if other has summary

 //strncpy(myConfigMapName, other.myConfigMapName, MAX_MAP_NAME_LENGTH);
 // myConfigMapName[MAX_MAP_NAME_LENGTH - 1] = '\0';

  reset();

} // end copy ctor


AREXPORT ArMapSimple &ArMapSimple::operator=(const ArMapSimple &other)
{
  if (&other != this) {

    // TODO: Change this to figure out deltas if desired...
    // (or add another method that would assign with delta compute)

    lock();

    myMapCategoryList = other.myMapCategoryList;
    myMapCategory     = other.myMapCategory;

    delete myChecksumCalculator;
    myChecksumCalculator = NULL;
    if (other.myChecksumCalculator != NULL) {
      myChecksumCalculator = new ArMD5Calculator();
    }

    myBaseDirectory     = other.myBaseDirectory;
    myFileName          = other.myFileName;
    myReadFileStat      = other.myReadFileStat;
  
    // myPreWriteCBList(),  // Do not overwrite the callbacks...?
    // myPostWriteCBList(), 

    myIsWriteToTempFile = other.myIsWriteToTempFile;
    myTempDirectory     = other.myTempDirectory;

    myMapId = other.myMapId;

    myMapId.log("ArMapSimple::operator=");
    //myLoadingParser = NULL; // TODO

    //myConfigParam = other.myConfigParam;
    myIgnoreEmptyFileName = other.myIgnoreEmptyFileName;
    myIgnoreCase = other.myIgnoreCase;

    // Do not copy the map changed helper

    // things for our config
    //myConfigProcessedBefore = other.myConfigProcessedBefore;
    //strncpy(myConfigMapName, other.myConfigMapName, MAX_MAP_NAME_LENGTH);
    //myConfigMapName[MAX_MAP_NAME_LENGTH - 1] = '\0';

    myLoadingGotMapCategory = other.myLoadingGotMapCategory; 
    myLoadingDataStarted = other.myLoadingDataStarted; 
    myLoadingLinesAndDataStarted = other.myLoadingLinesAndDataStarted; 

    *myMapInfo = *other.myMapInfo;
    *myMapObjects = *other.myMapObjects;
    *myMapSupplement = *other.myMapSupplement;

    delete mySummaryScan;
    mySummaryScan = NULL;

    myScanTypeList = other.myScanTypeList;
    
    ArUtil::deleteSetPairs(myTypeToScanMap.begin(), myTypeToScanMap.end());
    myTypeToScanMap.clear();

    for (ArTypeToScanMap::const_iterator iter = 
              other.myTypeToScanMap.begin();
         iter != other.myTypeToScanMap.end();
         iter++) {
      myTypeToScanMap[iter->first] = new ArMapScan(*(iter->second));
    }
    if (other.mySummaryScan != NULL) {
      mySummaryScan = new ArMapScan(*other.mySummaryScan);
    }  // end if other has summary

    myLoadingDataTag = other.myLoadingDataTag;
    myLoadingScan = NULL;
    if (other.myLoadingScan != NULL) {
      ArTypeToScanMap::iterator scanIter = 
        myTypeToScanMap.find(other.myLoadingScan->getScanType());
      if (scanIter != myTypeToScanMap.end()) {
        myLoadingScan = scanIter->second;
      }
    }

    *myInactiveInfo = *other.myInactiveInfo;
    *myInactiveObjects = *other.myInactiveObjects;
 
    *myChildObjects = *other.myChildObjects;

    // Since the myMapObjectNameToParamsMap is a cache, there's no
    // real need to copy the other one
    ArUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                           myMapObjectNameToParamsMap.end());
    myMapObjectNameToParamsMap.clear();

    ArUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
    myRemainderList.clear();
    for (std::list<ArArgumentBuilder *>::const_iterator remIter = other.myRemainderList.begin();
         remIter != other.myRemainderList.end();
         remIter++) {
      ArArgumentBuilder *arg = *remIter;
      if (arg == NULL) {
        continue; // Should never happen
      }
      myRemainderList.push_back(new ArArgumentBuilder(*arg));
    }

    // The various time flags represent the last time that mapChanged()
    // was invoked.  Since it has not yet been run for this instance of
    // the map, DO NOT update flags. (Doing so may prevent the updated 
    // map from being downloaded to client MobileEyes apps.)
    //
    // myTimeMapInfoChanged = other.myTimeMapInfoChanged; 
    // myTimeMapObjectsChanged = other.myTimeMapObjectsChanged; 
    // myTimeMapScanChanged = other.myTimeMapScanChanged; 
    // myTimeMapSupplementChanged = other.myTimeMapSupplementChanged;
    
    myIsQuiet = other.myIsQuiet; 
    myIsReadInProgress = other.myIsReadInProgress;
    myIsCancelRead = other.myIsCancelRead;

    // Primarily to get the new base directory into the file parser
    reset(); 

    // Think its best if the caller invokes mapChanged as necessary.
    // mapChanged();

    unlock();

  }

  return *this;

} // end operator=


AREXPORT ArMapSimple::~ArMapSimple(void)
{ 

  if (myIsReadInProgress) {

    ArLog::log(ArLog::Normal,
               "ArMapSimple::dtor() map file is being read");
    myIsCancelRead = true;
    if (myLoadingParser != NULL) {
      myLoadingParser->cancelParsing();
    }

    // Wait a little while to see if the file read can be cancelled
    for (int i = 0; ((i < 20) && (myIsReadInProgress)); i++) {
      ArUtil::sleep(5);
    }
    if (myIsReadInProgress) {
      ArLog::log(ArLog::Normal,
                 "ArMapSimple::dtor() map file is still being read");
    }

  } // end if read in progress

  delete myChecksumCalculator;
  myChecksumCalculator = NULL;

  delete myLoadingParser;
  myLoadingParser = NULL;

  delete myMapChangedHelper;
  myMapChangedHelper = NULL;

  delete myMapInfo;
  // const, so don't myMapInfo = NULL;

  delete myMapObjects;
  // const, so don't myMapObjects = NULL;

  delete myMapSupplement;
  // const, so don't myMapSupplement = NULL;
 
  myScanTypeList.clear();
  ArUtil::deleteSetPairs(myTypeToScanMap.begin(),
                         myTypeToScanMap.end());
  myTypeToScanMap.clear();


  delete mySummaryScan;
  mySummaryScan = NULL;

  // This is a reference to one of the scans deleted above, so just
  // clear the pointer.
  myLoadingScan = NULL;

  delete myInactiveInfo;
  // const, so don't myInactiveInfo = NULL;

  delete myInactiveObjects;
  // const, so don't myInactiveObjects = NULL;

  delete myChildObjects;
  // const, so don't myChildObjects = NULL;

  ArUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();

  ArUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();
        
} // end dtor 
  

AREXPORT ArMapInterface *ArMapSimple::clone()
{
  return new ArMapSimple(*this);
}

AREXPORT bool ArMapSimple::set(ArMapInterface *other)
{
  if (other == NULL) {
    return false;
  }
  if (getInfoCount() != other->getInfoCount()) {
    return false;
  }

  lock();

  myBaseDirectory = ((other->getBaseDirectory() != NULL) ?
                          other->getBaseDirectory() : "");

  myFileName = ((other->getFileName() != NULL) ?
                          other->getFileName() : "");

  myReadFileStat = other->getReadFileStat();

  myIsWriteToTempFile = (other->getTempDirectory() != NULL);
  myTempDirectory   = ((other->getTempDirectory() != NULL) ?
                                   other->getTempDirectory() : "");

  other->getMapId(&myMapId);

  myMapId.log("ArMapSimple::set");
  
  myLoadingLinesAndDataStarted = 
                          other->isLoadingLinesAndDataStarted();
  myLoadingDataStarted = 
                          other->isLoadingDataStarted();


  std::list<std::string> otherInfoNames = other->getInfoNames();
  for (std::list<std::string>::const_iterator infoIter = 
                                                  otherInfoNames.begin();
       infoIter != otherInfoNames.end();
       infoIter++) {
    const char *infoName = (*infoIter).c_str();
    setInfo(infoName, other->getInfo(infoName));
  }
  // int infoCount = other->getInfoCount();
  //for (int i = 0; i < infoCount; i++) {
  //  setInfo(i, other->getInfo(i));
  //} // end for each info type
 
  setMapObjects(other->getMapObjects());

  createScans((other->getScanTypes()));


  for (std::list<std::string>::iterator tIter1 = myScanTypeList.begin();
       tIter1 != myScanTypeList.end();
       tIter1++) {
    const char *scanType = (*tIter1).c_str();

    setPoints(other->getPoints(scanType), scanType, other->isSortedPoints());
    setLines(other->getLines(scanType), scanType, other->isSortedLines());
    setResolution(other->getResolution(scanType), scanType);

  } // end for each scan type

  setOriginLatLongAlt(other->hasOriginLatLongAlt(), 
                      other->getOriginLatLong(),
                      other->getOriginAltitude());


  // Not sure about the implications of doing this...
  for (std::list<std::string>::const_iterator infoIter2 = 
                                                  otherInfoNames.begin();
       infoIter2 != otherInfoNames.end();
       infoIter2++) {
    const char *infoName = (*infoIter2).c_str();
    setInactiveInfo(infoName, other->getInactiveInfo()->getInfo(infoName));
  }

  setInactiveObjects(other->getInactiveObjects()->getMapObjects());

  setChildObjects(other->getChildObjects()->getMapObjects());

  updateSummaryScan();
        
  // Since the myMapObjectNameToParamsMap is a cache, there's no
  // real need to copy the other one
  ArUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();


  ArUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();

  std::list<ArArgumentBuilder *> *otherRemainderList = other->getRemainder();

  if (otherRemainderList != NULL) {
    for (std::list<ArArgumentBuilder *>::const_iterator remIter = otherRemainderList->begin();
          remIter != otherRemainderList->end();
          remIter++) {
      ArArgumentBuilder *arg = *remIter;
      if (arg == NULL) {
        continue; // Should never happen
      }
      myRemainderList.push_back(new ArArgumentBuilder(*arg));
    } // end for each remainder
  } // end if remainder list not null

  updateMapCategory();

  reset();

  unlock();

  return true;

} // end method set 



AREXPORT void ArMapSimple::clear()
{
  lock();  // ???

  myFileName = "";
  myMapId = ArMapId();
  myMapId.log("ArMapSimple::clear");

  myMapInfo->clear();
  myMapObjects->clear();
  myMapSupplement->clear();
 
  for (ArTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    ArMapScan *scan = iter->second;
    if (scan != NULL) {
      scan->clear();
    }
  } // end for each scan type

  myInactiveInfo->clear();
  myInactiveObjects->clear();
  myChildObjects->clear();

  ArUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();

  ArUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();

  reset(); 

  mapChanged(); //???

  unlock();

} // end method clear
  
AREXPORT std::list<std::string> ArMapSimple::getScanTypes() const
{
  return myScanTypeList;
}

AREXPORT bool ArMapSimple::setScanTypes(const std::list<std::string> &scanTypeList)
{
  return createScans(scanTypeList);

} // end method setScanTypes


AREXPORT struct stat ArMapSimple::getReadFileStat() const
{
  return myReadFileStat;
}

AREXPORT void ArMapSimple::reset()
{

  myLoadingGotMapCategory = false; 
  myLoadingDataStarted = false; 
  myLoadingLinesAndDataStarted = false; 

  /// HERE ///

  if (myLoadingParser != NULL) {
    delete myLoadingParser;
    myLoadingParser = NULL;
  }
  myLoadingParser = new ArFileParser("./",  // base directory
                                     true); // precompress quotes


  myLoadingParser->setBaseDirectory(myBaseDirectory.c_str());

  // The map file cannot contain any comments.
  myLoadingParser->clearCommentDelimiters();

  myLoadingParser->setQuiet(myIsQuiet);

/// END HERE ////

  for (std::list<std::string>::iterator iter = myMapCategoryList.begin();
       iter != myMapCategoryList.end();
       iter++) {

    if (!myLoadingParser->addHandler((*iter).c_str(), &myMapCategoryCB)) {
      ArLog::log(ArLog::Terse, 
                 "ArMapSimple::reset() could not add map category handler for %s",
                 (*iter).c_str());
    }
  } // end for each map category

} // end method reset
  
  
AREXPORT bool ArMapSimple::refresh()
{
  ArLog::log(ArLog::Terse, 
             "ArMapSimple::refresh() not implemented");
  return true;
}

AREXPORT void ArMapSimple::updateMapFileInfo(const char *realFileName)
{
  ArUtil::filestat(realFileName, &myReadFileStat);

  if (myChecksumCalculator != NULL) {

    myMapId = ArMapId(myMapId.getSourceName(),
                      myMapId.getFileName(),
                      myChecksumCalculator->getDigest(),
                      ArMD5Calculator::DIGEST_LENGTH,
                      myReadFileStat.st_size,
                      myReadFileStat.st_mtime);

    // TODO Not entirely sure whether we want to register the entire path name,
    // or just the file name...?

  } 
  else { // checksums turned off

    myMapId = ArMapId(myMapId.getSourceName(),
                      myMapId.getFileName(),
                      NULL,
                      0,
                      myReadFileStat.st_size,
                      myReadFileStat.st_mtime);

  } // end else checksums turned off
    
  myMapId.log("ArMapSimple::updateMapFileInfo");

  // TODO ArMapRegistry::getIt()->registerMap(realFileName, myMapId);

} // end method updateMapFileInfo

AREXPORT const char *ArMapSimple::getMapCategory()
{
  if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {

    if (myScanTypeList.size() == 1) {

      const char *scanType = myScanTypeList.front().c_str();

      // Override the scan type only if it is the sick laser.  Other sensors
      // must have their scan type specified in the file.
      if (isDefaultScanType(scanType) || 
         (strcasecmp(scanType, "SickLaser") == 0)) {
        myMapCategory = MAP_CATEGORY_2D;
      }
    }
  }
  return myMapCategory.c_str();

} // end method getMapCategory


AREXPORT void ArMapSimple::updateMapCategory(const char *updatedInfoName)
{
  // The isDowngradeCategory flag indicates whether the map category can 
  // be downgraded (e.g. from 2D-Map-Ex2 to 2D-Map) when the map doesn't 
  // contain advanced features.  This would occur if the advanced feature 
  // was added on the server and was later removed.
  //
  // The default behavior of the server deactivates the removed items, 
  // saving them for later use.  If the map downgrades the map category,
  // big problems can occur if an old copy of the editor is used to 
  // modify the downgraded map.  If the server later re-activates the 
  // items, the map will most likely be corrupted.
  //
  // Hence the flag is set to false. 

  bool isDowngradeCategory = false;

  // This is the "top-most" map category.. If it's already been set, then there's
  // nothing to do
  if (!isDowngradeCategory &&
      strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_TEMPLATE) == 0) {
    return;
  }

  // If the Template features has been explicitly added (via the
  // Main Menu > Tools > Update Map Features... item), then it must be
  // the template category.
  if ((updatedInfoName == NULL) ||
      (strcasecmp(updatedInfoName, META_INFO_NAME) == 0)) {

    bool isTemplateFeatureSupported = infoContains(META_INFO_NAME,
                                                   "Feature",
                                                   0,
                                                   "Name=templates",
                                                   1);


    if (isTemplateFeatureSupported) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_TEMPLATE) != 0) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                   MAP_CATEGORY_2D_TEMPLATE,
                   myMapCategory.c_str(),
                   "CairnGroupType");
        myMapCategory = MAP_CATEGORY_2D_TEMPLATE;
      }
      return;
    } // end if template feature supported
  }

  // If a CairnGroupType MapInfo has been defined, then it must be the template
  // category.  
  if ((updatedInfoName == NULL) || 
      (strcasecmp(updatedInfoName, MAP_INFO_NAME) == 0)) {

    // Groups are now in the template category
    if (mapInfoContains("CairnGroupType")) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_TEMPLATE) != 0) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                   MAP_CATEGORY_2D_TEMPLATE,
                   myMapCategory.c_str(),
                   "CairnGroupType");
        myMapCategory = MAP_CATEGORY_2D_TEMPLATE;
      }
      return;
    } // end if map info contains group type

  } // end if updated info name not specified or is map info
  
  // If a MacroTemplate RouteInfo has been defined, then it must be the template
  // caegory.
  if ((updatedInfoName == NULL) ||
      (strcasecmp(updatedInfoName, ROUTE_INFO_NAME))) {
    // Check for the new MacroTemplate keyword
    if (routeInfoContains("MacroTemplate")) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_TEMPLATE) != 0) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                   MAP_CATEGORY_2D_TEMPLATE,
                   myMapCategory.c_str(),
                   "MacroTemplate");
        myMapCategory = MAP_CATEGORY_2D_TEMPLATE;
      }
      return;
    } // end if map info contains macro template 

  } // end if updated info name not specified or is route info

  const char *macroInfoNames[1];
  macroInfoNames[0] = MACRO_INFO_NAME;

  for (int i = 0; i < 1; i++) {
    if ((updatedInfoName != NULL) &&
        (strcasecmp(updatedInfoName, macroInfoNames[i]) != 0)) {
      continue;
    }

    if ((getInfo(macroInfoNames[i]) != NULL) &&
        (!getInfo(macroInfoNames[i])->empty())) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_TEMPLATE) != 0) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                   MAP_CATEGORY_2D_TEMPLATE,
                   myMapCategory.c_str(),
                   macroInfoNames[i]);
        myMapCategory = MAP_CATEGORY_2D_TEMPLATE;
      }
    }
    return;
  } // end for each extended info name

  // TODO If a parent / child map has been defined, then it must be the composite
  // category


  // If any CairnInfo or CustomInfo have been set, then it must be the extended
  // category
  const char *extendedInfoNames[2];
  extendedInfoNames[0] = CUSTOM_INFO_NAME;
  extendedInfoNames[1] = CAIRN_INFO_NAME;

  for (int i = 0; i < 2; i++) {
    if ((updatedInfoName != NULL) &&
        (strcasecmp(updatedInfoName, extendedInfoNames[i]) != 0)) {
      continue;
    }
        
    if ((getInfo(extendedInfoNames[i]) != NULL) && 
        (!getInfo(extendedInfoNames[i])->empty())) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_EXTENDED) != 0) {
        ArLog::log(ArLog::Normal,
                    "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                    MAP_CATEGORY_2D_EXTENDED,
                    myMapCategory.c_str(),
                    extendedInfoNames[i]);
        myMapCategory = MAP_CATEGORY_2D_EXTENDED;
      }
      return;
    }
  } // end for each extended info name


  // Similarly, if any MapInfo's contain an ArgDesc, then it must be the extended
  // category.  (It seems alright to check this because there shouldn't be 
  // thousands of map info lines...)
  if ((updatedInfoName == NULL) || 
      (strcasecmp(updatedInfoName, MAP_INFO_NAME) == 0)) {

    if (mapInfoContains("ArgDesc")) {
      if ((strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_COMPOSITE) != 0) &&
          (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_EXTENDED) != 0)) {
        ArLog::log(ArLog::Normal,
                  "ArMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                  MAP_CATEGORY_2D_EXTENDED,
                  myMapCategory.c_str(),
                  "ArgDesc");
        myMapCategory = MAP_CATEGORY_2D_EXTENDED;
      }
      return;
    }

  } // end if updated info name not specified or is map info


  // Otherwise, if there is more than one scan type, it must be the multi-source
  // category
  if (!isDowngradeCategory &&
      strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {
    return;
  }

  std::list<std::string> scanTypeList = getScanTypes();

  if (scanTypeList.size() > 1) {

    if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) != 0) {
      ArLog::log(ArLog::Normal,
                  "ArMapSimple::updateMapCategory() changing category to %s from %s because %i scan types",
                  MAP_CATEGORY_2D_MULTI_SOURCES,
                  myMapCategory.c_str(),
                  getScanTypes().size());
      myMapCategory = MAP_CATEGORY_2D_MULTI_SOURCES;
    }
    return;

  } // end if more than one scan type


  if (scanTypeList.size() == 1) {

    const char *scanType = scanTypeList.front().c_str();

    if (!isDefaultScanType(scanType) && 
        (strcasecmp(scanType, "SickLaser") != 0)) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) != 0) {
        ArLog::log(ArLog::Normal,
                    "ArMapSimple::updateMapCategory() changing category to %s from %s because scan type is %s",
                    MAP_CATEGORY_2D_MULTI_SOURCES,
                    myMapCategory.c_str(),
                    scanType);
        myMapCategory = MAP_CATEGORY_2D_MULTI_SOURCES;
      }
      return;
    }
  }
  

  if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D) != 0) {
    ArLog::log(ArLog::Normal,
                "ArMapSimple::updateMapCategory() changing category to %s from %s because no special cases found",
                MAP_CATEGORY_2D,
                myMapCategory.c_str());
    myMapCategory = MAP_CATEGORY_2D;
  }

  return;

} // end method updateMapCategory


AREXPORT bool ArMapSimple::mapInfoContains(const char *arg0Text) 
{
  return infoContains(MAP_INFO_NAME,
                      arg0Text,
                      0);
}

AREXPORT bool ArMapSimple::routeInfoContains(const char *arg0Text)
{
  return infoContains(ROUTE_INFO_NAME,
                      arg0Text,
                      0);
}

/// Determines whether the specified info contains the specified arg text.
AREXPORT bool ArMapSimple::infoContains(const char *infoName,
                                        const char *argText,
                                        int argIndex,
                                        const char *secondaryArgText,
                                        int secondaryArgIndex)
{
  if (ArUtil::isStrEmpty(infoName)) {
    return false;
  }
  if (ArUtil::isStrEmpty(argText)) {
    return false;
  }

  std::list<ArArgumentBuilder*> *infoList = getInfo(infoName);
  if (infoList != NULL) {

    for (std::list<ArArgumentBuilder*>::const_iterator iter = infoList->begin();
        iter != infoList->end();
        iter++) {
      
      ArArgumentBuilder *arg = *iter;
      if ((arg == NULL) || 
          (arg->getArgc() <= argIndex) || 
          (arg->getArg(argIndex) == NULL)) {
        continue;
      }
      std::string firstArgText = arg->getArg(argIndex);

      // If the first argument text is a match...
      if (ArUtil::strcasequotecmp(arg->getArg(argIndex), argText) == 0) {

        // If only the primary search string has been specified, then
        // the data has been found.
        if (ArUtil::isStrEmpty(secondaryArgText) ||
            (secondaryArgIndex < 0)) {
          return true;
        }
        // Otherwise, check the secondary arg to see if it matches
        // too.
        else if ((arg->getArgc() > secondaryArgIndex) &&
                 (arg->getArg(secondaryArgIndex) != NULL)) {

          // If the second argument text is a match...
          if (ArUtil::strcasequotecmp(arg->getArg(secondaryArgIndex),
                                      secondaryArgText) == 0) {
            return true;
          } // end if secondary is a match

        } // end if
      } // end if arg desc found
      
    } // end for each info line
  } // end if non-NULL info

  // Match was not found
  return false;

} // end method infoContains


AREXPORT void ArMapSimple::addPreWriteFileCB(ArFunctor *functor,
                                             ArListPos::Pos position)
{
  addToCallbackList(functor, position, &myPreWriteCBList);

} // end method addPreWriteFileCB

AREXPORT void ArMapSimple::remPreWriteFileCB(ArFunctor *functor)
{
  remFromCallbackList(functor, &myPreWriteCBList);

} // end method remPreWriteFileCB

AREXPORT void ArMapSimple::addPostWriteFileCB(ArFunctor *functor,
                                              ArListPos::Pos position)
{
  addToCallbackList(functor, position, &myPostWriteCBList);

} // end method addPostWriteFileCB

AREXPORT void ArMapSimple::remPostWriteFileCB(ArFunctor *functor)
{
  remFromCallbackList(functor, &myPostWriteCBList);

} // end method remPostWriteFileCB

AREXPORT void ArMapSimple::addPreParseFileCB(ArFunctor1<std::string> *functor)
{
  myPreParseCBList.push_back(functor);

} // end method addPreParseFileCB

AREXPORT void ArMapSimple::remPreParseFileCB(ArFunctor1<std::string> *functor)
{
  myPreParseCBList.remove(functor);
} // end method remPreParseFileCB


AREXPORT bool ArMapSimple::readFile(const char *fileName, 
			                              char *errorBuffer, 
                                    size_t errorBufferLen,
                                    unsigned char *md5DigestBuffer,
                                    size_t md5DigestBufferLen)
{

  if (ArUtil::isStrEmpty(fileName)) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::readFile() cannot read empty file name");
    return false;
  }

  IFDEBUG(
  ArLog::log(ArLog::Normal, 
             "ArMapSimple::readFile() reading %s",
             fileName);
  );

  lock();
  myIsReadInProgress = true;

  if (myMapInfo != NULL) {
    myMapInfo->clear();
  }
  if (myMapObjects != NULL) {
    ArLog::log(ArLog::Verbose, 
            "ArMapSimple::readFile() clearing map objects");
    myMapObjects->clear();
  }
  if (myMapSupplement != NULL) {
    ArLog::log(ArLog::Verbose, 
            "ArMapSimple::readFile() clearing map supplement");
    myMapSupplement->clear();
  }
  for (ArTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    ArMapScan *scan = iter->second;
    if (scan != NULL) {
      ArLog::log(ArLog::Verbose, 
              "ArMapSimple::readFile() clearing scan");
      scan->clear();
    }
  } // end for each scan type

  if (myInactiveInfo != NULL) {
    myInactiveInfo->clear();
  }
  if (myInactiveObjects != NULL) {
    myInactiveObjects->clear();
  }
  if (myChildObjects != NULL) {
    myChildObjects->clear();
  }

  reset();

  // stat(fileName, &myReadFileStat);
  FILE *file = NULL;

  char line[12000];

  std::string realFileName = createRealFileName(fileName);

  ArLog::log(ArLog::Normal, 
             "Opening map file %s, given %s", 
             realFileName.c_str(), fileName);


  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.
  // In addition, on Windows, specify that this is UTF-8 format for i18n.
  if ((file = ArUtil::fopen(realFileName.c_str(), "rb, ccs=UTF-8")) == NULL)
  {
    ArLog::log(ArLog::Terse, "Cannot open file '%s'", realFileName.c_str());
    // TODO This used to put the config param name into the error buffer
    if (errorBuffer != NULL) {
      snprintf(errorBuffer, errorBufferLen, 
               "Map invalid: cannot open file '%s'",
               fileName);
    }
    myIsReadInProgress = false;
    unlock();
    return false;
  }

  // Save the start position of the file stream.  This may be used later
  // if an error occurs.
  fpos_t startPosition;
  fgetpos(file, &startPosition);

  ArFunctor1<const char *> *parseFunctor = NULL;

  ArTime parseTime;
  parseTime.setToNow();

  if (myChecksumCalculator != NULL) {
      
    myChecksumCalculator->reset();

    parseFunctor = myChecksumCalculator->getFunctor();
    myLoadingParser->setPreParseFunctor(parseFunctor);
  }
  else {
    myLoadingParser->setPreParseFunctor(NULL);
  }

  bool isSuccess = true;

  char *localErrorBuffer = NULL;
  size_t localErrorBufferLen = 0;

  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    localErrorBufferLen = errorBufferLen;
    localErrorBuffer = new char[localErrorBufferLen];
    localErrorBuffer[0] = '\0';
  }

  // Read the first line of the file. This will be the keyword.
  fgets(line, sizeof(line), file);
  line[sizeof(line) - 1] = '\0';
  std::string firstLine = line;

  // Strip the newline characters from the end of the first line.
  size_t endPos = firstLine.find_last_not_of("\n\r");
  if (endPos != std::string::npos) {
    firstLine.erase(endPos + 1);
  }

  //this is a callback list to process stuff based on map versions
  invokeCallbackList(&myPreParseCBList, firstLine);

  fsetpos(file, &startPosition);

  if (// TODO !reset() || 
      !myLoadingParser->parseFile(file, line, 12000, 
                                  false,  // Do not continue on error
                                  localErrorBuffer, localErrorBufferLen))
  {
    // If the error buffer is specified, then just make absolutely sure
    // that it is null terminated
    if ((localErrorBuffer != NULL) && (localErrorBufferLen > 0)) {
      localErrorBuffer[localErrorBufferLen - 1] = '\0';
    }

    if (myLoadingDataTag.empty()) 
    // if (!myLoadingDataStarted && !myLoadingLinesAndDataStarted) 
    {
      // TODO reset();
      if (errorBuffer != NULL) {

    // TODO This used to put the config param name into the error buffer
        snprintf(errorBuffer, errorBufferLen, 
                 "Map invalid: '%s' not a valid map (%s)",
                 fileName, localErrorBuffer);
        errorBuffer[errorBufferLen - 1] = '\0';
      }
      //unlock();
      ArLog::log(ArLog::Terse, "Could not load map file '%s'", fileName);

      isSuccess = false;
    }
  } // end if parse success

  delete [] localErrorBuffer;

  if (!myLoadingGotMapCategory)
  {
    // The current position of the file read. This is saved just in case...
    // i.e. not to change the previous behavior.
    fpos_t position;
    fgetpos(file, &position);

    // Reset the file position to the beginning of the file
    fsetpos (file, &startPosition);
    // Read the first line of the file. This will be the keyword.
    fgets(line, sizeof(line), file);
    line[sizeof(line) - 1] = '\0';


    // Restore the file position
    fsetpos(file, &position);


    std::string firstLine = line;

    // Strip the newline characters from the end of the first line.
    size_t endPos = firstLine.find_last_not_of("\n\r");
    if (endPos != std::string::npos) {
      firstLine.erase(endPos + 1);
    }

    // Determine whether the first line begins with the 2D-Map tag. If so,
    // it is likely that the file is a more recent version.
    size_t startPos = firstLine.find(MAP_CATEGORY_2D);

    // TODO reset();
    if (errorBuffer != NULL) {

      // Customize the error message based on whether "2D-Map" was found.
      if (startPos == 0) {

        snprintf(errorBuffer, errorBufferLen,
                 "Map invalid, '%s' is a '%s'. This software version supports up to '%s'.",
                 fileName,
                 firstLine.c_str(),
                 myMapCategoryList.back().c_str());
      }
      else {

        snprintf(errorBuffer, errorBufferLen,
                 "Map invalid, '%s' was not a map file.",
                 fileName);
      } // end else
    } // end if error buffer

    //unlock();
    ArLog::log(ArLog::Terse, 
               "Could not load map file '%s' it was not a recognized map format (%s)",
               fileName, line);
    isSuccess = false;
  }

  bool isLineDataTag = false; // TODO 
  bool isEndOfFile = false;
  
  myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                          &isLineDataTag);

  isSuccess = (myLoadingScan != NULL);

  while (isSuccess && !isEndOfFile && !myIsCancelRead) {
    
    bool isDataTagFound = false;

    while ((fgets(line, sizeof(line), file) != NULL) && !myIsCancelRead) 
    {
      if (parseFunctor != NULL) {
        parseFunctor->invoke(line);
      }

      if (isDataTag(line)) // strncasecmp(line, "DATA", strlen("DATA")) == 0)
      {
        //myLoadingDataTag = line;
        isDataTagFound = true;
        break;
      }
      if (isLineDataTag && !readLineSegment(line))
      {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::readFile() error reading line data '%s'",
                   line);
        continue;
      }
      else if (!isLineDataTag && !readDataPoint(line))
      {
        continue;
      }
    } // end while more lines to read

    if (isDataTagFound) {
    
      myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                              &isLineDataTag);
      isSuccess = (myLoadingScan != NULL);

      if (myLoadingScan != NULL) {

        ArLog::log(ArLog::Verbose,
                   "ArMapSimple::readFile() found scan type %s for data tag %s (is line = %i)",
                   myLoadingScan->getScanType(),
                   myLoadingDataTag.c_str(),
                   isLineDataTag);
      }
      else {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::readFile() cannot find scan for data tag %s (is line = %i)",
                   myLoadingDataTag.c_str(),
                   isLineDataTag);
      }
    }
    else { // else must be end of file

      isEndOfFile = true;
      ArLog::log(ArLog::Verbose,
                 "ArMapSimple::readFile() end of file found");

    } // end else end of file

  }  // end while no error and not end of file



  double minX = ArUtil::findMin(getMinPose().getX(),
				getLineMinPose().getX());
  double minY = ArUtil::findMin(getMinPose().getY(),
				getLineMinPose().getY());
  double maxX = ArUtil::findMax(getMaxPose().getX(),
				getLineMaxPose().getX());
  double maxY = ArUtil::findMax(getMaxPose().getY(),
				getLineMaxPose().getY());

  /*
  ArLog::log(ArLog::Normal, "##### Before");
  ArLog::log(ArLog::Normal, "Points Min %.0f %.0f Max %.0f %.0f",
	     getMinPose().getX(), getMinPose().getY(),
	     getMaxPose().getX(), getMaxPose().getY());
  ArLog::log(ArLog::Normal, "Lines Min %.0f %.0f Max %.0f %.0f",
	     getLineMinPose().getX(), getLineMinPose().getY(),
	     getLineMaxPose().getX(), getLineMaxPose().getY());

  ArLog::log(ArLog::Normal, "Total Min %.0f %.0f Max %.0f %.0f",
	     minX, minY, maxX, maxY);

  ArLog::log(ArLog::Normal, "Dist X %.0f Dist Y %.0f",
	     maxX - minX, maxY - minY);
  */

  if (isSuccess && (maxX - minX > 2 * 1000 * 1000 || 
		    maxY - minY > 2 * 1000 * 1000))
  {
    ArLog::log(ArLog::Terse, 
	       "ArMapSimple: Problem loading map.  Map invalid: x or y is more than 2 km in length, so there was most likely corruption somewhere.  Min (%.0f, %.0f) Max (%.0f %.0f), dist X %.0f dist Y %.0f.  Recalculating mins and maxes.",
	       minX, minY, maxX, maxY, maxX - minX, maxY - minY);

    std::list<std::string>::iterator iter = myScanTypeList.end();
    
    // set points and lines so it recalculates
    for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {
      
      const char *scanType = (*iter).c_str();
      ArMapScan *mapScan = getScan(scanType);
      
      ArMapScan scanCopy(*mapScan);
      setPoints(scanCopy.getPoints(), scanType);
      setLines(scanCopy.getLines(), scanType);

    }

    /*
    ArLog::log(ArLog::Normal, "##### After");
    ArLog::log(ArLog::Normal, "Points Min %.0f %.0f Max %.0f %.0f",
	       getMinPose().getX(), getMinPose().getY(),
	       getMaxPose().getX(), getMaxPose().getY());
    ArLog::log(ArLog::Normal, "Lines Min %.0f %.0f Max %.0f %.0f",
	       getLineMinPose().getX(), getLineMinPose().getY(),
	       getLineMaxPose().getX(), getLineMaxPose().getY());

    ArLog::log(ArLog::Normal, "Total Min %.0f %.0f Max %.0f %.0f",
	       minX, minY, maxX, maxY);
    
    ArLog::log(ArLog::Normal, "Dist X %.0f Dist Y %.0f",
	       maxX - minX, maxY - minY);
    */

    minX = ArUtil::findMin(getMinPose().getX(),
				  getLineMinPose().getX());
    minY = ArUtil::findMin(getMinPose().getY(),
				  getLineMinPose().getY());
    maxX = ArUtil::findMax(getMaxPose().getX(),
				  getLineMaxPose().getX());
    maxY = ArUtil::findMax(getMaxPose().getY(),
				  getLineMaxPose().getY());

    if (maxX - minX > 2 * 1000 * 1000 || 
	maxY - minY > 2 * 1000 * 1000)
    {
      if (errorBuffer != NULL)
	snprintf(errorBuffer, errorBufferLen, 
		 "Map invalid: x or y is more than 2 km in length, so there was most likely corruption somewhere.  Min (%.0f, %.0f) Max (%.0f %.0f), dist X %.0f dist Y %.0f",
		 minX, minY, maxX, maxY, maxX - minX, maxY -minY);
      
      ArLog::log(ArLog::Terse, 
		 "ArMapSimple: Failed to load map.  Map invalid: x or y is more than 2 km in length, so there was most likely corruption somewhere.  Min (%.0f, %.0f) Max (%.0f %.0f), dist X %.0f dist Y %.0f.",  
		 minX, minY, maxX, maxY, maxX - minX, maxY - minY);
      
      isSuccess = false;
    
    }
    else
    {
      ArLog::log(ArLog::Terse, 
		 "ArMapSimple: Recalculating mins and maxes took care of the problem.  New values Min (%.0f, %.0f) Max (%.0f %.0f), dist X %.0f dist Y %.0f.",
		 minX, minY, maxX, maxY, maxX - minX, maxY - minY);

    }
  }


  updateSummaryScan();


  int elapsed = parseTime.mSecSince();

  ArLog::log(ArLog::Normal, 
             "ArMapSimple::readFile() %s took %i msecs to read map of %i points",
             realFileName.c_str(),
             elapsed,
             getNumPoints(ARMAP_SUMMARY_SCAN_TYPE));	

  fclose(file);

  if (!myIsCancelRead) {
    updateMapFileInfo(realFileName.c_str());

    //stat(realFileName.c_str(), &myReadFileStat);

    if (myChecksumCalculator != NULL) {
      
      if (md5DigestBuffer != NULL) {
        memset(md5DigestBuffer, 0, md5DigestBufferLen);
        memcpy(md5DigestBuffer, myChecksumCalculator->getDigest(), 
              ArUtil::findMin(md5DigestBufferLen, ArMD5Calculator::DIGEST_LENGTH));
      }

      myLoadingParser->setPreParseFunctor(NULL);
    }

    if (isSuccess) {
      // move the stuff over from reading to new
      myFileName = fileName;
    
      ArLog::log(myMapChangedHelper->getMapChangedLogLevel(), 
                "ArMapSimple:: Calling mapChanged()");	
      mapChanged();
      ArLog::log(myMapChangedHelper->getMapChangedLogLevel(), 
                "ArMapSimple:: Finished mapChanged()");

    }
  } // end if not cancelling

  myIsReadInProgress = false;

  unlock();
  return isSuccess;

} // end method readFile


AREXPORT bool ArMapSimple::isDataTag(const char *line) 
{
  // Pre: Line is not null
  ArDataTagToScanTypeMap::iterator typeIter =  
            myDataTagToScanTypeMap.find(line);

  if (typeIter != myDataTagToScanTypeMap.end()) {
    myLoadingDataTag = typeIter->first;
    return true;
  }
  return false;

} // end method isDataTag


AREXPORT ArMapScan *ArMapSimple::findScanWithDataKeyword
                                     (const char *loadingDataTag,
                                      bool *isLineDataTagOut)
{
  //ArLog::log(ArLog::Normal,
  //           "ArMapSimple::findScanWithDataKeyword() looking for scan with tag %s",
  //           loadingDataTag);

  if (ArUtil::isStrEmpty(loadingDataTag)) {
    ArLog::log(ArLog::Verbose,
               "ArMapSimple::findScanWithDataKeyword() cannot find scan for empty tag");
    return NULL;
  }
  ArDataTagToScanTypeMap::iterator typeIter =  
            myDataTagToScanTypeMap.find(loadingDataTag);
  if (typeIter == myDataTagToScanTypeMap.end()) {
    ArLog::log(ArLog::Verbose,
               "ArMapSimple::findScanWithDataKeyword() error finding tag '%s' data",
               loadingDataTag);
    return NULL;
  }

  ArTypeToScanMap::iterator scanIter =
            myTypeToScanMap.find(typeIter->second);

  if (scanIter == myTypeToScanMap.end()) {
    ArLog::log(ArLog::Verbose,
               "ArMapSimple::findScanWithDataKeyword() error finding scan data for '%s' ",
               loadingDataTag);
   return NULL;
  }

  ArMapScan *mapScan = scanIter->second;
  if (mapScan == NULL) {
   ArLog::log(ArLog::Verbose,
               "ArMapSimple::findScanWithDataKeyword() error finding scan for '%s' ",
               loadingDataTag);
    return NULL;
  }

  bool isLineDataTag = (ArUtil::strcasecmp(loadingDataTag,
                                           mapScan->getLinesKeyword()) == 0);
 
  ArLog::log(ArLog::Verbose,
             "ArMapSimple::findScanWithDataKeyword() found scan for tag %s, isLineData = %i",
             loadingDataTag,
             isLineDataTag);

  
  if (isLineDataTag) {
    myLoadingLinesAndDataStarted = true;
  }
  else {
    myLoadingDataStarted = true;
  }

  if (isLineDataTagOut != NULL) {
    *isLineDataTagOut = isLineDataTag;
  }

  return mapScan;
  
} // end method findScanWithDataKeyword


AREXPORT bool ArMapSimple::writeFile(const char *fileName, 
                                     bool internalCall,
                                     unsigned char *md5DigestBuffer,
                                     size_t md5DigestBufferLen,
                                     time_t fileTimestamp)
{ 
  FILE *file = NULL;
  if (!internalCall)
    lock();

  // Calling updateMapCategory here just in case the file is being 
  // written because the associated info has changed (as in the case
  // of AramMapInfoMinder).
  updateMapCategory();

  invokeCallbackList(&myPreWriteCBList);

	std::string realFileName = createRealFileName(fileName);
	std::string writeFileName;

  if (myIsWriteToTempFile) {

    char tempFileName[3200];
    int tempFileNameLen = 3200;

    tempFileName[0] = '\0';

    int fileNumber = getNextFileNumber();

// Hoping that this is highly temporary...
#ifdef WIN32
    snprintf(tempFileName, tempFileNameLen,
             "%sArMap.%d.%d", 
             myTempDirectory.c_str(), _getpid(), fileNumber);
#else // linux
    snprintf(tempFileName, tempFileNameLen,
             "%sArMap.%d.%d", 
             myTempDirectory.c_str(), getpid(), fileNumber);
#endif  // end else linux

    ArLog::log(ArLog::Normal,
               "Writing map %s to temp file %s\n",
               fileName,
               tempFileName);

    writeFileName = tempFileName;

  }
  else { // write to actual file

    writeFileName = realFileName;

  } // end else write to actual file

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.
  // In addition, on Windows, specify that this is UTF-8 format for i18n.
  if ((file = ArUtil::fopen(writeFileName.c_str(), "wb, ccs=UTF-8")) == NULL)
  {
    bool isFileSuccess = false;

    ArLog::log(ArLog::Terse, 
               "ArMap: Cannot open file '%s' for writing",
	             writeFileName.c_str());
  
    invokeCallbackList(&myPostWriteCBList);

    if (!internalCall)
      unlock();

    return false;

  } // end if error opening file for writing


  ArTime writeTime;
  writeTime.setToNow();

  ArFunctor1<const char *> *writeFunctor = NULL;

  ArGlobalFunctor2<const char *, FILE *> functor(&ArUtil::writeToFile, "", file);

  
  if (myChecksumCalculator != NULL) { 
    ArLog::log(ArLog::Normal, 
               "ArMapSimple::writeFile() recalculating checksum");

    myChecksumCalculator->reset();

    // Note that this is reset to NULL below before it leaves the scope
    // of this method.
    myChecksumCalculator->setSecondFunctor(&functor);
    writeFunctor = myChecksumCalculator->getFunctor();
  }
  else {
    writeFunctor = &functor;
  }

  writeToFunctor(writeFunctor, "\n");
    
  int elapsed = writeTime.mSecSince();

  ArLog::log(ArLog::Normal, 
             "ArMapSimple::writeFile() took %i msecs to write map of %i points",
             elapsed,
             getNumPoints());	


  fclose(file);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (myIsWriteToTempFile) {

    char systemBuf[6400];
    int  systemBufLen = 6400;

#ifndef WIN32
    const char *moveCmdName = "mv -f";
#else
    const char *moveCmdName = "move";
#endif

    int printLen = snprintf(systemBuf, systemBufLen,
                            "%s \"%s\" \"%s\"", 
                            moveCmdName, 
                            writeFileName.c_str(), 
                            realFileName.c_str());
    systemBuf[systemBufLen - 1] = '\0';

    int ret = -1;

    if ((printLen >= 0) && (printLen < systemBufLen)) {

      ret = system(systemBuf);

    } // end if success creating command
  
    if (ret != 0) {

      ArLog::log(ArLog::Terse, 
                 "Error saving map file %s.  Temp file cannot be moved. (%s)", 
                 fileName,
                 systemBuf);
      if (!internalCall)
        unlock();

      return false;

    } // end if error moving file

  } // end if write to temp file

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // The file info needs to be set to the real file name (not the possible temp one)

  if (fileTimestamp != -1) {

    ArLog::log(ArLog::Normal, 
               "ArMapSimple::writeFile() setting time of %s",
               realFileName.c_str());	
    bool isTimeChanged = ArUtil::changeFileTimestamp(realFileName.c_str(), 
                                                     fileTimestamp);

  }

// KMC TODO Wouldn't it make sense to change the myFileName member?
  myFileName = fileName;



  updateMapFileInfo(realFileName.c_str());

  // Reset the file statistics to reflect the newly written file.	
  ArUtil::filestat(realFileName, &myReadFileStat);

  if (myChecksumCalculator != NULL) {

    if (md5DigestBuffer != NULL) {
      if (md5DigestBufferLen != ArMD5Calculator::DIGEST_LENGTH) {
        // log warning
      }
      memset(md5DigestBuffer, 0, md5DigestBufferLen);
      memcpy(md5DigestBuffer, myChecksumCalculator->getDigest(), 
            ArUtil::findMin(md5DigestBufferLen, ArMD5Calculator::DIGEST_LENGTH));
    }

    // Reset to NULL before the functor leaves the scope of this method.
    myChecksumCalculator->setSecondFunctor(NULL);

  } // end if checksum calculated
    
  invokeCallbackList(&myPostWriteCBList);

  ArLog::log(ArLog::Normal, "Saved map file %s", fileName);
  if (!internalCall)
    unlock();


  return true;

} // end method writeFile


AREXPORT bool ArMapSimple::calculateChecksum(unsigned char *md5DigestBuffer,
                                             size_t md5DigestBufferLen)
{

  if ((md5DigestBuffer == NULL) || 
      (md5DigestBufferLen < ArMD5Calculator::DIGEST_LENGTH)) {
    return false;
  }

  lock();
  
  bool isLocalCalculator = false;
  ArMD5Calculator *calculator = myChecksumCalculator;
  if (calculator == NULL) {
    isLocalCalculator = true;
    calculator = new ArMD5Calculator();
  }

  memset(md5DigestBuffer, 0, md5DigestBufferLen);

  calculator->reset();
  writeToFunctor(calculator->getFunctor(), "\n");

  memcpy(md5DigestBuffer, calculator->getDigest(), 
         ArMD5Calculator::DIGEST_LENGTH);

  if (isLocalCalculator) {
    delete calculator;
  }

  unlock();

  return true;

} // end method calculateChecksum


AREXPORT const char *ArMapSimple::getBaseDirectory(void) const
{ 
  return myBaseDirectory.c_str();

} // end method getBaseDirectory


AREXPORT const char *ArMapSimple::getFileName(void) const 
{
  return myFileName.c_str();

} // end method getFileName


void ArMapSimple::setIgnoreEmptyFileName(bool ignore)
{ 
  myIgnoreEmptyFileName = ignore;

} // end method setIgnoreEmptyFileName


bool ArMapSimple::getIgnoreEmptyFileName(void)
{ 
  return myIgnoreEmptyFileName;

} // end method getIgnoreEmptyFileName


void ArMapSimple::setIgnoreCase(bool ignoreCase)
{ 
  myIgnoreCase = ignoreCase;

} // end method setIgnoreCase


bool ArMapSimple::getIgnoreCase(void) 
{ 
  return myIgnoreCase;

} // end method getIgnoreCase


AREXPORT void ArMapSimple::setBaseDirectory(const char *baseDirectory)
{ 
  if (baseDirectory != NULL) {
    myBaseDirectory = baseDirectory;
  }
  else {
    myBaseDirectory = "";
  }

} // end method setBaseDirectory

AREXPORT const char *ArMapSimple::getTempDirectory(void) const
{
  if (myIsWriteToTempFile) {
    return myTempDirectory.c_str();
  }
  else {
    return NULL;
  }
}

AREXPORT void ArMapSimple::setTempDirectory(const char *tempDirectory)
{
  if (tempDirectory != NULL) {
    myIsWriteToTempFile = true;
    myTempDirectory = tempDirectory;
  }
  else {
    myIsWriteToTempFile = false;
    myTempDirectory = "";
  }
}


AREXPORT void ArMapSimple::setSourceFileName(const char *sourceName,
                                             const char *fileName,
                                             bool isInternalCall)
{
  if (!isInternalCall) {
    lock();
  }
  myMapId.setSourceName(sourceName);

  std::string realFileName = ((fileName != NULL) ? fileName : "");

  if (!myBaseDirectory.empty()) {
    if (realFileName.find(myBaseDirectory) == 0) {
      size_t dirLen = myBaseDirectory.length();
      if ((myBaseDirectory[dirLen - 1] == '/') || 
          (myBaseDirectory[dirLen - 1] == '\\')) {
        realFileName = realFileName.substr(dirLen, 
                                           realFileName.length() - dirLen);
      }
      else {
        realFileName = realFileName.substr(dirLen + 1, 
                                           realFileName.length() - (dirLen + 1));
      }

      ArLog::log(ArLog::Normal,
                 "ArMapSimple::setSourceFileName(%s, %s) stripped base dir = %s",
                 sourceName,
                 fileName,
                 realFileName.c_str());
    }

  }

  myMapId.setFileName(realFileName.c_str());
  if (!isInternalCall) {
    unlock();
  }

} // end method setSourceFileName


AREXPORT bool ArMapSimple::getMapId(ArMapId *mapIdOut,
                                    bool isInternalCall)
{
  if (mapIdOut != NULL) {
    if (!isInternalCall) {
      lock();
    }
    *mapIdOut = myMapId;
    if (!isInternalCall) {
      unlock();
    }
    return true;
  }
  else {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::getMapId() null map ID param");
  }
  return false;

} // end method getMapId
  
  
AREXPORT ArArgumentBuilder *ArMapSimple::findMapObjectParams
                                           (const char *mapObjectName)
{
  if (ArUtil::isStrEmpty(mapObjectName)) {
    return NULL;
  }

  ArArgumentBuilder *params = NULL;

  std::map<std::string, ArArgumentBuilder *, ArStrCaseCmpOp>::iterator iter =
     myMapObjectNameToParamsMap.find(mapObjectName);

  if (iter == myMapObjectNameToParamsMap.end()) {
    
    std::list<ArArgumentBuilder*> *cairnInfoList = getInfo(CAIRN_INFO_NAME);

    if (cairnInfoList != NULL) {

      ArArgumentBuilder *paramInfo = NULL;
      std::list<ArArgumentBuilder *>::iterator infoIter = 
                                          findMapObjectParamInfo(mapObjectName,
                                                                 *cairnInfoList);
      if (infoIter != cairnInfoList->end()) {
        paramInfo = *infoIter;
      }
      if (paramInfo != NULL) {

        params = new ArArgumentBuilder(*paramInfo);
        params->compressQuoted(true);

        if (params->getArgc() >= 2) {
          params->removeArg(0);       // Remove the "Params" field
          params->removeArg(0, true); // Remove the map object name
        }
        myMapObjectNameToParamsMap[mapObjectName] = params;
      }

    } // end for each cairn info
  } // end if 
  // if we already have one, juset set that
  else
  {
    params = (*iter).second;
  }

  return params;

} // end method findMapObjectParams

/**
  ArMapObject *obj = myMap->findMapObject(objName);
  const ArArgumentBuilder *args = myMap->findMapObjectParams(objName);

  bool ok = true;
  int someNum = 0;

  if ((args != NULL) && (args->getArgc() > 0)) {
    
    someNum = args->getArgInt(0, &ok);
  }

  if (ok) {

    ArArgumentBuilder newArgs;
    char buf[128]
    snprintf(buf, sizeof(buf), "%i", someNum + 1);
    newArgs.add(buf);

    // Note that the newArgs will be copied, and that the original args 
    // pointer (above) will be invalidated by the following call.
    myMap->setMapObjectParams(objName, &newArgs);
    args = NULL;
  }

 
  mapObjectList.remove(obj);
  myMap->setMapObjectParams(obj->getName(), NULL);
  myMap->setMapObjects(mapObjectList);
  delete obj; 
  
**/

AREXPORT bool ArMapSimple::setMapObjectParams(const char *mapObjectName,
                                              ArArgumentBuilder *params,
                                              ArMapChangeDetails *changeDetails)
{
  if (ArUtil::isStrEmpty(mapObjectName)) {
    return false;
  }
  
  std::list<ArArgumentBuilder*> *cairnInfoList = getInfo(CAIRN_INFO_NAME);
  if (cairnInfoList == NULL) {
    return false;
  }
  
  ArMapFileLineSet origLines;

  if (changeDetails != NULL) {
    myMapInfo->createMultiSet(CAIRN_INFO_NAME, &origLines, changeDetails);
  }

  std::map<std::string, ArArgumentBuilder *, ArStrCaseCmpOp>::iterator iter =
     myMapObjectNameToParamsMap.find(mapObjectName);

  if (iter != myMapObjectNameToParamsMap.end()) {
    ArArgumentBuilder *oldParams = iter->second;
    myMapObjectNameToParamsMap.erase(iter);
    delete oldParams;
  }
  
  if (params != NULL) {
    myMapObjectNameToParamsMap[mapObjectName] = new ArArgumentBuilder(*params);
  }


  std::list<ArArgumentBuilder *>::iterator infoIter = 
                                     findMapObjectParamInfo
                                        (mapObjectName,
                                         *cairnInfoList);
  if (infoIter != cairnInfoList->end()) {
    ArArgumentBuilder *oldInfo = *infoIter;
    cairnInfoList->erase(infoIter);
    delete oldInfo;
  }
 
  if (params != NULL) {
 
    // Any need to make sure that mapObjectName is not started with quotes
    // already?
    std::string quotedMapObjectName = "\"";
    quotedMapObjectName += mapObjectName;
    quotedMapObjectName += "\"";

    ArArgumentBuilder *newInfo = new ArArgumentBuilder();
    newInfo->add("Params");
    newInfo->add(quotedMapObjectName.c_str());
    newInfo->add(params->getFullString());

    // This little alphabetization of the CairnInfo list is done just to 
    // make the ArMapChangeDetails processing better. (i.e. It's in sync
    // with the editor.)
    bool isInserted = false;
    for (infoIter = cairnInfoList->begin(); 
         infoIter != cairnInfoList->end(); 
         infoIter++) {
      ArArgumentBuilder *curArg = *infoIter;
      
      if ((curArg == NULL) || (curArg->getArgc() < 2)) {
        continue;
      }
      
      if (ArUtil::strcasequotecmp(mapObjectName, curArg->getArg(1)) <= 0) {
        cairnInfoList->insert(infoIter, newInfo);
        isInserted = true;
        break;
      }
    } // end for each info item

    if (!isInserted) {
      cairnInfoList->push_back(newInfo);
    }
    
  } // end if new params
  
  myMapInfo->setChanged();
  
  if (changeDetails != NULL) {

    ArMapFileLineSet newLines;
    myMapInfo->createMultiSet(CAIRN_INFO_NAME, &newLines, changeDetails);

    bool isSuccess = ArMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedInfoLines
                                          (CAIRN_INFO_NAME,
                                           ArMapChangeDetails::DELETIONS),
                         changeDetails->getChangedInfoLines
                                          (CAIRN_INFO_NAME,
                                           ArMapChangeDetails::ADDITIONS));
    if (!isSuccess) {
      ArLog::log(ArLog::Normal,
                 "ArMapInfo::setInfo() error calculating changes");
    }
  } // end if changeDetails

  return true;

} // end method setMapObjectParams


std::list<ArArgumentBuilder *>::iterator ArMapSimple::findMapObjectParamInfo
             (const char *mapObjectName,
              std::list<ArArgumentBuilder*> &cairnInfoList)
{
  // If the map object has parameters, then it must have a name.
  if (ArUtil::isStrEmpty(mapObjectName)) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::findMapObjectParamInfo() cannot find empty map object name");
    return cairnInfoList.end();
  }

  for (std::list<ArArgumentBuilder*>::iterator iter = cairnInfoList.begin();
       iter != cairnInfoList.end();
       iter++) {

    ArArgumentBuilder *arg = *iter;
    // The first arg in the CairnInfo line is the type of info (e.g. Params)
    // and the second arg name of the map object...
    if ((arg == NULL) || 
        (arg->getArgc() < 2) || 
        (ArUtil::isStrEmpty(arg->getArg(1)))) {
      ArLog::log(ArLog::Normal,
                 "AramMapInfoMinder::findObjectParams() skipping: %s",
                 ((arg != NULL) ? arg->getFullString(): "NULL"));
      continue;
    }

    if (ArUtil::strcasequotecmp(arg->getArg(1), mapObjectName) == 0) {
      return iter;
    }
  } // end for each cairn info list item

  // Not found
  return cairnInfoList.end();

} // end method findMapObjectParamInfo
  


AREXPORT std::list<ArArgumentBuilder *> *ArMapSimple::getRemainder()
{
  return &myRemainderList;
}

AREXPORT void ArMapSimple::setQuiet(bool isQuiet)
{ 
  ArLog::log(ArLog::Verbose, "ArMapSimple::setQuiet() = %i", isQuiet);
  myIsQuiet = isQuiet;

} // end method setQuiet
	

AREXPORT void ArMapSimple::mapChanged(void)
{ 
  ArTime maxScanTimeChanged = findMaxMapScanTimeChanged();
//  ArLog::log(level, "ArMap: Calling mapChanged callbacks");
  if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()) ||
      !myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()) ||
      !myTimeMapSupplementChanged.isAt(myMapSupplement->getTimeChanged()) ||
      !myTimeMapScanChanged.isAt(maxScanTimeChanged)) {
    
    ArLog::log(myMapChangedHelper->getMapChangedLogLevel(),
	       "ArMapSimple::mapChanged() msecs-objects: %i, msecs-points: %i, msecs-mapInfo: %i msecs-supplement: %i",
	       myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()),
	       !myTimeMapScanChanged.isAt(maxScanTimeChanged),
	       !myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()),
	       !myTimeMapSupplementChanged.isAt(
		       myMapSupplement->getTimeChanged()));
		

    // Since setInfo is not necessarily called, call updateMapCategory just to 
    // make sure that the category is correctly set based on the contents of the 
    // map file.
    if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged())) {
      updateMapCategory();
    }
    
    if (!myTimeMapScanChanged.isAt(maxScanTimeChanged)) {
      updateSummaryScan();
    } // end if scan was changed
    
    myMapChangedHelper->invokeMapChangedCallbacks();
    
    ArLog::log(myMapChangedHelper->getMapChangedLogLevel(),
	       "ArMapSimple: Done calling mapChanged callbacks");
  }
  else { // nothing changed
    
    ArLog::log(ArLog::Verbose,
	       "ArMapSimple::mapChanged(): Map was not changed");
    
  } // end else nothing changed
  
  myTimeMapObjectsChanged = myMapObjects->getTimeChanged();
  myTimeMapSupplementChanged = myMapSupplement->getTimeChanged();
  myTimeMapInfoChanged    = myMapInfo->getTimeChanged();
  myTimeMapScanChanged    = findMaxMapScanTimeChanged(); 

} // end method mapChanged

AREXPORT void ArMapSimple::mapChanged(bool invokePathPlanningCB,
                                   bool invokeLocalizationCB)
{
  ArTime maxScanTimeChanged = findMaxMapScanTimeChanged();
//  ArLog::log(level, "ArMap: Calling mapChanged callbacks");
  if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()) ||
      !myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()) ||
      !myTimeMapSupplementChanged.isAt(myMapSupplement->getTimeChanged()) ||
      !myTimeMapScanChanged.isAt(maxScanTimeChanged)) {

    ArLog::log(myMapChangedHelper->getMapChangedLogLevel(),
           "ArMapSimple::mapChanged() msecs-objects: %i, msecs-points: %i, msecs-mapInfo: %i msecs-supplement: %i",
           myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()),
           !myTimeMapScanChanged.isAt(maxScanTimeChanged),
           !myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()),
           !myTimeMapSupplementChanged.isAt(
               myMapSupplement->getTimeChanged()));

    // Since setInfo is not necessarily called, call updateMapCategory just to
    // make sure that the category is correctly set based on the contents of the
    // map file.
    if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged())) {
      updateMapCategory();
    }

    if (!myTimeMapScanChanged.isAt(maxScanTimeChanged)) {
      updateSummaryScan();
    } // end if scan was changed

    myMapChangedHelper->invokeMapChangedCallbacks(invokePathPlanningCB,
                                                  invokeLocalizationCB);

    ArLog::log(myMapChangedHelper->getMapChangedLogLevel(),
           "ArMapSimple: Done calling mapChanged callbacks");
  }
  else { // nothing changed

    ArLog::log(ArLog::Verbose,
           "ArMapSimple::mapChanged(): Map was not changed");

  } // end else nothing changed

  myTimeMapObjectsChanged = myMapObjects->getTimeChanged();
  myTimeMapSupplementChanged = myMapSupplement->getTimeChanged();
  myTimeMapInfoChanged    = myMapInfo->getTimeChanged();
  myTimeMapScanChanged    = findMaxMapScanTimeChanged();

} // end method mapChanged

AREXPORT void ArMapSimple::updateSummaryScan()
{ 
  if (mySummaryScan != NULL) {
    
    mySummaryScan->clear();

    for (ArTypeToScanMap::iterator iter = myTypeToScanMap.begin();
         iter != myTypeToScanMap.end();
         iter++) {
      mySummaryScan->unite(iter->second); 
    }
  } // end if summary
}


AREXPORT ArTime ArMapSimple::findMaxMapScanTimeChanged()
{
  ArTime maxMapScanTimeChanged;
  bool isFirst = true;

  for (ArTypeToScanMap::iterator iter = 
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    ArMapScan *scan = iter->second;
    if (scan != NULL) {
      if (isFirst || 
          (scan->getTimeChanged().isAfter(maxMapScanTimeChanged))) {
        isFirst = false;
        maxMapScanTimeChanged = scan->getTimeChanged();
      }
    }
  } // end for each scan
 
  return maxMapScanTimeChanged;


} // end method findMaxMapScanTimeChanged


AREXPORT void ArMapSimple::addMapChangedCB(ArFunctor *functor, 
					   int position)
{ 
  myMapChangedHelper->addMapChangedCB(functor, position);

} // end method addMapChangedCB


AREXPORT void ArMapSimple::remMapChangedCB(ArFunctor *functor)
{ 
  myMapChangedHelper->remMapChangedCB(functor);

} // end method remMapChangedCB

AREXPORT void ArMapSimple::addMapChangedPathPlanningCB(ArFunctor *functor,
                       int position)
{
  myMapChangedHelper->addMapChangedPathPlanningCB(functor, position);

} // end method addMapChangedPathPlanningCB


AREXPORT void ArMapSimple::remMapChangedPathPlanningCB(ArFunctor *functor)
{
  myMapChangedHelper->remMapChangedPathPlanningCB(functor);

} // end method remMapChangedPathPlanningCB

AREXPORT void ArMapSimple::addMapChangedLocalizationCB(ArFunctor *functor,
                       int position)
{
  myMapChangedHelper->addMapChangedLocalizationCB(functor, position);

} // end method addMapChangedLocalizationCB


AREXPORT void ArMapSimple::remMapChangedLocalizationCB(ArFunctor *functor)
{
  myMapChangedHelper->remMapChangedLocalizationCB(functor);

} // end method remMapChangedLocalizationCB

AREXPORT void ArMapSimple::addPreMapChangedCB(ArFunctor *functor,
                                              int position)
{ 
  myMapChangedHelper->addPreMapChangedCB(functor, position);
} // end method addPreMapChangedCB


AREXPORT void ArMapSimple::remPreMapChangedCB(ArFunctor *functor)
{ 
  myMapChangedHelper->remPreMapChangedCB(functor);

} // end method remPreMapChangedCB


AREXPORT void ArMapSimple::setMapChangedLogLevel(ArLog::LogLevel level)
{ 
  myMapChangedHelper->setMapChangedLogLevel(level);

} // end method setMapChangedLogLevel

AREXPORT ArLog::LogLevel ArMapSimple::getMapChangedLogLevel(void)
{ 
  return myMapChangedHelper->getMapChangedLogLevel();

} // end method getMapChangedLogLevel


AREXPORT int ArMapSimple::lock()
{ 
  return myMutex.lock();

} // end method lock

AREXPORT int ArMapSimple::tryLock()
{ 
  return myMutex.tryLock();

} // end method tryLock

AREXPORT int ArMapSimple::unlock()
{ 
  return myMutex.unlock();

} // end method unlock

// ---------------------------------------------------------------------------
// ArMapInfoInterface
// ---------------------------------------------------------------------------

AREXPORT std::list<ArArgumentBuilder *> *ArMapSimple::getInfo(const char *infoName)
{ 
  return myMapInfo->getInfo(infoName);

} // end method getInfo

AREXPORT std::list<ArArgumentBuilder *> *ArMapSimple::getInfo(int infoType)
{ 
  return myMapInfo->getInfo(infoType);

} // end method getInfo

AREXPORT std::list<ArArgumentBuilder *> *ArMapSimple::getMapInfo(void)
{ 
  return myMapInfo->getInfo(ArMapInfo::MAP_INFO_NAME);

} // end method getMapInfo

AREXPORT int ArMapSimple::getInfoCount() const 
{
  return myMapInfo->getInfoCount();
}

AREXPORT std::list<std::string> ArMapSimple::getInfoNames() const
{
  return myMapInfo->getInfoNames();
}

AREXPORT bool ArMapSimple::setInfo(const char *infoName,
						                       const std::list<ArArgumentBuilder *> *infoList,
                                   ArMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(infoName, infoList, changeDetails);

  // updateMapCategory(infoName);

  return b;

} // end method setInfo

AREXPORT bool ArMapSimple::setInfo(int infoType,
						                       const std::list<ArArgumentBuilder *> *infoList,
                                   ArMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(infoType, infoList, changeDetails);
  
  // updateMapCategory(NULL);

  return b;

} // end method setInfo

AREXPORT bool ArMapSimple::setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                      ArMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(ArMapInfo::MAP_INFO_NAME, mapInfo, changeDetails);

  // updateMapCategory(ArMapInfo::MAP_INFO_NAME);

  return b;

} // end method setMapInfo


AREXPORT void ArMapSimple::writeInfoToFunctor
				(ArFunctor1<const char *> *functor, 
			        const char *endOfLineChars)
{ 
  return myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

} // end method writeInfoToFunctor


AREXPORT const char *ArMapSimple::getInfoName(int infoType)
{ 
  return myMapInfo->getInfoName(infoType);

} // end method getInfoName

// ---------------------------------------------------------------------------
// ArMapObjectsInterface
// ---------------------------------------------------------------------------

AREXPORT ArMapObject *ArMapSimple::findFirstMapObject(const char *name, 
                                                      const char *type,
                                                      bool isIncludeWithHeading)
{ 
  return myMapObjects->findFirstMapObject(name, type, isIncludeWithHeading);

} // end method findFirstMapObject


AREXPORT ArMapObject *ArMapSimple::findMapObject(const char *name, 
				                                         const char *type,
                                                 bool isIncludeWithHeading)
{ 
  return myMapObjects->findMapObject(name, type, isIncludeWithHeading);

} // end method findMapObject

AREXPORT std::list<ArMapObject *> ArMapSimple::findMapObjectsOfType
                                                (const char *type,
                                                 bool isIncludeWithHeading)
{
  return myMapObjects->findMapObjectsOfType(type, isIncludeWithHeading);
}


AREXPORT std::list<ArMapObject *> *ArMapSimple::getMapObjects(void)
{ 
  return myMapObjects->getMapObjects();

} // end method getMapObjects


AREXPORT void ArMapSimple::setMapObjects
                             (const std::list<ArMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              ArMapChangeDetails *changeDetails)
{
  
  myMapObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);
 
} // end method setMapObjects


AREXPORT void ArMapSimple::writeObjectsToFunctor(ArFunctor1<const char *> *functor, 
			                                           const char *endOfLineChars,
                                                 bool isOverrideAsSingleScan,
                                                 const char *maxCategory)
{
  std::string category = getMapCategory();
  if (maxCategory != NULL) {
    if (strcasecmp(maxCategory, MAP_CATEGORY_2D_TEMPLATE) == 0) {
      category = MAP_CATEGORY_2D_TEMPLATE;
    }
    else if (strcasecmp(maxCategory, MAP_CATEGORY_2D_COMPOSITE) == 0) {
      category = MAP_CATEGORY_2D_COMPOSITE;
    }
    else if (strcasecmp(maxCategory, MAP_CATEGORY_2D_EXTENDED) == 0) {
      category = MAP_CATEGORY_2D_EXTENDED;
    } 
    else if (strcasecmp(maxCategory, MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {
      category = MAP_CATEGORY_2D_MULTI_SOURCES;
    }
    else {
      category = MAP_CATEGORY_2D;
      isOverrideAsSingleScan = true;
    }
  }
  else if (isOverrideAsSingleScan) {
    category = MAP_CATEGORY_2D;
  }

  ArUtil::functorPrintf(functor, "%s%s", 
                        category.c_str(),
                        endOfLineChars);

  if (!isOverrideAsSingleScan) {
    writeScanTypesToFunctor(functor, endOfLineChars);
  }

  if (!isOverrideAsSingleScan) {
                        
    for (std::list<std::string>::iterator iter = myScanTypeList.begin();
         iter != myScanTypeList.end();
         iter++) {
      const char *scanType = (*iter).c_str();
      ArMapScanInterface *mapScan = getScan(scanType);
      if (mapScan != NULL) {
        mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
      }
    } // end for each scan type
  }
  else { // else send single scan

    ArMapScanInterface *mapScan = getScan(ARMAP_SUMMARY_SCAN_TYPE);
    if (mapScan != NULL) {
      mapScan->writeScanToFunctor(functor, 
                                  endOfLineChars, 
                                  ARMAP_SUMMARY_SCAN_TYPE);
    }
  } // end else just send single scan 

  myMapSupplement->writeSupplementToFunctor(functor, endOfLineChars);

  myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

} // end method writeObjectsToFunctor


AREXPORT void ArMapSimple::writeObjectListToFunctor(ArFunctor1<const char *> *functor, 
			                                              const char *endOfLineChars)
{ 
  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

} // end method writeObjectListToFunctor




// ---------------------------------------------------------------------------
// ArMapScanInterface
// ---------------------------------------------------------------------------

AREXPORT const char *ArMapSimple::getDisplayString(const char *scanType)
{
  if (isSummaryScanType(scanType)) {
    // TODO Could return a special "Summary" string instead...
    ArLog::log(ArLog::Terse,
               "ArMapSimple::getDisplayString() summary display is not supported");
    return "";
  }

  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getDisplayString(scanType);
  }
  return "";

} // end method getDisplayString


AREXPORT std::vector<ArPose> *ArMapSimple::getPoints(const char *scanType)
{
  if (isSummaryScanType(scanType)) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::getPoints() summary of points is not supported");
    return NULL;
  }

  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getPoints(scanType);
  }
  return NULL;

} // end method getPoints


AREXPORT std::vector<ArLineSegment> *ArMapSimple::getLines(const char *scanType)
{ 
  if (isSummaryScanType(scanType)) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::getLines() summary of lines is not supported");
    return NULL;
  }
  
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLines(scanType);
  }
  return NULL;

} // end method getLines


AREXPORT ArPose ArMapSimple::getMinPose(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getMinPose(scanType);
  }
  return ArPose();

} // end method getMinPose

AREXPORT ArPose ArMapSimple::getMaxPose(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getMaxPose(scanType);
  }
  return ArPose();

} // end method getMaxPose

AREXPORT int ArMapSimple::getNumPoints(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getNumPoints(scanType);
  }
  return 0;

} // end method getNumPoints

AREXPORT ArPose ArMapSimple::getLineMinPose(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLineMinPose(scanType);
  }
  return ArPose();

} // end method getLineMinPose

AREXPORT ArPose ArMapSimple::getLineMaxPose(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLineMaxPose(scanType);
  }
  return ArPose();

} // end method getLineMaxPose

AREXPORT int ArMapSimple::getNumLines(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getNumLines(scanType);
  }
  return 0;

} // end method getNumLines

AREXPORT int ArMapSimple::getResolution(const char *scanType)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getResolution(scanType);
  }
  return 0;

} // end method getResolution



AREXPORT bool ArMapSimple::isSortedPoints(const char *scanType) const
{
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->isSortedPoints(scanType);
  }
  return false;
}

AREXPORT bool ArMapSimple::isSortedLines(const char *scanType) const
{
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->isSortedLines(scanType);
  }
  return false;
}

AREXPORT void ArMapSimple::setPoints(const std::vector<ArPose> *points,
                                     const char *scanType,
                                     bool isSorted,
                                     ArMapChangeDetails *changeDetails)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setPoints(points, 
                       scanType,
                       isSorted, 
                       changeDetails);
  }

} // end method setPoints

AREXPORT void ArMapSimple::setLines(const std::vector<ArLineSegment> *lines,
                                    const char *scanType,
                                    bool isSorted,
                                    ArMapChangeDetails *changeDetails)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setLines(lines, 
                      scanType,
                      isSorted, 
                      changeDetails);
  }

} // end method setLines

AREXPORT void ArMapSimple::setResolution(int resolution,
                                         const char *scanType,
                                         ArMapChangeDetails *changeDetails)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setResolution(resolution, 
                           scanType,
                           changeDetails);
  }
} // end method setResolution




AREXPORT void ArMapSimple::writeScanToFunctor(ArFunctor1<const char *> *functor, 
			                                        const char *endOfLineChars,
                                              const char *scanType)
{
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
  }

} // end method writeScanToFunctor


AREXPORT void ArMapSimple::writePointsToFunctor
		(ArFunctor2<int, std::vector<ArPose> *> *functor,
     const char *scanType,
     ArFunctor1<const char *> *keywordFunctor)
{
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writePointsToFunctor(functor, scanType, keywordFunctor);
  }

} // end method writePointsToFunctor

AREXPORT void ArMapSimple::writeLinesToFunctor
	(ArFunctor2<int, std::vector<ArLineSegment> *> *functor,
   const char *scanType,
   ArFunctor1<const char *> *keywordFunctor)
{ 
  ArMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writeLinesToFunctor(functor, scanType, keywordFunctor);
  }

} // end method writeLinesToFunctor


AREXPORT bool ArMapSimple::readDataPoint( char *line)
{
  // TODO Locking?
  if (myLoadingScan != NULL) {
    return myLoadingScan->readDataPoint(line);
  }
  return false;

} // end method readDataPoint

AREXPORT bool ArMapSimple::readLineSegment( char *line)
{
  if (myLoadingScan != NULL) {
    return myLoadingScan->readLineSegment(line);
  }
  else {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::readLineSegment() NULL loading scan for '%s'",
               line);
  }
  return false;

} // end method readLineSegment


AREXPORT void ArMapSimple::loadDataPoint(double x, double y)
{
  if (myLoadingScan != NULL) {
    myLoadingScan->loadDataPoint(x, y);
  }

} // end method loadDataPoint


AREXPORT void ArMapSimple::loadLineSegment(double x1, double y1, double x2, double y2)
{
  if (myLoadingScan != NULL) {
    myLoadingScan->loadLineSegment(x1, y1, x2, y2);
  }
} // end method loadLineSegment


AREXPORT bool ArMapSimple::addToFileParser(ArFileParser *fileParser)
{
  ArLog::log(ArLog::Verbose, 
             "ArMapSimple::addToFileParser: called");

  if (myTypeToScanMap.empty()) {
    ArLog::log(ArLog::Terse,
               "ArMapSimple::addToFileParser() error: no scans in map");
    return false;
  }
  bool isAdded = true;

  for (ArTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end(); iter++) {
    ArMapScan *mapScan = iter->second;
    if (mapScan != NULL) {
      isAdded = mapScan->addToFileParser(fileParser) && isAdded;
    }
  }
  return isAdded; 
}

AREXPORT bool ArMapSimple::remFromFileParser(ArFileParser *fileParser)
{
  if (myTypeToScanMap.empty()) {
    ArLog::log(ArLog::Verbose, 
               "ArMapSimple::remFromFileParser: called");
    return false;
  }
  bool isRemoved = true;

  for (ArTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end(); iter++) {
    ArMapScan *mapScan = iter->second;
    if (mapScan != NULL) {
      isRemoved = mapScan->remFromFileParser(fileParser) && isRemoved;
    }
  }
  return isRemoved; 
}


AREXPORT void ArMapSimple::writeScanTypesToFunctor
                                (ArFunctor1<const char *> *functor, 
			                           const char *endOfLineChars)
{
  bool hasSourceList = false;
  if (!myScanTypeList.empty()) {
    hasSourceList = !ArUtil::isStrEmpty(myScanTypeList.front().c_str());
  }
  if (hasSourceList) {
  
    std::string sourceString = "Sources:";

    //ArUtil::functorPrintf(functor, "Sources:%s", "");
    
    for (std::list<std::string>::iterator iter = myScanTypeList.begin(); 
         iter != myScanTypeList.end(); 
         iter++) {

      const char *scanType = (*iter).c_str();
      sourceString += " ";
      sourceString += scanType;

     // ArUtil::functorPrintf(functor, " %s%s", scanType, "");
      
    } // end for each scan type
    
    ArUtil::functorPrintf(functor, "%s%s", sourceString.c_str(), endOfLineChars);

  } // end if source list

} // end method writeScanTypesToFunctor


AREXPORT bool ArMapSimple::hasOriginLatLongAlt()
{ 
  return myMapSupplement->hasOriginLatLongAlt();

} // end method hasOriginLatLongAlt

AREXPORT ArPose ArMapSimple::getOriginLatLong()
{ 
  return myMapSupplement->getOriginLatLong();

} // end method getOriginLatLong

AREXPORT double ArMapSimple::getOriginAltitude()
{ 
  return myMapSupplement->getOriginAltitude();

} // end method getOriginAltitude

AREXPORT void ArMapSimple::setOriginLatLongAlt
                                  (bool hasOriginLatLong,
                                   const ArPose &originLatLong,
                                   double originAltitude,
                                   ArMapChangeDetails *changeDetails)
{
  myMapSupplement->setOriginLatLongAlt(hasOriginLatLong, 
                                       originLatLong, 
                                       originAltitude, 
                                       changeDetails);
} // end method setOriginLatLongAlt

AREXPORT void ArMapSimple::writeSupplementToFunctor(ArFunctor1<const char *> *functor, 
			                                              const char *endOfLineChars)
{
  myMapSupplement->writeSupplementToFunctor(functor, 
                                            endOfLineChars);


} // end method writeSupplementToFunctor



// ---------------------------------------------------------------------------

AREXPORT void ArMapSimple::writeToFunctor(ArFunctor1<const char *> *functor, 
			                                    const char *endOfLineChars)
{ 
  // Write the header information and Cairn objects...
  ArUtil::functorPrintf(functor, "%s%s", 
                        getMapCategory(),
                        endOfLineChars);

  std::list<std::string>::iterator iter = myScanTypeList.end();
  
  writeScanTypesToFunctor(functor, endOfLineChars);

  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    ArMapScan *mapScan = getScan(scanType);

    if (mapScan != NULL) {
      mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
    }
  }

  myMapSupplement->writeSupplementToFunctor(functor, endOfLineChars);

  myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

  myInactiveInfo->writeInfoToFunctor(functor, endOfLineChars);

  myInactiveObjects->writeObjectListToFunctor(functor, endOfLineChars);

  myChildObjects->writeObjectListToFunctor(functor, endOfLineChars);

  // Write out any unrecognized (remainder) lines -- just to try to prevent them
  // from being accidentally lost

  for (std::list<ArArgumentBuilder*>::const_iterator remIter = myRemainderList.begin();
       remIter != myRemainderList.end();
       remIter++) {
    ArArgumentBuilder *remArg = *remIter;
    if (remArg == NULL) {
      continue;
    }
    ArUtil::functorPrintf(functor, "%s%s", 
                          remArg->getFullString(),
                          endOfLineChars);

  } // end for each remainder line


  // Write the lines...
  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    ArMapScan *mapScan = getScan(scanType);
    
    if (mapScan != NULL) {
      mapScan->writeLinesToFunctor(functor, endOfLineChars, scanType);
    }
  }

  // Write the points...
  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    ArMapScan *mapScan = getScan(scanType);
    
    if (mapScan != NULL) {
      mapScan->writePointsToFunctor(functor, endOfLineChars, scanType);
    }
  } 

} // end method writeToFunctor


AREXPORT ArMapInfoInterface *ArMapSimple::getInactiveInfo()
{
  return myInactiveInfo;
}

AREXPORT ArMapObjectsInterface *ArMapSimple::getInactiveObjects()
{
  return myInactiveObjects;
}

AREXPORT ArMapObjectsInterface *ArMapSimple::getChildObjects()
{
  return myChildObjects;
}

AREXPORT bool ArMapSimple::parseLine(char *line)
{ 
  return myLoadingParser->parseLine(line);

} // end method parseLine

AREXPORT void ArMapSimple::parsingComplete(void)
{ 
  lock();
  mapChanged();
  unlock();

} // end method parsingComplete


AREXPORT bool ArMapSimple::isLoadingDataStarted()
{ 
  return myLoadingDataStarted;

} // end method  isLoadingDataStarted


AREXPORT bool ArMapSimple::isLoadingLinesAndDataStarted()
{ 
  return myLoadingLinesAndDataStarted;

} // end method isLoadingLinesAndDataStarted
            

std::string ArMapSimple::createRealFileName(const char *fileName)
{ 
  return ArMapInterface::createRealFileName(myBaseDirectory.c_str(),
                                            fileName,
                                            myIgnoreCase);

} // end method createRealFileName

bool ArMapSimple::handleMapCategory(ArArgumentBuilder *arg)
{ 
  ArLog::log(ArLog::Verbose, 
             "ArMapSimple::handleMapCategory() read category %s",
             arg->getExtraString());


  if (!addScansToParser() || 
      !myLoadingParser->addHandler("Sources:", &mySourcesCB) ||
      !myMapInfo->addToFileParser(myLoadingParser) ||
      !myMapSupplement->addToFileParser(myLoadingParser) ||
      !myMapObjects->addToFileParser(myLoadingParser) ||
      !myInactiveInfo->addToFileParser(myLoadingParser) ||
      !myInactiveObjects->addToFileParser(myLoadingParser) ||
      !myChildObjects->addToFileParser(myLoadingParser) ||
      // Add a handler for unrecognized lines...
      !myLoadingParser->addHandler(NULL, &myRemCB)) 
  {
    ArLog::log(ArLog::Terse, 
               "ArMapSimple::handleMapCategory: could not add handlers");
    return false;
  }  
  
  // If all of the parsers were successfully added, then remove the map 
  // category handlers and return
 
  myMapCategory = "";
  for (std::list<std::string>::iterator iter = myMapCategoryList.begin();
       iter != myMapCategoryList.end();
       iter++) {
    if (strncasecmp(arg->getExtraString(), 
                    (*iter).c_str(),
                    (*iter).length()) == 0) {
      myMapCategory = *iter;
    }
  
    myLoadingParser->remHandler((*iter).c_str());

  } // end for each category

  if (myMapCategory.empty()) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::handleMapCategory() error finding category for %s",
               arg->getExtraString());
  }
  
  myLoadingGotMapCategory = true;

  return true;

} // end method handleMapCategory
  
  
bool ArMapSimple::handleSources(ArArgumentBuilder *arg)
{
  ArLog::log(ArLog::Verbose,
            "ArMapSimple::handleSources() called");
   
  std::list<std::string> scanTypeList;

  for (size_t i = 0; i < arg->getArgc(); i++) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::handleSources() source #%i = %s",
               i, arg->getArg(i));
    scanTypeList.push_back(arg->getArg(i));
  }

  if (scanTypeList.empty()) {
    ArLog::log(ArLog::Terse,
              "ArMapSimple::handleSources() at least one source must be specified");
    return false;
  }

  remScansFromParser(true);

  createScans(scanTypeList);

  addScansToParser();

  return true;

} // end method handleSources


bool ArMapSimple::createScans(const std::list<std::string> &scanTypeList)
{
  if (scanTypeList.empty()) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::createScans() scan type list must be non-empty");

    return false;
  }

  bool isListValid = true;

  // Perform some simple validations on the scan type list...
  if (scanTypeList.size() > 1) {

    std::map<std::string, bool, ArStrCaseCmpOp> typeToExistsMap;

    // Make sure that none of the scan types are empty
    for (std::list<std::string>::const_iterator iter1 = scanTypeList.begin();
        iter1 != scanTypeList.end();
        iter1++) {
      const char *scanType = (*iter1).c_str();
      if (ArUtil::isStrEmpty(scanType)) {
        isListValid = false;
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::createScans() empty scan name is valid only when there is one scan type");
        break;
      }
    
      // Make sure that there are no duplicates...
      std::map<std::string, bool, ArStrCaseCmpOp>::iterator tIter = 
                                                typeToExistsMap.find(scanType);
      if (tIter != typeToExistsMap.end()) {
        isListValid = false;
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::createScans() duplicate scan names are not allowed (%s)",
                   scanType);
        break;
      }

      typeToExistsMap[scanType] = true;

    } // end for each scan type
  } // end if more than one entry

  if (!isListValid) {

    ArLog::log(ArLog::Terse,
                "ArMapSimple error setting up map for multiple scan types");
    return false;

  }

  
  delete mySummaryScan;
  mySummaryScan = NULL;

  myScanTypeList.clear();
  ArUtil::deleteSetPairs(myTypeToScanMap.begin(), myTypeToScanMap.end());
  myTypeToScanMap.clear();

  ArMapScan *mapScan = NULL;

  for (std::list<std::string>::const_iterator iter = scanTypeList.begin();
       iter != scanTypeList.end();
       iter++) {
    std::string scanType = *iter;

    mapScan = new ArMapScan(scanType.c_str());
  
    myScanTypeList.push_back(scanType);
    myTypeToScanMap[scanType] = mapScan;

  } // end for each scan type

  if (myScanTypeList.size() > 1) {
    mySummaryScan = new ArMapScan(ARMAP_SUMMARY_SCAN_TYPE);
  }
  return true;

} // end method createScans


bool ArMapSimple::addScansToParser()
{
  if (myLoadingParser == NULL) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::addScansToParser() error, loading parser is null");
    return false;
  }
  if (myTypeToScanMap.empty()) {
    ArLog::log(ArLog::Normal,
               "ArMapSimple::addScansToParser() error, no maps scans");
    return false;
  }

  bool isLoaded = true;
  
  for (ArTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {

    ArMapScan *mapScan = iter->second;
    if (mapScan == NULL) {
      continue;
    }
    if (!mapScan->addToFileParser(myLoadingParser)) {
      ArLog::log(ArLog::Normal,
                 "ArMapSimple::addScansToParser() error, could not add scan for %s",
                 iter->first.c_str());
      isLoaded = false;
      continue;
    }

    ArLog::log(ArLog::Verbose,
               "ArMapSimple::addScansToParser() adding for type %s points keyword %s lines keyword %s",
               mapScan->getScanType(),
               mapScan->getPointsKeyword(),
               mapScan->getLinesKeyword());
    
    if (!ArUtil::isStrEmpty(mapScan->getPointsKeyword())) {
      
      myDataTagToScanTypeMap[mapScan->getPointsKeyword()] = iter->first;
      if (!myLoadingParser->addHandler(mapScan->getPointsKeyword(), 
                                       &myDataIntroCB)) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::addScansToParser() error, could not handler for %s",
                   mapScan->getPointsKeyword());
        isLoaded = false;
      }
    }
    if (!ArUtil::isStrEmpty(mapScan->getLinesKeyword())) {
      myDataTagToScanTypeMap[mapScan->getLinesKeyword()] = iter->first;
      if (!myLoadingParser->addHandler(mapScan->getLinesKeyword(), 
                                       &myDataIntroCB)) {
        ArLog::log(ArLog::Normal,
                   "ArMapSimple::addScansToParser() error, could not handler for %s",
                   mapScan->getLinesKeyword());
        isLoaded = false;
      }
    }
  } // end for each scan

  return isLoaded;

} // end method addScansToParser


bool ArMapSimple::remScansFromParser(bool isRemovePointsAndLinesKeywords)
{
  if (myLoadingParser == NULL) {
    return false;
  }
  if (myTypeToScanMap.empty()) {
    return false;
  }
  bool isRemoved = true;

  for (ArTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    ArMapScan *mapScan = iter->second;
    if (mapScan == NULL) {
      continue;
    }
    if (!mapScan->remFromFileParser(myLoadingParser)) {
      isRemoved = false;
      continue;
    }
   
    if (isRemovePointsAndLinesKeywords) {
      if (!ArUtil::isStrEmpty(mapScan->getPointsKeyword())) {
        if (!myLoadingParser->remHandler(mapScan->getPointsKeyword(), 
                                        &myDataIntroCB)) {
          isRemoved = false;
        }
      }
      if (!ArUtil::isStrEmpty(mapScan->getLinesKeyword())) {
        if (!myLoadingParser->remHandler(mapScan->getLinesKeyword(), 
                                        &myDataIntroCB)) {
          isRemoved = false;
        }
      }
    } // end if remove points and lines keywords
  } // end for each scan

  return isRemoved;

} // end method remScansFromParser


bool ArMapSimple::handleDataIntro(ArArgumentBuilder *arg)
{
  remScansFromParser(false);

  myMapSupplement->remFromFileParser(myLoadingParser);
  myMapInfo->remFromFileParser(myLoadingParser);
  myMapObjects->remFromFileParser(myLoadingParser);

  myInactiveInfo->remFromFileParser(myLoadingParser);
  myInactiveObjects->remFromFileParser(myLoadingParser);
  myChildObjects->remFromFileParser(myLoadingParser);

  myLoadingParser->remHandler("Sources:");

  // Remove the remainder handler
  myLoadingParser->remHandler((const char *)NULL);

  // All of the info types have been read by now... If there is
  // an extended one, then update the map's category.
  updateMapCategory();
  
  ArLog::log(ArLog::Verbose,
             "ArMapSimple::handleDataIntro %s",
             arg->getExtraString());

  // The "extra string" contains the keyword - in all lowercase 
  if (arg->getExtraString() != NULL) {
    myLoadingDataTag = arg->getExtraString();
  }
  else {
    myLoadingDataTag = "";
  }
  
  // Need to set the myLoadingScan so that calls from ArQ to loadDataPoint
  // and loadDataLine are processed correctly.  In addition, the findScan...
  // method sets the myLoadingData / myLoadingLinesAndData attributes --
  // which are also needed by ArQ
  bool isLineDataTag = false;

  if (myLoadingScan != NULL) {
    myLoadingScan->remExtraFromFileParser(myLoadingParser);
  }

  myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                          &isLineDataTag);

  if (myLoadingScan != NULL) {
    myLoadingScan->addExtraToFileParser(myLoadingParser, isLineDataTag);
  }

  return false;

} // end method handleDataIntro


bool ArMapSimple::handleRemainder(ArArgumentBuilder *arg)
{
  if (arg != NULL) {
    myRemainderList.push_back(new ArArgumentBuilder(*arg));
  }
  return true;

} // end method handleRemainder


bool ArMapSimple::setInactiveInfo(const char *infoName,
						                               const std::list<ArArgumentBuilder *> *infoList,
                                           ArMapChangeDetails *changeDetails)
{ 
  return myInactiveInfo->setInfo(infoName, infoList, changeDetails);

} // end method setInactiveInfo

void ArMapSimple::setInactiveObjects
                             (const std::list<ArMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              ArMapChangeDetails *changeDetails)
{ 
  myInactiveObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);

} // end method setInactiveObjects
 

void ArMapSimple::setChildObjects
                             (const std::list<ArMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              ArMapChangeDetails *changeDetails)
{ 
  myChildObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);

} // end method setChildObjects



AREXPORT ArMapScan *ArMapSimple::getScan(const char *scanType) const
{
  // The summary scan type is a special designation that allows the user
  // to return the total number of points in the map, the bounding box of
  // all the scans, etc.  If there are multiple scan types in the map, then
  // the mySummaryScan member is created.  Otherwise, the summary is the
  // same as the single scan.
  if (isSummaryScanType(scanType)) {
    if (mySummaryScan != NULL) {
      return mySummaryScan;
    }
    else {
      scanType = ARMAP_DEFAULT_SCAN_TYPE;
    }
  } // end if summary scan type

  ArTypeToScanMap::const_iterator iter = myTypeToScanMap.find(scanType);
  
  // If the specified scan type was not found, then see if the special 
  // ARMAP_DEFAULT_SCAN_TYPE was specified.  If so, this is equivalent to the
  // first scan in the scan list.
  if ((iter == myTypeToScanMap.end()) &&
      (isDefaultScanType(scanType)) &&
      (!myScanTypeList.empty())) {
    scanType = myScanTypeList.front().c_str();
    iter = myTypeToScanMap.find(scanType);
  }

  if (iter != myTypeToScanMap.end()) {
    return iter->second;
  }
 
  return NULL;

} // end method getScan




