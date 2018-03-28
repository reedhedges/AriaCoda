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


/*
  Utility that forwards data back and forth from a tcp port to a serial
  port.  This can be used to run programs on your workstation instead
  of the robot's onboard computer, or to bridge networks.
  However, it can negatively impact performance, and may cause some things
  to simply not work; we do not recommend or support using this program
  other than for quick testing or development.
*/

void usage(char *progName)
{
  printf("There are four ways to use %s\n", progName);
  printf("%s\t\t\tUses a robot, port 8101 and %s\n", progName, ArUtil::COM1);
  printf("%s laser\t\tUses a laser, port 8102 and %s\n", progName, ArUtil::COM3);
  printf("%s <portNumberToForwardFrom> <portNameToForwardTo>\n", 	 progName);
  printf("%s <portNumberToForwardFrom> <portNameToForwardTo> laser\n", 	 progName);
  printf("-----------------------------------------------------------\n");
  printf("portNumToForwardFrom: The tcp port to listen on.\n");
  printf("portNameToForwardTo: The serial port name to open.\n");
  printf("laser: option, if given it'll use laser packet receivers and not robot packet receivers.\n");
}

int main(int argc, char **argv)
{
  // this is how long to wait after there's been no data to close the
  // connection.. if its 0 and its using robot it'll set it to 5000 (5
  // seconds), if its 0 and using laser, it'll set it to 60000 (60
  // seconds, which is needed if the sick driver is controlling power
  int timeout = 0;
  // true will print out packets as they come and go, false won't
  bool tracePackets = false;

  // The socket objects
  ArSocket masterSock, clientSock;
  // The connections
  ArTcpConnection clientConn;
  ArSerialConnection robotConn;
  // the receivers, first for the robot
  ArRobotPacketReceiver clientRec(&clientConn);
  ArRobotPacketReceiver robotRec(&robotConn);
  // then for the laser
  ArSickPacketReceiver clientSickRec(&clientConn, 0, false, true);
  ArSickPacketReceiver robotSickRec(&robotConn);
  // how about a packet
  ArBasePacket *packet;
  // our timer for how often we test the client
  ArTime lastClientTest;
  ArTime lastData;
  // where we're forwarding from and to
  int portNumber;
  const char *portName;
  // if we're using the robot or the laser
  bool useRobot;
  
  if (argc == 1)
  {
    printf("Using robot and port 8101 and serial connection %s, by default.\n", ArUtil::COM1);
    useRobot = true;
    portNumber = 8101;
    portName = ArUtil::COM1;
  }
  else if (argc == 2)
  {
    // if laser isn't the last arg, somethings wrong
    if (strcmp(argv[1], "laser") != 0)
    {
      usage(argv[0]);
      return -1;
    }
    useRobot = false;
    portNumber = 8102;
    portName = ArUtil::COM3;
    printf("Using laser and port %d and serial connection %s, by command line arguments.\n", portNumber, portName);
    printf("(Note: Requests to change BAUD rate cannot be fulfilled; use 9600 rate only.)\n");
  }
  else if (argc == 3)
  {
    if ((portNumber = atoi(argv[1])) <= 0)
    {
      usage(argv[0]);
      return -1;
    }
    portName = argv[2];
    printf("Using robot and port %d and serial connection %s, by command line arguments.\n", portNumber, portName);
  }
  else if (argc == 4)
  {
    if ((portNumber = atoi(argv[1])) <= 0)
    {
      usage(argv[0]);
      return -1;
    }
    // if laser isn't the last arg, somethings wrong
    if (strcmp(argv[3], "laser") != 0)
    {
      usage(argv[0]);
      return -1;
    }
    useRobot = false;
    portName = argv[2];
    printf("Using laser and port %d and serial connection %s, by command line arguments.\n", portNumber, portName);
    printf("(Note: Requests to change BAUD rate cannot be fulfilled; use 9600 rate only.)\n");
  }
  else
  {
    usage(argv[0]);
    return -1;
  }
  if (timeout == 0 && useRobot)
    timeout = 5000;
  else if (timeout == 0)
    timeout = 60000;

  // Initialize Aria. For Windows, this absolutely must be done. Because
  // Windows does not initialize the socket layer for each program. Each
  // program must initialize the sockets itself.
  Aria::init(Aria::SIGHANDLE_NONE);

  // Lets open the master socket
  if (masterSock.open(portNumber, ArSocket::TCP))
    printf("Opened the master port at %d\n", portNumber);
  else
  {
    printf("Failed to open the master port at %d: %s\n",
	   portNumber, masterSock.getErrorStr().c_str());
    return -1;
  }

  // just go forever
  while (1)
  {
    // Lets wait for the client to connect to us.
    if (masterSock.accept(&clientSock))
      printf("Client has connected\n");
    else
      printf("Error in accepting a connection from the client: %s\n",
	     masterSock.getErrorStr().c_str());
   
    // now set up our connection so our packet receivers work
    clientConn.setSocket(&clientSock);
    clientConn.setStatus(ArDeviceConnection::STATUS_OPEN);
    lastClientTest.setToNow();
    lastData.setToNow();
    // open up the robot port
    if (robotConn.open(portName) != 0)
    {
      printf("Could not open robot port %s.\n", portName);
      return -1;
    }

    // while we're open, just read from one port and write to the other
    while (clientSock.getFD() >= 0)
    {
      // get our packet
      if (useRobot)
	packet = clientRec.receivePacket(1);
      else
	packet = clientSickRec.receivePacket(1);
      // see if we had one
      if (packet != NULL)
      {
	if (tracePackets)
	{
	  printf("Client ");
	  packet->log();
	}
	robotConn.writePacket(packet);
	lastData.setToNow();
      }
      // get our packet
      if (useRobot)
	packet = robotRec.receivePacket(1);
      else
	packet = robotSickRec.receivePacket(1);
      // see if we had one
      if (packet != NULL)
      {
	if (tracePackets)
	{
	  printf("Robot ");
	  packet->log();
	}
	clientConn.writePacket(packet);
	lastData.setToNow();
      }
      ArUtil::sleep(1);
      // If no datas gone by in timeout ms assume our connection is broken
      if (lastData.mSecSince() > timeout)
      {
	printf("No data received in %d milliseconds, closing connection.\n", 
	       timeout);
	clientConn.close();
      }
    }
    // Now lets close the connection to the client
    clientConn.close();
    printf("Socket to client closed\n");
    robotConn.close();
  }
  // And lets close the master port
  masterSock.close();
  printf("Master socket closed and program exiting\n");

  // Uninitialize Aria
  Aria::uninit();

  // All done
  return(0);
}
