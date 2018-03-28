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
#include "ArSystemStatus.h"

void printNewStatus()
{
    printf("CPU Usage: %0.1f%%  Uptime: %f hours  Wireless Link: %d  (Signal:%d Noise:%d)  Discarded Packets: %d  (NetConflict:%d)\n",
        ArSystemStatus::getCPUPercent(), ArSystemStatus::getUptimeHours(),
        ArSystemStatus::getWirelessLinkQuality(), ArSystemStatus::getWirelessLinkSignal(), ArSystemStatus::getWirelessLinkNoise(),
        ArSystemStatus::getWirelessDiscardedPackets(), ArSystemStatus::getWirelessDiscardedPacketsBecauseNetConflict()
    );
}

int main(int argc, char** argv)
{
  Aria::init();
  printf("Running with sleep time of 1sec...\n");

  for(int i = 0; i < 10; i++)
  {
    printNewStatus();
    ArUtil::sleep(1000);
  }

  printf("\nRunning with sleep time of 5ms!...\n");
  for(int i = 0; i < 50; i++)
  {
    printNewStatus();
    ArUtil::sleep(5);
  }

  printf("\nRunning with sleep time of 10 sec...\n");
  for(int i = 0; i < 4; i++)
  {
    printNewStatus();
    ArUtil::sleep(10000);
  }
}
