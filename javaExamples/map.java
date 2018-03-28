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

public class map {

  static {
    try {
        System.loadLibrary("AriaJava");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library libAriaJava failed to load. Make sure that its directory is in your library path; See javaExamples/README.txt and the chapter on Dynamic Linking Problems in the SWIG Java documentation (http://www.swig.org) for help.\n" + e);
      System.exit(1);
    }
  }

  public static void main(String argv[]) {
    System.out.println("Starting Java Map Test");

    Aria.init();

    //System.out.println("Will be able to load maps from the c++ maps directory:
    //../maps.");
    //ArMap map = new ArMap("../maps/");
    ArMap map = new ArMap();
    System.out.println("loading map: ../maps/columbia.map");
    if(!map.readFile("../maps/columbia.map"))
    {
        System.out.println("Error loading map.");
    }
    else
    {
        //System.out.println("writing map: out.map in ../maps directory");
        System.out.println("writing map: out.map");
        if(!map.writeFile("out.map"))
        {
            System.out.println("error writing map");
        }
    }

    Aria.exit(0);
  }
}
