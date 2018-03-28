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
  Shows how to add a task callback to ArRobot's synchronization/processing cycle

  This program will just have the robot wander around, it uses some avoidance 
  routines, then just has a constant velocity.  A sensor interpretation task callback is invoked
  by the ArRobot object every cycle as it runs, which records the robot's current 
  pose and velocity.

  In Java, to pass an ArFunctor object to Aria, you must define a subclass 
  of ArFunctor which overrides the invoke() method.

  Note that tasks must take a small amount of time to execute, to avoid delaying the
  robot cycle.
*/

import com.mobilerobots.Aria.*;

class PrintingTask extends ArFunctor
{
  private ArRobot myRobot;


  // Constructor. Adds our 'user task' to the given robot object.
  public PrintingTask(ArRobot robot)
  {
    System.out.println("Java PrintingTask: adding sensor interpretation task to ArRobot...");
    myRobot = robot;
    myRobot.addSensorInterpTask("PrintingTask", 50, this);
  }

  // This method will be called when Aria invokes the task functor
  public void invoke() 
  {
    // print out some info about the robot
    System.out.println("Java PrintingTask: x " + (int) myRobot.getX() +
                      " y " + (int) myRobot.getY() +
                      " th " + (int) myRobot.getTh() +
                      " vel " + (int) myRobot.getVel() +
                      " mpacs " + (int) myRobot.getMotorPacCount());

    // Need sensor readings? Try myRobot.getRangeDevices() to get all 
    // range devices, then for each device in the list, call lockDevice(), 
    // getCurrentBuffer() to get a list of recent sensor reading positions, then
    // unlockDevice().
  }

}


public class robotSyncTaskExample 
{
  static {
    try {
      System.loadLibrary("AriaJava");
    } catch(UnsatisfiedLinkError e) {
      System.err.println("Native code library (libAriaJava.so or AriaJava.dll) failed to load. Make sure that it is in your library path.");
      System.exit(1);
    }
  }

  public static void main(String argv[])
  {
    Aria.init();
    ArArgumentParser parser = new ArArgumentParser(argv);
    parser.loadDefaultArguments();
    ArSimpleConnector con = new ArSimpleConnector(parser);
    if(!Aria.parseArgs())
    {
      Aria.logOptions();
      System.exit(1);
    }

    ArRobot robot = new ArRobot();

    ArSonarDevice sonar = new ArSonarDevice();

    PrintingTask printingTask = new PrintingTask(robot);

    // the actions we will use to wander
    ArActionStallRecover recover = new ArActionStallRecover();
    ArActionAvoidFront avoidFront = new ArActionAvoidFront();
    ArActionConstantVelocity constantVelocity = new ArActionConstantVelocity("Constant Velocity", 400);


    // add the sonar object to the robot
    robot.addRangeDevice(sonar);

    // open the connection to the robot; if this fails exit
    if(!con.connectRobot(robot))
    {
      System.err.println("Could not connect to the robot.");
      System.exit(1);
    }
    System.out.println("Java robotSyncTaskExample: Connected to the robot. (Press Ctrl-C to exit)");
    
    
    // turn on the motors
    robot.enableMotors();

    // add the wander actions
    robot.addAction(recover, 100);
    robot.addAction(avoidFront, 50);
    robot.addAction(constantVelocity, 25);
    
    // Start the robot process cycle running. Each cycle, it calls the robot's
    // tasks. When the PrintingTask was created above, it added a new
    // task to the robot. 'true' means that if the robot connection
    // is lost, then ArRobot's processing cycle ends and this call returns.
    robot.run(true);

    System.out.println("Java robotSyncTaskExample: Disconnected. Goodbye.");
    
  }
}
