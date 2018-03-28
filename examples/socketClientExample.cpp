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
#include "Aria.h"


/** @example socketClientExample.cpp ArSocket example client program
 *
 *
  This is an example to show how to use ArSocket to create a client and 
  a server. This  is the client program.  The server is
  socketServerExample.cpp.  
  
  This connects to a server on the host 'localhost' on port 7777. It waits for the server
  to say hello, then it says hello in reponse. It then closes the socket and
  exits.

  First run the server program socketServerExample. Then run
  this client program on the same computer, perhaps in a different
  terminal window. You should see the server accept the connection,
  send its greeting to this client program, and see this client
  respond with its greeting text to the server.

  @see ArSocket
  @see socketServerExample.cpp

  Also see the ArNetworking library for a more complete
  networking framework.
*/


int main()
{
  // The string to send to the server. 
  const char *strToSend="Hello Server";
  // The buffer in which to recieve the hello from the server
  char buff[100];
  // The size of the string the server sent
  size_t strSize;

  // The socket object
  ArSocket sock;

  // Initialize Aria.  It is especially important to do
  // this on Windows, because it will initialize Window's
  // sockets system. 
  Aria::init();

  // Connect to the server
  ArLog::log(ArLog::Normal, "socketClientExample: Connecting to localhost TCP port 7777...");
  if (sock.connect("localhost", 7777, ArSocket::TCP))
    ArLog::log(ArLog::Normal, "socketClientExample: Connected to server at localhost TCP port 7777.");
  else
  {
    ArLog::log(ArLog::Terse, "socketClientExample: Error connecting to server at localhost TCP port 7777: %s", sock.getErrorStr().c_str());
    return(-1);
  }

  // Read data from the socket. read() will block until
  // data is received. 
  strSize=sock.read(buff, sizeof(buff));

  // If the amount read is 0 or less, its an error condition.
  if (strSize > 0)
  {
    buff[strSize]='\0'; // Terminate the string with a NULL character:
    ArLog::log(ArLog::Normal, "socketClientExample: Server said: \"%s\"", buff);
  }
  else
  {
    ArLog::log(ArLog::Terse, "socketClientExample: Error in waiting/reading from the server.");
    Aria::exit(-1);
    return(-1);
  }

  // Send the string 'Hello Server' to the server. write() should
  // return the same number of bytes that we told it to write. Otherwise,
  // its an error condition.
  if (sock.write(strToSend, strlen(strToSend)) == strlen(strToSend))
    ArLog::log(ArLog::Normal, "socketClientExample: Said hello to the server.");
  else
  {
    ArLog::log(ArLog::Terse, "socketClientExample: Error sending hello string to the server.");
    Aria::exit(-1);
    return(-1);
  }

  // Now close the connection to the server
  sock.close();
  ArLog::log(ArLog::Normal, "socketClientExample: Socket to server closed.");

  // Uninitialize Aria and exit
  Aria::exit(0);
  return(0);
}
