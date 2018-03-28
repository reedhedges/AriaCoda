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

#include "Aria.h"

/** @example customRangeSensorExampleRobotTask.cpp example showing how to implement an
 * ArRangeDevice class using an ArRobot task to update with new data.
 * @sa customRangeSensorExample.cpp
 *
 * This example uses an ArRobot task callback to update the range sensor data.
 */


class ExampleRangeDevice : public virtual ArRangeDevice
{
  ArFunctorC<ExampleRangeDevice> myUpdateTask;
  unsigned int myCounter;
  std::vector<ArSensorReading> myReadings;
public:
  ExampleRangeDevice() : ArRangeDevice(
    100,  // Current buffer size
    200,  // Cumulative buffer size
    "ExampleRangeDevice", // Name
    25000 // Maximum range
  ),
    myUpdateTask(this, &ExampleRangeDevice::updateTask),
    myCounter(0)
  {
    // this simulates something like a scanning laser rangefinder in which all
    // range measurements are at different angles relative to the same origin
    // point.   we will simulate a sensor placed on the robot 250 mm in front
    // of the robot's center point, and we will generate 5 fake readings: every
    // 9 degrees over a +/-45 degree range.
    for(float angle = -45; angle <= 45; angle += 10)
    {
      myReadings.push_back(ArSensorReading(250, 0, angle));
    }

    ArLog::log(ArLog::Normal, "ExampleRangeDevice: Will provide a set of %d readings each update", myReadings.size());

    // configure visualization properties:
    setCurrentDrawingData(new ArDrawingData("polyDots", ArColor(200, 50, 0), 100, 75), true);
    setCumulativeDrawingData(new ArDrawingData("polyDots", ArColor(100, 20, 0), 120, 74), true);
  }

  virtual ~ExampleRangeDevice()
  {
    if(ArRobot *r = getRobot())
      r->remSensorInterpTask("ExampleRangeDevice");
  }

  // ArRobot::addRangeDevice() calls ArRangeDevice::setRobot(), override to
  // get pointer to ArRobot object and add our task.  ArRangeDevice() also
  // stores a pointer to the ArRobot object which can be obtained from
  // ArRangeDevice::getRobot().
  virtual void setRobot(ArRobot *robot)
  {
    if(robot)
      robot->addSensorInterpTask("ExampleRangeDevice", 20, &myUpdateTask);
    ArRangeDevice::setRobot(robot);
  }

  /**
     @note since this is called from ArRobot's task thread, it is
     essential that no blocking or other calls of indeterminate or long
     duration are made here to fetch the data.  You should read new data only,
     don't wait on reading the data.  You can use an ArRangeDeviceThreaded
     instead to create a new thread to wait for data from the device if that
     is neccesary.
  */
  void updateTask()
  {
    lockDevice();

    // Set new fake data in our set of ArSensorReading objects. For a real
    // sensor, you would have received this data from the sensor device itself.
    // After updating each ArSensorReading object its position is added to our
    // ArRangeBuffer objects from the ArRangeDevice class.
    //
    // If you have an unknown or variable set of readings, you can resize the 
    // ArSensorReading vector like this:
    // @code
    //       myReadings.resize(size);
    // @endcode
    // However, new ArSensorReading objects added will be initialized to have an origin position on the
    // robot of (0,0,0) so you will need to reposition each ArSensorReading in
    // the vector with resetPosition().
    
    for(std::vector<ArSensorReading>::iterator i = myReadings.begin(); i != myReadings.end(); ++i)
    {
      // fake range. for a real sensor, use a range value provided by the
      // sensor:
      double fakeRange = 5000;

      // Get current state of robot. In this example, this method (updateTask())
      // is being called by ArRobot in its task thread. If you were acessing
      // ArRobot from a different thread instead (main thread, new thread), then
      // you must lock the robot while accessing it (robot->lock(),
      // robot->unlock()).
      ArRobot *robot = getRobot();
      ArPose robotPose = robot->getPose();
      ArPose robotEncoderPose = robot->getEncoderPose();
      ArTime currentTime; // will be set to current system time. You can adjust this if sensor has a constant latency sending the data or provides time delay information.
      ArTransform toGlobal = robot->getToGlobalTransform();
      ++myCounter;


      // Update the ArSensorReading object and add the global position to our
      // current buffer.
      i->newData(fakeRange, robotPose, robotEncoderPose, toGlobal, myCounter, currentTime);
      myCurrentBuffer.addReading(i->getX(), i->getY());

      ArLog::log(ArLog::Normal, "CustomSensorExample: Updated reading range %f => (%.2f, %2f)   (update #%u)", fakeRange, i->getX(), i->getY(), myCounter);
    }

    unlockDevice();
  }
    
};


int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ExampleRangeDevice exampleRangeDevice;

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "customRangeSensorExample: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "customRangeSensorExample: Connected to robot.");

  robot.addRangeDevice(&exampleRangeDevice);

  // For an ArNetworkingServer with an ArServerHandlerDrawings object named
  // drawings:
  // drawings.addRangeDevice(&exampleRangeDevice);

  robot.enableMotors();

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  while(robot.isRunning())
  {
    // Print out some data.  We must "lock" the ArRobot object
    // before calling its methods, and "unlock" when done, to prevent conflicts
    // with the background thread started by the call to robot.runAsync() above.
    // See the section on threading in the manual for more about this.
    robot.lock();
    bool soc = robot.hasStateOfCharge();
    float battv = 0.0;
    if(soc)
      battv = robot.getStateOfCharge();
    else
      battv = robot.getBatteryVoltage();
    ArLog::log(ArLog::Normal, "customRangeSensorExample: Pose=(%.2f,%.2f,%.2f), Trans.  Vel=%.2f, Battery=%.2f%c",
      robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), battv, soc?'%':'V');
    double angle = 0;
    double dist = robot.checkRangeDevicesCurrentPolar(-180, 180, &angle);
    ArLog::log(ArLog::Normal, "customRangeSensorExample: Closest range reading is %f mm at %f degrees.", dist, angle);
    robot.unlock();

    // Sleep for 0.5 seconds.
    ArUtil::sleep(500);
  }

  
  ArLog::log(ArLog::Normal, "customRangeSensorExample: Ending robot thread...");
  robot.stopRunning();

  // wait for the thread to stop
  robot.waitForRunExit();

  // exit
  ArLog::log(ArLog::Normal, "customRangeSensorExample: Exiting.");
  Aria::exit(0);
  return 0;
}
