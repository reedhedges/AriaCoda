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

#ifndef _ARIAC_H_
#define _ARIAC_H_

#ifndef ARIA_STATIC
#ifndef AREXPORT
#ifdef WIN32

#define AREXPORT __declspec(dllimport) extern
#else
#define AREXPORT   
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @mainpage 
ARIAC is a highly simplified C interface (wrapper) to ARIA. It includes
a subset of ArRobot functions for basic connection and control of the robot.

ariac links dynamically to ARIA.  Any program linking to it must also link to
ARIA and dependent libraries:

On Linux, link to libAria, libdl, librt, libm, e.g.:
  gcc -fPIC -o myprog myprog.c -L/usr/local/Aria/lib -lariac -lAria -lpthread -ldl -lrt -lm

On Windows, link to Aria or AriaDebug, ws2_32 and winmm, and include the ARIA
lib directory in the linker path. (Include the ARIA bin directory in system path
or copy DLLs when running the program.)

To use, first initialize ARIA with aria_init() or aria_init_with_args(argc,
argv), with which you can pass in program command line options which will 
be parsed and loaded into ARIA.  (Various ARIA options can be configured via 
command-line arguments if aria_init_with_args() is used.)

Next, to connect to the robot, then call arrobot_connect().  1 will be 
returned on error.  ARIA's ArRobot background processing thread for
communications with the robot and other background tasks will be started.

Once connection is finished, then accessor functions (arrobot_get* and arrobot_set*) may be called from multiple
threads (since they access ARIA in a thread safe way), but connection and
disconnection must happen separately from access. 

If any accessor functions are called before the robot connection is made, they are ignored and exit. (This is intentional, to
support use in environments such as Simulink that will be continuously syncronizing robot state, even before the user 
initiates the robot connection.)

*/
    

/** Initialize library, store references to command line arguments if given.
 * Only needs to be called once before any other functions.  You must call either aria_shutdown() or aria_exit() to uninitialize ARIA or exit program. */
AREXPORT int aria_init_with_args(int argc, char **argv);

AREXPORT int aria_init();

/** Set a log message handler to call in addition to printing to normal ArLog output destination (which is console/stdout by default) */
AREXPORT void aria_setloghandler(void(*logfn)(const char*));

/** Clear a log message handler set with aria_setloghandler(). */
AREXPORT void aria_clearloghandler();

/** Print internal ariac debugging information */
AREXPORT void arloginfo(const char *m);

/** Try connecting to the robot, enable robot motors, then
    start ArRobot's background communications thread.
    Various ARIA objects are created and stored. Call disconnect() or exit() to
    disconnect from the robot and delete stored objects.
    May be called multiple times. If a previous call to connect() resulted
    in a successful connection, then nothing will be done. If there was no
    previous successful connection, try connection (again). May also be called
    after disconnect(), in which case, reconnect to the robot.
    @arg argc Number of command line arguments. Pass 0 if none.
    @arg argv Command line arguments. Pass NULL if none.
    @return 1 If successfully connected, 0 if failed.
*/
AREXPORT int arrobot_connect();

/** Disconnect from robot and clear any saved information. */
AREXPORT void arrobot_disconnect();

/** Block until robot connection is broken (e.g. by robot reset). */
AREXPORT void arrobot_wait();

/** Exit program after quick disconnection from robot and other devices. */
AREXPORT void aria_exit(int code);

/** Uninitialize ARIA but don't exit program. */
AREXPORT void aria_shutdown();

typedef struct {double x; double y; double th;} arpose;

AREXPORT double arrobot_getx();
AREXPORT double arrobot_gety();
AREXPORT double arrobot_getth();
AREXPORT arpose arrobot_getpose();
AREXPORT double arrobot_getvel();
AREXPORT double arrobot_getrotvel();
AREXPORT double arrobot_getlatvel();
AREXPORT arpose arrobot_getvels();
AREXPORT double arrobot_radius();
AREXPORT double arrobot_width();
AREXPORT double arrobot_length();
AREXPORT double arrobot_getleftvel();
AREXPORT double arrobot_getrightvel();
AREXPORT int arrobot_isleftstalled();
AREXPORT int arrobot_isrightstalled();
AREXPORT int arrobot_isstalled();
AREXPORT char arrobot_getdigin();
AREXPORT double arrobot_getsonarrange(int i);
AREXPORT int arrobot_getnumsonar();
#define AR_MAX_NUM_SONAR 16
AREXPORT void arrobot_getsonar(double s[AR_MAX_NUM_SONAR]);

/** Change (move) stored robot position to the pose given by @a p,
 *  and transform current range sensor readings and other positions affected by
 *  robot pose. All future position updates received from the robot will
 *  continue to be relative to this point.
 */
AREXPORT void arrobot_movepose_p(arpose p);

/** Change (move) stored robot position to the pose given by @a x, @a y, @a z,
 *  and transform current range sensor readings and other positions affected by
 *  robot pose. All future position updates received from the robot will
 *  continue to be relative to this point.
 */
AREXPORT void arrobot_setpose(double x, double y, double th);

/** Change (move) stored robot position to the pose given by @a p,
 *  and transform current range sensor readings and other positions affected by
 *  robot pose. All future position updates received from the robot will
 *  continue to be relative to this point.
 */
void arrobot_setpose_p(arpose p)
{
	arrobot_setpose(p.x, p.y, p.th);
}

AREXPORT void arrobot_stop();
AREXPORT void arrobot_setvel(double vel);
AREXPORT void arrobot_setwheelvels(double left, double right);

/** @deprecated */
void arrobot_setvel2(double left, double right)
{
  arrobot_setwheelvels(left, right);
}

AREXPORT void arrobot_setrotvel(double rotvel);
AREXPORT void arrobot_setlatvel(double vel);
AREXPORT void arrobot_setvels(arpose v);
AREXPORT void arrobot_setdeltaheading(double dh);
AREXPORT void arrobot_setdigout(char c);
AREXPORT double arrobot_getbatteryvoltage();
AREXPORT int arrobot_num_front_bumpers();
AREXPORT int arrobot_num_rear_bumpers();
AREXPORT char arrobot_get_front_bumpers();

/** Return 1 if front bumper @arg i is currently pressed */
AREXPORT int arrobot_get_front_bumper(int i);

AREXPORT char arrobot_get_rear_bumpers();

/** Return 1 if front bumper @arg i is currently pressed */
AREXPORT int arrobot_get_rear_bumper(int i);

/** Send any protocol command to the robot */
AREXPORT void arrobot_command(int c);

/** Send any protocol command to the robot with one 16-bit argument */
AREXPORT void arrobot_command_int(int c, int a);

/** Send any protocol command to the robot with two 8-bit (byte) arguments */
AREXPORT void arrobot_command_2bytes(int c, char b1, char b2);

/** Send any command to the robot with two 32-bit (4byte) arguments */
AREXPORT void arrobot_command_2int4(int c, int i1, int i2);

/** Send any command to the robot with three 32-bit (4byte) arguments */
AREXPORT void arrobot_command_3int4(int c, int i1, int i2, int i3);

AREXPORT void arrobot_resetpos();

/** Request discrete forward movement (mm) */
AREXPORT void arrobot_move(double d);

AREXPORT int arrobot_motors_enabled();
AREXPORT void arrobot_enable_motors();
AREXPORT void arrobot_disable_motors();

#ifdef __cplusplus
} /* close extern "C" */
#endif

#endif
