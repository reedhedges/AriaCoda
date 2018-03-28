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
#ifndef ARSEEKURIMU_H
#define ARSEEKURIMU_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArFunctor.h"

class ArRobot;
class ArRobotPacket;

/// Access extra data provided by optional Seekur IMU
/**
   Seekur and Seekur Jr. robots have either a single-axis "gyro" (accelerometor)
   or 6-axis IMU connected to the robot controller. The IMU is optional 
   and provides acceleration on 3 linear axes and rotation data on pitch, yaw
   and roll.  The standard Gyro provides yaw rotation data.
   The Gyro/IMU's primary purpose is to improve large yaw rotation ("theta" component of
   robot pose) errors due to wheel
   slippage, wheels coming off the ground, and similar factors.
   The robot controller will automatically use the yaw (Z) rotational axis 
   of the IMU to correct heading (theta), but you can use this class
   to also get additional data if you have the optional IMU, such as 
   rotational movement data from all three rotational axes,
   and linear acceleration in three axes.

   If rotational speeds are received, it will be integrated over time to compute
   an approximate rotational position as well.
    
   If your Seekur robot is not equipped with the optional IMU, then 
   a single-axis accelerometer (gyro) is used instead, and only Z rotational
   speed will be available.

   The IMU self-calibrates during robot startup, so the robot should
   remain stationary for about a minute after starting up.  If an
   ArSeekurIMU object is created before robot connection, ArSeekurIMU
   will delay completion of robot connection for a few seconds 
   while it reads some initial data, this will help let the IMU
   stabilize and self calibrate during startup.

   Note: In each robot cycle (100ms), the robot may provide multiple IMU
   readings acquired since the previous cycle.  These are sent together in one
   packet.   Only the last acceleration and speed values are stored in
   myAccel and mySpeed, but each reading is used to compute myPos (assuming
   that they were acquired by the robot at regular intervals over the last
   100ms robot cycle.)  (A possible improvement to this class is to also store
   a short history of acceleration and speed values. Let us know if you would
   like to contribute any such improvements.)

   Speed, acceleration and position data can be obtained as an array of
   three components (the Vec3<double> type), or a value from an individual
   axis can be returned.  Yaw and Z refer to the same data; Roll and X are
   equivalent, as are Pitch and Y. These are just different ways to access 
   the same received data.

   @ingroup DeviceClasses

*/
class ArSeekurIMU
{
public:
  AREXPORT ArSeekurIMU(ArRobot *robot);
  AREXPORT virtual ~ArSeekurIMU();

  typedef double Vec3[3];

  /// @return true if a valid IMU packet was received from the robot in the most recent cycle, false if not.
  bool haveData() const { return myHaveData; }

  /// @return 3 if we have received IMU data for 3 axes, 1 if one (Z/Yaw) axis, 0 if no data has been received.
  int numAxes() const { return myNumAxes; }    

  const Vec3& getSpeed() const { return mySpeed; }

  double getYawSpeed() const { return mySpeed[Z]; }
  double getZSpeed() const { return mySpeed[Z]; }

  double getRollSpeed() const { return mySpeed[X]; }
  double getXSpeed() const { return mySpeed[X]; }

  double getPitchSpeed() const { return mySpeed[Y]; }
  double getYSpeed() const { return mySpeed[Y]; }

  const Vec3& getAccel() const { return myAccel; }

  double getYawAccel() const { return myAccel[Z]; }
  double getZAccel() const { return myAccel[Z]; }

  double getRollAccel() const { return myAccel[X]; }
  double getXAccel() const { return myAccel[X]; }

  double getPitchAccel() const { return myAccel[Y]; }
  double getYAccel() const { return myAccel[Y]; }

  const Vec3& getPos() const { return myPos; }

  double getYawPos() const { return myPos[Z]; }
  double getZPos() const { return myPos[Z]; }

  double getRollPos() const { return myPos[X]; }
  double getXPos() const { return myPos[X]; }

  double getPitchPos() const { return myPos[Y]; }
  double getYPos() const { return myPos[Y]; }
  
  /// Gets the average temperature the IMU has on all axes
  float getTemperature(void) const { return myAvgTemperature; }

  ArTime getTimeLastPacketReceived() { return myTimeLastPacketReceived; }

private:
  /// Internal packet handler for the IMU packets
  bool handleIMUPacket(ArRobotPacket *packet);

  /// Internal connection callback; delays for a short amount of time to give the IMU enough time to stabilize before we try to use it, then requests data.
  void stabilizingCallback(void);

  bool myHaveData;
  int myNumAxes;

  ArTime myTimeLastPacketReceived;

  enum {X=0, Y=1, Z=2};

  double mySpeed[3];
  double myAccel[3];
  double myPos[3];

  float myAvgTemperature;

  ArRobot *myRobot;
  ArRetFunctor1C<bool, ArSeekurIMU, ArRobotPacket *> myHandleIMUPacketCB;
  ArFunctorC<ArSeekurIMU> myStabilizingCB;

};

#endif // ARANALOGGYRO_H


