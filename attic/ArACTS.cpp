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
#include "ArACTS.h"


AREXPORT ArACTS_1_2::ArACTS_1_2() :
  mySensorTaskCB(this, &ArACTS_1_2::actsHandler)
{
  myRobot = NULL;
  myBlobsBad = true;
  myInverted = false;
}

AREXPORT ArACTS_1_2::~ArACTS_1_2()
{

}

/** 
    Opens the port to the ACTS server
    @param robot the robot to attach this to, which puts a sensorInterp on the
    robot so that ArACTS will always have fresh data from ACTS... giving a 
    NULL value is perfectly acceptable, in this case ArACTS will not do any
    processing or requesting and you'll have to use receiveBlobInfo and 
    requestPacket (or just call actsHandler)
    @param port the port the ACTS server is running on, default of 5001
    @param host the host the ACTS server is running on, default is localhost
    (ie this machine)
    @return true if the connection was established, false otherwise
*/
AREXPORT bool ArACTS_1_2::openPort(ArRobot *robot, const char *host, int port)
{
  int ret;
  std::string str;
  if ((ret = myConn.open(host, port)) != 0)
  {
    str = myConn.getOpenMessage(ret);
    ArLog::log(ArLog::Terse, "ArACTS_1_2: Could not connect to ACTS running on %s:%d (%s)", host, port, str.c_str()); 
    return false;

  }
  myRequested = false;
  setRobot(robot);
  return true;
}

/**
   Closes the port to the ACTS server
   @return true if the connection was closed properly, false otherwise
*/
AREXPORT bool ArACTS_1_2::closePort(void)
{
  return myConn.close();
}

/**
   Requests a packet from the ACTS server, specifically it sends the 
   request to the acts server over its connection
   @return true if the command was sent succesfully, false otherwise
*/
AREXPORT bool ArACTS_1_2::requestPacket(void)
{
  const char c = '0';
  if (myConn.getStatus() != ArDeviceConnection::STATUS_OPEN)
  {
    ArLog::log(ArLog::Verbose, 
	       "ArACTS_1_2::requestPacket: No connection to ACTS.\n");
    return false;
  }
  return myConn.write(&c, 1);
}

/**
   Sends a command to the ACTS server requesting that ACTS quit
   @return true if the request was sent succesfully, false otherwise
*/
AREXPORT bool ArACTS_1_2::requestQuit(void)
{
  const char c = '1';
  if (myConn.getStatus() != ArDeviceConnection::STATUS_OPEN)
  {
    ArLog::log(ArLog::Verbose, 
	       "ArACTS_1_2::requestQuit: No connection to ACTS.\n");
    return false;
  }
  return myConn.write(&c, 1);
}

/**
   Checks the connection to the ACTS server for data, if data is there it 
   fills in the blob information, otherwise just returns false
   @return true if there was new data and the data could be read succesfully
*/
AREXPORT bool ArACTS_1_2::receiveBlobInfo(void)
{
  int i;
  char *data;
  int numBlobs = 0;

  myBlobsBad = true;
  if (myConn.getStatus() != ArDeviceConnection::STATUS_OPEN)
  {
    ArLog::log(ArLog::Verbose,
	       "ArACTS_1_2::receiveBlobInfo: No connection to ACTS.\n");
    return false;
  }

  if (!myConn.read(myData, NUM_CHANNELS*4, 20))
  {
    ArLog::log(ArLog::Verbose, 
	       "ArACTS_1_2::receiveBlobInfo: Couldn't get the blob stats.\n");
    return false;
  }
  data = myData;
  for (i = 0; i < NUM_CHANNELS; i++)
  {
    myBlobIndex[i] = (*(data++))-1;
    myBlobIndex[i] = myBlobIndex[i] << 6;
    myBlobIndex[i] |= (*(data++))-1;
    
    myBlobNum[i] = (*(data++))-1;
    myBlobNum[i] = myBlobNum[i] << 6;
    myBlobNum[i] |= (*(data++))-1;
    numBlobs += myBlobNum[i];
  }
  if (numBlobs == 0)
    return true;
  if (!myConn.read(myData, numBlobs * ACTS_BLOB_DATA_SIZE, 10))
  {
    ArLog::log(ArLog::Normal, 
	       "ArACTS_1_2::receiveBlobInfo: Couldn't read blob data.\n");
    return false;
  }
  myBlobsBad = false;
  return true;
}

/**
   @return the number of blobs on the channel, or -1 if the channel is invalid
*/
AREXPORT int ArACTS_1_2::getNumBlobs(int channel)
{

  if (channel >= 1 && channel <= NUM_CHANNELS)
  {
    --channel;
    return myBlobNum[channel];
  }
  else
    return -1;
}

int ArACTS_1_2::getData(char *rawData)
{
  int temp;
  temp = (*(rawData++)) - 1;
  temp = temp << 6;
  temp |= (*(rawData++)) - 1;
  
  return temp;
}

/**
   Gets the blobNumber from the channel given, fills the information for 
   that blob into the given blob structure.
   @param channel the channel to get the blob from
   @param blobNumber the number of the blob to get from the given channel
   @param blob the blob instance to fill in with the data about the requested
   blob
   @return true if the blob instance could be filled in from the 
*/
AREXPORT bool ArACTS_1_2::getBlob(int channel, int blobNumber, ArACTSBlob *blob)
{
  char * blobInfo;
  int i;
  int temp;

  if (myBlobsBad)
  {
    ArLog::log(ArLog::Verbose, 
	       "ArACTS_1_2::getBlob: There is no valid blob data.\n");
    return false;
  }

  if (channel <= 0 || channel > NUM_CHANNELS) 
  {
    ArLog::log(ArLog::Normal,
	       "ArACTS_1_2::getBlob: Channel %d out of range 1 to %d\n", 
	       channel, NUM_CHANNELS);
    return false;
  }
  --channel;
  
  if (blobNumber <= 0 || blobNumber > myBlobNum[channel])
  {
    ArLog::log(ArLog::Normal, 
	       "ArACTS_1_2::getBlob: Blob number %d out of range 1 to %d", 
	       blobNumber,  myBlobNum[channel]);
    return false;
  }
  --blobNumber;

  blobInfo = myData + (myBlobIndex[channel]+blobNumber) * ACTS_BLOB_DATA_SIZE;
  
  temp = 0;
  for (i = 0; i < 4; i++)
  {
    temp = temp << 6;
    temp |= (*(blobInfo++)) - 1;
  }
  blob->setArea(temp);
  
  blob->setXCG(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setYCG(invertY(getData(blobInfo)));
  blobInfo += 2;

  blob->setLeft(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setRight(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setTop(invertY(getData(blobInfo)));
  blobInfo += 2;

  blob->setBottom(invertY(getData(blobInfo)));
  blobInfo += 2;

  return true;
}

int ArACTS_1_2::invertX(int before)
{
  if (myInverted)
    return myWidth - before;
  else
    return before;
}

int ArACTS_1_2::invertY(int before)
{
  if (myInverted)
    return myHeight - before;
  else
    return before;
}

/**
   This inverts the image, but since ACTS doesn't tell this driver the
   height or width, you need to provide both of those for the image,
   default is 160x120.
   @param width the width of the images acts is grabbing (pixels)
   @param height the height of the images acts is grabbing (pixels)
**/
AREXPORT void ArACTS_1_2::invert(int width, int height)
{
  myInverted = true;
  myWidth = true;
  myHeight = true;
}

AREXPORT bool ArACTS_1_2::isConnected(void)
{
  if (myConn.getStatus() != ArDeviceConnection::STATUS_OPEN)
    return false;
  else
    return true;
}

AREXPORT ArRobot *ArACTS_1_2::getRobot(void)
{
  return myRobot;
}
AREXPORT void ArACTS_1_2::setRobot(ArRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL) 
  {
    myRobot->remSensorInterpTask(&mySensorTaskCB);
    myRobot->addSensorInterpTask("acts Handler", 50, &mySensorTaskCB);
  }
}

AREXPORT void ArACTS_1_2::actsHandler(void)
{
  if (!myRequested || receiveBlobInfo())
  {
    myRequested = true;
    requestPacket();
  }

}
