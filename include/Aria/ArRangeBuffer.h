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
#ifndef ARRANGEBUFFER_H
#define ARRANGEBUFFER_H

#include "Aria/ariaUtil.h"
#include "Aria/ariaTypedefs.h"
#include "Aria/ArTransform.h"
#include <list>
#include <vector>

/** Stores a point cloud of timestamped positions in global space representing sensor readings or responses, into which recently received sensor readings are added by ArRangeDevice objects, and old or otherwise no-longer-useful readings are removed.
 *  Each ArRangeDevice implementation keeps a "current" ArRangeBuffer of relatively recent readings, and a "cumulative" buffer representing a longer history of readings. 
 *  Each ArRangeDevice may have different conditions for when sensor readings are omitted or removed for each of these buffers, depending on the actual sensing characteristics of its sensor.
 *  (For example, a laser rangefinder may remove readings which are behind or shadowed by a more recently received reading).
 *  As ArRangeDevice objects store sets of readings in ArRangeBuffer, robot position information is also updated (see getPoseTaken() and getEncoderPoseTaken()).  
 *  Some additional utility methods are provided such as finding the closest reading, transforming the reading positions, these are generally used internally or by equivalent API in ArRangeDevice.
 *  New readings are added to the front of the list, and if the list is at capacity, the last reading at the back is discarded. When addReadingConditional() is used, the updated reading is also moved to the front.
 *  Therefore the list is in reverse chronological order.  However, most ArRangeDevice implementations also periodically invalidate old readings, especially for their "current" buffer, which will prevent particularly old readings from remaining.
 *  This class stores the readings as a std::list of ArPoseWithTime objects.  ArRangeDevice provides accessors for the current and cumulative ArRangeBuffer objects, as well as convenience accesors for the underlying std::list<ArPoseWithTime> containers.
 *  Prior to AriaCoda 3.x, these were stored as a std::list<ArPoseWithTime*> (pointers to allocated objects), and only provided access to a pointer to this std::list, and also included a mechanism to manage this list to reduce re-allocations of ArPoseWithTime objects. 
 *  In AriaCoda 3.x this has been modified to more directly use std::list<ArPoseWithTime>. This avoids allocation, reallocation, and custom management of the list, though sometimes ArPoseWithTime objects are internally copied.
 */
class ArRangeBuffer
{
public:
  /// Constructor
  ArRangeBuffer(size_t maxsize) : myCapacity(maxsize) {}

  /// Destructor
  //AREXPORT virtual ~ArRangeBuffer();
  // XXX TODO should include copy/move constructors and copy/move assignment operators.

  /// Gets the maximum size (capacity) of the buffer
  size_t getCapacity() const { return myCapacity; }

  [[deprecated]] size_t getSize() const {
    return myCapacity;
  }

  /// Gets the current number of readings stored in the buffer.
  size_t getCurrentSize() { return myBuffer.size(); }

  /// Sets the size (capacity) of the buffer
  [[deprecated]] void setSize(size_t size) { setCapacity(size); }

  /// Set max size (capacity)
  void setCapacity(size_t size);

  /// Gets the pose of the robot when most recently added readings were taken
  ArPose getPoseTaken() const {
    return myRobotPose;
  }
  /// For ArRangeDevice implementations: Sets the pose of the robot when most recently added readings were taken
  void setPoseTaken(const ArPose& p) {
    myRobotPose = p;
  }
  /// Gets the encoder pose of the robot when recently added readings were taken
  ArPose getEncoderPoseTaken() const {
    return myRobotEncoderPose;
  }
  /// For ArRangeDevice implementations: Sets the pose of the robot when recently added readings were last added
  void setEncoderPoseTaken(const ArPose& p) {
    myRobotEncoderPose = p;
  }

  /// for ArRangeDevice implementations: Adds a new reading to the buffer
  void addReading(double x, double y) {
    addReading(ArPoseWithTime(x, y));
  }

  /// For ArRangeDevice implementations: Adds a new reading to the buffer
  AREXPORT void addReading(const ArPoseWithTime &p);

  /// For ArRangeDevice implementations: Adds a new reading to the buffer unless the new reading is nearby any existing reading.
  void addReadingConditional(double x, double y, 
				      double closeDistSquared, 
				      bool *wasAdded = NULL) 
  {
    addReadingConditional(ArPoseWithTime(x, y), closeDistSquared, wasAdded);
  }

  /// For ArRangeDevice implementations: Adds a new reading to the buffer unless the new reading is nearby any existing reading.
  AREXPORT void addReadingConditional(const ArPoseWithTime& p, 
				      double closeDistSquared, 
				      bool *wasAdded = NULL);

#ifndef SWIG
  /// For ArRangeDevice implementations: Begins a walk through the list of readings. ArRangeDevice implementations. call this before invalidating one or more readings with invalidateReading(). Do not add any readings or adjust the capacity (size), until after the end of the invalidation sweep.
  /// @internal
  AREXPORT void beginInvalidationSweep();

  /// For ArRangeDevice implementations: While doing an invalidation sweep, adds a reading to the list to be invalidated. Called by ArRangeDevice implementations only.
  /// @internal
  AREXPORT void invalidateReading(std::list<ArPoseWithTime>::const_iterator readingIt);
  //AREXPORT void invalidateReading(std::list<ArPoseWithTime>::iterator readingIt);

  /// For ArRangeDevice implementations:  Ends the invalidation sweep. ArRangeDevice implementations this after invalidating any readings with invalidateReading().
  /// @internal
  AREXPORT void endInvalidationSweep();

  /** Return reference to list of readings.  You can use
  * ArRangeDevice::lockDevice() and ArRangeDevice::unlockDevice() for mutual
  * exclusion of this data if accessing asynchronously from another thread (Note that some range device implementations will be accessing the buffer from their own asynchronous thread (most laser rangefinders, for example), or from the ArRobot task cycle thread (in a Sensor Interp. task))
  * This method replaces the previous getBuffer() method which returned a
  * pointer to the list.  Use getBufferPtr() if you still need to receive a
  * pointer, but that method is deprecated and will be removed in the future.
  * Also note that this method may need to be removed or replaced if internal implementation of ArRangeBuffer changes.  getBegin() and getEnd() are preferred.
  */
  const std::list<ArPoseWithTime>& getBuffer() const { return myBuffer; }
  // when this is (temporary?) rvalue: std::list<ArPoseWithTime> getBuffer() && { return std::move(myBuffer);  }

  /// Get const_iterator pointing to the beginning or start of buffer items (most recent)
  auto getBegin() const { return myBuffer.cbegin(); }

  /// Get const_iterator pointing to the end of the buffer items (oldest)
  auto getEnd() const { return myBuffer.cend(); }

  /** 
   *  @swigomit
   * @deprecated
   */
  PUBLICDEPRECATED("Use ArRangeBuffer::getBuffer() or ArRangeBuffer::getBegin() and ArRangeBuffer::getEnd() instead") 
  const std::list<ArPoseWithTime>* getBufferPtr() const
  {
    return &myBuffer;
  }
#endif

  /** 
    @deprecated
  */
  PUBLICDEPRECATED("Use ArRangeBuffer::getBuffer() or ArRangeBuffer::getBegin() and ArRangeBuffer::getEnd() instead") 
  std::list<ArPoseWithTime> *getBufferPtr()
  {
    return &myBuffer;
  }

  /** Create a list of pointers to the reading positions. For backward compatibility only.
   * @deprecated */
  PUBLICDEPRECATED("Use ArRangeBuffer::getBuffer() or ArRangeBuffer::getBegin() and ArRangeBuffer::getEnd() instead") 
  std::list<ArPoseWithTime *> *getBufferPtrsPtr() const;


  /// Gets the closest reading, on a polar system 
  AREXPORT double getClosestPolar(double startAngle, double endAngle, 
				  const ArPose& position, unsigned int maxRange,
				  double *angle = NULL) const;
  /// Gets the closest reading, from a rectangular box, in robot LOCAL coords
  AREXPORT double getClosestBox(double x1, double y1, double x2, double y2,
				const ArPose& position, unsigned int maxRange, 
				ArPose *readingPos = NULL,
				ArPose targetPose = ArPose(0, 0, 0)) const;

  /// For ArRangeDevice implementations: Applies a transform to all readings in the buffer
  AREXPORT void applyTransform(const ArTransform& trans);

  /// For ArRangeDevice implementations: Clears all the readings in the range buffer
  AREXPORT void clear();
  /// For ArRangeDevice implementations: Resets the readings older than this many seconds
  AREXPORT void clearOlderThan(int milliSeconds);
  /// For ArRangeDevice implementations: Resets the readings older than this many seconds
  AREXPORT void clearOlderThanSeconds(int seconds);
  /// @deprecated
  [[deprecated("use clear()")]]
  AREXPORT void reset();

  /// For ArRangeDevice implementations: This begins a redoing of the buffer
  /// @internal
  AREXPORT void beginRedoBuffer();
  /// For ArRangeDevice implementations: Add a reading to the redoing of the buffer
  /// @internal
  AREXPORT void redoReading(double x, double y);   
  /// For ArRangeDevice implementations: End redoing the buffer
  /// @internal
  AREXPORT void endRedoBuffer();

  /// Copy the current buffer into a new std::vector (stored in this object), return pointer
  AREXPORT std::vector<ArPoseWithTime> *getBufferAsVectorPtr();
  /// Copy the current buffer into a new std::vector (stored in this object), return reference.
  AREXPORT const std::vector<ArPoseWithTime>& getBufferAsVector();

  /* 
  /// Gets the closest reading, from an arbitrary buffer
  AREXPORT static double getClosestPolarInList(
	  double startAngle, double endAngle, ArPose position, 
	  unsigned int maxRange, double *angle, 
	  const std::list<ArPoseWithTime *> *buffer);
  /// Gets the closest reading, from an arbitrary buffer
  AREXPORT static double getClosestBoxInList(
	  double x1, double y1, double x2, double y2, ArPose position, 
	  unsigned int maxRange, ArPose *readingPos, 
	  ArPose targetPose, const std::list<ArPoseWithTime *> *buffer); 
  */

  /// Write a log message using ArLog containing all data in the buffer. (For debugging.)
  AREXPORT void logData(ArLog::LogLevel level = ArLog::Normal, const char *linePrefix = "", const char *sensorName = "", const char *bufferName = "") const;

  /// Write a message containing internal information such as sizes of internal lists. For testing and debugging.
  /// write to fp if not null, otherwise use ArLog.
  AREXPORT void logInternal(ArLog::LogLevel level = ArLog::Normal, const char *name = "") const;
  AREXPORT void logInternal(FILE *fp, const char *linePrefix = "", const char *name = "") const;

protected:
  ArPose myRobotPose;		// where the robot was when readings were acquired
  ArPose myRobotEncoderPose;		// where the robot was when readings were acquired

  std::list<ArPoseWithTime> myBuffer;
  std::list<ArPoseWithTime> myReserved; /// < Invalidated items that were removed from the buffer (e.g. after "invalidation sweep") are spliced into this list to avoid deallocation and reallocation of items.
  std::vector<std::list<ArPoseWithTime>::const_iterator> myInvalidSweepList; ///< Items that will be removed from the buffer at the end of an "invalidation sweep"

  std::list<ArPoseWithTime>::iterator myRedoIt;
  int myNumRedone;
  bool myHitEnd;
  
  size_t myCapacity;

  std::vector<ArPoseWithTime> myVector; // copy of myBuffer, recreated whenever getBufferAsVector() is called.  TODO remove
};

#endif // ARRANGEBUFFER_H
