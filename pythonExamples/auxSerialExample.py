"""
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
"""

# @example auxSerialExample.py
# Demonstrates the use of a robot packet handler to recieve data from a device 
# attached to the robot microcontroller's auxilliary serial port.
#
# This example shows how to use the GETAUX command and how to recieve SERAUX
# serial port data.  
# To use this example, you must have the auxilliary serial port AUX1 on the
# microcontroller (inside the robot) connected to a device that is sending text data.

# You can connect AUX1 to a computer through an RS-232 cable with a NULL modem adapter, 
# and run a program such as minicom (Linux) or HyperTerminal (Windows). Then just
# type to send data to the AUX1 port.  Configure minicom or HyperTerminal to have 
# the following serial port settings:
#   9600 baud
#   8 data bits, 1 stop bit, no parity
#   No flow control (not hardware, not software)!
# 
# This program defines a packet handler function (handleAuxSerialPacket)
# and adds it to the ArRobot object. All packets recieved from the robot
# are passed to all packet handlers. handleAuxSerialPacket() checks whether the packet
# is an SERAUX packet, and if so, prints out the data contained within the packet.
# The packet handler then sends a GETAUX command to the robot to request more data.

from AriaPy import *
import sys

#   Packet handler that simply prints out each byte of a packet as a
#   character, then sends another request for more data (using GETAUX command).
#   When it receives a newline or carriage return, it also replies 
#   using the TTY2 command with a string ("Hello, world!\n\r").
def handleAuxSerialPacket(packet):

  # If this is not an aux. serial data packet, then return false to allow other
  # packet handlers to recieve the packet. The packet type ID numbers are found
  # in the description of the GETAUX commands in the robot manual.
  if (packet.getID() != 0xb0): # 0xB0 is SERAUXpac. SERAUX2pac is 0xB8, SERAUX3pac is 0xC8.
    return False

  # Displays the contents of the packet
  packet.log()

  # You can also extract each byte or sets of bytes 
  # from the packet's "data buffer" in sequence.
  # As data is removed, getReadLength() is reduced accordingly.
  # getLength() returns the original packet length including   
  # ID byte and length byte.
  n = 0
  while (packet.getReadLength() < packet.getLength() - 2):
    # Values of different sizes can be removed from the packet:
    c = packet.bufToUByte()
    # i = packet.bufToUByte2()
    # i = packet.bufToByte2()
    # i = packet.bufToUByte4()
    # i = packet.bufToByte4()
    # i = packet.bufToUByte8()
    # i = packet.bufToByte8()
    # To get a NULL-terminated string:
    # s = packet.bufToString()
    ++n
  

  ArLog.log(ArLog.Normal, "auxSerialExample: handled SERAUX packet with %d bytes of data"%(n) )

  # How to send data to the serial port. See robot manual
  # (but note that TTY2 is for the AUX1 port, TTY3 for AUX2, etc.)
  #robot.comStr(ArCommands.TTY2, "Hello, World!\n\r")

  global robot
  
  # Request more data:
  robot.comInt(ArCommands.GETAUX, 1)

  # To request 12 bytes at a time, specify that instead:
  #robot.comInt(ArCommands.GETAUX, 12)

  # If you wanted to recieve information from the second aux. serial port, use
  # the GETAUX2 command instead; but the packet returned will also have a
  # different type ID.
  #robot.comInt(ArCommands.GETAUX2, 1)
  

  # Return true to indicate to ArRobot that we have handled this packet.
  return True
  

Aria.init()
parser = ArArgumentParser(sys.argv)
parser.loadDefaultArguments()

robot = ArRobot()

# Connect to the robot, get some initial data from it such as type and name,
# and then load parameter files for this robot.
robotConnector = ArRobotConnector(parser, robot)
if (! robotConnector.connectRobot()):
  ArLog.log(ArLog.Terse, "auxSerialExample: Could not connect to the robot.")
  if(parser.checkHelpAndWarnUnparsed()):
      # -help not given
      Aria.logOptions()
      Aria.exit(1)

if (!Aria.parseArgs() || !parser.checkHelpAndWarnUnparsed()):
  Aria.logOptions()
  Aria.exit(1)


# add our packet handler as the first one in the list
robot.addPacketHandler(handleAuxSerialPacket, ArListPos.FIRST)

# Set BAUD rate of serial port
ArLog.log(ArLog.Normal, "auxSerialExample: Connected to the robot. Sending command to change AUX1 baud rate to 9600...")
robot.comInt(ArCommands.AUX1BAUD, 0); # 0=9600. See robot manual for other rates

# Send the first GETAUX request
robot.comInt(ArCommands.GETAUX, 1)

# If you wanted to recieve information from the second aux. serial port, use
# the GETAUX2 command instead; but the packet returned will also have a
# different type ID.
#robot.comInt(ArCommands.GETAUX2, 1)

# run the robot until disconnect, then shutdown and exit.
robot.run(True)
Aria.exit(0)

