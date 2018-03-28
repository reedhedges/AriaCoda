This directory has various example pieces of code and Microsoft VC++
project files.  

The two most useful example programs are teleopActionsExample and 
wander.  TeleopActionsExample does guarded teleoperation, so you 
can drive the robot around without running into walls.  
Wander just has the robot wander, ie drive forward until there is 
an obstacle, avoid it, then keep driving.  lasers shows how to
connect to laser rangefinder(s) and use the data from them.

The smallest, simplest example program is simpleConnect. It just
connects to the robot, prints some information, then disconnects
and exits.


Partial list of examples:
-------------------------------------------------------------------------------


demo - Uses "Modes" defined in ARIA to provide keyboard control of many
different robot features. Use this utility to experiment and test the robot's 
hardware.

simpleConnect - The smallest example program. It just connects to the robot.

simpleUserTask - Demonstrates how to make a simple user task

robotSyncTaskExample - simple example of ArRobot synchronized task callbacks

lasers - Demonstrates how to connect to one or more laser rangefinder devices
as defined in robot and program parameters, and obtain data from them.

simpleMotionCommands - Drives the robot around using the basic direct
motion commands (no ArActions or obstacle avoidance). 

actionExample - Defines a couple of custom ArAction classes that generate
robot motion requests, and uses them.

actionGroup - Program that uses action groups to switch back and forth 
between wander and teleop mode

cameraPTZExample - Connect to PTZ camera or PTU (depending on robot parameter
file configuration or command line arguments).

dpptuExample - A program to control the Directed Perception PTU with the keyboard.

functor - An example program on the basic use of functors.

gotoActionExample - An example of how to use ArActionGoto to go to many different
points and not just one

gpsExample - An example showing how to get data from a GPS

gripperDemo - Program that moves the robot and controls the gripper with the 
joystick, note this doesn't do obstacle avoidance

actsColorFolowingExample - A simple program that uses ACTS and a VC-C4 camera to move the
robot toward a color blob.

joydriveActionExample - Uses an action that reads the robot to drive the joystick,
does not do obstacle avoidance

joydriveThreaded - Program to drive the robot with a joystick. This one
uses its own ArASyncTask to drive the joystick handler.  This is a good
example to look at to see how threading works.  This does not do obstacle
avoidance, this also has a connection handler

joydriveUserTask - Program to drive the robot with a joystick. This one
uses a user task to drive the joystick handler, does not do obstacle avoidance

getAuxExample - An example program that uses the getAux command and
talks about how to use the getAux to do actual work

moduleExample - Tests the loadable modules in a simple way

socketClientExample - This program works with socketServerExample to
demonstrate ArSockets

socketServerExample - A program to demonstrate ArSocket, works with
socketClientExample

sickLineFinderSimple - An example of using the laser sensor line finding
class and lets you save the lines

threadExample - Demonstrates ARIA's threading tools

soundsQueueExamples - Demonstrates use of the ArSoundsQueue sound/speech queue

teleopActionsExample - This uses ARIA's powerful Actions system to drive
the robot around using input from the keyboard or a joystick, but does 
obstacle avoidance so the robot won't run into things (if it can sense 
them with sonar or laser). 

wander - Makes the robot wander around, turning to avoid any sensed obstacles. 
Uses sonar and laser (if available) to detect obstacles.

wanderAndLogData - similar to wander, but prints out all kinds of runtime
information about the robot

(others) - there are several other example programs in this directory not 
listed here. Some are for older or less common hardware, or show some
advanced features of ARIA that only some users will need. Browse them
and read their descriptions (in comments at the top of source files)
for more information.
