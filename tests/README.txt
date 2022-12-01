
Tests
=====

Note: Tests should not be compiled with -DNDEBUG in EXTRA_CXXFLAGS
or any other source of compilation flags, since many rely
on aborting on a failed assert().

Help wanted: We especially need automated unit/regression tests for:
  * ArArgumentBuilder
  * Packet parsing, robot packet parsing
  * RangeDevice data processing (ArRangeDevice, ArRangeBuffer, ArSensorReading)
  * ArLaserFilter
  * all of ArMath and ArUtil (a few parts are currently tested)
  * ArDeviceConnection, ArSerialConnection, ArSocket.
  * Map loading and parsing (current mapTest just prints stuff out, should use ApprovalTest or just check for known correct data)
  * ArForbiddenRangeDevice
  * ArRobotParams, parameter wrangling and parameter file reading
  * more
  

Automatic unit/regression tests
-------------------------------

These test various ARIA utilities, will exit with nonzero 
status if any tests fail (usually aborted from a failed assert()).

Note: Test coverage of these tests is extremely incomplete.
Any contributions are welcome. If you are investigating or fixing
a bug, please create a test and add it to RUNNABLE_TESTS in 
Makefile.

"make runTests" will run these and report failures. (Note, "make runTest" adds
`../lib` to `LD_LIBRARY_PATH` before running tests, so `../lib/libAria.so` will be
used.)

* angleBetweenTest - Tests ArMath::angleBetween
* angleFixTest - Tests ArMath::angleFix
* angleTest - Tests some various things with angles
* arsectors - Tests ArSectors class
* configTest, configSectionTest - Tests ArConfig reading in a file and writing files
* fileParserTest - just tests the file parser and shows how to use it a little
* functorTest - Does some extensive tests of functors
* getValuesFromCharBuf
* gpsCoordsTest
* interpolationTest - Tests the position interpolation functions on ArRobot
* lineTest - Tests the used functionality of ArLine and ArLineSegment
* lms1xxPacket - Tests reading/writing ArLMS1XXPacket
* moreStringTests - Test some string utilities in ArUtil
* nmeaParser - Tests ArNMEAParser used in ArGPS
* poseTest - Tests out ArPose
* stripQuoteTest - Test ArUtil::stripQuotes
* transformTest - Tests out ArTransform

"Slow" automatic tests
----------------------

Same as above, but may take several seconds or minutes to run, so
are omitted from "make runTests":


* timeTest - Just does a simple test of the functions related to ArTime

Interactive/Robot tests
-----------------------

These are functional tests that expect to connect either with the 
simulator (AMRISim or MobileSim) or a robot, or a specific hardware
accessory, or run forever without terminating, and exercise or 
demonstrate some functionality which you must observe and judge to 
be correct.

absoluteHeadingActionTest - Tests an action with absolute heading.

actionArgumentTest - Tests the ArArg class and the argument part of ArAction.

actionAverageTest - Tests to make sure actions average right if at
same priority (differently than samePriorityActionTest, slightly)

actionManagementTest - Tests adding, removing, and finding actions.

actsTest - Does a test print out from ACTS

asyncConnectTest - Connects to the robot, disconnects, and tries to break the
connection state. This was designed to test the connection sequence.  This
uses ArRobot::asyncConnect.

auxSerialTest - Dumps a lot of things out to aux serial port with TTY commands

callbackTest - Tests the connection callbacks in ArRobot

chargeTest - A test for charging with a powerbot dock

connectTest - Connects to the robot, disconnects, and tries to break the
connection state. This was designed to test the connection sequence.  This
uses ArRobot::blockingConnect.

connectionTest - Tests the connection by requesting IO packets as it
drives about hard and fast (make sure it won't hurt anyone)

driveFast - a test that drives the robot fast for a given distance

encoderCorrectionTest - Connects to a robot with a joystick, pressing button
two will set the encoder correction callback, just run it to see the args

gotoTest - Uses the ArActionGoto to go somewhere in a very naive way

hardDriveWander - This drives about very hard and fast, only run this
in lots of space where no one will get hurt


ioTest - Tests the response time for IOREQUEST commands sent to the robot

keyHandlerTest - Tests the keyhandler out

keys - Lower level test of the keyhandler


moveRobotTest - Drives the robot around, has different actions for pushing 
button 2, its to make sure that the ArRobot::moveTo(pos) command works in
some fashion, and to check the transforms, just run the program to have it
print its usage

optoIOtest - This is a very simple test of using the Opto22 interface on the 
Versalogic motherboards in P2 and P3 robots.  It also tests the analog

p2osSlamTest - Sends lots of packets to the P2 to try and mess it up



robotListTest - Tests some of the Aria:: functions that have to do with
the robot list

robotConfigPacketReaderTest - A test of getting the robot config packet

rotvelActionExample - Tests out an action that drives using rot vel

runtimeTest - Times how long the robot will run when doing a period of
wandering, then a period of resting, then wandering and so on, until the
battery dies.  It runs ACTS while wandering, and pipes the display to another computer.

samePriorityActionTest - Sees if actions average right when at the
same priority (differently than actionAverageTest, slightly)

serialTest - Test for checking for interference on a serial port

serialTest2 - Another test for checking for interference on a serial port

sickSimpleTest - A test used in the development of the sick driver

sickMiddleTest - A test that prints out the middle readings of the
laser continually

sickTest - A test that connects to the laser then prints out a few
sets of readings

sickTestAll - A test that connects to the laser, turns off all
filtering then prints out readings until killed

signalTest - Prints out signals that come in to the program

sonarDeviceTest - Prints out the sonar readings from the sonarDevice

sonarTest - Prints out the sonar disk positions then prints out the sonar
readings

stallTest - Tests the stall behavior by going forward and ramming things, you
should only use this one in the simulator, also can be set to go backwards

stressTest - Creates 2 busy loops, and sees if the syncLoop still winds up 
with the right amount of time between runs

tcm2Test - Connects to the tcm2 compass and prints out its information

threadTest - Does a rudamentary test on threading

timingTest - Does a test of how long the syncLoop takes to run, prints out 
the results

triangleAccuracyTest - Tests out the repeatability of ArActionTriangleDriveTo

usertasktest - Tests the user task list that ArRobot maintains.

vcc4Test - Test and exercise a vcc4 camera

velTest - has the robot drive at set velocities (trans and rot) and
prints out how fast it says its going
