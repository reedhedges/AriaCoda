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
#include "Aria/ArRangeBuffer.h"
#include "Aria/ArLog.h"

#include <algorithm>



/** @class ArRangeBuffer
 * 
 *  @impnote The data in ArRangeBuffer is currently stored as a std::list of ArPoseWithTime objects.  When new readings are added
 *  to the buffer, an ArPoseWithTime object is added to the beginning of the list, or if buffer is at capacity, the last (oldest) reading
 *  is moved (spliced) to the front and re-used, or else if a previously created ArPoseWithTime is available 
 *  (e.g. was previously removed by being invalidated by the ArRangeDevice or internal condition), it will be re-used and spliced to the beginning 
 *  of the list.   An alternative implementation would be to use a std::vector or std::array (since the max capacity is generally set at compile time),
 *  used as a circular buffer with our own begin/end iterators maintained.  This would enable users to more efficiently make a copy of the buffer
 *  for asynchronous processing.  For this reason, getBegin() and getEnd() are the preferred means of acessing buffer data. (You also generally need to lock/unlock the ArRangeDevice object while accessing the buffer)
 */

/**
   If the new size is smaller than the current buffer it chops off the 
   readings that are excess from the oldest readings... if the new size
   is larger then it just leaves room for the buffer to grow
   @param size number of readings to set the buffer to
*/
AREXPORT void ArRangeBuffer::setCapacity(size_t size) 
{
  myCapacity = size;
  if(myCapacity < myBuffer.size())
    myBuffer.resize(myCapacity);
}


/**
   Gets the closest reading in a region defined by startAngle going to 
   endAngle... going counterclockwise (neg degrees to poseitive... with 
   how the robot is set up, thats counterclockwise)... from -180 to 180... 
   this means if you want the slice between 0 and 10 degrees,
   you must enter it as 0, 10, if you do 10, 0 you'll get the 350 degrees
   between 10 and 0... be especially careful with negative... for example
   -30 to -60 is everything from -30, around through 0, 90, and 180 back to 
   -60... since -60 is actually to clockwise of -30
   @param startAngle where to start the slice
   @param endAngle where to end the slice, going clockwise from startAngle
   @param startPos the position to find the closest reading to (usually
   the robots position)
   @param maxRange the maximum range to return (and what to return if nothing
   found)
   @param angle a pointer return of the angle to the found reading
   @return if the return is >= 0 and <= maxRange then this is the distance
   to the closest reading, if it is >= maxRange, then there was no reading 
   in the given section
*/
AREXPORT double ArRangeBuffer::getClosestPolar(double startAngle, 
					       double endAngle, 
					       const ArPose& startPos, 
					       unsigned int maxRange,
					       double *angle) const
{
  double closest = 0;
  bool foundOne = false;
  double closeTh = 0;
  double dist = 0;

  startAngle = ArMath::fixAngle(startAngle);
  endAngle = ArMath::fixAngle(endAngle);

  for (auto it = myBuffer.begin(); it != myBuffer.end(); ++it)
  {
    const double angle1=startPos.findAngleTo(*it);
    const double angle2=startPos.getTh();
    const double th = ArMath::subAngle(angle1, angle2);
    if (ArMath::angleBetween(th, startAngle, endAngle))
    {
      if (!foundOne || (dist = it->findDistanceTo(startPos)) < closest)
      {
        closeTh = th;	
        if (!foundOne)
          closest = it->findDistanceTo(startPos);
        else
          closest = dist;
        foundOne = true;
      }
    }
  }
  if (!foundOne)
    return maxRange;
  if (angle != NULL)
    *angle = closeTh;
  if (closest > maxRange)
    return maxRange;
  else
    return closest;  
}



/**
   Get closest reading in a region defined by two points (opposeite points
   of a rectangle)

   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param startPos the position to find the closest reading to (usually
   the robots position)
   @param maxRange the maximum range to return (and what to return if nothing
   found)
   @param readingPos a pointer to a position in which to store the location of
   the closest position
   @param targetPose the origin of the local coords for the definition of the 
   coordinates, normally just ArRobot::getPosition()
   @param buffer Use the reading positions from this list 
   @param targetPose the pose to see if we're closest too (in local coordinates), this should nearly always be the default of 0 0 0
   @return if the return is >= 0 and <= maxRange then this is the distance
   to the closest reading, if it is >= maxRange, then there was no reading 
   in the given section
*/
AREXPORT double ArRangeBuffer::getClosestBox(
	double x1, double y1, double x2, double y2, const ArPose& startPos,
	unsigned int maxRange, ArPose *readingPos, ArPose targetPose) const

{
  double closest = maxRange;
  double dist;
  ArPose closestPos;
  //ArPoseWithTime pose;
  ArPose zeroPos(0, 0, 0);;
  ArTransform trans(startPos, zeroPos);

  // double temp;


  if (x1 >= x2)
  {
    std::swap(x1, x2);
/*     temp = x1, 
    x1 = x2;
    x2 = temp; */
  }
  if (y1 >= y2)
  {
    std::swap(y1, y2);
/*     temp = y1, 
    y1 = y2;
    y2 = temp; */
  }
  
  for (auto it = myBuffer.begin(); it != myBuffer.end(); ++it)
  {
    const ArPoseWithTime pose = trans.doTransform(*it);

    // see if its in the box
    if (pose.getX() >= x1 && pose.getX() <= x2 &&
	      pose.getY() >= y1 && pose.getY() <= y2)
    {
      dist = pose.findDistanceTo(targetPose);
      //pose.log();
      if (dist < closest)
      {
        closest = dist;
        closestPos = pose;
      }
    }
  }

  if (readingPos != NULL)
    *readingPos = closestPos;
  if (closest > maxRange)
    return maxRange;
  else
    return closest;
}

/** 
    Applies a transform to the all items in buffer.. this is mostly useful for translating
    to/from local/global coords, but may have other uses
    @param trans the transform to apply to the data
*/
AREXPORT void ArRangeBuffer::applyTransform(const ArTransform &trans)
{
  std::for_each(myBuffer.begin(), myBuffer.end(), [&](ArPoseWithTime &p) { p = trans.doTransform(p); } );
}

AREXPORT void ArRangeBuffer::clear()
{
  beginRedoBuffer();
  endRedoBuffer();
}

AREXPORT void ArRangeBuffer::reset()
{
  clear();
}

AREXPORT void ArRangeBuffer::clearOlderThan(int milliSeconds)
{
  
  // TODO if new readings are always added to the front of the buffer, then this could probably just splice all readings after the first reading found into the reserve list.

  beginInvalidationSweep();
  for (auto it = myBuffer.begin(); it != myBuffer.end(); ++it)
  {
    if (it->getTime().mSecSince() > milliSeconds)
      invalidateReading(it);
  }
  endInvalidationSweep();
}

AREXPORT void ArRangeBuffer::clearOlderThanSeconds(int seconds)
{
  clearOlderThan(seconds*1000);
}

/**
   To redo the buffer means that you want to replace all
   of the readings in the buffer with new pose values, and get rid of the 
   readings that you didn't update with new values (invalidate them).  
   The three functions beginRedoBuffer(), 
   redoReading(), and endRedoBuffer() are all made to enable you to do this.
   First call beginRedoBuffer(). Then for each reading you want
   to update in the buffer, call redoReading(double x, double y), then
   when you are done, call endRedoBuffer().
**/     
AREXPORT void ArRangeBuffer::beginRedoBuffer()
{
  myRedoIt = myBuffer.begin();
  myHitEnd = false;
  myNumRedone = 0;
}

/**
   For a description of how to use this, see beginRedoBuffer()
   @param x the x param of the coord to add to the buffer
   @param y the x param of the coord to add to the buffer
*/
AREXPORT void ArRangeBuffer::redoReading(double x, double y)
{
  if (myRedoIt != myBuffer.end() && !myHitEnd)
  {
    myRedoIt->setPose(x, y);
    // TODO sholud we update timestamp?
    myRedoIt++;
  }
  // We re-used as many items in myBuffer as we could, we have reached the end of myBuffer. Just append them now.
  else
  {
    addReading(x,y);
    myHitEnd = true;
  }
  myNumRedone++;
}

/**
   For a description of how to use this, see beginRedoBuffer()
**/
AREXPORT void ArRangeBuffer::endRedoBuffer()
{
  if (!myHitEnd)
  {
    // There were still some readings left in the old myBuffer, remove the rest.
    // todo just remove the items
    beginInvalidationSweep();
    while (myRedoIt != myBuffer.end())
    {
      invalidateReading(myRedoIt);
      myRedoIt++;
    }
    endInvalidationSweep();
  } 
}

/**
   @param x the x position of the reading
   @param y the y position of the reading

   @param closeDistSquared if squared distance between any reading already in the buffer, and the new point to add, is less than @a closeDistSquared then the old point is re-used and its timestamp is updated. If no nearby reading is found, add it.
   @param wasAdded pointed to set to true if the reading was added, or false if not

   This prevents multiple readings very close to each other in the buffer.
*/
AREXPORT void ArRangeBuffer::addReadingConditional(
	const ArPoseWithTime& p, double closeDistSquared, bool *wasAdded)
{
  // find an existing reading to replace with this one.
  // TODO could be optimized if a spatially sorted list is used.
  if (closeDistSquared >= 0)
  {  
    for (auto it = myBuffer.begin(); it != myBuffer.end(); ++it)
    {
      if (it->squaredFindDistanceTo(p) < closeDistSquared)
      {
        it->setTimeToNow();
        // XXX TODO move item to front of list, to keep it sorted with newest at the front?
        if (wasAdded != NULL)
          *wasAdded = false;
        return;
      }
    }
  }

  if (wasAdded != NULL)
    *wasAdded = true;
  addReading(p);    
}

/**
   @param x the x position of the reading
   @param y the y position of the reading
*/
AREXPORT void ArRangeBuffer::addReading(const ArPoseWithTime& p) 
{

  // If the buffer is full, reuse the last item, which is the oldest added, and splice it to the beginning.
  if(myBuffer.size() == myCapacity)
  {
    myBuffer.splice(myBuffer.cbegin(), myBuffer, --myBuffer.end());
    *(myBuffer.begin()) = p; // todo could we std::move from p to buffer?
  }
  else
  {
    // if there are any items in myReserved, splice them into myBuffer.
    if(!myReserved.empty())
    {
      myBuffer.splice(myBuffer.cbegin(), myReserved, myReserved.cbegin());
      *(myBuffer.begin()) = p;
    }
    else
      myBuffer.emplace_front(p);
  }

  /* another Naive solution
  myBuffer.emplace_front(ArPoseWithTime(x, y));
  while(myBuffer.size() >= myCapacity)
    myBuffer.pop_back();
  */
}

/**
   Begin building a list of readings to remove from the buffer when endInvalidationSweep() is called.  This allows you to remove multiple readings 
   from the list while iterating over it (using a const_iterator), since removing an item from a list invalidates an iterator.  (Alternatively, you may be able to use
   standard container algorithms from e.g. <algorithm> to remove items directly without iterating.)
   @see invalidateReading
   @see endInvalidationSweep
*/
void ArRangeBuffer::beginInvalidationSweep()
{
  myInvalidSweepList.clear();
}

/**
   See the description of beginInvalidationSweep(). 
   @param readingIt the iterator to the reading you want to get rid of
   @see beginInvaladationSweep
   @see endInvalidationSweep
*/
AREXPORT void ArRangeBuffer::invalidateReading(
	std::list<ArPoseWithTime>::const_iterator readingIt)
{
  myInvalidSweepList.push_back(readingIt);
}

/* REXPORT void ArRangeBuffer::invalidateReading(
	std::list<ArPoseWithTime>::iterator readingIt)
{
  myInvalidSweepList.push_front(readingIt);
} */


/**
   See the description of beginInvalidationSweep()
   @see beginInvalidationSweep
   @see invalidateReading
*/
void ArRangeBuffer::endInvalidationSweep()
{

  for(auto i = myInvalidSweepList.cbegin(); i != myInvalidSweepList.cend(); ++i)
  {
    // naive implementation, just remove myBuffer.remove(*i);
    myReserved.splice(myReserved.cend(), myBuffer, *i); // reserve item for future reuse. (*i) is an iterator into myBuffer.
  }
  myInvalidSweepList.clear();
}

/**
   Copy the readings from this buffer to a vector stored within
   this object, and return a pointer to that vector. 
   Note that the actual vector object is stored within ArRangeBuffer,
   be careful if accessing it from multiple threads.
   @return Pointer to reading vector.
*/
AREXPORT std::vector<ArPoseWithTime> *ArRangeBuffer::getBufferAsVectorPtr()
{
  myVector.reserve(myBuffer.size());
  myVector.clear();
  for (auto it = myBuffer.cbegin(); it != myBuffer.cend(); it++)
  {
    myVector.insert(myVector.begin(), (*it));
  }
  return &myVector;
}


AREXPORT std::list<ArPoseWithTime*> *ArRangeBuffer::getBufferPtrsPtr() const
{
  static std::list<ArPoseWithTime *> ptrlist(myBuffer.size());
  ptrlist.clear();
   for (auto i = myBuffer.begin(); i != myBuffer.end(); ++i)
    ptrlist.push_back(const_cast<ArPoseWithTime*>(&(*i)));
  return &ptrlist;
}

AREXPORT void ArRangeBuffer::logData(ArLog::LogLevel level, const char *linePrefix, const char *sensorName, const char *bufferName) const
{
  ArLog::beginWrite(level);
  ArLog::write(level, "%s%s %s: %u RobotPose: (%.0f, %.0f) RobotEncoderPose: (%.0f, %.0f) Readings: ", 
    linePrefix, sensorName, bufferName, myBuffer.size(), 
    myRobotPose.getX(), myRobotPose.getY(),
    myRobotEncoderPose.getX(), myRobotEncoderPose.getY()
  );
  for(auto i = myBuffer.begin(); i != myBuffer.end(); ++i)
    ArLog::write(level, "(%.0f,%.0f) ", i->getX(), i->getY());
  ArLog::endWrite();
}

AREXPORT void ArRangeBuffer::logInternal(ArLog::LogLevel level, const char *name) const
{
    ArLog::log(level, "ArRangeBuffer %s: Buffer.size=%lu Reserved.size=%lu Capacity=%lu, Buffer.max_size=%lu Reserved.max_size=%lu", name, myBuffer.size(), myReserved.size(), myCapacity, myBuffer.max_size(), myReserved.max_size());
}

AREXPORT void ArRangeBuffer::logInternal(FILE *fp, const char *prefix, const char *name) const
{
  assert(fp);
  fprintf(fp, "%sArRangeBuffer %s: Buffer.size=%lu Reserved.size=%lu, Capacity=%lu, Buffer.max_size=%lu Reserved.max_size=%lu", prefix, name, myBuffer.size(), myReserved.size(), myCapacity, myBuffer.max_size(), myReserved.max_size());
}
