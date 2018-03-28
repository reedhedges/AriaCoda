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
#include "ArInterpolation.h"

AREXPORT ArInterpolation::ArInterpolation(size_t numberOfReadings)
{
  mySize = numberOfReadings;
  myDataMutex.setLogName("ArInterpolation");
  setAllowedMSForPrediction();
  setAllowedPercentageForPrediction();
  setLogPrediction();
}

AREXPORT ArInterpolation::~ArInterpolation()
{

}

AREXPORT bool ArInterpolation::addReading(ArTime timeOfReading, 
					  ArPose position)
{
  myDataMutex.lock();
  if (myTimes.size() >= mySize)
  {
    myTimes.pop_back();
    myPoses.pop_back();
  }
  myTimes.push_front(timeOfReading);
  myPoses.push_front(position);
  myDataMutex.unlock();
  return true;
}

/**
   @param timeStamp the time we are interested in
   @param position the pose to set to the given position

   @param mostRecent the most recent data in the interpolation relevant to this call... for a return of 1 this is the near side it interpolated between, for a return of 0 or 1 this is the most recent data in the interpolation.... this is only useful if the return is 1, 0, or -1, and is mostly for use with ArRobot::applyEncoderOffset 

   @return 1 its good interpolation, 0 its predicting, -1 its too far to 
   predict, -2 its too old, -3 there's not enough data to predict
   
**/

AREXPORT int ArInterpolation::getPose(
	ArTime timeStamp, ArPose *position, ArPoseWithTime *mostRecent)
{
  std::list<ArTime>::iterator tit;
  std::list<ArPose>::iterator pit;
  
  ArPose thisPose;
  ArTime thisTime;
  ArPose lastPose;
  ArTime lastTime;

  // MPL don't use nowtime, use the time stamp that was passed in...
  // that was bad
  //ArTime nowTime;
  long total;
  long toStamp;
  double percentage;
  ArPose retPose;
  
  myDataMutex.lock();
  // find the time we want
  for (tit = myTimes.begin(), pit = myPoses.begin();
       tit != myTimes.end() && pit != myPoses.end(); 
       ++tit, ++pit)
  {
    lastTime = thisTime;
    lastPose = thisPose;

    thisTime = (*tit);
    thisPose = (*pit);

    //printf("## %d %d %d b %d at %d after %d\n", timeStamp.getMSec(), thisTime.getMSec(), timeStamp.mSecSince(thisTime), timeStamp.isBefore(thisTime), timeStamp.isAt(thisTime), timeStamp.isAfter(thisTime));
    //if (timeStamp.isBefore(thisTime) || timeStamp.isAt(thisTime))
    if (!timeStamp.isAfter(thisTime))
    {
      //printf("Found one!\n");
      break;
    } 

  }

  if (mostRecent != NULL)
  {
    mostRecent->setPose(thisPose);
    mostRecent->setTime(thisTime);
  }
    

  // if we're at the end then it was too long ago
  if (tit == myTimes.end() || pit == myPoses.end())
  {
    //printf("Too old\n");
    myDataMutex.unlock();
    return -2;
  }
  
  // this is for forecasting (for the brave)
  if ((tit == myTimes.begin() || pit == myPoses.begin()) && 
      !timeStamp.isAt((*tit)))
  {
    //printf("Too new %d %d\n", tit == myTimes.begin(), pit == myPoses.begin());
  
    thisTime = (*tit);
    thisPose = (*pit);
    tit++;
    pit++;  
    if (tit == myTimes.end() || pit == myPoses.end())
    {
      //printf("Not enough data\n");
      myDataMutex.unlock();
      return -3;
    }
    lastTime = (*tit);
    lastPose = (*pit);

    // MPL don't use nowtime, use the time stamp that was passed in...
    //nowTime.setToNow();
    total = thisTime.mSecSince(lastTime);
    if (total == 0)
      total = 100;
    // MPL don't use nowtime, use the time stamp that was passed in...
    //toStamp = nowTime.mSecSince(thisTime);
    toStamp = timeStamp.mSecSince(thisTime);
    percentage = (double)toStamp/(double)total;
    //if (percentage > 50)
    if (myAllowedPercentageForPrediction >= 0 && 
	percentage * 100 > myAllowedPercentageForPrediction)
    {
      if (myLogPrediction)
	      ArLog::log(ArLog::Normal, "%s: returningPercentage Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, myAllowedPercentageForPrediction);
      
      myDataMutex.unlock();
      return -1;
    }

    if (myAllowedMSForPrediction >= 0 && 
	abs(toStamp) > myAllowedMSForPrediction)
    {
      if (myLogPrediction)
        ArLog::log(ArLog::Normal, "%s: returningMS Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, myAllowedMSForPrediction);
      myDataMutex.unlock();
      return -1;
    }

    if (myLogPrediction)
      ArLog::log(ArLog::Normal, "%s: Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100,
		 myAllowedPercentageForPrediction);

    retPose.setX(thisPose.getX() + 
		 (thisPose.getX() - lastPose.getX()) * percentage);
    retPose.setY(thisPose.getY() + 
		 (thisPose.getY() - lastPose.getY()) * percentage);
    retPose.setTh(ArMath::addAngle(thisPose.getTh(),
				   ArMath::subAngle(thisPose.getTh(),
						    lastPose.getTh())
				   * percentage));

    if (retPose.findDistanceTo(thisPose) > 1000)
      ArLog::log(ArLog::Normal, "%s: finaldist %.0f thislastdist %.0f Total time %d, to stamp %d, percentage %.2f", getName(), 
		 retPose.findDistanceTo(thisPose), thisPose.findDistanceTo(lastPose), total, toStamp, percentage * 100);


    *position = retPose;
    myDataMutex.unlock();
    return 0;
  }

  // this is the actual interpolation

  //printf("Woo hoo!\n");

  total = thisTime.mSecSince(lastTime);
  toStamp = thisTime.mSecSince(timeStamp);
  percentage = (double)toStamp/(double)total;

  if (total == 0)
    percentage = 0;

  //if (total == 0)
  //printf("Total time %d, to stamp %d, percentage %.2f\n", 	 total, toStamp, percentage * 100);

  retPose.setX(thisPose.getX() + 
	       (lastPose.getX() - thisPose.getX()) * percentage); 
  retPose.setY(thisPose.getY() + 
	       (lastPose.getY() - thisPose.getY()) * percentage); 
  retPose.setTh(ArMath::addAngle(thisPose.getTh(),
				 ArMath::subAngle(lastPose.getTh(), 
						  thisPose.getTh())
				 * percentage));
/*
  printf("original:");
  thisPose.log();
  printf("After:");
  lastPose.log();
  printf("ret:");
  retPose.log();
*/
  *position = retPose;
  myDataMutex.unlock();
  return 1;
  
}

AREXPORT size_t ArInterpolation::getNumberOfReadings(void) const
{
  return mySize;
}

AREXPORT void ArInterpolation::setNumberOfReadings(size_t numberOfReadings)
{
  myDataMutex.lock();
  while (myTimes.size() > numberOfReadings)
  {
    myTimes.pop_back();
    myPoses.pop_back();
  }
  mySize = numberOfReadings;  
  myDataMutex.unlock();
}

AREXPORT void ArInterpolation::reset(void)
{
  myDataMutex.lock();
  while (myTimes.size() > 0)
    myTimes.pop_back();
  while (myPoses.size() > 0)
    myPoses.pop_back();
  myDataMutex.unlock();
}

AREXPORT void ArInterpolation::setName(const char *name)
{
  myDataMutex.lock();
  myName = name;
  std::string mutexLogName;
  mutexLogName = myName;
  mutexLogName += "::DataMutex";
  myDataMutex.setLogName(mutexLogName.c_str());
  myDataMutex.unlock();
}

AREXPORT const char * ArInterpolation::getName(void)
{
  return myName.c_str();
}

AREXPORT void ArInterpolation::setAllowedMSForPrediction(int ms)
{
  myDataMutex.lock();
  myAllowedMSForPrediction = ms;
  myDataMutex.unlock();
}

AREXPORT int ArInterpolation::getAllowedMSForPrediction(void)
{
  int ret;
  myDataMutex.lock();
  ret = myAllowedMSForPrediction;
  myDataMutex.unlock();
  return ret;
}

AREXPORT void ArInterpolation::setAllowedPercentageForPrediction(int percentage)
{
  myDataMutex.lock();
  myAllowedPercentageForPrediction = percentage;
  myDataMutex.unlock();
}

AREXPORT int ArInterpolation::getAllowedPercentageForPrediction(void)
{
  int ret;
  myDataMutex.lock();
  ret = myAllowedPercentageForPrediction;
  myDataMutex.unlock();
  return ret;
}

AREXPORT void ArInterpolation::setLogPrediction(bool logPrediction)
{
  myDataMutex.lock();
  myLogPrediction = logPrediction;
  myDataMutex.unlock();
}

AREXPORT bool ArInterpolation::getLogPrediction(void)
{
  bool ret;
  myDataMutex.lock();
  ret = myLogPrediction;
  myDataMutex.unlock();
  return ret;
}

