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

#include <string>
#include <vector>
#include <sstream>

#include "ArExport.h"
#include "ariaUtil.h"
#include "ariaInternal.h"
#include "ArLog.h"
#include "ArFunctor.h"
#include "ArPTZ.h"
#include "ArPTZConnector.h"
#include "ArConfig.h"
#include "ArRobot.h"
#include "ArRobotParams.h"

std::map<std::string, ArPTZConnector::PTZCreateFunc*> ArPTZConnector::ourPTZCreateFuncs;


AREXPORT ArPTZConnector::ArPTZConnector(ArArgumentParser* argParser, ArRobot *robot) :
  myArgParser(argParser),
  myRobot(robot),
  myParseArgsCallback(this, &ArPTZConnector::parseArgs),
  myLogOptionsCallback(this, &ArPTZConnector::logOptions)
  //myPopulateRobotParamsCB(this, &ArPTZConnector::populateRobotParams)
{
  myParseArgsCallback.setName("ArPTZConnector parse args callback");
  myLogOptionsCallback.setName("ArPTZConnector log options callback");
  Aria::addParseArgsCB(&myParseArgsCallback);
  Aria::addLogOptionsCB(&myLogOptionsCallback);
  //ArRobotParams::addPopulateParamsCB(&myPopulateRobotParamsCB);
}
  

AREXPORT ArPTZConnector::~ArPTZConnector()
{
  ///@todo not in Aria but should be: Aria::remParseArgsCB(&myParseArgsCallback);
  ///@todo not in Aria but should be: Aria::remLogOptionsCB(&myLogOptionsCallback);
  ///@todo delete objects from myConnectedPTZs
  ///@todo depopulate parameter slots from any ArRobotParams object that called our populate callback.
//  ArRobotParams::remPopulateParamsCB(&myPopulateRobotParamsCB);
}

AREXPORT bool ArPTZConnector::connect()
{
  // Copy ArRobot's default parameters:
  myParams.resize(Aria::getMaxNumPTZs());
  if(myRobot)
  {
	  if(myRobot->getRobotParams())
		 myParams = myRobot->getRobotParams()->getPTZParams();
	  else
		  ArLog::log(ArLog::Normal, "ArPTZConnector: Warning: no robot parameters, cannot set defaults for the robot type or loaded from parameter file. (To do so, connect to robot before connecting to PTZs.)");
  }
  else
  {
	  ArLog::log(ArLog::Normal, "ArPTZConnector: Warning: cannot use defaults for specific robot type or get configuration from robot parameter file, no robot connection.");
  }

  // "arguments" are from program command line. "parameters"/"params" are from
  // robot parameter file(s) or ARIA's internal defaults (ArRobotTypes.cpp)
  myConnectedPTZs.reserve(myParams.size());  // index in myConnectedPTZs corresponds to index from parameters and command line options, but the ArPTZ* may be NULL if not connected.
  size_t i = 0;
  size_t picount = 0;
  //assert(myArguments.size() >= myParams.size());
  for(std::vector<ArPTZParams>::iterator pi = myParams.begin(); 
      pi != myParams.end(); 
      ++pi, ++i, ++picount)
  {
  //printf("]] myArguments.size is %d, i is %d\n", myArguments.size(), i);
    if(i < myArguments.size())
    {
//      printf("]] merging myArguments[%d] into myParams[%d]\n", i, picount);
      pi->merge(myArguments[i]); 
    }

    if(pi->type == "" || pi->type == "none" || pi->connect == false)   // this ptz # was not specified in any parameters, or it was but with false connect flag
    {
        //puts("null type or \"none\" type or false connect flag, not creating or connecting.");
        //printf("connectflag=%d\n", pi->connect);
      //myConnectedPTZs.push_back(NULL);
      continue;
    }
    ArLog::log(ArLog::Normal, "ArPTZConnector: Connecting to PTZ #%d (type %s)...", i+1, pi->type.c_str());
    if(ourPTZCreateFuncs.find(pi->type) == ourPTZCreateFuncs.end())
    {
      ArLog::log(ArLog::Terse, "ArPTZConnector: Error: unrecognized PTZ type \"%s\" for PTZ #%d", pi->type.c_str(), i+1);
      //myConnectedPTZs.push_back(NULL);
      return false;
    }
    PTZCreateFunc *func = ourPTZCreateFuncs[pi->type];
    ArPTZ *ptz = func->invokeR(i, *pi, myArgParser, myRobot);
    if(!ptz)
    {
      ArLog::log(ArLog::Terse, "ArPTZConnector: Error connecting to PTZ #%d (type %s).", i+1, pi->type.c_str());
      ArLog::log(ArLog::Normal, "ArPTZConnector: Try specifying -ptzType (and -ptzSerialPort, -ptzRobotAuxSerialPort or -ptzAddress) program arguments, or set type and connection options in your robot's parameter file. Run with -help for all connection program options.");
      //myConnectedPTZs.push_back(NULL);
      return false;
    }
    if(pi->serialPort != "" && pi->serialPort != "none")
    {
      // memory leak? who is responsible for destroying serial connection, do we
      // need to store it and destroy it in our destructor or a disconnectAll
      // function?
      std::string logname = pi->type;
      logname += " ";
      logname += i;
      logname += " control serial connection on ";
      logname += pi->serialPort;
      ArDeviceConnection *serCon = ArDeviceConnectionCreatorHelper::createSerialConnection(pi->serialPort.c_str(), NULL, logname.c_str());
      ptz->setDeviceConnection(serCon);
    }
    else if(pi->robotAuxPort != -1)
    {
      ptz->setAuxPort(pi->robotAuxPort);
    }

    ptz->setInverted(pi->inverted);

    if(ptz->init())
      ArLog::log(ArLog::Verbose, "ArPTZConnector: Sucessfully initialized %s PTZ #%d ", ptz->getTypeName(), i+1);
    else
      ArLog::log(ArLog::Normal, "ArPTZConnector: Warning: Error initializing PTZ #%d (%s)", i+1, ptz->getTypeName());
  
	// Resize ConnectedPTZs vector so that we can place this framegrabber at its correct index, even if any previous 
	// PTZs were not stored because of errors creating them or they are not present in parameters or program options.
	// Any new elements created here are set to NULL.
	myConnectedPTZs.resize(i+1, NULL);

	// Add this PTZ to the connected list at its proper index (matching its index in parameters or program options)
    myConnectedPTZs[i] = ptz;

  }
  return true;
}

bool ArPTZConnector::parseArgs() 
{
  if(!myArgParser) return false;
  return parseArgs(myArgParser);
}

bool ArPTZConnector::parseArgs(ArArgumentParser *parser)
{
  // Store command-line arguments in myArguments. They will be merged into
  // parameters in connect().

//  puts("]]] ArPTZConnector::parseArgs");
  if(!parser) return false;
  // -1 is a special case, checks for arguments implied for first PTZ, e.g.
  // -ptzType is the same as -ptz1Type. 
  // Then proceeds normally with 0 for the first ptz (-ptz1...).
  for(int i = -1; i < (int)getMaxNumPTZs(); ++i)
  {
    //printf("parse args for %d\n", i);
    if(!parseArgsFor(parser, i))
      return false;
  }
  return true;
}

bool ArPTZConnector::parseArgsFor(ArArgumentParser *parser, int which)
{
  ArPTZParams newargs;
  const char *type = NULL;
  const char *serialPort = NULL;
  int auxPort = -1;
  const char *address = NULL;
  int tcpPort = -1;
  bool inverted = false;
  const char *username = NULL;
  const char *password = NULL;

  std::stringstream prefixconcat;
  prefixconcat << "-ptz";

  // If which is -1 then we check for just -ptz... with no number, but it is
  // same as first ptz ("-ptz1...") 
  if(which >= 0)
    prefixconcat << which+1;
  if(which < 0)
    which = 0;

  std::string prefix = prefixconcat.str();
  //printf("checking for arguments for PTZ #%lu with prefix %s (e.g. %s)\n", which, prefix.c_str(), (prefix+"Type").c_str());

  if(parser->checkParameterArgumentString( (prefix+"Type").c_str(), &type) && type != NULL)
  {
    if(ourPTZCreateFuncs.find(type) == ourPTZCreateFuncs.end())
    {
      ArLog::log(ArLog::Terse, "ArPTZConnector: Error parsing arguments: unrecognized PTZ type \"%s\" given with %sType.", type, prefix.c_str());
      return false;
    }
    newargs.type = type;
    newargs.connect = true;
    newargs.connectSet = true;
    //printf("found command line argument %sType. set type to %s and set connect and connectSet to true for ptz %d.\n", prefix.c_str(), type, which);
  }

  size_t nConOpt = 0;

  if(parser->checkParameterArgumentString( (prefix+"SerialPort").c_str(), &serialPort) && serialPort != NULL) 
  {
    ++nConOpt;
    newargs.serialPort = serialPort;
    //printf("got serial port %s\n", serialPort);
  }

  if(parser->checkParameterArgumentInteger( (prefix+"RobotAuxSerialPort").c_str(), &auxPort) && auxPort != -1)
  {
     ++nConOpt;
    newargs.robotAuxPort = auxPort;
    //printf("got aux port %d\n", auxPort);
  }

  if(parser->checkParameterArgumentString( (prefix+"Address").c_str(), &address) && address != NULL) 
  {
    ++nConOpt;
    newargs.address = address;
    //printf("got address %s\n", address);
  }

  // only one of serial port, aux port or address can be given
  if(nConOpt > 1)
  {
    ArLog::log(ArLog::Terse, "ArPTZConnector: Error: Only one of %sSerialPort, %sRobotAuxSerialPort or %sAddress may be given to select connection.",
      prefix.c_str(),
      prefix.c_str(),
      prefix.c_str()
    );
    return false;
  }

  if(parser->checkParameterArgumentInteger( (prefix+"TCPPort").c_str(), &tcpPort) && tcpPort != -1)
  {
    newargs.tcpPort = tcpPort;
    newargs.tcpPortSet = true;
    //printf("got tcp port %d\n", tcpPort);
  }

  bool wasSet = false;
  if(parser->checkArgument((prefix+"Inverted").c_str()))
  {
    newargs.inverted = true;
    newargs.invertedSet = true;
    //printf("got inverted %d\n", inverted);
  }
  if(parser->checkParameterArgumentBool( (prefix+"Inverted").c_str(), &inverted, &wasSet) && wasSet)
  {
    newargs.inverted = inverted;
    newargs.invertedSet = true;
    //printf("got inverted %d\n", inverted);
  }

  if(parser->checkParameterArgumentString( (prefix+"Username").c_str(), &username) && username != NULL) 
    newargs.username = username;

  if(parser->checkParameterArgumentString( (prefix+"Password").c_str(), &password) && password != NULL) 
    newargs.password = password;
  
  if((size_t)which >= myArguments.size())
  {
    // try not to assume we will be called with regular increasing which parameters,
    // (though we probably will be) so resize and set rather than use push_back:
    //printf("setting new, resized myArguments[%d] to these new params.\n", which);
    myArguments.resize(which+1);
  }

  // merge these rather than replacing them -- don't replace items in existing
  // myArguments[which] if not set in newargs.
  //myArguments[which] = newargs;
  myArguments[which].merge(newargs);

  return true;
}

AREXPORT void ArPTZConnector::logOptions() const
{
  ArLog::log(ArLog::Terse, "Common PTU and Camera PTZ options:\n");
  ArLog::log(ArLog::Terse, "\t-ptzType <type>\tSelect PTZ/PTU type. Required.  Available types are:");
  for(std::map<std::string, PTZCreateFunc*>::const_iterator i = ourPTZCreateFuncs.begin();
      i != ourPTZCreateFuncs.end();
      ++i)
  {
    ArLog::log(ArLog::Terse, "\t\t%s", (*i).first.c_str());
  }
  ArLog::log(ArLog::Terse, "\t\tnone");
  ArLog::log(ArLog::Terse, "\t-ptzInverted <true|false>\tIf true, reverse tilt and pan axes for cameras mounted upside down.");
  ArLog::log(ArLog::Terse, "\nOnly one of the following sets of connection parameters may be given:");
  ArLog::log(ArLog::Terse, "\nFor computer serial port connections:");
  ArLog::log(ArLog::Terse, "\t-ptzSerialPort <port>\tSerial port name.");
  ArLog::log(ArLog::Terse, "\nFor Pioneer robot auxilliary serial port connections:");
  ArLog::log(ArLog::Terse, "\t-ptzRobotAuxSerialPort <1|2|3>\tUse specified Pioneer robot auxilliary serial port.");
  ArLog::log(ArLog::Terse, "\nFor network connections:");
  ArLog::log(ArLog::Terse, "\t-ptzAddress <address>\tNetwork address or hostname for network connection.");
  ArLog::log(ArLog::Terse, "\t-ptzTcpPort <port>\tTCP port number for network connections.");
  ArLog::log(ArLog::Terse, "\t-ptzUsername <username>\tUsername, if camera requires it");
  ArLog::log(ArLog::Terse, "\t-ptzPassword <password>\tPassword, if camera requires it");
  ArLog::log(ArLog::Terse, "\nParameters for multiple cameras/units may be given like: -ptz1Type, -ptz2Type, -ptz3Type, etc.");
  ArLog::log(ArLog::Terse, "Some PTZ/PTU types may accept additional type-specific options. Refer to option documentation text specific to those types.");
}






  AREXPORT  void ArPTZConnector::registerPTZType(const std::string& typeName, ArPTZConnector::PTZCreateFunc* func)
  {
    ourPTZCreateFuncs[typeName] = func;
  }
