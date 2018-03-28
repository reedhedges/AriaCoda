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
#ifndef ARTCM2_H
#define ARTCM2_H

#include "ariaUtil.h"
#include "ArFunctor.h"
#include "ArRobot.h"


#ifdef WIN32
#define ARTCM2_DEFAULT_SERIAL_PORT "COM4"
#else
#define ARTCM2_DEFAULT_SERIAL_PORT "/dev/ttyS3"
#endif


/** Interface to the PNI TCM 2, TCM 2.5, and TCM 2.6  3-axis compass (magnetometer)  that can sense absolute heading, as well as pitch, roll, and includes a temperature sensor.
 *
 * See subclasses and ArCompassConnector for more information, as well as 
 * your robot manuals.  PNI's TCM manual is also available for download at the 
 * MobileRobots support website, documentation section.
 *
 * This is an abstract interface. To create a compass interface object,
 * instantiate a subclass or use ArCompassConnector.

    @ingroup OptionalClasses
  @ingroup DeviceClasses
 *
 * @note The compass returns a heading relative to magnetic north, which varies
 * depending on your location in the Earth's magnetic field (declination also
 * varies slowly over time as the Earth's magnetic field changes).  To find true
 * north, you must apply an offset, or declination. For example, on 
 * June 1, 2007, the magnetic declination of MobileRobots' location 
 * of Amherst, NH, USA was 15 1/6 degrees West, so a value of approx. 
 * -15.166666 would have to be applied. You can look up declination values for 
 * various locations on Earth at 
 * http://www.ngdc.noaa.gov/seg/geomag/jsp/Declination.jsp
 */
class ArTCM2
{
public:
  
  AREXPORT ArTCM2();
  virtual ~ArTCM2() {}

  /** If a connection/initialization procedure is required, perform it, and
      return true on success, false on failure. Otherwise, just return true.
  */
  AREXPORT virtual bool connect();

  /** If a connection/initialization procedure is required, perform it, wait
   * until data is recieved from the compass, and
      return true on success, false on failure. Otherwise, just return true.
  */
  AREXPORT virtual bool blockingConnect(unsigned long connectTimeout = 5000);


  /// Get the compass heading (-180, 180] degrees
  double getHeading(void) const { return myHeading; }
  bool haveHeading() const { return myHaveHeading; }

  /** Get the compass heading (-180, 180] degrees
   *  @deprecated Use getHeading()
   */
  double getCompass(void) const { return getHeading(); }

  /// Get the pitch (-180,180] degrees.
  double getPitch(void) const { return myPitch; }
  bool havePitch() const { return myHavePitch; }

  /// Get the roll (-180,180] degrees.
  double getRoll(void) const { return myRoll; }
  bool haveRoll() const { return myHaveRoll; }

  /// Get the magnetic field X component (See TCM2 Manual)
  double getXMagnetic(void) const { return myXMag; }
  bool haveXMagnetic() const { return myHaveXMag; }

  /// Get the magnetic field Y component (See TCM2 Manual)
  double getYMagnetic(void) const { return myYMag; }
  bool haveYMagnetic() const { return myHaveYMag; }

  /// Get the magnetic field Z component (See TCM2 Manual)
  double getZMagnetic(void) const { return myZMag; }
  bool haveZMagnetic() const { return myHaveZMag; }

  /// Get the temperature (degrees C)
  double getTemperature(void) const { return myTemperature; }
  bool haveTemperature() const { return myHaveTemperature; }

  // Get last error code (see TCM manual) recieved. 0 means no error recieved.
  int getError(void) const { return myError; }

  /// Get the calibration H score (See TCM Manual)
  double getCalibrationH(void) const { return myCalibrationH; }
  bool haveCalibrationH() const { return myHaveCalibrationH; }

  /// Get the calibration V score (See TCM Manual)
  double getCalibrationV(void) const { return myCalibrationV; }
  bool haveCalibrationV() const { return myHaveCalibrationV; }

  /// Get the calibration M score (See TCM Manual)
  double getCalibrationM(void) const { return myCalibrationM; }
  bool haveCalibrationM() const { return myHaveCalibrationM; }

  /// Turn sending of data off (Compass remains powered on, this is not its not low power standby mode)
  virtual void commandOff(void) = 0;

  /// Get one reading from the compass
  virtual void commandOnePacket(void) = 0;

  /// Start the compass sending a continuous stream of readings at its fastest rate
  virtual void commandContinuousPackets(void) = 0;

  /// Start user calibration
  virtual void commandUserCalibration(void) = 0;

  /// Start auto calibration
  virtual void commandAutoCalibration(void) = 0;

  /// Stop calibration
  virtual void commandStopCalibration(void)  = 0;

  /// Command to do a soft reset of the compass
  virtual void commandSoftReset(void) = 0;

  /// Command to just send compass heading data
  virtual void commandJustCompass(void) = 0;

  /// Gets the number of readings recieved in the last second
  int getPacCount(void) {
    if(myTimeLastPacket == time(NULL)) return myPacCount;
    if(myTimeLastPacket == time(NULL) - 1) return myPacCurrentCount;
    return 0;
  }

  // Add a callback to be invoked when a new heading is recieved
  void addHeadingDataCallback(ArFunctor1<double> *f) {
    myHeadingDataCallbacks.push_back(f);
  }


protected:  
  double myHeading;
  double myPitch;
  double myRoll;
  double myXMag;
  double myYMag;
  double myZMag;
  double myTemperature;
  int myError;
  double myCalibrationH;
  double myCalibrationV;
  double myCalibrationM;

  bool myHaveHeading;
  bool myHavePitch;
  bool myHaveRoll;
  bool myHaveXMag;
  bool myHaveYMag;
  bool myHaveZMag;
  bool myHaveTemperature;
  bool myHaveCalibrationH;
  bool myHaveCalibrationV;
  bool myHaveCalibrationM;

  std::list< ArFunctor1<double>* > myHeadingDataCallbacks;

  // packet count
  time_t myTimeLastPacket;
  int myPacCurrentCount;
  int myPacCount;		

  void incrementPacketCount() {
    if (myTimeLastPacket != time(NULL)) 
    {
      myTimeLastPacket = time(NULL);
      myPacCount = myPacCurrentCount;
      myPacCurrentCount = 0;
    }
    myPacCurrentCount++;
  }


  // call the heading data callbacks
  void invokeHeadingDataCallbacks(double heading) {
    for(std::list<ArFunctor1<double>*>::iterator i = myHeadingDataCallbacks.begin(); i != myHeadingDataCallbacks.end(); ++i)
      if(*i) (*i)->invoke(heading);
  }
  
};


/** Use this class to create an instance of a TCM2 subclass
 * and connect to the device based on program command line options. 
 * This allows the user of a program to select a different kind
 * of compass configuration (for example, if the compass is connected
 * to a computer serial port, use ArTCMCompassDirect instead of
 * the normal ArTCMCompassRobot.)
 *
 * The following command-line arguments are checked:
 * @verbinclude ArCompassConnector_options
 */
class ArCompassConnector
{
protected:
  ArArgumentParser *myArgParser;
  ArRetFunctorC<bool, ArCompassConnector> myParseArgsCallback;
  ArFunctorC<ArCompassConnector> myLogArgsCallback;
  typedef enum {
    Robot,
    SerialTCM,
    None
  } DeviceType;
  DeviceType myDeviceType;
  const char *mySerialPort;
  AREXPORT bool parseArgs();
  ArFunctor *mySerialTCMReadFunctor;
  ArRobot *myRobot;
  AREXPORT void logOptions();
public:
  AREXPORT ArCompassConnector(ArArgumentParser *argParser);
  AREXPORT ~ArCompassConnector();
  AREXPORT ArTCM2 *create(ArRobot *robot);
  AREXPORT bool connect(ArTCM2*) const;
};

#endif // ARTCM2_H
