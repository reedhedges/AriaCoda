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


import com.mobilerobots.Aria.*;

public class socket {

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
    ArSocket s1 = new ArSocket();         // server socket. accepts clients.
    ArSocket s2 = new ArSocket();         // client
    ArSocket s1client = new ArSocket();   // used by server to communicate with client
    if (!s1.open(6789, ArSocket.Type.TCP)) {
      System.err.println("Error opening first TCP socket on port 6789.");
      Aria.exit(1);
    }
    System.out.println("First socket opened and listening on port 6789.");
    System.out.println("Connecting second socket to localhost port 6789...");
    if (!s2.connect("localhost", 6789)) {
      System.err.println("Error connecting second TCP socket to local host port 6789");
      Aria.exit(2);
    }
    if (!s1.accept(s1client)) {
      System.err.println("Error accepting client on first socket.");
      Aria.exit(3);
    }
    System.out.println("Connected.");
    System.out.println("Second socket is sending \"hello\" to first...");
    s2.write("hello");
    String text = new String("");
    text = s1client.read(512, 5000); // arguments are buffer size and timeout to wait
    /*
    while(text.length() == 0)
    {
      text = s1client.read(512, 5000); // arguments are buffer size and timeout to wait
    }
    */
    System.out.println("=> First socket recieved \""+text+"\" ("+text.length()+")");
    System.out.println("First socket is sending \"hello\" to second...");
    s1client.write("hello");
    text = s2.read(512, 5000);
    System.out.println("=> Second socket recieved \""+text+"\"");
    s2.close();
    Aria.exit(0);
  }
}
