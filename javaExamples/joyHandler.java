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

/* How to use ArJoyHandler in Java */

import com.mobilerobots.Aria.*;

public class joyHandler {

  static {
    try {
        System.loadLibrary("AriaJava");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library libAriaJava failed to load. Make sure that its directory is in your library path; See javaExamples/README.txt and the chapter on Dynamic Linking Problems in the SWIG Java documentation (http://www.swig.org) for help.\n" + e);
      System.exit(1);
    }
  }

  public static void main(String argv[]) {
    Aria.init();
    ArJoyHandler joyHandler = new ArJoyHandler();
    if(!joyHandler.init()) {
      System.out.println("Error initializing joy handler. (No joystick or no joystick OS device installed?)");
      Aria.exit(1);
    }
    boolean havez = joyHandler.haveZAxis();
    System.out.printf("Initialized? %s\tHave Z? %s\tNum Axes %d\tNum Buttons %d\n", 
      joyHandler.haveJoystick()?"yes":"no", havez?"yes":"no", joyHandler.getNumAxes(), joyHandler.getNumButtons());
    while(true) {
      ArJoyVec3f pos = joyHandler.getDoubles();
      ArJoyVec3i adj = joyHandler.getAdjusted();
      ArJoyVec3i unf = joyHandler.getUnfiltered();
      ArJoyVec3i speed = joyHandler.getSpeeds();
      System.out.print("("+pos.getX()+", "+pos.getY()+", "+pos.getZ()+") "+ 
        "\tAdjusted: ("+adj.getX()+", "+adj.getY()+", "+adj.getZ()+") "+
        "\tUnfiltered: ("+unf.getX()+", "+unf.getY()+", "+unf.getZ()+") "+
        "\tSpeed: ("+speed.getX()+", "+speed.getY()+", "+speed.getZ()+")\tButtons: [");
      for(int i = 0; i < joyHandler.getNumButtons(); ++i) {
        if (joyHandler.getButton(i)) {
          System.out.print(i + " ");
        }
      }
      System.out.println("]");
      ArUtil.sleep(1000);
    }
    //Aria.exit(0);
  }
}
