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
#include "ariaTypedefs.h"
#include "ArFileDeviceConnection.h"
#include "ArLog.h"
#include "ariaUtil.h"
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

AREXPORT ArFileDeviceConnection::ArFileDeviceConnection() :
  myInFD(-1), myOutFD(-1), myStatus(STATUS_NEVER_OPENED),
  myForceReadBufferSize(0), myReadByteDelay(0.0)
{
}

AREXPORT ArFileDeviceConnection::~ArFileDeviceConnection()
{
  close();
}


/** 
 * @param infilename Input file name, or NULL to use stdin
 * @param outfilename Output file name, or NULL to use stdout
 * @param outflags Additional flags to pass to system open() function when opening
 * the output file in addition to O_WRONLY. For example, pass O_APPEND to append rather than rewrite the
 * file.
 */
AREXPORT int ArFileDeviceConnection::open(const char *infilename, const char *outfilename, int outflags)
{
  myStatus = STATUS_OPEN_FAILED;

  if(infilename)
  {
    myInFileName = infilename;
    myInFD = ArUtil::open(infilename, O_RDONLY);
    if(myInFD == -1)
    {
      ArLog::log(ArLog::Terse, "ArFileDeviceConnection: error opening input file \"%s\": %s", infilename, strerror(errno));
      return errno;
    }
  }
  else
  {
#ifdef WIN32
    myInFD = _fileno(stdin);
#else
    myInFD = STDIN_FILENO;
#endif
  }

  if(outfilename)
  {
    myOutFileName = outfilename;
    myOutFD = ArUtil::open(outfilename, O_WRONLY|outflags);
    if(myInFD == -1)
    {
      ArLog::log(ArLog::Terse, "ArFileDeviceConnection: error opening output file \"%s\": %s", outfilename, strerror(errno));
      return errno;
    }
  }
  else
  {
#ifdef WIN32
    myOutFD = _fileno(stdout);
#else
    myOutFD = STDOUT_FILENO;
#endif
  }

  myStatus = STATUS_OPEN;

  return 0;
}

AREXPORT bool ArFileDeviceConnection::close(void)
{
  ArUtil::close(myInFD);
  ArUtil::close(myOutFD);
  myStatus = STATUS_CLOSED_NORMALLY;
  return true;
}

AREXPORT int ArFileDeviceConnection::read(const char *data, unsigned int size, unsigned int msWait)
{
  unsigned int s = myForceReadBufferSize > 0 ? ArUtil::findMinU(size, myForceReadBufferSize) : size;
#ifdef WIN32
  int r = _read(myInFD, (void*)data, s);
#else
  int r =  ::read(myInFD, (void*)data, s);
#endif
  if(myReadByteDelay > 0)
    ArUtil::sleep(r * myReadByteDelay);
  // TODO add option for intermittent data by returning 0 until a timeout has passed.
  // TODO add option to delay full lines (up to \n\r or \n) in above behavior
  return r;
}

AREXPORT int ArFileDeviceConnection::write(const char *data, unsigned int size)
{
#ifdef WIN32
  return (int) _write(myOutFD, (void*)data, (size_t)size);
#else
  return (int) ::write(myOutFD, (void*)data, (size_t)size);
#endif
}


AREXPORT bool ArFileDeviceConnection::isTimeStamping(void)
{
  return false;
}

AREXPORT ArTime ArFileDeviceConnection::getTimeRead(int index)
{
  ArTime now;
  now.setToNow();
  return now;
}

AREXPORT const char * ArFileDeviceConnection::getOpenMessage(int err)
{
  return strerror(err);
}

