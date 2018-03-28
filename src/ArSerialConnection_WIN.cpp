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
#include "ArSerialConnection.h"
#include "ArLog.h"
#include "ariaUtil.h"


// PS 7/11/11 - nothing is done different for windows for 422
AREXPORT ArSerialConnection::ArSerialConnection(bool is422)
{
  myPort = INVALID_HANDLE_VALUE;
  myBaudRate = 9600;
  myStatus = STATUS_NEVER_OPENED;
  myHardwareControl = false;
  buildStrMap();
  if (is422)
    setPortType("serial422");
  else
    setPortType("serial");
}

AREXPORT ArSerialConnection::~ArSerialConnection()
{
  if (myPort != INVALID_HANDLE_VALUE)
    close();
}

void ArSerialConnection::buildStrMap(void)
{
  myStrMap[OPEN_COULD_NOT_OPEN_PORT] = "Could not open serial port.";
  myStrMap[OPEN_COULD_NOT_SET_UP_PORT] = "Could not set up serial port.";
  myStrMap[OPEN_INVALID_BAUD_RATE] = "Baud rate invalid, could not set baud on serial port.";
  myStrMap[OPEN_COULD_NOT_SET_BAUD] = "Could not set baud rate on serial port.";
  myStrMap[OPEN_ALREADY_OPEN] = "Serial port already open.";
}

AREXPORT const char * ArSerialConnection::getOpenMessage(int messageNumber)
{
  return myStrMap[messageNumber].c_str();
}

AREXPORT bool ArSerialConnection::openSimple(void)
{
  if (internalOpen() == 0)
    return true;
  else
    return false;
}

/**
   @param port The serial port to connect to, or NULL which defaults to 
   COM1 for windows and /dev/ttyS0 for linux
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
AREXPORT void ArSerialConnection::setPort(const char *port)
{
  if (port == NULL)
    myPortName = "COM1";
  else
    myPortName = port;
  setPortName(myPortName.c_str());
}

/**
   @return The seiral port to connect to
**/
AREXPORT const char * ArSerialConnection::getPort(void)
{
  return myPortName.c_str();
}

/**
   @param port The serial port to connect to, or NULL which defaults to 
   COM1 for windows and /dev/ttyS0 for linux
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
AREXPORT int ArSerialConnection::open(const char *port)
{
  setPort(port);
  return internalOpen();
}



AREXPORT int ArSerialConnection::internalOpen(void)
{
  DCB dcb;


  if (myStatus == STATUS_OPEN) 
  {
    ArLog::log(ArLog::Terse, 
	       "ArSerialConnection::open (%s): Serial port already open", getPort());
    return OPEN_ALREADY_OPEN;
  }

  ArLog::log(ArLog::Verbose, "ArSerialConnection::internalOpen: Connecting to serial port '%s'", myPortName.c_str());


  myPort = CreateFile(myPortName.c_str(),
		      GENERIC_READ | GENERIC_WRITE,
		      0,	/* exclusive access  */
		      NULL,	/* no security attrs */
		      OPEN_EXISTING,
		      0,
		      NULL );

  if (myPort == INVALID_HANDLE_VALUE) {
    ArLog::logErrorFromOS(ArLog::Terse, 
	       "ArSerialConnection::open: Could not open serial port '%s'",
	       myPortName.c_str());
    return OPEN_COULD_NOT_OPEN_PORT;
  }
             
  if ( !GetCommState(myPort, &dcb) )
  {  
    ArLog::logErrorFromOS(ArLog::Terse, 
			  "ArSerialConnection::open: Could not get port data to set up port %s", getPort());
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }
  
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = 0;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fNull = FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX =  FALSE;

  // set these explicitly and here now, instead of before when 
  // we used to set these after we did the first SetCommState...
  // This is this way since a Japanese windows xp user had some problems with 
  // changing baud rates after this first set
  if (myBaudRate > 0)
    dcb.BaudRate = myBaudRate;
  if (myHardwareControl == 0)
  {
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
  }
  else
  {
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
  }

  if ( !SetCommState(myPort, &dcb) )
  {  
    ArLog::logErrorFromOS(ArLog::Terse, 
	       "ArSerialConnection::open: Could not set up port");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }

  myStatus = STATUS_OPEN;

  /* these are now set above, see the comments there for why
  if (!setBaud(myBaudRate)) 
  {
    ArLog::log(ArLog::Terse, 
	       "ArSerialConnection::open: Could not set baud rate.");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_BAUD;
  }
  
  if (!setHardwareControl(myHardwareControl)) 
  {
    ArLog::log(ArLog::Terse, 
	       "ArSerialConnection::open: Could not set hardware control.");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }
*/
  ArLog::log(ArLog::Verbose,
	     "ArSerialConnection::open: Successfully opened and configured serial port %s.", getPort());
  return 0;
}



AREXPORT bool ArSerialConnection::close(void)
{
  bool ret;

  if (myPort == INVALID_HANDLE_VALUE)
    return true;

  /* disable event notification  */
  SetCommMask( myPort, 0 ) ;
  /* drop DTR	*/
  EscapeCommFunction( myPort, CLRDTR ) ;
  /* purge any outstanding reads/writes and close device handle  */
  PurgeComm( myPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

  myStatus = STATUS_CLOSED_NORMALLY;

  ret = CloseHandle( myPort ) ;
  if (ret)
    ArLog::log(ArLog::Verbose,
	       "ArSerialConnection::close: Successfully closed serial port %s.", getPort());
  else
    ArLog::logErrorFromOS(ArLog::Verbose, 
	       "ArSerialConnection::close: Unsuccessfully closed serial port %s.", getPort());
  myPort = (HANDLE) INVALID_HANDLE_VALUE;
  return ret;
}

AREXPORT int ArSerialConnection::getBaud(void)
{
   return myBaudRate;
}

AREXPORT bool ArSerialConnection::setBaud(int baud)
{
  DCB dcb;
  
  myBaudRate = baud;

  if (getStatus() != STATUS_OPEN)
    return true;

  if (baud == 0)
    return true;

  if ( !GetCommState(myPort, &dcb) )
  {
    ArLog::logErrorFromOS(ArLog::Terse, "ArSerialConnection::setBaud (%s): Could not get port data.", getPort());
    return false;
  }

  dcb.BaudRate = myBaudRate;

  if ( !SetCommState(myPort, &dcb) )
  {  
    ArLog::logErrorFromOS(ArLog::Terse,
	       "ArSerialConnection::setBaud (%s): Could not set port data (trying baud %d).", getPort(), myBaudRate);
    return false;
  }  

  return true;
}

AREXPORT bool ArSerialConnection::getHardwareControl(void)
{
  return myHardwareControl;
}

AREXPORT bool ArSerialConnection::setHardwareControl(bool hardwareControl)
{
  DCB dcb;

  myHardwareControl = hardwareControl;

  if (getStatus() != STATUS_OPEN)
    return true;
 
  if ( !GetCommState(myPort, &dcb) )
  {
    ArLog::logErrorFromOS(ArLog::Terse,
	       "ArSerialConnection::setBaud (%s): Could not get port Data.", getPort());
    return false;
  }
  
  if (myHardwareControl == 0) /* set control lines */
  {
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
  }
  else
  {
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
  }

  if ( !SetCommState(myPort, &dcb) )
  {  
    ArLog::logErrorFromOS(ArLog::Terse, "ArSerialConnection::setBaud (%s): Could not set port Data.", getPort());
    return false;
  }  
  
  return true;
}

AREXPORT int ArSerialConnection::write(const char *data, unsigned int size) 
{
  unsigned long ret;

  if (myPort != INVALID_HANDLE_VALUE && myStatus == STATUS_OPEN) 
  {
    if (!WriteFile(myPort, data, size, &ret, NULL)) 
    {
      ArLog::logErrorFromOS(ArLog::Terse, "ArSerialConnection::write (%s): Error on writing.", getPort());
      return -1;
    }
    return ret;
  }
  ArLog::log(ArLog::Terse, "ArSerialConnection::write (%s): Connection invalid.", getPort());
  return -1;
}

AREXPORT int ArSerialConnection::read(const char *data, unsigned int size, 
				      unsigned int msWait) 
{
  COMSTAT stat;
  unsigned long ret;
  unsigned int numToRead;
  ArTime timeDone;

  if (myPort != INVALID_HANDLE_VALUE && myStatus == STATUS_OPEN)
  {
    if (msWait > 0)
    {
      timeDone.setToNow();
      if (!timeDone.addMSec(msWait)) {
        ArLog::log(ArLog::Normal,
                   "ArSerialConnection::read() (%s) error adding msecs (%i)", getPort(),
                   msWait);
      }
      while (timeDone.mSecTo() >= 0) 
      {
	if (!ClearCommError(myPort, &ret, &stat))
	  return -1;
	if (stat.cbInQue < size)
	  ArUtil::sleep(2);
	else
	  break;
      }
    }
    if (!ClearCommError(myPort, &ret, &stat))
      return -1;
    if (stat.cbInQue == 0)
      return 0;
    if (stat.cbInQue > size)
      numToRead = size;
    else
      numToRead = stat.cbInQue;
    if (ReadFile( myPort, (void *)data, numToRead, &ret, NULL))
    {
      return (int)ret;
    }
    else 
    {
      ArLog::logErrorFromOS(ArLog::Terse, "ArSerialConnection::read (%s):  Read failed.", getPort());
      return -1;
    }
  }
  ArLog::log(ArLog::Terse, "ArSerialConnection::read (%s): Connection invalid.", getPort());
  return -1;
}


AREXPORT int ArSerialConnection::getStatus(void)
{
  return myStatus;
}

AREXPORT bool ArSerialConnection::isTimeStamping(void)
{
  return false;
}

AREXPORT ArTime ArSerialConnection::getTimeRead(int index)
{
  ArTime now;
  now.setToNow();
  return now;
}

AREXPORT bool ArSerialConnection::getCTS(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_CTS_ON);
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArSerialConnection::getCTS() (%s): problem with GetCommModemStatus", getPort());
    return false;
  }
} 

AREXPORT bool ArSerialConnection::getDSR(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_DSR_ON);
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArSerialConnection::getDSR() (%s): problem with GetCommModemStatus", getPort());
    return false;
  }
} 

AREXPORT bool ArSerialConnection::getDCD(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_RLSD_ON);
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArSerialConnection::getDCD() (%s): problem with GetCommModemStatus", getPort());
    return false;
  }
}

AREXPORT bool ArSerialConnection::getRing(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_RING_ON);
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArSerialConnection::getRing() (%s): problem with GetCommModemStatus", getPort());
    return false;
  }
}

