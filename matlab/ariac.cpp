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
/*
 (C)Copyright 2012 Adept Technology

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
#include "ariaInternal.h"
#include "ArRobot.h"
#include "ArRobotConnector.h"
#include "ArArgumentParser.h"
#include "ArCommands.h"
#include "ArRobotConfigPacketReader.h"

#ifndef AREXPORT
#error ARIA headers did not define AREXPORT
#endif

#include "ariac.h"

#ifdef MATLAB
#include <mex.h>

AREXPORT void mexLog(const char *s)
{
    mexPrintf("aria: %s\n", s);
}
#endif


/* Ideas for changes or additions:
  * 1. Put ArRobot and other objects into a struct returned by arrobot_connect()
  * and passed in to all other functions (or which can be "set" in a static
  * global).
  * 2. Use a sensor interp task to cache data from SIP in static variables (i.e.
  * local to instance of this library). If neccesary, have separate mutexes for
  * each variable. (I.e. optimize for very frequent independent access of each
  * value.)
  * 3. cycle counter
  * 4. Include access to ArRobot callbacks including robot cycle tasks and connection callbacks.  
  */


#define AR_ASSERT_RETURN(ptr) { \
    if(!ptr) { \
       ArLog::log(ArLog::Terse, "ariac: failed assertion: (%s) at %s:%d", #ptr, __FILE__, __LINE__); \
       return; \
    } \
}

#define AR_ASSERT_RETURN_VAL(ptr, retval) { \
    if(!ptr) { \
       ArLog::log(ArLog::Terse, "ariac: failed assertion: (%s) at %s:%d", #ptr, __FILE__, __LINE__); \
       return retval; \
    } \
}

// Switch the following to print debugging information at the start of every function call
/*
#ifdef _MSC_VER
#define AR_DEBUG_LOGINFO() { arloginfo(__FUNCTION__); }
#else
#define AR_DEBUG_LOGINFO() { arloginfo(__func__); }
#endif
*/
#define AR_DEBUG_LOGINFO() {}

bool init = false;
bool connected = false;
ArArgumentParser *argParser = NULL;
ArRobot *robot = NULL;
ArRobotConnector *robotConnector = NULL;
int argc = 0;
char **argv = NULL;
ArGlobalFunctor1<const char*> *logFunc = NULL;

AREXPORT void arloginfo(const char* m)
{
    std::string args;
    for(int i = 0; i < argc; ++i)
    {
        args += argv[i];
        args += " ";
    }   
    ArLog::log(ArLog::Terse, "%s: init=%d connected=%d argParser=0x%x robot=0x%x robotConnector=0x%x argc=%d argv=0x%x (%s)",
            m, init, connected, argParser, robot, robotConnector, argc, argv, args.c_str());
	if(robot)
		robot->getOrigRobotConfig()->log();
	else
		ArLog::log(ArLog::Terse, "no robot object.");
 
}
    
AREXPORT int aria_init()
{
  return aria_init_with_args(0, NULL);
}

AREXPORT int aria_init_with_args(int _argc, char **_argv)
{
  AR_DEBUG_LOGINFO();
  if(init) {
	ArLog::log(ArLog::Normal, "aria_init: Already initialized.");
	return 1;
  }
  Aria::init();
#ifdef MATLAB
  aria_setloghandler(&mexLog);
#endif
  argc = _argc;
  argv = _argv;
  ArLog::log(ArLog::Normal, "aria_init(%d, 0x%x)", argc, argv);
  argParser = new ArArgumentParser(&argc, argv);
  argParser->loadDefaultArguments();
  init = true;
  return 1;
}
  
AREXPORT void aria_setloghandler(void(*logfn)(const char*))
{
    logFunc = new ArGlobalFunctor1<const char*>(logfn);
    ArLog::setFunctor(logFunc);
}

AREXPORT void aria_clearloghandler()
{
	ArLog::clearFunctor();
}
    
void disconnected()
{
    ArLog::log(ArLog::Terse, "ariac: robot disconnected.");
    arrobot_disconnect();
}

ArGlobalFunctor disconnectCB(&disconnected);

AREXPORT int arrobot_connect()
{
  AR_DEBUG_LOGINFO();
  if(!init)
  {
    ArLog::log(ArLog::Terse, "arrobot_connect: Error: aria_init() must be called before arrobot_connect().");
    return 0;
  }
  if(connected) {
	  ArLog::log(ArLog::Normal, "arrobot_connect: Already connected.");
	  return 1;
  }
  argParser->log();
  ArLog::log(ArLog::Normal, "arrobot_connect: Connecting to robot...");
  robot = new ArRobot;
  robotConnector = new ArRobotConnector(argParser, robot);
  robot->addDisconnectOnErrorCB(&disconnectCB);
  if(!robotConnector->connectRobot())
  {
    ArLog::log(ArLog::Normal, "arrobot_connect: Could not connect to the robot. Check connections or set connection options as program arguments in init.");
    /*if(argParser->checkHelpAndWarnUnparsed())
    {
      Aria::logOptions();
    }
    */
    arrobot_disconnect();
    return 0;
  }
  if(!Aria::parseArgs() || !argParser->checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    arrobot_disconnect();
    return 0;
  }
  robot->enableMotors();
  robot->runAsync(true);
  ArUtil::sleep(300);
  ArLog::log(ArLog::Normal, "arrobot_connect: Connected to robot. ArRobot processing cycle is running in background thread.");
  connected = true;
  return 1;
}



AREXPORT void arrobot_disconnect()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  connected = false;
  if(!robot) { ArLog::log(ArLog::Terse, "arrobot_disconnect: internal error: NULL robot!"); return; }
  robot->clearDirectMotion();
  robot->stopRunning();
  robot->waitForRunExit();
  delete robot;
  robot = NULL;
  if(!robotConnector) { ArLog::log(ArLog::Terse, "arrobot_disconnect: internal error: NULL robotConnector!"); return; }
  delete robotConnector;
  robotConnector = NULL;
}

/** Block until robot connection is broken (e.g. by robot reset). */
AREXPORT void arrobot_wait()
{
  AR_DEBUG_LOGINFO();
  AR_ASSERT_RETURN(robot);
  robot->waitForRunExit();
}

/** Exit program after quick disconnection from robot and other devices. */
AREXPORT void aria_exit(int code)
{
  AR_DEBUG_LOGINFO();
  Aria::exit(code);
}

/** Uninitialize ARIA without exiting program. */
AREXPORT void aria_shutdown()
{
	AR_DEBUG_LOGINFO();
#ifdef MATLAB
	aria_clearloghandler();
#endif
	Aria::shutdown();
}

AREXPORT double arrobot_getx()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double x = robot->getX();
  robot->unlock();
  return x;
}

AREXPORT double arrobot_gety()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double y = robot->getY();
  robot->unlock();
  return y;
}

AREXPORT double arrobot_getth()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double th = robot->getTh();
  robot->unlock();
  return th;
}

AREXPORT arpose arrobot_getpose()
{
  AR_DEBUG_LOGINFO();
  arpose cp;
  if(!connected)
  {
	cp.x = 0;
	cp.y = 0;
	cp.th = 0;
	return cp;
  }
  robot->lock();
  ArPose p = robot->getPose();
  robot->unlock();
  cp.x = p.getX();
  cp.y = p.getY();
  cp.th = p.getTh();
  return cp;
}

AREXPORT double arrobot_getvel()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double v = robot->getVel();
  robot->unlock();
  return v;
}

AREXPORT double arrobot_getrotvel()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double v = robot->getRotVel();
  robot->unlock();
  return v;
}

AREXPORT double arrobot_getlatvel()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double v = robot->getLatVel();
  robot->unlock();
  return v;
}

AREXPORT arpose arrobot_getvels()
{
  AR_DEBUG_LOGINFO();
  arpose p;
  if(!connected) {
	p.x = 0;
    p.y = 0;
    p.th = 0;
    return p;
  }
  robot->lock();
  p.x = robot->getVel();
  p.y = robot->getLatVel();
  p.th = robot->getRotVel();
  robot->unlock();
  return p;
}

AREXPORT double arrobot_radius()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  return robot->getRobotRadius();
}

AREXPORT double arrobot_width()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  return robot->getRobotWidth();
}

AREXPORT double arrobot_length()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  return robot->getRobotLength();
}

AREXPORT double arrobot_getleftvel()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double v = robot->getLeftVel();
  robot->unlock();
  return v;
}

AREXPORT double arrobot_getrightvel()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0.0);
  robot->lock();
  double v = robot->getRightVel();
  robot->unlock();
  return v;
}

AREXPORT int arrobot_isleftstalled()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 1;
  AR_ASSERT_RETURN_VAL(robot, 1);
  robot->lock();
  bool v = robot->isLeftMotorStalled();
  robot->unlock();
  return v;
}

AREXPORT int arrobot_isrightstalled()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 1;
  AR_ASSERT_RETURN_VAL(robot, 1);
  robot->lock();
  bool v = robot->isRightMotorStalled();
  robot->unlock();
  return v;
}

AREXPORT int arrobot_isstalled()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 1;
  AR_ASSERT_RETURN_VAL(robot, 1);
  robot->lock();
  bool v = robot->isRightMotorStalled() || robot->isLeftMotorStalled();
  robot->unlock();
  return v;
}

AREXPORT char arrobot_getdigin()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0;
  AR_ASSERT_RETURN_VAL(robot, 0);
  robot->lock();
  char v = robot->getDigIn();
  robot->unlock();
  return v;
}

AREXPORT void arrobot_setdigout(char c)
{
    AR_DEBUG_LOGINFO();
	if(!connected) return;
    AR_ASSERT_RETURN(robot);
    robot->lock();
	robot->com2Bytes(ArCommands::DIGOUT, (char)0xFF, c);
    robot->unlock();
}

AREXPORT double  arrobot_getsonarrange(int i)
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0.0;
  AR_ASSERT_RETURN_VAL(robot, 0);
  robot->lock();
  int v = robot->getSonarRange(i);
  robot->unlock();
  return v;
}

AREXPORT int arrobot_getnumsonar()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return 0;
  AR_ASSERT_RETURN_VAL(robot, 0);
  robot->lock();
  int n = robot->getNumSonar();
  robot->unlock();
  return n;
}


AREXPORT void arrobot_getsonar(double s[AR_MAX_NUM_SONAR])
{
    AR_DEBUG_LOGINFO();
	if(!connected) return;
    robot->lock();
    for(int i = 0; i < robot->getNumSonar(); ++i)
    {
        s[i] = robot->getSonarRange(i);
    }
    robot->unlock();
}

AREXPORT void arrobot_setpose(double x, double y, double th)
{

  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->moveTo(ArPose(x, y, th));
  robot->unlock();
}

AREXPORT void arrobot_stop()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->stop();
  robot->unlock();
}

AREXPORT void arrobot_setvels(arpose v)
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  robot->lock();
  robot->setVel(v.x);
  robot->setLatVel(v.y);
  robot->setRotVel(v.th);
  robot->unlock();
}


AREXPORT void arrobot_setvel(double vel)
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->setVel(vel);
  robot->unlock();
}

AREXPORT void arrobot_setwheelvels(double left, double right)
{
	AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->setVel2(left, right);
  robot->unlock();
}

AREXPORT void arrobot_setrotvel(double rotvel)
{
	AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->setRotVel(rotvel);
  robot->unlock();
}

AREXPORT void arrobot_setlatvel(double vel)
{
	AR_DEBUG_LOGINFO();
	if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->setLatVel(vel);
  robot->unlock();
}

AREXPORT double arrobot_getbatteryvoltage()
{
	double v;
	AR_DEBUG_LOGINFO();
	if(!connected) return 0.0;
	AR_ASSERT_RETURN_VAL(robot, 0.0);
	robot->lock();
	v = robot->getRealBatteryVoltage();
	robot->unlock();
	return v;
}


AREXPORT int arrobot_num_front_bumpers()
{
	int n;
	AR_DEBUG_LOGINFO();
	if(!connected) return 0;
	AR_ASSERT_RETURN_VAL(robot, 0);
	robot->lock();
	n = robot->getNumFrontBumpers();
	robot->unlock();
	return n;
}

AREXPORT int arrobot_num_rear_bumpers()
{
	int n;
	AR_DEBUG_LOGINFO();
	if(!connected) return 0;
	AR_ASSERT_RETURN_VAL(robot, 0);
	robot->lock();
	n = robot->getNumRearBumpers();
	robot->unlock();
	return n;
}


AREXPORT void arrobot_get_bumpers(char *front, char *rear)
{
	int s;
	AR_DEBUG_LOGINFO();
	if(!connected) return;
	AR_ASSERT_RETURN(robot);
	robot->lock();
	s = robot->getStallValue();
	robot->unlock();
	// Extract just the bumper flags from the stall and bumper flags.
	// Bit 0 will be the first bumper, bit 2 the second, etc. in each byte.
	if(front)
		*front = ((robot->getStallValue() & 0xff00) >> 8) >> 1; // second byte is front bumpers; BIT0 is the left stall flag, shift over it
	if(rear)
		*rear = (robot->getStallValue() & 0x00ff) >> 1; // LSB is front bumpers; BIT0 is the right stall flag, shift over it
}

AREXPORT int arrobot_get_front_bumper(int i)
{
	char fb;
	AR_DEBUG_LOGINFO();
	if(!connected) return 0;
	AR_ASSERT_RETURN_VAL(robot, 0);
	arrobot_get_bumpers(&fb, NULL);
	return fb & (ArUtil::BIT0 << i);
}


AREXPORT int arrobot_get_rear_bumper(int i)
{
	char rb;
	AR_DEBUG_LOGINFO();
	if(!connected) return 0;
	AR_ASSERT_RETURN_VAL(robot, 0);
	arrobot_get_bumpers(NULL, &rb);
	return rb & (ArUtil::BIT0 << i);
}

AREXPORT void arrobot_setdeltaheading(double d)
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->setDeltaHeading(d);
  robot->unlock();
}

AREXPORT void arrobot_command(int c)
{
  AR_DEBUG_LOGINFO();
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->com(c);
  robot->unlock();
}

AREXPORT void arrobot_command_int(int c, int a)
{
  AR_DEBUG_LOGINFO();
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->comInt(c, a);
  robot->unlock();
}

AREXPORT void arrobot_command_2bytes(int c, char b1, char b2)
{
  AR_DEBUG_LOGINFO();
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->com2Bytes(c, b1, b2);
  robot->unlock();
}

AREXPORT void arrobot_resetpos()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  ArPose p = robot->getPose();
  robot->com(ArCommands::SETO);
  robot->moveTo(ArPose(0, 0, 0), p);
  ArLog::log(ArLog::Normal, "arrobot_resetpos: Reset robot odometery to origin and transformed ArRobot pose, it is now %0.2f, %0.2f, %0.2f.\n", robot->getX(), robot->getY(), robot->getTh());
  robot->unlock();
}

AREXPORT void arrobot_move(double d)
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->move(d);
  robot->unlock();
}

AREXPORT int arrobot_motors_enabled()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return false;
  AR_ASSERT_RETURN_VAL(robot, false);
  robot->lock();
  int b = robot->areMotorsEnabled();
  robot->unlock();
  return b;
}

AREXPORT void arrobot_enable_motors()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->enableMotors();
  robot->unlock();
}


AREXPORT void arrobot_disable_motors()
{
  AR_DEBUG_LOGINFO();
  if(!connected) return;
  AR_ASSERT_RETURN(robot);
  robot->lock();
  robot->disableMotors();
  robot->unlock();
}


AREXPORT void arrobot_command_2int4(int c, int i1, int i2)
{
	AR_DEBUG_LOGINFO();
	if(!connected) return;
	AR_ASSERT_RETURN(robot);
	ArRobotPacket pkt;
	pkt.setID(c);
	pkt.uByteToBuf(0); // argument type: ignored.
	pkt.byte4ToBuf((ArTypes::Byte4)i1);
	pkt.byte4ToBuf((ArTypes::Byte4)i2);
	pkt.finalizePacket();
	robot->lock();
	robot->getDeviceConnection()->writePacket(&pkt);
	robot->unlock();
}


AREXPORT void arrobot_command_3int4(int c, int i1, int i2, int i3)
{
	AR_DEBUG_LOGINFO();
	if(!connected) return;
	AR_ASSERT_RETURN(robot);
	ArRobotPacket pkt;
	pkt.setID(c);
	pkt.uByteToBuf(0); // argument type: ignored.
	pkt.byte4ToBuf((ArTypes::Byte4)i1);
	pkt.byte4ToBuf((ArTypes::Byte4)i2);
	pkt.byte4ToBuf((ArTypes::Byte4)i3);
	pkt.finalizePacket();
	robot->lock();   // Omit this if in an ArRobot task callback (e.g. user task or sensor interpretation task)
	robot->getDeviceConnection()->writePacket(&pkt);
	robot->unlock(); // Omit this if in an ArRobot task callback (e.g. user task or sensor interpretation task)
}
