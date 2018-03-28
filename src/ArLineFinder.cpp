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
#include "ArLineFinder.h"
#include "ArConfig.h"

AREXPORT ArLineFinder::ArLineFinder(ArRangeDevice *rangeDevice) 
{
  myRangeDevice = rangeDevice;
  myPrinting = false;
  myPoints = NULL;
  myLines = NULL;  
  myNonLinePoints = NULL;
  myFlippedFound = false;

  mySinMultiplier = (ArMath::sin(1) / ArMath::sin(5));

  setLineCreationParams();
  setLineCombiningParams();
  setLineFilteringParams();
  setLineValidParams();
  setMaxDistBetweenPoints();
}

AREXPORT ArLineFinder::~ArLineFinder()
{

}

AREXPORT std::map<int, ArLineFinderSegment *> *ArLineFinder::getLines(void)
{
  // fill the laser readings into myPoints
  fillPointsFromLaser();
  // make lines out of myPoints into myLines
  findLines();
  // put the lines from myLines into combined lines
  // this will recurse itself until there are no more
  combineLines();
  // now filter out the short lines
  filterLines();
  // combines the lines again
  combineLines();
  return myLines;
}

AREXPORT std::map<int, ArPose> *ArLineFinder::getNonLinePoints(void)
{
  std::map<int, ArLineFinderSegment *>::iterator lineIt;
  ArLineFinderSegment *segment;
  int i;

  getLines();

  if (myLines == NULL)
    return NULL;

  if (myNonLinePoints != NULL)
    delete myNonLinePoints;

  myNonLinePoints = new std::map<int, ArPose>;

  *myNonLinePoints = *myPoints;

  for (lineIt = myLines->begin(); lineIt != myLines->end(); lineIt++)
  {
    segment = (*lineIt).second;
    for (i = segment->getStartPoint(); i <= segment->getEndPoint(); i++)
    {
      myNonLinePoints->erase(i);
    }
  }
  return myNonLinePoints;
}


AREXPORT void ArLineFinder::fillPointsFromLaser(void)
{
  const std::list<ArSensorReading *> *readings;
  std::list<ArSensorReading *>::const_iterator it;
  std::list<ArSensorReading *>::const_reverse_iterator rit;
  ArSensorReading *reading;
  int pointCount = 0;

  if (myPoints != NULL)
    delete myPoints;

  myPoints = new std::map<int, ArPose>;
  
  myRangeDevice->lockDevice();
  readings = myRangeDevice->getRawReadings();

  if (!myFlippedFound)
  {
    if (readings->begin() != readings->end())
    {
      int size;
      size = readings->size();
      it = readings->begin();
      // advance along 10 readings
      for (int i = 0; i < 10 && i < size / 2; i++)
	it++;
      // see if we're flipped
      if (ArMath::subAngle((*(readings->begin()))->getSensorTh(), 
			   (*it)->getSensorTh()) > 0)
	myFlipped = true;
      else
	myFlipped = false;
      myFlippedFound = true;
      //printf("@@@ LINE %d %.0f\n", myFlipped, ArMath::subAngle((*(readings->begin()))->getSensorTh(), (*it)->getSensorTh()));

      
    }
  }




  if (readings->begin() == readings->end())
  {
    myRangeDevice->unlockDevice();
    return;
  }
  myPoseTaken = (*readings->begin())->getPoseTaken();

  if (myFlipped)
  {
    for (rit = readings->rbegin(); rit != readings->rend(); rit++)
    {
      reading = (*rit);
      if (reading->getRange() > 5000 || reading->getIgnoreThisReading())
	continue;
      (*myPoints)[pointCount] = reading->getPose();
      pointCount++;
    }
  }
  else
  {
    for (it = readings->begin(); it != readings->end(); it++)
    {
      reading = (*it);
      if (reading->getRange() > 5000 || reading->getIgnoreThisReading())
	continue;
      (*myPoints)[pointCount] = reading->getPose();
      pointCount++;
    }
  }
  myRangeDevice->unlockDevice();
}

AREXPORT void ArLineFinder::findLines(void)
{
  int start = 0;
  int pointsLen = myPoints->size();
  int end;

  if (myLines != NULL)
  {
    ArUtil::deleteSetPairs(myLines->begin(), myLines->end());
    delete myLines;
    myLines = NULL;
  }
  myLines = new std::map<int, ArLineFinderSegment *>;
  int numLines = 0;

  FILE *lineFile = NULL;
  /*
  if ((lineFile = ArUtil::fopen("firstLines", "w+")) == NULL)
  {
    printf("Couldn't open 'lines' for writing\n");
    return;
  }
  */
  ArLineFinderSegment *newLine;
  double totalDistFromLine = 0;
  double dist;
  int i;
  bool maxDistTriggered;

  while (1)
  {
    maxDistTriggered = false;
    // first we try to find the first place we'll check for lines
    // move out from the start as far as we should for the first one
    for (end = start; ; end++)
    {
      // if we hit the end stop
      if (end >= pointsLen)
	break;
      // if we've moved at least two spots AND at least 50 mm then go
      if (end - start >= myMakingMinPoints && 
  (*myPoints)[start].findDistanceTo((*myPoints)[end]) > myMakingMinLen)
	break;
      // if the distance between any of the points is too great than
      // break (to try and get rid of spots where a laser spot half
      // way between things hurts us)
      if (myMaxDistBetweenPoints > 0 && end > start && 
	  ((*myPoints)[end-1].findDistanceTo((*myPoints)[end]) > 
	   myMaxDistBetweenPoints))
      {
	maxDistTriggered = true;
	break;
      }
    } 
    if (end < pointsLen)
    {
      // if the distance between any of the points is too great don't
      // make a line out of it (to try and get rid of spots where a
      // laser spot half way between things hurts us)
      if (maxDistTriggered)
      {
	if (myPrinting)
	  ArLog::log(ArLog::Normal, "too great a distance between some points on the line %d %d", start, end);
      }
      // see if its too far between these line segments
      else if ((*myPoints)[start].findDistanceTo((*myPoints)[end]) <
	       ((*myPoints)[start].findDistanceTo(myPoseTaken) * mySinMultiplier))
      {
	if (lineFile != NULL)
	  fprintf(lineFile, "%.0f %.0f %.0f %.0f\n",
		  (*myPoints)[start].getX(), (*myPoints)[start].getY(),
		  (*myPoints)[end].getX() - (*myPoints)[start].getX(),
		  (*myPoints)[end].getY() - (*myPoints)[start].getY());
	
	newLine = new ArLineFinderSegment(
		(*myPoints)[start].getX(), 
		(*myPoints)[start].getY(),
		(*myPoints)[end].getX(),
		(*myPoints)[end].getY(), 
		1, start, end);
	
	totalDistFromLine = 0;
	// Make sure none of the points are too far away from the new line
	for (i = newLine->getStartPoint(); i <= newLine->getEndPoint(); i++)
	{
	  dist = newLine->getDistToLine((*myPoints)[i]);
	  totalDistFromLine += dist;
	}
	newLine->setAveDistFromLine(totalDistFromLine / (end - start));
	
	(*myLines)[numLines] = newLine;
	numLines++;
      }
      else
      {
	if (myPrinting)
	  ArLog::log(ArLog::Normal, "too great a distance between the two line points %d %d", start, end);
      }
    }
    
    start += 1;
    if (start >= pointsLen)
      break;
  }
  
  if (lineFile != NULL)
    fclose(lineFile);
}

AREXPORT bool ArLineFinder::combineLines(void)
{
  int start = 0;
  int len = myLines->size();
  // this is the min line distance
  std::map<int, ArLineFinderSegment *> *newLines;
  int numNewLines = 0;
  int numNewMerges = 0;
  ArLineFinderSegment *newLine;
  
  newLines = new std::map<int, ArLineFinderSegment *>;

  if (myPrinting)
    ArLog::log(ArLog::Normal, "new iteration\n");
  
  bool nextMerged = false;
  for (start = 0; start < len; start++)
  {
    if (nextMerged)
    {
      nextMerged = false;
      continue;
    }

    if (start + 1 == len)
    {
      if (myPrinting)
	ArLog::log(ArLog::Normal, "inserted last one %g",
 	     ArPose((*myLines)[start]->getX1(), 
		    (*myLines)[start]->getY1()).findDistanceTo(
		  ArPose((*myLines)[start]->getX2(), (*myLines)[start]->getY2())));
      (*newLines)[numNewLines] = new ArLineFinderSegment(*((*myLines)[start]));
      numNewLines++;
      continue;
    }

    newLine = averageSegments((*myLines)[start], (*myLines)[start+1]);
    if (newLine != NULL)
    {
      
      if (myPrinting)
	ArLog::log(ArLog::Normal, "merged %g %g to %g", 
		   (*myLines)[start]->getLength(),
		   (*myLines)[start+1]->getLength(),
		   newLine->getLength());
      (*newLines)[numNewLines] = newLine;
      numNewLines++;
      numNewMerges++;
      nextMerged = true;
    }
    else
    {
      if (myPrinting)
	ArLog::log(ArLog::Normal, "inserted anyways %g", 
		   (*myLines)[start]->getLength());
      (*newLines)[numNewLines] = new ArLineFinderSegment(*((*myLines)[start]));
      numNewLines++;
    }
    
  }

  // move the new lines over and delete the old ones
  if (myLines != NULL && myLines->begin() != myLines->end())
  {
    ArUtil::deleteSetPairs(myLines->begin(), myLines->end());
    delete myLines;
    myLines = NULL;
  } 
  else if (myLines != NULL)
  {  
    delete myLines;
    myLines = NULL;
  }
  myLines = newLines;
  // if we didn't merge any just return
  if (numNewMerges == 0)
    return true;
  
  // otherwise do it again
  return combineLines();
}

AREXPORT ArLineFinderSegment *ArLineFinder::averageSegments(
	ArLineFinderSegment *line1,
	ArLineFinderSegment *line2)
{

  // the angles can be myCombiningAngleTol diff but if its more than myCombiningAngleTol / 2
  // then the resulting line angle should be between the other two
  if (myPrinting)
    ArLog::log(ArLog::Normal,
	       "%3.0f %5.0f    %3.0f %3.0f      (%5.0f %5.0f) <%d %d> (%5.0f %5.0f) <%d %d>", 
	   ArMath::subAngle(line1->getLineAngle(),
			    line2->getLineAngle()),
	       line1->getEndPoint2().findDistanceTo(line2->getEndPoint1()),
	       line1->getLineAngle(), line2->getLineAngle(),
	       line1->getX2(), line1->getY2(),
	       line1->getStartPoint(), line1->getEndPoint(),
	       line2->getX1(), line2->getY1(),
	       line2->getStartPoint(), line2->getEndPoint());

  if (myMaxDistBetweenPoints > 0 && 
      (line1->getEndPoint2().findDistanceTo(line2->getEndPoint1()) > 
       myMaxDistBetweenPoints))
  {
    if (myPrinting)
      ArLog::log(ArLog::Normal, 
		 "distance between the two line end points greater than maxDistBetweenPoints");
    return NULL;
  }

  // see if its too far between these line segments
  if (line1->getEndPoint2().findDistanceTo(line2->getEndPoint1()) >
      line1->getEndPoint2().findDistanceTo(myPoseTaken) * mySinMultiplier)
  {
    if (myPrinting)
      ArLog::log(ArLog::Normal, 
		 "too great a distance between the two line points");
    return NULL;
  }
  // make sure they're pointing in the same direction at least
  double angleOff;
  if ((angleOff = ArMath::fabs(
	  ArMath::subAngle(line1->getLineAngle(),
			   line2->getLineAngle()))) > myCombiningAngleTol)
  {
    if (myPrinting)
      ArLog::log(ArLog::Normal, "greater than angle tolerance");
    return NULL;	
  }

  ArPose endPose2(line2->getX2(), line2->getY2());
  ArPose intersection1;
  ArLine line1Line(*(line1->getLine()));
  ArLine perpLine1;

  // make sure that the lines are close to each other
  line1Line.makeLinePerp(&endPose2, &perpLine1);
  if (!line1Line.intersects(&perpLine1, &intersection1) ||
      intersection1.findDistanceTo(endPose2) > myCombiningLinesCloseEnough)
  {
    //printf("e1 %d %.0f\n", line1Line.intersects(&perpLine1, &intersection1), intersection1.findDistanceTo(endPose2));
    
    if (myPrinting)
      ArLog::log(ArLog::Normal, "endPose2 too far from line1");
    return NULL;
  }

  ArPose endPose1(line1->getX1(), line1->getY1());
  ArPose intersection2;
  ArLine line2Line(*(line2->getLine()));
  ArLine perpLine2;


  // make sure that the lines are close to each other
  line2Line.makeLinePerp(&endPose1, &perpLine2);
  if (!line2Line.intersects(&perpLine2, &intersection2) ||
      intersection2.findDistanceTo(endPose1) > myCombiningLinesCloseEnough)
  {
    //printf("e2 %d %.0f\n", line2Line.intersects(&perpLine2, &intersection2), 	   intersection2.findDistanceTo(endPose1));
    if (myPrinting)
      ArLog::log(ArLog::Normal, "endPose1 too far from line2");
    return NULL;
  }



  ArLineFinderSegment *newLine;
  /*
  newLine = new ArLineFinderSegment((endPose1.getX() + intersection2.getX()) / 2,
			      (endPose1.getY() + intersection2.getY()) / 2,
			      (endPose2.getX() + intersection1.getX()) / 2,
			      (endPose2.getY() + intersection1.getY()) / 2,
			      line1->getCounter() + line2->getCounter());
  */
  // make the new line so that it averages the position based on how
  // many points are in each line
  int l1C = line1->getNumPoints();
  int l2C = line2->getNumPoints();
  newLine = new ArLineFinderSegment((endPose1.getX() * l1C +
				     intersection2.getX() * l2C) / (l1C + l2C),
				    (endPose1.getY() * l1C + 
				     intersection2.getY() * l2C) / (l1C + l2C),
				    (endPose2.getX() * l2C +
				     intersection1.getX() * l1C) / (l1C + l2C),
				    (endPose2.getY() * l2C +
				     intersection1.getY() * l1C) / (l1C + l2C),
				    (line1->getNumPoints() + 
				     line2->getNumPoints()),
				    line1->getStartPoint(), 
				    line2->getEndPoint());

  //printf("%d %d\n", newLine->getStartPoint(), newLine->getEndPoint());
  double totalDistFromLine = 0;
  double dist;
  int i;
  // Make sure none of the points are too far away from the new line
  for (i = newLine->getStartPoint(); i <= newLine->getEndPoint(); i++)
  {
    if ((dist = newLine->getDistToLine((*myPoints)[i])) > 
	myValidMaxDistFromLine && 
	i != newLine->getStartPoint() &&
	i != newLine->getEndPoint())
    {
      if (myPrinting)
	ArLog::log(ArLog::Normal, 
		   "Had a point %d that was to far from our line at %.0f (max %d)",
		   i, dist, myValidMaxDistFromLine);

      delete newLine;
      return NULL;
    }
    //printf("d %.0f\n", dist);
    totalDistFromLine += dist;
  }
  newLine->setAveDistFromLine(totalDistFromLine / (newLine->getEndPoint() - newLine->getStartPoint()));

  //printf("ave dist %.3f\n", newLine->getAveDistFromLine());
  if (newLine->getAveDistFromLine() > myValidMaxAveFromLine)
  {
    if (myPrinting)
      ArLog::log(ArLog::Normal, 
		 "Ave dist from line was too great at %.0f (max %d)",
		 newLine->getAveDistFromLine(), myValidMaxDistFromLine);
    
    delete newLine;
    return NULL;
  }
  if (newLine->getAveDistFromLine() > (line1->getAveDistFromLine() + 
				       line2->getAveDistFromLine()) * 1.25)
  {
    if (myPrinting)
      ArLog::log(ArLog::Normal, 
		 "Ave dist from line greater than component lines at %.0f (component lines %.0f %.0f)",
		 newLine->getAveDistFromLine(), 
		 line1->getAveDistFromLine(), 
		 line2->getAveDistFromLine());
    
    delete newLine;
    return NULL;

  }
  // if we're in myCombiningAngleTol / 2 then its close enough
  if (angleOff < myCombiningAngleTol / 2)
    return newLine;

  // if the new angle is in between the two lines and within myCombiningAngleTol we're ok
  if ((ArMath::subAngle(newLine->getLineAngle(), line2->getLineAngle()) > 0 &&
       ArMath::subAngle(line1->getLineAngle(), newLine->getLineAngle()) > 0) ||
      (ArMath::subAngle(newLine->getLineAngle(), line1->getLineAngle()) > 0 &&
       ArMath::subAngle(line2->getLineAngle(), newLine->getLineAngle()) > 0))
    return newLine;
  
  //printf("%g\n", newLine->getLineAngle());
  if (myPrinting)
    ArLog::log(ArLog::Normal, "angles wonky");
  // if we got down here hte line didn't work
  delete newLine;
  return NULL; 
}

AREXPORT void ArLineFinder::filterLines(void)
{
  int start = 0;
  int len = myLines->size();

  // this is the min line distance
  std::map<int, ArLineFinderSegment *> *newLines;
  int numNewLines = 0;

  newLines = new std::map<int, ArLineFinderSegment *>;

  if (myPrinting)
    ArLog::log(ArLog::Normal, "filtering lines\n");
  
  for (start = 0; start < len; start++)
  {
    if ((*myLines)[start]->getNumPoints() >= myFilteringMinPointsInLine &&
	(*myLines)[start]->getEndPoint1().findDistanceTo(
		(*myLines)[start]->getEndPoint2()) > myFilteringMinLineLength)
    {
      if (myPrinting)
	ArLog::log(ArLog::Normal, "kept %g (%d points)", 
		   (*myLines)[start]->getLength(),
		   (*myLines)[start]->getNumPoints());
      (*newLines)[numNewLines] = new ArLineFinderSegment(*((*myLines)[start]));
      numNewLines++;
    }
    else
    {
      if (myPrinting)
	ArLog::log(ArLog::Normal, "Clipped %g (%d points)",
		   (*myLines)[start]->getLength(),
		   (*myLines)[start]->getNumPoints());
    }
    
  }

  // move the new lines over and delete the old ones
  if (myLines != NULL && myLines->begin() != myLines->end())
  {
    ArUtil::deleteSetPairs(myLines->begin(), myLines->end());
    delete myLines;
    myLines = NULL;
  } 
  else if (myLines != NULL)
  {  
    delete myLines;
    myLines = NULL;
  }
  myLines = newLines;

}



/**
   Saves the points in the "points" with all the points file in the
   current directory and saves the "lines" file with the final lines
   in the current directory.
**/
AREXPORT void ArLineFinder::saveLast(void)
{
  int len = myPoints->size();
  int i;
  
  FILE *points;
  if ((points = ArUtil::fopen("points", "w+")) == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLineFinder::log: Could not open 'points' file for output");
    return;
  }
  for (i = 0; i < len; i++)
  {
    fprintf(points, "%.0f %.0f\n", 
	    (*myPoints)[i].getX(), (*myPoints)[i].getY());
  }
  fclose(points);


  len = myLines->size();
  
  FILE *lines;
  if ((lines = ArUtil::fopen("lines", "w+")) == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLineFinder::log: Could not open 'lines' file for output");
    return;
  }
  for (i = 0; i < len; i++)
  {
    fprintf(lines, "%.0f %.0f %.0f %.0f\n", 
	    (*myLines)[i]->getX1(), (*myLines)[i]->getY1(),
	    (*myLines)[i]->getX2() - (*myLines)[i]->getX1(),
	    (*myLines)[i]->getY2() - (*myLines)[i]->getY1());	    
  }
  fclose(lines);

  ArLog::log(ArLog::Normal, "Saved points and lines");
}

AREXPORT void ArLineFinder::getLinesAndSaveThem(void)
{
  getLines();
  saveLast();
}

AREXPORT void ArLineFinder::addToConfig(ArConfig *config,
					const char *section)
{
  
  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section,
		     ArPriority::NORMAL);
  config->addParam(
	  ArConfigArg("CreatingMinLineLength", &myMakingMinLen,
		      "The minimum possible line length for creating lines", 0),
	  section, ArPriority::TRIVIAL);
  config->addParam(
	  ArConfigArg("CreatingMinLinePoints", &myMakingMinPoints,
		      "The minimum number of points in a line for creating lines", 0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("CreatingMaxDistBetweenPoints", 
		      &myMaxDistBetweenPoints,
		      "The max dist between points for creating lines", 
		      0),
	  section, ArPriority::TRIVIAL);


  config->addParam(
	  ArConfigArg("CombiningAngleTol", &myCombiningAngleTol,
		      "The angle tolerance when combining lines", 0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("CombiningCloseEnough", &myCombiningLinesCloseEnough,
		      "How far apart lines can be when combining lines", 0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("FilteringMinPointsInLine", &myFilteringMinPointsInLine,
		      "How many points need to be in a line when filtering", 
		      0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("FilteringMinLineLength", &myFilteringMinLineLength,
		      "How many points are needed in a line when filtering", 
		      0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("ValidMaxDistFromLine", &myValidMaxDistFromLine,
		      "For the validation phase, the max dist from line", 
		      0),
	  section, ArPriority::TRIVIAL);

  config->addParam(
	  ArConfigArg("ValidMaxAveDistFromLine", &myValidMaxAveFromLine,
		      "For the validation phase, the max ave dist from line", 
		      0),
	  section, ArPriority::TRIVIAL);

}

AREXPORT std::set<ArLineFinderSegment*> ArLineFinder::getLinesAsSet()
{
  std::map<int, ArLineFinderSegment*> *lines = getLines();
  std::set<ArLineFinderSegment*> lineSegPtrs;
  for(std::map<int, ArLineFinderSegment*>::const_iterator i = lines->begin(); i != lines->end(); ++i)
  {
    lineSegPtrs.insert( (*i).second );
  }
  return lineSegPtrs;
}

AREXPORT std::set<ArPose> ArLineFinder::getNonLinePointsAsSet()
{
  std::map<int, ArPose> *pointsPtr = getNonLinePoints();
  std::set<ArPose> points;
  for(std::map<int, ArPose>::const_iterator i = pointsPtr->begin(); i != pointsPtr->end(); ++i)
  {
    points.insert( (*i).second );
  }
  return points;
}

