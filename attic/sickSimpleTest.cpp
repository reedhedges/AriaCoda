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

int main(int argc, char **argv)
{
  int ret;
  std::string str;
  ArSerialConnection con;
  ArSickPacket sick;
  ArSickPacket *packet;
  ArSickPacketReceiver receiver(&con);
  ArTime start;
  unsigned int value;
  int numReadings;
  ArTime lastReading;
  ArTime packetTime;

  start.setToNow();

  // open the connection, if it fails, exit
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }

  start.setToNow();

  printf("Waiting for laser to power on\n");
  sick.empty();
  sick.uByteToBuf(0x10);
  sick.finalizePacket();
  con.write(sick.getBuf(), sick.getLength());

  while (start.secSince() < 70 && 
	 ((packet = receiver.receivePacket(100)) == NULL
	  ||  (packet->getID() != 0x90)));
  if (packet != NULL)
    printf("Laser powered on\n");
  else
    exit(1);

  printf("Changing baud\n");
  sick.empty();
  sick.byteToBuf(0x20);
  sick.byteToBuf(0x40);
  sick.finalizePacket();
  con.write(sick.getBuf(), sick.getLength());

  ArUtil::sleep(10);
  if (!con.setBaud(38400))
  {
    printf("Could not set baud, exiting\n");
  }
  
  
  /*packet = receiver.receivePacket(100);
  if (packet != NULL) 
    packet->log();
  */
  sick.empty();
  sick.uByteToBuf(0x3B);
  sick.uByte2ToBuf(180);
  sick.uByte2ToBuf(100);
  sick.finalizePacket();
  con.write(sick.getBuf(), sick.getLength());

  packet = receiver.receivePacket(100);
  if (packet != NULL) 
    packet->log();

  sick.empty();
  sick.byteToBuf(0x20);
  sick.byteToBuf(0x24);
  sick.finalizePacket();
  con.write(sick.getBuf(), sick.getLength());

  packet = receiver.receivePacket(100);
  if (packet != NULL) 
    packet->log();



  printf("Starting to report back from port, it took %ld ms to get here:\n",
	 start.mSecSince());
  start.setToNow();
  while (start.secSince() < 6)
  {
    packetTime.setToNow();
    packet = receiver.receivePacket();
    if (packet != NULL)
      printf("####### %ld ms was how long the packet took\n", packetTime.mSecSince());
    if (packet != NULL)
    {
      if (packet->getLength() < 10)
	packet->log();
      else if (packet->getID() == 0x90)
      {
	char strBuf[512];
	packet->log();
	//printf("%x\n", packet->bufToUByte());
	packet->bufToStr(strBuf, 512);
	printf("0x%x %s\n", packet->getID(), strBuf);
	sick.empty();
	sick.uByteToBuf(0x3B);
	sick.uByte2ToBuf(180);
	sick.uByte2ToBuf(100);
	sick.finalizePacket();
	con.write(sick.getBuf(), sick.getLength());
	packet = receiver.receivePacket(100);
	sick.empty();
	sick.uByteToBuf(0x20);
	sick.uByteToBuf(0x24);
	sick.finalizePacket();
	con.write(sick.getBuf(), sick.getLength());
      }
      else
      {
	value = packet->bufToUByte2();
	numReadings = value & 0x3ff;
	printf("%ld ms after last reading.\n", lastReading.mSecSince());
	/*
	printf("Reading number %d, complete %d, unit: %d %d:\n", value & 0x3ff, !(bool)(value & ArUtil::BIT13), (bool)(value & ArUtil::BIT14), (bool)(value & ArUtil::BIT15));
	for (i = 0; i < numReadings; i++)
	{
	  value = packet->bufToUByte2();
	  if (value & ArUtil::BIT13)
	    printf("D");
	  printf("%d ", value & 0x1fff);
	}
	printf("\n");
	*/
	lastReading.setToNow();
      }
    }
    else
    {
      //printf("No packet\n");
    }
  }
}

