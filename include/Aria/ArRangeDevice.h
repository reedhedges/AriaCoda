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
#ifndef ARRANGEDEVICE_H
#define ARRANGEDEVICE_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRangeBuffer.h"
#include "Aria/ArSensorReading.h"
#include "Aria/ArDrawingData.h"
#include "Aria/ArMutex.h"
#include <set>

class ArRobot;

/** 
    @brief The base class for all sensing devices which return range
    information from the device (mounted on a robot) to an object in
    the environment.

    This class maintains two ArRangeBuffer objects: a current buffer
    (getCurrentBuffer())
    for storing very recent readings, and a cumulative buffer 
    (getCumulativeBuffer()) for a
    longer history of readings.  The maximum sizes of each buffer can
    be set in the constructor or resized later. Range device readings
    are most often represented as a point in space (X,Y) where the
    sensor detected an object.  (Therefore an ArPose object may only
    have X and Y components set).  

    Some devices provide an original set of "raw" ArSensorReading 
    objects (getRawReadings()) (that it used to add data to the current buffer) 
    which may also include extra device specific information as well.
    Not all devices provide raw readings.

    Subclasses are used for specific sensor implementations like
    ArLaser and subclasses for laser rangefinders and ArSonarDevice for the Pioneer sonar
    array. It can also be useful to treat "virtual" objects like
    forbidden areas specified by the user in a map like range devices.
    Some of these subsclasses may use a separate thread to update the
    range reading buffers, and so this base class provides "lock" and
    "unlock" methods which you should use when accessing device data.

    A range device may have an ArRobot object associated with it. A
    range device may also be associated with an ArRobot by calling
    ArRobot::addRangeDevice().  ArRobot provides functions which
    operate on all such associated ArRangeDevice objects.  This is a
    convenient (and thread-safe) way to access all range device data
    without depending on a specific set of individual range
    devices. For example, you can find the closest reading in a box or
    a polar section, no matter if that reading originated from the
    sonar, a laser, or other device.  

    @ingroup ImportantClasses
**/

// TODO most of these methods do not need to be virtual. 
// TODO add copy constructors/ops

class ArRangeDevice
{
public:
  /// Constructor
  AREXPORT ArRangeDevice(size_t currentBufferSize, 
			 size_t cumulativeBufferSize,
			 const char *name, unsigned int maxRange,
			 int maxSecondsToKeepCurrent = 0,
			 int maxSecondsToKeepCumulative = 0,
			 double maxDistToKeepCumulative = 0,
			 bool locationDependent = false);
  /// Destructor
  AREXPORT virtual ~ArRangeDevice();
  /// Gets the name of the device
  AREXPORT virtual const char *getName() const;
  /// Sets the robot this device is attached to
  AREXPORT virtual void setRobot(ArRobot *robot);
  /// Gets the robot this device is attached to
  AREXPORT virtual ArRobot *getRobot();
  /// Sets the maximum size of the buffer for current readings
  /// Will be initialized to default size by ArRangeDevice implementation subclass, you normally do not need to set this externally.
  AREXPORT virtual void setCurrentBufferSize(size_t size);
  /// Gets the maximum size of the buffer for current readings
  AREXPORT virtual size_t getCurrentBufferSize() const;
  /// Sets the maximum size of the buffer for cumulative readings
  /// Will be initialized to default size by ArRangeDevice implementation subclass, you normally do not need to set this externally.
  AREXPORT virtual void setCumulativeBufferSize(size_t size);
  /// Sets the maximum size of the buffer for cumulative readings
  AREXPORT virtual size_t getCumulativeBufferSize() const;
  /// Adds a reading to the buffer
  /// For use by subclasses only
  AREXPORT virtual void addReading(double x, double y, bool *wasAdded = NULL);
  /// Gets if this device is location dependent or not
  bool isLocationDependent() { return myIsLocationDependent; }
  /// Gets the closest current reading in the given polar region
  AREXPORT virtual double currentReadingPolar(double startAngle, 
					      double endAngle,
					      double *angle = NULL) const;
  /// Gets the closest cumulative reading in the given polar region
  AREXPORT virtual double cumulativeReadingPolar(double startAngle, 
						 double endAngle,
						 double *angle = NULL) const;
  /// Gets the closest current reading from the given box region
  AREXPORT virtual double currentReadingBox(double x1, double y1, double x2,
					    double y2, 
					    ArPose *readingPos = NULL) const;
  /// Gets the closest current reading from the given box region
  AREXPORT virtual double cumulativeReadingBox(double x1, double y1, double x2,
					       double y2, 
					       ArPose *readingPos = NULL) const;

  /// Gets the current range buffer
  /// @since AriaCoda 3.0
  /// @note you can check for the ARIACODA preprocessor symbol to determine whether to use this or getCurrentBufferPtr() to get pointer instead of reference.
  const ArRangeBuffer& getCurrentRangeBuffer() const
    { return myCurrentBuffer; }

  /// Gets the "cumulative" range buffer
  /// @since AriaCoda 3.0
  /// @note you can check for the ARIACODA preprocessor symbol to determine whether to use this or getCurrentBufferPtr() to get pointer instead of reference.
  const ArRangeBuffer& getCumulativeRangeBuffer() const
    { return myCumulativeBuffer; }

  /// Gets the list of sensor reading positions from the "current" buffer
  /// @since AriaCoda 3.0
  /// @note you can check for the ARIACODA preprocessor symbol to determine whether to use this or getCurrentBufferPtr() to get pointer instead of reference.
  const std::list<ArPoseWithTime>& getCurrentReadings()  const
    { return myCurrentBuffer.getBuffer(); }

 /// Gets the list of sensor reading positions from the "cumulative" buffer
  /// @since AriaCoda 3.0
  /// @note you can check for the ARIACODA preprocessor symbol to determine whether to use this or getCurrentBufferPtr() to get pointer instead of reference.
  const std::list<ArPoseWithTime>& getCumulativeReadings()  const
    { return myCumulativeBuffer.getBuffer(); }

  /// Gets the current range buffer
  /// @deprecated
  PUBLICDEPRECATED("Use 'const ArRangeBuffer& getCurrentRangeBuffer()' instead.")
  virtual ArRangeBuffer *getCurrentRangeBufferPtr()
    { return &myCurrentBuffer; }

  /// Gets the cumulative range buffer
  /// @deprecated
  PUBLICDEPRECATED("Use 'const ArRAngeBuffer& getCumulativeRangeBuffer()' instead.")
  virtual ArRangeBuffer *getCumulativeRangeBufferPtr() 
    { return &myCumulativeBuffer; }

  /// Gets the current buffer of readings
  /// @deprecated
  PUBLICDEPRECATED("Use getCurrentReadings() instead.")
  AREXPORT virtual std::list<ArPoseWithTime *> *getCurrentBufferPtr();

#ifndef SWIG

  /** @brief Gets the current range buffer
   *  @swigomit See getCurrentBufferAsVector()
      @deprecated
   */
  PUBLICDEPRECATED("Use 'const ArRangeBuffer& getCurrentRangeBuffer()' instead.")
  virtual const ArRangeBuffer *getCurrentRangeBufferPtr() const
    { return &myCurrentBuffer; }
  
  /** @brief Gets the cumulative range buffer
   *  @swigomit See getCumulativeBufferAsVector()
      @deprecated
   */
  PUBLICDEPRECATED("Use 'const ArRangeBuffer& getCumulativeRangeBuffer()' instead.")
  virtual const ArRangeBuffer *getCumulativeRangeBufferPtr() const
    { return &myCumulativeBuffer; }
  
  /** @brief Gets the current buffer of readings
   *  @swigomit See getCurrentBufferAsVector()
      @deprecated
   */
  PUBLICDEPRECATED("Use getCurrentReadings() instead.")
  AREXPORT virtual const std::list<ArPoseWithTime *> *getCurrentBufferPtrsPtr() const;

  /** @brief Gets the current buffer of readings
   *  @swigomit See getCumulativeBufferAsVector()
      @deprecated
   */
  PUBLICDEPRECATED("Use getCumulativeReadings() instead.")
  AREXPORT virtual const std::list<ArPoseWithTime *> *getCumulativeBufferPtrsPtr() const;

#endif // SWIG

  /** @brief Gets the current buffer of readings as a vector
   *  @swignote The return type will be named 
   *   ArPoseWithTimeVector instead of the std::vector template.
   * @deprecated
   */
  PUBLICDEPRECATED("Use getCumulativeReadings() instead.")
  AREXPORT virtual std::vector<ArPoseWithTime> *getCurrentBufferAsVectorPtr();

  /// Gets the current buffer of readings
  /// @deprecated
  PUBLICDEPRECATED("Use getCumulativeReadings() instead.")
  AREXPORT virtual std::list<ArPoseWithTime *> *getCumulativeBufferPtrsPtr();

  /** @brief Gets the cumulative buffer of readings as a vector
   *  @swignote The return type will be named ArPoseWithTimeVector
   *    instead of the std::vector template.
   * @deprecated
   */
  PUBLICDEPRECATED("Use getCumulativeReadings() or getCumulativeRangeBuffer() instead.")
  AREXPORT virtual std::vector<ArPoseWithTime> *getCumulativeBufferAsVectorPtr();

  /// Gets the raw unfiltered readings from the device
  /** The raw readings are the full set of unfiltered readings from the device.
      They are the latest readings. You should not manipulate the list you get from
      this function, the only manipulation of this list should be done by
      the range device itself. 
      Returns NULL if raw readings are not available.

      @note Only laser subclasses provide this data currently.  Sonar, bumpers,
      etc. do not provide raw readings.
      This method was added to this base class for use by multiple laser or
      laser-like subclasses of ArRangeDevice and ArRangeDeviceThreaded
      similar devices.
      Other kinds of range devices are sufficiently different from lasers that
      any "raw" information provided would usually require very different interpretation.
  **/
  virtual const std::list<ArSensorReading *> *getRawReadings() const
    { return myRawReadings; }

  ///  Gets the raw unfiltered readings from the device into a vector 
  /// @deprecated
  PUBLICDEPRECATED("")
  AREXPORT virtual std::vector<ArSensorReading> *getRawReadingsAsVectorPtr();

  /// Gets the raw unfiltered readings from the device (but robot poses are corrected)
  /** The raw readings are the full set of unfiltered readings from
      the device.  They are the latest readings. You should not
      manipulate the list you get from this function, the only
      manipulation of this list should be done by the range device
      itself.  (Its only pointers for speed.)
      
      This is like the raw readings but they were corrected for the
      robot odometry offset (just the pose taken, and encoder pose
      taken).

      TODO change to remove pointers

      @note Only lasers provides this data currently.  Sonar, bumpers,
      etc. do not provide raw readings.
  **/
  virtual const std::list<ArSensorReading *> *getAdjustedRawReadings() const
    { return myAdjustedRawReadings; }

  ///  Gets the raw adjusted readings from the device into a vector 
  /// @deprecated
  PUBLICDEPRECATED("")
  AREXPORT virtual std::vector<ArSensorReading> *getAdjustedRawReadingsAsVectorPtr();


  /// Sets the maximum seconds to keep current readings around
  /**
   @param maxSecondsToKeepCurrent this is the number of seconds to
   keep current readings around, if less than 0 then they are not
   automatically removed because of this
  **/
  void setMaxSecondsToKeepCurrent(int maxSecondsToKeepCurrent)
    { myMaxSecondsToKeepCurrent = maxSecondsToKeepCurrent; }

  /// gets the maximum seconds to keep current readings around
  /**
   @return this is the number of seconds current readings are kept
   around for, if less than 0 then they are not automatically removed
   because of this
  **/
  int getMaxSecondsToKeepCurrent() { return myMaxSecondsToKeepCurrent; }

  /// Sets the minimum distance between current readings
  /**
     @param minDistBetweenCurrent The minimum distance between current
     readings, this is applied in the addReading call so range devices
     need to call that for this to take effect.     
  **/
  void setMinDistBetweenCurrent(double minDistBetweenCurrent)
    {
      myMinDistBetweenCurrent = minDistBetweenCurrent;
      myMinDistBetweenCurrentSquared = (minDistBetweenCurrent * 
					minDistBetweenCurrent);
    }

  /// Gets the minimum distance between current readings
  /**
     @return The minimum distance between current readings, this is
     applied in the addReading call so range devices need to call that
     for this to take effect.
  **/
  double getMinDistBetweenCurrent()
    {
      return myMinDistBetweenCurrent;
    }
  

  /// gets the maximum seconds to keep cumulative readings around
  /**
   @param maxSecondsToKeepCumulative this is the number of seconds to keep
   cumulative readings around, if less than 0 then they are not automatically
   removed because of this
  **/
  void setMaxSecondsToKeepCumulative(int maxSecondsToKeepCumulative)
    { myMaxSecondsToKeepCumulative = maxSecondsToKeepCumulative; }
  /// gets the maximum seconds to keep current readings around
  /**
   @return this is the number of seconds cumulative readings are kept
   around for, if less than 0 then they are not automatically removed
   because of this
  **/
  int getMaxSecondsToKeepCumulative() 
    { return myMaxSecondsToKeepCumulative; }

  /// sets the maximum distance cumulative readings can be from current pose
  /**
   @param maxDistToKeepCumulative if cumulative readings are further than
   this from where the current pose they are removed, if this is less
   than 0 they are not removed because of this
  **/
  void setMaxDistToKeepCumulative(double maxDistToKeepCumulative) 
    { 
      myMaxDistToKeepCumulative = maxDistToKeepCumulative; 
      myMaxDistToKeepCumulativeSquared = (maxDistToKeepCumulative * 
					  maxDistToKeepCumulative); 
    } 

  /// sets the maximum distance cumulative readings can be from current pose
  /**
   @return if cumulative readings are further than this from where the
   current pose they are removed, if this is less than 0 they are not
   removed because of this
  **/
  double getMaxDistToKeepCumulative() { return myMaxDistToKeepCumulative; }

  /// Sets the minimum distance between cumulative readings
  /**
     @param minDistBetweenCumulative The minimum distance between cumulative
     readings, this is applied in the addReading call so range devices
     need to call that for this to take effect.     
  **/
  void setMinDistBetweenCumulative(double minDistBetweenCumulative)
    {
      myMinDistBetweenCumulative = minDistBetweenCumulative;
      myMinDistBetweenCumulativeSquared = (minDistBetweenCumulative * 
					   minDistBetweenCumulative);
    }

  /// Gets the minimum distance between cumulative readings
  /**
     @return The minimum distance between cumulative readings, this is
     applied in the addReading call so range devices need to call that
     for this to take effect.
  **/
  double getMinDistBetweenCumulative()
    {
      return myMinDistBetweenCumulative;
    }

  /// Sets the maximum distance a cumulative reading can be from the robot and still be inserted
  /**
     @param maxInsertDistCumulative The maximum distance a cumulative
     reading can have from the robot's current position and still be
     inserted into the cumulative readings, this is applied in the
     addReading call so range devices need to call that for this to
     take effect.
  **/
  void setMaxInsertDistCumulative(double maxInsertDistCumulative)
    {
      myMaxInsertDistCumulative = maxInsertDistCumulative;
      myMaxInsertDistCumulativeSquared = (maxInsertDistCumulative * 
					   maxInsertDistCumulative);
    }

  /// Gets the maximum distance a cumulative reading can be from the robot and still be inserted
  /**
     @return The maximum distance a cumulative reading can have from
     the robot's current position and still be inserted into the
     cumulative readings, this is applied in the addReading call so
     range devices need to call that for this to take effect.
  **/
  double getMaxInsertDistCumulative()
    {
      return myMaxInsertDistCumulative;
    }

  /// Clears all the current readings
  virtual void clearCurrentReadings() { myCurrentBuffer.clear(); }
  /// Clears all the cumulative readings
  virtual void clearCumulativeReadings() { myCumulativeBuffer.clear(); }
  /// Clears all the cumulative readings older than this number of milliseconds
  virtual void clearCumulativeOlderThan(int milliSeconds) 
    { myCumulativeBuffer.clearOlderThan(milliSeconds); }

  /// Clears all the cumulative readings older than this number of seconds
  virtual void clearCumulativeOlderThanSeconds(int seconds) 
    { myCumulativeBuffer.clearOlderThanSeconds(seconds); }
  
  /// Gets the maximum range for this device
  virtual unsigned int getMaxRange() const { return myMaxRange; }
  /// Sets the maximum range for this device
  virtual void setMaxRange(unsigned int maxRange) 
    { myMaxRange = maxRange; }


  /// Applies a transform to the buffers
  AREXPORT virtual void applyTransform(ArTransform trans, 
				       bool doCumulative = true);

  /// Gets data used for visualizing the current buffer (see ArNetworking)
  virtual ArDrawingData *getCurrentDrawingData() 
    { return myCurrentDrawingData; }
  /// Gets data used for visualizing the cumulative buffer (see ArNetworking)
  virtual ArDrawingData *getCumulativeDrawingData() 
    { return myCumulativeDrawingData; }
  /// Sets data for visualizing the current buffer (and if we own it)
  AREXPORT virtual void setCurrentDrawingData(ArDrawingData *data, 
					      bool takeOwnershipOfData);
  /// Sets data for visualizing the cumulative buffer (and if we own it)
  AREXPORT virtual void setCumulativeDrawingData(ArDrawingData *data, 
						 bool takeOwnershipOfData);

  

  /// Lock this device
  virtual int lockDevice() { return(myDeviceMutex.lock());}
  /// Try to lock this device
  virtual int tryLockDevice() {return(myDeviceMutex.tryLock());}
  /// Unlock this device
  virtual int unlockDevice() {return(myDeviceMutex.unlock());}

  /// Lock this device. Same as lockDevice().
  virtual int lock() { return(myDeviceMutex.lock());}
  /// Try to lock this device. Same as tryLockDevice().
  virtual int tryLock() {return(myDeviceMutex.tryLock());}
  /// Unlock this device. Same as unlockDevice().
  virtual int unlock() {return(myDeviceMutex.unlock());}

#ifndef ARIA_WRAPPER
  /// Internal function to filter the readings based on age and distance
  /// @internal
  AREXPORT void filterCallback();
#endif

  /// Write log message containing current configuration and metadata
  AREXPORT void logConfig(ArLog::LogLevel level = ArLog::Normal, const char *prefix="");

  /// write log messages containing all current, cumulative and summary data
  AREXPORT void logData(ArLog::LogLevel level = ArLog::Normal, const char *prefix = "");

protected:
  /**
    This call should be called by the range device every robot cycle
    before the range device makes new readings (and even if it isn't
    adding any that cycle)... it will adjust the raw readings by the
    robot odometry offset.  The robot should be locked when this
    happens (which should be the case if you're doing it in the robot
    callback). The code currently assumes that all readings were taken
    at the same point, so if that isn't true with your device then you
    can't use this mechanism.
  **/
  AREXPORT void adjustRawReadings(bool interlaced);



  ArRobot *myRobot = nullptr;

  double myMinDistBetweenCurrent = 0;
  double myMinDistBetweenCurrentSquared = 0;
  double myMaxDistToKeepCumulative;
  double myMaxDistToKeepCumulativeSquared;
  double myMinDistBetweenCumulative;
  double myMinDistBetweenCumulativeSquared;

  double myMaxInsertDistCumulative = 0; 
  double myMaxInsertDistCumulativeSquared = 0;

  /// Subclasses may optionally set this pointer to a list of "raw" unprocessed readings. 
  // XXX TODO change these to list<ArSensorReading> pointers, maybe std::optional instead of allocating.
  std::list<ArSensorReading *> *myRawReadings = nullptr;

  /// Subclasses may optionally set this pointer to a list of "raw" unprocessed readings (adjusted for robot position)
  // XXX TODO change these to list<ArSensorReading> pointers, maybe std:optional instead of allocating.
  std::list<ArSensorReading *> *myAdjustedRawReadings = nullptr;

  ArDrawingData *myCurrentDrawingData = nullptr;
  ArDrawingData *myCumulativeDrawingData = nullptr;

  std::vector<ArSensorReading> myRawReadingsVector;
  std::vector<ArSensorReading> myAdjustedRawReadingsVector;

  ArPose myMaxInsertDistCumulativePose;

  std::string myName;

  ArFunctorC<ArRangeDevice> myFilterCB;

  ArRangeBuffer myCurrentBuffer;
  ArRangeBuffer myCumulativeBuffer;

  ArMutex myDeviceMutex;

  unsigned int myMaxRange; 
  int myMaxSecondsToKeepCurrent;
  int myMaxSecondsToKeepCumulative;

  bool myOwnCurrentDrawingData = false;
  bool myOwnCumulativeDrawingData = false;
  bool myIsLocationDependent = false;
};

#endif // ARRANGEDEVICE_H
