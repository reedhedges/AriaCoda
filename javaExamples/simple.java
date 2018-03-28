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

/* A simple example of connecting to and driving the robot with direct
 * motion commands.
 */

import com.mobilerobots.Aria.*;

public class simple {

  static {
    try {
        System.loadLibrary("AriaJava");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library libAriaJava failed to load. Make sure that its directory is in your library path; See javaExamples/README.txt and the chapter on Dynamic Linking Problems in the SWIG Java documentation (http://www.swig.org) for help.\n" + e);
      System.exit(1);
    }
  }

  public static void main(String argv[]) {
    System.out.println("Starting Java Test");

    Aria.init();

    ArRobot robot = new ArRobot();
    ArSimpleConnector conn = new ArSimpleConnector(argv);
 
    if(!Aria.parseArgs())
    {
      Aria.logOptions();
      Aria.exit(1);
    }

    if (!conn.connectRobot(robot))
    {
      System.err.println("Could not connect to robot, exiting.\n");
      System.exit(1);
    }
    robot.runAsync(true);
    robot.lock();
    System.out.println("Sending command to move forward 1 meter...");
    robot.enableMotors();
    robot.move(1000);
    robot.unlock();
    System.out.println("Sleeping for 5 seconds...");
    ArUtil.sleep(5000);
    robot.lock();
    System.out.println("Sending command to rotate 90 degrees...");
    robot.setHeading(90);
    robot.unlock();
    System.out.println("Sleeping for 5 seconds...");
    ArUtil.sleep(5000);
    robot.lock();
    System.out.println("Robot coords: robot.getX()=" + robot.getX() + ", robot.getY()=" + robot.getY() + ", robot.getTh()=" + robot.getTh()); 
    ArPose p = robot.getPose();
    System.out.println("               pose.getX()=" + p.getX() +     ", pose.getY()="  + p.getY() +     ",  pose.getTh()=" + p.getTh());
    double[] xout = {0};
    double[] yout = {0};
    double[] thout = {0};
    p.getPose(xout, yout, thout);
    System.out.println("              pose.getPose(): x=" + xout[0] + ", y=" + yout[0] + ", th=" + thout[0]);
    robot.unlock();
    robot.lock();
    System.out.println("exiting.");
    robot.stopRunning(true);
    robot.unlock();
    robot.lock();
    robot.disconnect();
    robot.unlock();
    Aria.exit(0);
  }
}
