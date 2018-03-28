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
 
example program demonstrating how to make and use new actions.

This example program creates two new actions, Go and Turn. Go will drive the robot forward safely,
while Turn will avoid obstacles detected by the sonar by turning. 
This program also adds a predefined
action from Aria which tries to recover from stalls (hit something and 
can't move forward) by backing and turning.

Each of these actions have the normal constructor and destructor, note that 
the constructors use constructor chaining to create their ArAction part
correctly.  Each action then also implements the essential virtual method, 
fire(). This fire function is called by the action resolver, and
returns values that, in combination with other actions' desired behavior,
determine the driving commands sent to the robot.

Also note that each of these actions override the setRobot function; these 
implementations obtain the sonar device from the robot in addition to doing the
needed caching of the robot pointer.  This is what you should do if you
care about the presence or absence of a particular sensor.  If you don't
care about any particular sensor you could just use one of the  checkRangeDevice...
methods in ArRobot (there are four of them).
Also note that these are very naive actions, they are simply an example
of how to use actions.

See the @ref actions Actions section of the Aria reference manual overview for more details about actions.

Note that actions must take a small amount of time to execute, to avoid
delaying the robot synchronization cycle.

*/



import com.mobilerobots.Aria.*;

/** 
 * Example Action class that drives the robot forward, but stops if obstacles are
 * detected by sonar. 
 */
class ExampleGoAction extends ArAction
{
  // constructor, sets myMaxSpeed and myStopDistance
  public ExampleGoAction(double maxSpeed, double stopDistance) 
  {
    super("Go");
    myMaxSpeed = maxSpeed;
    myStopDistance = stopDistance;
    myDesired = new ArActionDesired();
  }


  /**
    This fire is the whole point of the action.
    currentDesired is the combined desired action from other actions
    previously processed by the action resolver.  In this case, we're
    not interested in that, we will set our desired 
    forward velocity in the myDesired member, and return it.

  */
  public ArActionDesired fire(ArActionDesired currentDesired)
  {
    double speed;


    // reset the actionDesired (must be done), to clear
    // its previous values.
    myDesired.reset();

    // get the range of the sonar
	double robotRadius = 400;
    double range = mySonar.currentReadingPolar(-70, 70) - robotRadius;
    // if the range is greater than the stop distance, find some speed to go
    if (range > myStopDistance)
    {
      // just an arbitrary speed based on the range
      speed = range * .3;
      // if that speed is greater than our max, cap it
      if (speed > myMaxSpeed)
        speed = myMaxSpeed;
      // now set the velocity
      myDesired.setVel(speed);
    }
    // the range was less than the stop distance, so request stop
    else
    {
      myDesired.setVel(0);
    }
    // return the actionDesired to the resolver to make our request
    return myDesired;
  }


  /**
    Override ArAction.setRobot() to get the sonar device from the robot, or deactivate this action if it is missing.
    You must also call ArAction.setRobot() to properly store
    the ArRobot pointer in the ArAction base class.
  */
  public void setRobot(ArRobot robot)
  {
    setActionRobot(robot);
    mySonar = robot.findRangeDevice("sonar");
  }

  // the sonar device object obtained from the robot by setRobot()
  protected ArRangeDevice mySonar;


  /** Our current desired action: fire() modifies this object and returns
      to the action resolver a pointer to this object.
      This object is kept as a class member so that it persists after fire()
      returns (otherwise fire() would have to create a new object each invocation,
      but would never be able to delete that object).
  */
  protected ArActionDesired myDesired;

  protected double myMaxSpeed;
  protected double myStopDistance;
}

/** Example action class that turns the robot away from obstacles detected by the 
 * sonar. Used by actionExample.java.  */
class ExampleTurnAction extends ArAction
{

  public ExampleTurnAction(double turnThreshold, double turnAmount) 
  {
    super("Turn");
    myTurnThreshold = turnThreshold;
    myTurnAmount = turnAmount;
    myTurning = 0;
    myDesired = new ArActionDesired();
  }

  /**
    This is the guts of the Turn action.
  */
  public ArActionDesired fire(ArActionDesired currentDesired)
  {
    double leftRange, rightRange;
    // reset the actionDesired (must be done)
    myDesired.reset();
    double robotRadius = 400;
    // Get the left readings and right readings off of the sonar
    leftRange = (mySonar.currentReadingPolar(0, 100) - robotRadius);
    rightRange = (mySonar.currentReadingPolar(-100, 0) - robotRadius);
    // if neither left nor right range is within the turn threshold,
    // reset the turning variable and don't turn
    if (leftRange > myTurnThreshold && rightRange > myTurnThreshold)
    {
      myTurning = 0;
      myDesired.setDeltaHeading(0);
    }
    // if we're already turning some direction, keep turning that direction
    else if (myTurning != 0)
    {
      myDesired.setDeltaHeading(myTurnAmount * myTurning);
    }
    // if we're not turning already, but need to, and left is closer, turn right
    // and set the turning variable so we turn the same direction for as long as
    // we need to
    else if (leftRange < rightRange)
    {
      myTurning = -1;
      myDesired.setDeltaHeading(myTurnAmount * myTurning);
    }
    // if we're not turning already, but need to, and right is closer, turn left
    // and set the turning variable so we turn the same direction for as long as
    // we need to
    else 
    {
      myTurning = 1;
      myDesired.setDeltaHeading(myTurnAmount * myTurning);
    }
    return myDesired;
  }


  /**
    Calls the ArAction.setRobot() method to store the robot object (all setRobot overloaded functions must do this),
    finds the sonar device from the robot.
  */
  public void setRobot(ArRobot robot)
  {
    setActionRobot(robot);
    mySonar = robot.findRangeDevice("sonar");
  }

  // this is to hold the sonar device form the robot
  protected ArRangeDevice mySonar;
  // what the action wants to do; used by the action resover after fire()
  protected ArActionDesired myDesired;
  // distance at which to start turning
  protected double myTurnThreshold;
  // amount to turn when turning is needed
  protected double myTurnAmount;
  // remember which turn direction we requested, to help keep turns smooth
  protected int myTurning; // -1 == left, 1 == right, 0 == none
}



public class actionExample {


  /* Try to load the Aria and ArNetworking wrapper libraries when the
   * program starts:
   */
  static {
    try {
      System.loadLibrary("AriaJava");
    } catch(UnsatisfiedLinkError e) {
      System.err.println("Native code library libAriaJava failed to load. Make sure that its directory is in your library path. See javaExamples/README.txt and the chapter on Dynamic Linking Problems in the SWIG Java Documentation (http://www.swig.org) for help.\n" + e);
      System.exit(1);
    }
  }


  public static void main(String argv[]) {
    Aria.init();

    ArSimpleConnector conn = new ArSimpleConnector(argv);
    ArRobot robot = new ArRobot();
    ArSonarDevice sonar = new ArSonarDevice();

    // Create instances of the actions defined above, plus ArActionStallRecover, 
    ExampleGoAction go = new ExampleGoAction(500, 350);
    ExampleTurnAction turn = new ExampleTurnAction(400, 10);

    // a predefined action from Aria.
    ArActionStallRecover recover = new ArActionStallRecover();

      
    // Parse all command-line arguments
    if(!Aria.parseArgs())
    {
      Aria.logOptions();
      System.exit(1);
    }
    
    // Connect to the robot
    if(!conn.connectRobot(robot))
    {
      ArLog.log(ArLog.LogLevel.Terse, "actionExample: Could not connect to robot! Exiting.");
      System.exit(2);
    }

    // Add the range device to the robot. You should add all the range 
    // devices and such before you add actions
    robot.addRangeDevice(sonar);

   
    // Add our actions in order. The second argument is the priority, 
    // with higher priority actions going first, and possibly pre-empting lower
    // priority actions.
    robot.addAction(recover, 60);
    robot.addAction(turn, 50);
    robot.addAction(go, 40);

    // Enable the motors, disable amigobot sounds
    robot.enableMotors();

    // Run the robot processing cycle.
    // 'true' means to return if it loses connection,
    // after which we exit the program.
    robot.run(true);
    
    Aria.exit(0);
  }
}

