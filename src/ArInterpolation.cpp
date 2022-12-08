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
#include "Aria/ArInterpolation.h"

AREXPORT ArInterpolation::ArInterpolation(size_t numberOfReadings)
{
  mySize = numberOfReadings;
  myDataMutex.setLogName("ArInterpolation");
  setAllowedMSForPrediction();
  setAllowedPercentageForPrediction();
  setLogPrediction();
}



AREXPORT bool ArInterpolation::addReading(ArPoseWithTime reading)
{
  myDataMutex.lock();
  if (myPoses.size() >= mySize)
  {
    //myTimes.pop_back();
    myPoses.pop_back();
  }
  //myTimes.push_front(timeOfReading);
  myPoses.push_front(reading);
  myDataMutex.unlock();
  return true;
}

/**
   @param timeStamp the time we are interested in
   @param position the pose to set to the given position

   @param mostRecent the most recent data in the interpolation relevant to this call... for a return of 1 this is the near side it interpolated between, for a return of 0 or 1 this is the most recent data in the interpolation.... this is only useful if the return is 1, 0, or -1, and is mostly for use with ArRobot::applyEncoderOffset 

   @return 1 its good interpolation, 0 its predicting, -1 its too far to 
   predict, -2 its too old, -3 there's not enough data to predict

  @todo return tuple or expected of flag and ArPoseWithTime.
   
**/

AREXPORT int ArInterpolation::getPose(ArTime timeStamp, ArPose *position, ArPoseWithTime *mostRecent)
{
  //std::list<ArTime>::iterator tit;
  //std::list<ArPose>::iterator pit;
  

  // MPL don't use nowtime, use the time stamp that was passed in...
  // that was bad
  //ArTime nowTime;
  
  myDataMutex.lock();



  // find the time we want

  /*
  for (tit = myTimes.begin(), pit = myPoses.begin();
       tit != myTimes.end() && pit != myPoses.end(); 
       ++tit, ++pit)
  */
  ArPoseWithTime thisPose;
  //ArTime thisTime;
  ArPoseWithTime lastPose;
  //ArTime lastTime;

  std::list<ArPoseWithTime>::const_iterator pit = myPoses.end();
  for(pit = myPoses.begin(); pit != myPoses.end(); ++pit)
  {
    //lastTime = thisTime;
    lastPose = thisPose;

    //thisTime = (*tit);
    thisPose = (*pit);

    //printf("## %d %d %d b %d at %d after %d\n", timeStamp.getMSec(), thisTime.getMSec(), timeStamp.mSecSince(thisTime), timeStamp.isBefore(thisTime), timeStamp.isAt(thisTime), timeStamp.isAfter(thisTime));
    //if (timeStamp.isBefore(thisTime) || timeStamp.isAt(thisTime))
    if (!timeStamp.isAfter(thisPose.getTime()))
    {
      //printf("Found one!\n");
      break;
    } 

  }

  if (mostRecent != NULL)
  {
    *mostRecent = thisPose;
    //mostRecent->setPose(thisPose);
    //mostRecent->setTime(thisPose.getTime());
  }
    

  // if we're at the end then it was too long ago
  if (/*tit == myTimes.end() ||*/ pit == myPoses.end())
  {
    //printf("Too old\n");
    myDataMutex.unlock();
    return -2;
  }

  
  // this is for forecasting (for the brave)
  if ((/*tit == myTimes.begin() ||*/ pit == myPoses.begin()) && 
      !timeStamp.isAt(thisPose.getTime()))
  {
    //printf("Too new %d %d\n", tit == myTimes.begin(), pit == myPoses.begin());
  
    //thisTime = (*tit);
    //thisPose = (*pit);
    //tit++;
    pit++;  
    if (/*tit == myTimes.end() ||*/ pit == myPoses.end())
    {
      //printf("Not enough data\n");
      myDataMutex.unlock();
      return -3;
    }
    //lastTime = (*tit);
    lastPose = thisPose;

    // copy these before unlocking our mutex:
    auto allowedPercent = myAllowedPercentageForPrediction;
    auto allowedMSForPredict= myAllowedMSForPrediction;
    bool logPrediction = myLogPrediction;

    // no longer using myPoses list or any other class data that might be expected to be locked:
    myDataMutex.unlock();

    // MPL don't use nowtime, use the time stamp that was passed in...
    //nowTime.setToNow();
    long total = thisPose.getTime().mSecSince(lastPose.getTime());
    if (total == 0)
      total = 100;
    // MPL don't use nowtime, use the time stamp that was passed in...
    //toStamp = nowTime.mSecSince(thisTime);
    long toStamp = timeStamp.mSecSince(thisPose.getTime());
    double percentage = (double)toStamp/(double)total;
    //if (percentage > 50)
    if (allowedPercent >= 0 && percentage * 100 > allowedPercent)
    {
      if (logPrediction)
	      ArLog::log(ArLog::Normal, "%s: returningPercentage Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, allowedPercent);
      
      //myDataMutex.unlock();
      return -1;
    }

    if (allowedMSForPredict >= 0 && abs(toStamp) > allowedMSForPredict)
    {
      if (myLogPrediction)
        ArLog::log(ArLog::Normal, "%s: returningMS Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, allowedMSForPredict);
      //myDataMutex.unlock();
      return -1;
    }

    if (logPrediction)
      ArLog::log(ArLog::Normal, "%s: Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, allowedPercent);


    ArPose &retPose = *position;
    retPose.setX(thisPose.getX() + (thisPose.getX() - lastPose.getX()) * percentage);
    retPose.setY(thisPose.getY() + (thisPose.getY() - lastPose.getY()) * percentage);
    retPose.setTh(ArMath::addAngle(
           thisPose.getTh(),
           ArMath::subAngle(thisPose.getTh(), lastPose.getTh()) * percentage));

    if (retPose.findDistanceTo(thisPose) > 1000)
      ArLog::log(ArLog::Normal, "%s: finaldist %.0f thislastdist %.0f Total time %d, to stamp %d, percentage %.2f", getName(), 
     retPose.findDistanceTo(thisPose), thisPose.findDistanceTo(lastPose), total, toStamp, percentage * 100);

    return 0;
  }
  else
  {

    // this is the actual interpolation

    //printf("Woo hoo!\n");

    long total = thisPose.getTime().mSecSince(lastPose.getTime());
    long toStamp = thisPose.getTime().mSecSince(timeStamp);
    double percentage = (double)toStamp/(double)total;

    if (total == 0)
      percentage = 0;

    //if (total == 0)
    //printf("Total time %d, to stamp %d, percentage %.2f\n", 	 total, toStamp, percentage * 100);

    ArPose& retPose = *position;
    retPose.setX(thisPose.getX() + (lastPose.getX() - thisPose.getX()) * percentage); 
    retPose.setY(thisPose.getY() + (lastPose.getY() - thisPose.getY()) * percentage); 
    retPose.setTh(ArMath::addAngle(
           thisPose.getTh(),
           ArMath::subAngle(lastPose.getTh(), thisPose.getTh()) * percentage));
  /*
    printf("original:");
    thisPose.log();
    printf("After:");
    lastPose.log();
    printf("ret:");
    retPose.log();
  */

    //*position = retPose;
    //myDataMutex.unlock();

    return 1;

  }
  
}

AREXPORT size_t ArInterpolation::getNumberOfReadings() const
{
  return mySize;
}

AREXPORT void ArInterpolation::setNumberOfReadings(size_t numberOfReadings)
{
  myDataMutex.lock();
  myPoses.resize(numberOfReadings);
  /*
  while (myPoses.size() > numberOfReadings)
  {
    //myTimes.pop_back();
    myPoses.pop_back();
  }
  */
  mySize = numberOfReadings;  
  myDataMutex.unlock();
}

AREXPORT void ArInterpolation::reset()
{
  myDataMutex.lock();
  //while (myTimes.size() > 0)
  //  myTimes.pop_back();
  myPoses.clear();
  /*
  while (myPoses.size() > 0)
    myPoses.pop_back();
  */
  // Note, mySize remains at previous value, which represents the capacity of the buffer.
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

AREXPORT const char * ArInterpolation::getName() const
{
  return myName.c_str();
}

AREXPORT void ArInterpolation::setAllowedMSForPrediction(int ms)
{
  myDataMutex.lock();
  myAllowedMSForPrediction = ms;
  myDataMutex.unlock();
}

AREXPORT int ArInterpolation::getAllowedMSForPrediction()
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

AREXPORT int ArInterpolation::getAllowedPercentageForPrediction()
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

AREXPORT bool ArInterpolation::getLogPrediction()
{
  bool ret;
  myDataMutex.lock();
  ret = myLogPrediction;
  myDataMutex.unlock();
  return ret;
}

