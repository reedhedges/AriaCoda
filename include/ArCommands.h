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
#ifndef ARCOMMANDS_H
#define ARCOMMANDS_H

/// A class containing names for most robot microcontroller system commands
/**
   A class with an enum of the commands that can be sent to the robot, see the 
   robot operations manual for more detailed descriptions.
 @ingroup OptionalClasses
*/
class ArCommands
{
public:
  enum Commands { 
  PULSE = 0, ///< none, keep alive command, so watchdog doesn't trigger
  OPEN = 1, ///< none, sent after connection to initiate connection
  CLOSE = 2, ///< none, sent to close the connection to the robot
  POLLING = 3, ///< string, string that sets sonar polling sequence
  ENABLE = 4, ///< int, enable (1) or disable (0) motors
  SETA = 5, ///< int, sets translational accel (+) or decel (-) (mm/sec/sec)
  SETV = 6, ///< int, sets maximum velocity (mm/sec)
  SETO = 7, ///< int, resets robots origin back to 0, 0, 0
  MOVE = 8, ///< int, translational move (mm)
  ROTATE = 9, ///< int, set rotational velocity, duplicate of RVEL (deg/sec)
  SETRV = 10, ///< int, sets the maximum rotational velocity (deg/sec)
  VEL = 11, ///< int, set the translational velocity (mm/sec)
  HEAD = 12, ///< int, turn to absolute heading 0-359 (degrees)
  DHEAD = 13, ///< int, turn relative to current heading (degrees)
  //DROTATE = 14, does not really exist
  SAY = 15, /**< string, makes the robot beep.
	     up to 20 pairs of duration (20 ms incrs) and tones (halfcycle) */
  JOYINFO = 17, // int, requests joystick packet, 0 to stop, 1 for 1, 2 for continuous
  CONFIG = 18, ///< int, request configuration packet
  ENCODER = 19, ///< int, > 0 to request continous stream of packets, 0 to stop
  SETRVDIR = 20, ///< int, Sets the rot vel max in each direction
  RVEL = 21, ///< int, set rotational velocity (deg/sec)
  DCHEAD = 22, ///< int, colbert relative heading setpoint (degrees)
  SETRA = 23, ///< int, sets rotational accel(+) or decel(-) (deg/sec)
  IMUREQUEST = 26,///< int, requests IMU packets (1 for 1, 2 for continuous, 0 to stop) 
  SEEKURIMU = 26, 
  SONAR = 28, ///< int, enable (1) or disable (0) sonar 
  STOP = 29, ///< int, stops the robot
  DIGOUT = 30, ///< int, sets the digout lines
  //TIMER = 31, ... no clue about this one
  VEL2 = 32, /**< int, independent wheel velocities,
		first 8 bits = right, second 8 bits = left, multiplied by Vel2 divisor. See manual.  */
  GRIPPER = 33, ///< int, gripper server command, see gripper manual for detail
  //KICK = 34, um...
  ADSEL = 35, ///< int, select the port given as argument
  GRIPPERVAL = 36, ///< p2 gripper server value, see gripper manual for details
  GRIPPERPACREQUEST = 37, ///< p2 gripper packet request
  IOREQUEST = 40, ///< request iopackets from p2os
  PTUPOS = 41, ///< most-sig byte is port number, least-sig byte is pulse width
  TTY2 = 42, ///< string, send string argument to serial dev connected to aux1 
  GETAUX = 43, ///< int, requests 1-200 bytes from aux1 serial channel, 0 flush
  BUMPSTALL = 44, /**< int, stop and register a stall if front (1), rear (2),
		   or both (3) bump rings are triggered, Off (default) is 0 */
  TCM2 = 45, ///< TCM2 module commands, see tcm2 manual for details 
  JOYDRIVE = 47, /**< Command to tell p2os to drive with the joystick 
		    plugged into the robot */
  MOVINGBLINK = 49, ///< int, 1 to blink lamp quickly before moving, 0 not to (for patrolbot)
  HOSTBAUD = 50, ///< int, set baud rate for host port - 0=9600, 1=19200, 2=38400, 3=57600, 4=115200
  AUX1BAUD = 51, ///< int, set baud rate for Aux1 - 0=9600, 1=19200, 2=38400, 3=57600, 4=115200
  AUX2BAUD = 52, ///< int, set baud rate for Aux2 - 0=9600, 1=19200, 2=38400, 3=57600, 4=115200
  ESTOP = 55, ///< none, emergency stop, overrides decel
  ESTALL = 56, // ?
  GYRO = 58, ///< int, set to 1 to enable gyro packets, 0 to disable
  TTY4 = 60,
  GETAUX3 = 61,
  BATTERYINFO = 62, ///< int, requests battery info packets, 0 to stop, 1 for 1, 2 for continuous
  TTY3 = 66,
  GETAUX2 = 67,
  BATTEST = 250,

  // SRISIM specific:
  LOADPARAM = 61, ///< @deprecated  only supported by SRISim. Conflicts with GETAUX3
  OLDSIM_LOADPARAM = 61, ///< @deprecated only supported by SRISim. Conflicts with GETAUX3
  ENDSIM = 62, ///< @deprecated use SIM_EXIT 
  OLDSIM_EXIT = 62, ///< @deprecated use SIM_EXIT
  LOADWORLD = 63, ///< @deprecated only supported by SRISim
  OLDSIM_LOADWORLD = 63, ///< @deprecated only supported by SRISim
  STEP = 64, ///< @deprecated only supported by SRISim
  OLDSIM_STEP = 64, ///< @deprecated only supported by SRISim

  // for calibrating the compass:
  CALCOMP = 65, ///< int, commands for calibrating compass, see compass manual

  // SRISIM specific:
  // SETORIGINX and SETORIGINY overlap with TTY3 and GETAUX2 so they are disabled:
  //SETSIMORIGINX = 66, 
  //SETSIMORIGINY = 67, 
  //OLDSIM_SETORIGINX = 66,
  //OLDSIM_SETORIGINY = 67,
  SETSIMORIGINTH = 68, ///< @deprecated use SIM_SET_POSE
  OLDSIM_SETORIGINTH = 68, ///< @deprecated use SIM_SET_POSE
  RESETSIMTOORIGIN = 69, ///< @deprecated use SIM_RESET
  OLDSIM_RESETTOORIGIN = 69, ///< @deprecated use SIM_RESET

  // AmigoBot-H8 specific:
  SOUND = 90, ///< int, AmigoBot (old H8 model) specific, plays sound with given number
  PLAYLIST = 91, /**< int, AmigoBot (old H8 model) specific, requests name of sound, 
		    0 for all, otherwise for specific sound */
  SOUNDTOG = 92, ///< int, AmigoBot (old H8 model) specific, enable(1) or diable(0) sound

  // Power commands
  POWER_PC = 95, ///< int, Powers on or off the PC (if the firwmare is set up to do this in its power settings)
  POWER_LRF = 96, ///< int, Powers on or off the laser (if the firwmare is set up to do this in its power settings)
  POWER_5V = 97, ///< int, Powers on or off the 5v accessories (if the firwmare is set up to do this in its power settings)
  POWER_12V = 98, ///< int, Powers on or off the 12v accessories (if the firwmare is set up to do this in its power settings)
  POWER_24V = 98, ///< int, Powers on or off the 24v accessories (if the firwmare is set up to do this in its power settings)

  // For SEEKUR or later lateral-capable robots
  LATVEL = 110, ///< int, sets the lateral velocity (mm)
  LATACCEL = 113, ///< int, sets the lateral acceleration (+, mm/sec2) or lateral deceleration (-, mm/sec2)
  SETLATV = 0, /// set max. lat. vel. (not available yet)
  RECENTERWHEELS = 120, // recenter seekur steering wheels

  ACCELINFO = 123, ///< int, requests accelerometer packet, 0 to stop, 1 for 1, 2 for continuous
  TORQUEINFO = 124, ///< int, requests torque packet, 0 to stop, 1 for 1, 2 for continuous

  POWER_AUX_PC = 125, ///< int, Powers on or off the auxilliary PC (if the firwmare is set up to do this in its power settings)
  POWER_TOUCHSCREEN = 126, ///< int, Powers on or off the touchscreen (if the firwmare is set up to do this in its power settings)
  POWER_PTZ = 127, ///< int, Powers on or off the PTZ (if the firwmare is set up to do this in its power settings)
  POWER_AUDIO = 128, ///< int, Powers on or off the audio (if the firwmare is set up to do this in its power settings)
  POWER_LRF2 = 129, ///< int, Powers on or off the second laser (if the firwmare is set up to do this in its power settings)

  HEADINGINFO = 149, ///< int, requests heading packet, 0 to stop, 1 for 1, 2 for continuous
  ROTVEL_CLOSEDLOOP_ENABLE = 150, ///< int, 0 = Disabled, 1 = Enabled

  // MTX commands
  SRECORD = 210, /// < int, (for downloading MTX firmware) byte with 0 for wait, 1 for OK, -1 for ERROR (waiting might take 3-5 seconds
  MARCDEBUG = 211, ///<  for debug messages from MARC, possibly responses, possibly pushed... they are ascii strings that should be logged
  WHEEL_LIGHT = 212, ///<  For the wheel lights
  ABSOLUTE_MAXES = 213, ///<  To set the absolute maxes

  SAFETY_STATE_INFO = 214, ///< int, request safety state info packets (0 == stop, 1 == send once, 2 == send continuous),
  SAFETY_SET_STATE = 215, ///< 2 bytes, first byte which system, second byte for value
  SAFETY_DISABLE_POWER_OFF_TIMER = 216, ///< int, 0 set off, 1 set on
  SAFETY_ZONE_ACTIVE = 217, ///< int, tell MarcOS which safety zone the lidar is reporting active
  WHEELINFO_REQUEST = 218, ///< int, enable detailed motion debugging over CAN

  MARC_CONFIG_SET = 219, ///< Used to set Marc configuration
  MARC_CONFIG_GET = 220, ///< Request Marc config info

  // MobileSim specific:
  SIM_SET_POSE = 224,       ///< int4,int4,int4 Move robot to global pose in simulator (does not change odometry). Each value is a 4-byte integer.
  SIM_RESET= 225,         ///< none, Reset robot's state to original in simulator and reset odometry to 0,0,0.
  SIM_MODEL_CONFIG = 226, ///< variable, allows a simulated robot to send configuration info about it's physical parameters to MobileSim, so it can dynamically create an internal model
  SIM_LRF_ENABLE = 230,   ///< int, 1 to begin sending packets of data from a simulated laser rangefinder (on the same socket connection), 2 to send extended-information laser packets (with reading flags), 0 to disable LRF
  SIM_LRF_SET_FOV_START = 231,  ///< int Set angle (degrees from center) at which the simulater laser takes its first reading (normally -90).
  SIM_LRF_SET_FOV_END = 232,  ///< int Set angle (degrees from center) at which the simulated laser takes its last reading (normally 90).
  SIM_LRF_SET_RES = 233,  ///< int Set the number of degrees between laser readings (in combination with FOV, determines the number of readings per sweep) (normally 1)
  SIM_CTRL = 236,         ///< int,..., Send a simulator meta-command (an operation on the simulator itself). The initial 2-byte integer argument selects the operation. See simulator documentation.
  SIM_STAT = 237,         ///< none, Request that the simulator reply with a SIMSTAT (0x62) packet. You must have a packet handler registered with ArRobot to receive its output. See simulator documentation.
  SIM_MESSAGE = 238,      ///< string, Display a log message in the simulator. Argument is a length-prefixed ASCII byte string.
  SIM_EXIT = 239,          ///< int, Exit the simulator. Argument is the exit code (use 0 for a "normal" exit).

  POWERONLIKE_RESET = 253
  };
  
};

#endif // ARCOMMANDS_H


