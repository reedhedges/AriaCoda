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
#include "ArVersalogicIO.h"
#include "ariaInternal.h"

#include <sys/ioctl.h>
#include <fcntl.h>

/* These are taken from the amrio_dev.h file, which is part of the amrio module itself */

#define AMRIO_MAJOR_NUM       (250)
#define AMRIO_MODULE_NAME     ("amrio")

#define ANALOG_GET_VALUE                _IOWR('v', 1, unsigned int)
#define ANALOG_SET_PORT                 _IOWR('v', 2, unsigned char)
#define DIGITAL_GET_NUM_BANKS           _IOWR('v', 3, int)
#define DIGITAL_SET_BANK0               _IOWR('v', 4, unsigned char)
#define DIGITAL_SET_BANK1               _IOWR('v', 5, unsigned char)
#define DIGITAL_SET_BANK2               _IOWR('v', 6, unsigned char)
#define DIGITAL_SET_BANK3               _IOWR('v', 7, unsigned char)
#define DIGITAL_SET_BANK_DIR_IN         _IOWR('v', 8, int)
#define DIGITAL_SET_BANK_DIR_OUT        _IOWR('v', 9, int)
#define DIGITAL_GET_BANK0               _IOWR('v', 10, unsigned char)
#define DIGITAL_GET_BANK1               _IOWR('v', 11, unsigned char)
#define DIGITAL_GET_BANK2               _IOWR('v', 12, unsigned char)
#define DIGITAL_GET_BANK3               _IOWR('v', 13, unsigned char)
#define GET_SPECIAL_CONTROL_REGISTER    _IOWR('v', 14, unsigned char)

ArMutex ArVersalogicIO::myMutex;

/** Constructor for the ArVersalogicIO class.  This will open the device
 * named by @a dev (the default is "/dev/amrio" if the argument is omitted).
 It will find the number of digital banks and set the to inputs.  It will also
 attempt to take an analog reading, which will fail if there is not analog chip
 present.  If the conversion fails it will disable the analog portion of the
 code.

 Check isEnabled() to see if the device was properly opened during construction.
*/
AREXPORT ArVersalogicIO::ArVersalogicIO(const char * dev) :
  myDisconnectCB(this, &ArVersalogicIO::closeIO)
{
  myMutex.setLogName("ArVersalogicIO::myMutex");
  myNumBanks = 0;

  myDigitalBank0 = 0;
  myDigitalBank1 = 0;
  myDigitalBank2 = 0;
  myDigitalBank3 = 0;

  ArLog::log(ArLog::Terse, "ArVersalogicIO::ArVersalogicIO: opening device %s", dev);
  myFD = ArUtil::open(dev, O_RDWR);

  if (myFD == -1)
  {
    ArLog::log(ArLog::Terse, "ArVersalogicIO::ArVersalogicIO: open %s failed.  Disabling class", dev);
    myEnabled = false;
  }
  else
  {
    if(ioctl(myFD, DIGITAL_GET_NUM_BANKS, &myNumBanks))
      ArLog::log(ArLog::Terse, "ArVersalogicIO::ArVersalogicIO: failed to get the number of digital IO banks");
    // set the digital banks to inputs so as to not drive anything
    int i;
    for(i=0;i<myNumBanks;i++)
      setDigitalBankDirection(i, DIGITAL_INPUT);
    myEnabled = true;

    // check to see if the analog works
    double val;
    ArLog::log(ArLog::Verbose, "ArVersalogicIO::ArVersalogicIO: testing analog functionality");
    if ((ioctl(myFD, ANALOG_SET_PORT, 0) != 0) || (ioctl(myFD,ANALOG_GET_VALUE, &val) != 0))
    {
      ArLog::log(ArLog::Verbose, "ArVersalogicIO::ArVersalogicIO: analog conversion failed.  Disabling analog functionality");
      myAnalogEnabled = false;
    }
    else
    {
      ArLog::log(ArLog::Verbose, "ArVersalogicIO::ArVersalogicIO: analog conversion succeeded.");
      myAnalogEnabled = true;
    }

    Aria::addExitCallback(&myDisconnectCB);
    ArLog::log(ArLog::Terse, "ArVersalogicIO::ArVersalogicIO: device opened");
  }
}

/** Destructor.  Attempt to close the device if it was opened
 **/
AREXPORT ArVersalogicIO::~ArVersalogicIO(void)
{
  if (myEnabled)
    closeIO();
}

/** Close the device when Aria exits
 **/
AREXPORT bool ArVersalogicIO::closeIO(void)
{
  myEnabled = false;

  if (close(myFD) == -1)
  {
    ArLog::log(ArLog::Terse, "ArVersalogicIO::~ArVersalogicIO: close failed on file descriptor!");
    return false;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArVersalogicIO::~ArVersalogicIO: closed device");
    return true;
  }
}

/** Returns the raw bit value as read by the chip.

 @param port the port number, between 0 and 7
 @param val the address of the integer to store the reading in
 @return true if a reading was acquired.  false otherwise
 **/
AREXPORT bool ArVersalogicIO::getAnalogValueRaw(int port, int *val)
{
  if (!myEnabled || !myAnalogEnabled)
    return false;

  unsigned int tmp;

  if (ioctl(myFD, ANALOG_SET_PORT, port) != 0)
  {
    ArLog::log(ArLog::Terse, "ArVersalogicIO::getAnalogValueRaw: failed to set analog port %d", port);
    return false;
  }

  if (ioctl(myFD, ANALOG_GET_VALUE, &tmp) != 0)
  {
    ArLog::log(ArLog::Terse, "ArVersalogicIO::getAnalogValueRaw: failed to get analog port %d", port);
    return false;
  }

  *val = (int) tmp;

  return true;
}

/** Returns an analog value converted from the raw reading to a scale
 if 0-5V.

 @param port the port number, between 0 and 7
 @param val the address of the double to store the reading in
 @return true if a reading was acquired.  false otherwise
 **/
AREXPORT bool ArVersalogicIO::getAnalogValue(int port, double *val)
{
  int tmp;

  // the reading is 4096 units, for a range of 0-5V
  if (getAnalogValueRaw(port, &tmp)) {
    *val = 5 * (tmp/4096.0);
    return true;
  }
  else {
    return false;
  }
}

// this doesn't work, yet
AREXPORT ArVersalogicIO::Direction ArVersalogicIO::getDigitalBankDirection(int bank)
{
  return DIGITAL_OUTPUT;
}

AREXPORT bool ArVersalogicIO::setDigitalBankDirection(int bank, Direction dir)
{
  if (!myEnabled)
    return false;

  if (dir == DIGITAL_INPUT)
  {
    if (ioctl(myFD, DIGITAL_SET_BANK_DIR_IN, bank) != 0)
      return false;
  }
  else if (dir == DIGITAL_OUTPUT)
  {
    if (ioctl(myFD, DIGITAL_SET_BANK_DIR_OUT, bank) != 0)
      return false;
  }
  else
  {
    ArLog::log(ArLog::Verbose, "ArVersalogicIO::setDigitalBankDirection: invalid argument for direction");
    return false;
  }

  return true;
}

/** Returns the bits of the digital input bank.  A zero is 'on' for the hardware
 side of things, so this function negates it to make it more intuitive.
 
  @return true if the ioctl call was succcessfull, false otherwise
  **/
AREXPORT bool ArVersalogicIO::getDigitalBankInputs(int bank, unsigned char *val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  unsigned char tmp;
  switch (bank)
  {
    case 0:
      if (ioctl(myFD, DIGITAL_GET_BANK0, &tmp) != 0)
	return false;
      break;
    case 1:
      if (ioctl(myFD, DIGITAL_GET_BANK1, &tmp) != 0)
	return false;
      break;
    case 2:
      if (ioctl(myFD, DIGITAL_GET_BANK2, &tmp) != 0)
	return false;
      break;
    case 3:
      if (ioctl(myFD, DIGITAL_GET_BANK3, &tmp) != 0)
	return false;
      break;
    default:
      return false;
  }

  // bitwise negate it because the hardware calls a 'high' off
  *val = ~tmp;
  return true;
}


/** Returns the state of the bits as they are currently set in the outputs.  It
 doesn't reconfirm with the hardware, but instead keeps track of how it last
 set them.
 @param bank which bank to check
 @param val the byte to write the values into
 @return true if the request was satisfied, false otherwise
 **/
AREXPORT bool ArVersalogicIO::getDigitalBankOutputs(int bank, unsigned char *val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  // should check for input vs. output
  switch (bank)
  {
    case 0:
      *val = myDigitalBank0;
      break;
    case 1:
      *val = myDigitalBank1;
      break;
    case 2:
      *val = myDigitalBank2;
      break;
    case 3:
      *val = myDigitalBank3;
      break;
  }

  return true;
}

/** The bits on the hardware side of the digital I/O ports are inverse-logic.
  The bit must be set high in the register for the output to be off, and be
  set low to be turned on.  This function negates that so that it is more
  intuitive.

  @param bank the bank number to use.  0 is the lowest bank
  @param val the status of the 8-bits in a single byte.  
 **/
AREXPORT bool ArVersalogicIO::setDigitalBankOutputs(int bank, unsigned char val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  // bitwise negate val because the hardware calls a 'high' off
  switch (bank)
  {
    case 0:
      if (ioctl(myFD, DIGITAL_SET_BANK0, ~val) != 0)
	return false;
      myDigitalBank0 = val;
      break;
    case 1:
      if (ioctl(myFD, DIGITAL_SET_BANK1, ~val) != 0)
	return false;
      myDigitalBank1 = val;
      break;
    case 2:
      if (ioctl(myFD, DIGITAL_SET_BANK2, ~val) != 0)
	return false;
      myDigitalBank2 = val;
      break;
    case 3:
      if (ioctl(myFD, DIGITAL_SET_BANK3, ~val) != 0)
	return false;
      myDigitalBank3 = val;
      break;
    default:
      return false;
  }

  return true;
}

/** The special_control_register contains various status bits, of which can
  be found in the manuals for the motherboards.  One interesting bit is the
  temperature bit, which gets set high if the CPU is over the specified threshold
  as set in the BIOS

 **/
AREXPORT bool ArVersalogicIO::getSpecialControlRegister(unsigned char *val)
{
  if (!myEnabled)
    return false;

  unsigned char tmp;
  if (ioctl(myFD, GET_SPECIAL_CONTROL_REGISTER, &tmp) != 0)
    return false;

  *val = tmp;
  return true;
}
