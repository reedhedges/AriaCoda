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


/** @example socketServerExample.cpp ArSocket example server program
 *
  This is an example to show how to use ArSocket. 
  a server. This is the server program. The client is socketClientExample.cpp
  
  This program opens a server on port 7777. It waits for the
  client to connect and says hello to the client. It then waits for the
  client to say hello and then exits.

  First run this server program. Then run the client program,
  socketClientExample, perhaps in a different terminal window.
  You should see the server accept the connection, send its
  greeting to the client, recieve a response from the client,
  and then the connection closes.

  This server only accepts one client connection, and then
  it exits.  In practice, most servers would loop, accepting
  many clients, and simultaneously handling curently open
  client connections.  You can use threads to do that (see
  ArASyncTask), but ArSocket is not inherently thread-safe,
  you would also need to use an ArMutex object to protect 
  ArSocket calls. 

  @see ArSocket
  @see socketClientExample.cpp

  Also see the ArNetServer to help manage a server, or the  
  ArNetworking library for a more complete networking framework.
*/


int main()
{
  // The string to send to the client. 
  const char *strToSend="Hello Client";
  // The buffer in which to recieve the hello from the client
  char buff[100];
  // The size of the string the client sent
  size_t strSize;

  // Initialize Aria.  This is especially essential on Windows,
  // because it will initialize Windows's sockets sytem.
  Aria::init();

  // The socket objects: one for accepting new client connections,
  // and another for communicating with a client after it connects.
  ArSocket serverSock, clientSock;


  // Open the server socket
  if (serverSock.open(7777, ArSocket::TCP))
    ArLog::log(ArLog::Normal, "socketServerExample: Opened the server port.");
  else
  {
    ArLog::log(ArLog::Normal, "socketServerExample: Failed to open the server port: %s.",
	   serverSock.getErrorStr().c_str());
    Aria::exit(-1);
    return(-1);
  }

  for(int clientNo = 0; Aria::getRunning(); ++clientNo)
  {

    // Wait for a client to connect to us.
    ArLog::log(ArLog::Normal, "socketServerExample: Waiting for a client to connect. Press CTRL-C to exit.");

    if (serverSock.accept(&clientSock))
      ArLog::log(ArLog::Normal, "socketServerExample: Client %d has connected.", clientNo);
    else
      ArLog::log(ArLog::Terse, "socketServerExample: Error in accepting a connection from the client: %s.",
         serverSock.getErrorStr().c_str());

    // Send the string 'Hello Client' to the client. write() should
    // return the same number of bytes that we told it to write. Otherwise,
    // its an error condition.
    if (clientSock.write(strToSend, strlen(strToSend)) == (int) strlen(strToSend))
      ArLog::log(ArLog::Normal, "socketServerExample: Said hello to the client.");
    else
    {
      ArLog::log(ArLog::Normal, "socketServerExample: Error in sending hello string to the client.");
      Aria::exit(-1);
      return(-1);
    }

    // Read data from the client. read() will block until data is
    // received. 
    strSize=clientSock.read(buff, sizeof(buff));

    // If the amount read is 0 or less, its an error condition.
    if (strSize > 0)
    {
      // Terminate the string with a NULL character.
      buff[strSize]='\0';
      ArLog::log(ArLog::Normal, "socketServerExample: Client said: %s.", buff);
    }
    else
    {
      ArLog::log(ArLog::Normal, "socketServerExample: Error in waiting/reading the hello from the client.");
      Aria::exit(-1);
      return(-1);
    }

    // Now lets close the connection to the client
    clientSock.close();
    ArLog::log(ArLog::Normal, "socketServerExample: Socket to client closed.");
    
  }

  // And lets close the server port
  serverSock.close();
    ArLog::log(ArLog::Normal, "socketServerExample: Server socket closed.");


  // Uninitialize Aria and exit the program
  Aria::exit(0);

  return(0);
}
