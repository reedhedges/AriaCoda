These are test programs, users may not want to look at these to
understand how ARIA works, but after understanding ARIA may want to
look at specific files in this directory to see some more complicated
things.  These are not documented much, each program trying only to do
one or two things, which should be fairly self-evident from these
descriptions and from the code.

-------------------------------------------------------------------------------

absoluteHeadingActionTest - Tests an action with absolute heading.

actionArgumentTest - Tests the ArArg class and the argument part of ArAction.

actionAverageTest - Tests to make sure actions average right if at
same priority (differently than samePriorityActionTest, slightly)

actionManagementTest - Tests adding, removing, and finding actions.

actsTest - Does a test print out from ACTS

angleBetweenTest - Tests ArMath::angleBetween

angleFixTest - Tests ArMath::angleFix

angleTest - Tests some various things with angles

asyncConnectTest - Connects to the robot, disconnects, and tries to break the
connection state. This was designed to test the connection sequence.  This
uses ArRobot::asyncConnect.

auxSerialTest - Dumps a lot of things out to aux serial port with TTY commands

callbackTest - Tests the connection callbacks in ArRobot

chargeTest - A test for charging with a powerbot dock

configTest - Tests ArConfig reading in a file and writing files

connectTest - Connects to the robot, disconnects, and tries to break the
connection state. This was designed to test the connection sequence.  This
uses ArRobot::blockingConnect.

connectionTest - Tests the connection by requesting IO packets as it
drives about hard and fast (make sure it won't hurt anyone)

driveFast - a test that drives the robot fast for a given distance

encoderCorrectionTest - Connects to a robot with a joystick, pressing button
two will set the encoder correction callback, just run it to see the args

fileParserTest - just tests the file parser and shows how to use it a little

functorTest - Does some extensive tests of functors

gotoTest - Uses the ArActionGoto to go somewhere in a very naive way

hardDriveWander - This drives about very hard and fast, only run this
in lots of space where no one will get hurt

interpolationTest - Tests the position interpolation functions on ArRobot

ioTest - Tests the response time for IOREQUEST commands sent to the robot

keyHandlerTest - Tests the keyhandler out

keys - Lower level test of the keyhandler

lineTest - Tests the used functionality of ArLine and ArLineSegment

moveRobotTest - Drives the robot around, has different actions for pushing 
button 2, its to make sure that the ArRobot::moveTo(pos) command works in
some fashion, and to check the transforms, just run the program to have it
print its usage

optoIOtest - This is a very simple test of using the Opto22 interface on the 
Versalogic motherboards in P2 and P3 robots.  It also tests the analog

p2osSlamTest - Sends lots of packets to the P2 to try and mess it up

paramTest - Tests out some of the ArPreference parameter stuff

poseTest - Tests out ArPose

robotListTest - Tests some of the Aria:: functions that have to do with
the robot list

robotConfigPacketReaderTest - A test of getting the robot config packet

rotvelActionExample - Tests out an action that drives using rot vel

runtimeTest - Times how long the robot will run when doing a period of
wandering, then a period of resting, then wandering and so on, until the
battery dies.  It runs ACTS while wandering, and pipes the display to another computer.

samePriorityActionTest - Sees if actions average right when at the
same priority (differently than actionAverageTest, slightly)

segvTest - Causes a seg fault to see if its handled right

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

stripQuotesTest - Just a tests that tests ArUtil::stripQuotes

systemCallTest - Tests doing system calls and signals

tcm2Test - Connects to the tcm2 compass and prints out its information

threadTest - Does a rudamentary test on threading

timeTest - Just does a simple test of the functions related to ArTime

timingTest - Does a test of how long the syncLoop takes to run, prints out 
the results

transformTest - Tests out ArTransform

triangleAccuracyTest - Tests out the repeatability of ArActionTriangleDriveTo

usertasktest - Tests the user task list that ArRobot maintains.

vcc4Test - Test and exercise a vcc4 camera

velTest - has the robot drive at set velocities (trans and rot) and
prints out how fast it says its going
