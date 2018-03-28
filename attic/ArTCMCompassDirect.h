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

#ifndef ARTCMCOMPASSDIRECT_H
#define ARTCMCOMPASSDIRECT_H


#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArDeviceConnection.h"
#include "ArTCM2.h"
#include "ArNMEAParser.h"


/** @brief Talk to a compass directly over a computer serial port
 *  
 *  This class configures and recieves compass heading data from a TCM2 or TCM2.5 compass
 *  directly over a computer serial port, rather than via tha robot.
 *  This class cannot recieve pitch, roll or temperature data from the compass.
 *  On all Pioneer robots, the TCM compass (as originally installed by
 *  MobileRobots) is connected to the robot microcontroller, so if you 
 *  have a Pioneer with this configuration, you  should instead use the 
 *  ArTCMCompassRobot class. Only use this class if you
 *  have connected the compass to the computer serial port.
 *
 *  You can create an instance of this class directly, or using an
 *  ArCompassConnector object and giving the "-compassType serialtcm" program
 *  argument. 
 *
 *  If you create your own ArTCMCompassDirect object, you must call the read()
 *  method periodically (ideally at the same rate the compass sends data,
 *  approx. 8 hz by default) to read and parse incoming data. You can use an
 *  ArRobot callback to do this, for example:
 *  @code
 *    ArRetFunctor1C<int, ArTCMCompassDirect, unsigned int> compassReadFunc(myCompass, &ArTCMCompassDirect::read, 10);
 *    myRobot->addSensorInterpTask("ArTCMCompassDirect read", 200, &compassReadFunc);
 *  @endcode
 *
 *  If you use ArCompassConnector, however, it will automatically do this.
 *
 */
class ArTCMCompassDirect : public virtual ArTCM2  
{
private:
  ArDeviceConnection *myDeviceConnection;
  bool myCreatedOwnDeviceConnection;
  const char *mySerialPortName;
  ArNMEAParser myNMEAParser;
  bool sendTCMCommand(const char *str, ...);
  ArFunctor1C<ArTCMCompassDirect, ArNMEAParser::Message>  myHCHDMHandler;
  void handleHCHDM(ArNMEAParser::Message);
public:
  AREXPORT ArTCMCompassDirect(ArDeviceConnection *devCon);
  AREXPORT ArTCMCompassDirect(const char *serialPortName = ARTCM2_DEFAULT_SERIAL_PORT);
  AREXPORT ~ArTCMCompassDirect();

  /** Open device connection if not yet open  and send commands to configure compass. */
  AREXPORT virtual bool connect();
  AREXPORT virtual bool blockingConnect(unsigned long connectTimeout = 5000);


  /** Send commands to begin calibrating */
  AREXPORT virtual void commandAutoCalibration();
  AREXPORT virtual void commandUserCalibration();
  AREXPORT virtual void commandStopCalibration();

  /** Send commands to start/stop sending data.  */
  //@{
  AREXPORT virtual void commandContinuousPackets();
  AREXPORT virtual void commandOnePacket();
  AREXPORT virtual void commandOff();
  //@}

  /** Not implemented yet. @todo */
  AREXPORT virtual void commandSoftReset() { /* TODO */ }

  /** Same as commandContinuousPackets() in this implementation. */
  AREXPORT virtual void commandJustCompass() { commandContinuousPackets(); }

  /** Read all available data, store, and call callbacks if any were added. 
   *  unsigned int msWait If 0, wait indefinately for new data. Otherwise, wait
   *  a maximum of this many milliseconds for data to arrive.
   *  @return A value > 0 if messages were recieved from the compass, 0 if no
   *  data was recieved, and a value < 0 on error reading from the compass.
   * */
  AREXPORT int read(unsigned int msWait = 1);
 
  AREXPORT void setDeviceConnection(ArDeviceConnection *devCon) { myDeviceConnection = devCon; }
  AREXPORT ArDeviceConnection *getDeviceConnetion() { return myDeviceConnection; }


};

#endif 


