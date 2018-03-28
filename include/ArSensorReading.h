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
#ifndef ARSENSORREADING_H
#define ARSENSORREADING_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArTransform.h"

/// Used to convert and store data from  and/or about a range sensor
/** This class holds sensor information and a sensor reading position and other
  data
    (X,Y location of the reading (typically in robot's global coordinate system) plus a counter and timestamp for that reading,
    position of the robot when the reading was taken, and other information).

    This class can optionally be used to only store information about the sensor, but no reading
    data, in which case the range (getRange()) will be -1, and the counter
    (getCounterTaken()) value will be 0, and isNew() will return false as well.
   
    The ignoreThisReading() indicates whether applications should ignore the
    data or not.   (Used to disable sensors data in robot and application
    configuration.)

    Typical use is to create an ArSensorReading object representing an indidiual
    sensor that can sense distance (range) in one direction, or a set of
    ArSensorReadings corresponding to the set of range data returned by a sensor
    that provides multiple range measurements (e.g. most scanning laser
    rangefinders provide a set of readings, each at different angles but from the
    same measurement origin point. The ArRangeDevice subclasses for laser
    rangefinders in ARIA use a set of ArSensorReading objects to store and convert
    the raw range data read from the laser rangefinder device, then those are used
    to update the "raw", "current" and other ArRangeBuffer objects in
    ArRangeDevice.)

    Provide the position and orientation of the sensor reading relative to the
    center of the robot in the ArSensorReading constructor, or call
    resetSensorPosition() to change.  

    Update data in an ArSensorReading object by calling newData().  The range
    value provided will be projected to a local cartesian cooridate based on the
    ArSensorReadings sensor position on the robot as supplied in the constructor or call to
    resetSensorPosition(), and alrso transformed a global coordinate system based on a supplied 
    transform (usually this is the robot's global coordinate system using
    ArRobot::getToGlobalTransform()).  An incrementing counter must also be provided, and
    a timestamp.  The counter is used to check for updated data (by this class
    and other classes using ArSensorReading objects), so it should 
    increment when data is updated.  The timestamp may be used by other classes 
    to determine age of data.  
*/

class ArSensorReading
{
public:
  /// Constructor, the three args are the physical location of the sensor
  AREXPORT ArSensorReading(double xPos = 0.0, double yPos = 0.0, double thPos = 0.0);
   /// Copy constructor
  AREXPORT ArSensorReading(const ArSensorReading & reading);
  /// Assignment operator
  AREXPORT ArSensorReading &operator=(const ArSensorReading &reading);
  /// Destructor
  AREXPORT virtual ~ArSensorReading();

  /// Gets the range from sensor of the reading
  /**
     @return the distance to the reading from the sensor itself
  */
  unsigned int getRange(void) const { return myRange; }

  /// Given the counter from the robot, it returns whether the reading is new
  /**
     @param counter the counter from the robot at the current time
     @return true if the reading was taken on the current loop
     @see getCounter
  */
  bool isNew(unsigned int counter) const { return counter == myCounterTaken; }
  /// Gets the X location of the sensor reading
  double getX(void) const { return myReading.getX(); }
  /// Gets the Y location of the sensor reading
  double getY(void) const { return myReading.getY(); }
  /// Gets the position of the reading 
  /// @return the position of the reading (ie where the sonar pinged back)
  ArPose getPose(void) const { return myReading; }

  /// Gets the X location of the sensor reading in local coords
  double getLocalX(void) const { return myLocalReading.getX(); }
  /// Gets the Y location of the sensor reading
  double getLocalY(void) const { return myLocalReading.getY(); }
  /// Gets the position of the reading 
  /// @return the position of the reading (ie the obstacle where the sonar pinged back)
  ArPose getLocalPose(void) const { return myLocalReading; }

  /** Gets the pose of the robot at which the reading was taken 
      @sa getEncoderPoseTaken()
      @sa getTimeTaken()
      @sa ArRobot::getPose()
  */
  ArPose getPoseTaken(void) const { return myReadingTaken; }

  /** Gets the robot's encoder pose the reading was taken at
      @sa getPoseTaken()
      @sa ArRobot::getEncoderPose()
  */
  ArPose getEncoderPoseTaken(void) const { return myEncoderPoseTaken; }

  /** Gets the X location of the sonar on the robot
      @sa getSensorPosition()
  */
  double getSensorX(void) const { return mySensorPos.getX(); }
  /** Gets the Y location of the sensor on the robot
      @sa getsensorPosition()
  */
  double getSensorY(void) const { return mySensorPos.getY(); }
  /** Gets the heading of the sensor on the robot
      @sa getsensorPosition()
  */
  double getSensorTh(void) const { return mySensorPos.getTh(); }
  
  /// Gets whether this reading should be ignore or not. e.g. the sensor
  /// encountered an error or did not actually detect anything.
  bool getIgnoreThisReading(void) const { return myIgnoreThisReading; }

  /// Gets the extra int with this reading
  /**
     Some range devices provide extra device-dependent information
     with each reading.  What that means depends on the range device,
     if a range device doesn't give the meaning in its constructor
     description then it has no meaning at all.

     Note that for all laser like devices this should be a value
     between 0 - 255 which is the measure of reflectance.  It should
     be 0 if that device doesn't measure reflectance (the default).
   **/
  int getExtraInt(void) const { return myExtraInt; }


  /// Gets the sensor's position on the robot
  /** 
      @return the position of the sensor on the robot
  */
  ArPose getSensorPosition(void) const { return mySensorPos; }

  /// Gets the cosine component of the heading of the sensor reading
  double getSensorDX(void) const { return mySensorCos; }
  /// Gets the sine component of the heading of the sensor reading
  double getSensorDY(void) const { return mySensorSin; }

  /** Gets the X locaiton of the robot when the reading was received
      @sa getPoseTaken()
  */
  double getXTaken(void) const { return myReadingTaken.getX(); }
  /** Gets the Y location of the robot when the reading was received
      @sa getPoseTaken()
  */
  double getYTaken(void) const { return myReadingTaken.getY(); }
  /** Gets the th (heading) of the robot when the reading was received
      @sa getPoseTaken()
  */
  double getThTaken(void) const { return myReadingTaken.getTh(); }

  /// Gets the counter from when the reading arrived
  /**
     @return the counter from the robot when the sonar reading was taken
     @see isNew
  */
  unsigned int getCounterTaken(void) const { return myCounterTaken; }

  ArTime getTimeTaken(void) const { return myTimeTaken; }
  
  /**
    Update data. 
    @param range Sensed distance. Will be projected to a global X,Y position based on the sensor position and @a robotPose
    @param robotPose Robot position in global coordinates space when the sensor data was received.
    @param encoderPose Robot encoder-only position in global coordinate space when the sensor data was received.
    @param trans Transform reading position from robot-local coordinate system.
For example, pass result of ArRobot::getToGlobalTransform() transform to robot's global
coordinate system.
    @param counter an incrementing counter used to check for updated data (by this class
    and other classes using ArSensorReading objects)
    @param timeTaken System time when this measurement was taken or received.
    @param ignoreThisReading Set the "ignore" flag for this reading. Data is stored but applications (e.g. navigation) may use this flag to ignore some sensor readings based on robot or user configuration.
    @param extraInt extra device-specific data. @see getExtraInt()
*/
  AREXPORT void newData(int range, ArPose robotPose, ArPose encoderPose,
			ArTransform trans, unsigned int counter, 
			ArTime timeTaken, bool ignoreThisReading = false,
			int extraInt = 0);

  /**
    Update data. 
    @param sx the coords of the sensor return relative to sensor (mm)
    @param sy the coords of the sensor return relative to sensor (mm)
    @param robotPose Robot position in global coordinates space when the sensor data was received.
    @param encoderPose Robot encoder-only position in global coordinate space when the sensor data was received.
    @param trans Transform reading position from robot-local coordinate system.
For example, pass result of ArRobot::getToGlobalTransform() transform to robot's global
coordinate system.
    @param counter an incrementing counter used to check for updated data (by this class
    and other classes using ArSensorReading objects)
    @param timeTaken System time when this measurement was taken or received.
    @param ignoreThisReading Set the "ignore" flag for this reading. Data is stored but applications (e.g. navigation) may use this flag to ignore some sensor readings based on robot or user configuration.
    @param extraInt extra device-specific data. @see getExtraInt()
  */
  AREXPORT void newData(int sx, int sy, ArPose robotPose,
			ArPose encoderPose,
			ArTransform trans, 
			unsigned int counter,
			ArTime timeTaken,
			bool ignoreThisReading = false,
			int extraInt = 0);

  /// Resets the sensors idea of its physical location on the robot
  AREXPORT void resetSensorPosition(double xPos, double yPos, double thPos,
				    bool forceComputation = false);

  /// Sets that we should ignore this reading
  AREXPORT void setIgnoreThisReading(bool ignoreThisReading) 
    { myIgnoreThisReading = ignoreThisReading; }

  /// Sets the extra int
  AREXPORT void setExtraInt(int extraInt) 
    { myExtraInt = extraInt; }


  /// Applies a transform to the reading position, and where it was taken
  /// @internal
  AREXPORT void applyTransform(ArTransform trans);
  /// Applies a transform to the encoder pose taken
  /// @internal
  AREXPORT void applyEncoderTransform(ArTransform trans);
  /// Whether a transform to this reading's position was applied (An adjustment
  /// transform due to robot position and motion, etc. is normally initiated
  /// automatically by the range device class which is providing this sensor
  /// reading.)
  AREXPORT bool getAdjusted(void) { return myAdjusted; }
  /// Applies a transform to the reading position, and where it was taken
  /// @internal
  AREXPORT void setAdjusted(bool adjusted) { myAdjusted = adjusted; }
protected:
  unsigned int myCounterTaken;
  ArPose myReading;
  ArPose myLocalReading;  
  ArPose myReadingTaken;
  ArPose myEncoderPoseTaken;
  ArPose mySensorPos;
  double mySensorCos, mySensorSin;
  double myDistToCenter;
  double myAngleToCenter;
  int myRange;
  ArTime myTimeTaken;
  bool myIgnoreThisReading;
  int myExtraInt;
  bool myAdjusted;
};

#endif
