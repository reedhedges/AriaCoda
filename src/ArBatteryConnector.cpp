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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArBatteryConnector.h"
#include "ArRobot.h"
#include "ArBatteryMTX.h"
#include "ariaInternal.h"
#include "ArCommands.h"
#include "ArRobotConfigPacketReader.h"
/** @warning do not delete @a parser during the lifetime of this
 ArBatteryConnector, which may need to access its contents later.
 @param parser the parser with the arguments to parse
 @param robot the robot these batteries are attached to (or NULL for none)
 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)
 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly
 @param infoLogLevel The log level for information about creating
 batteries and such, this is also passed to all the batteries created as
 their infoLogLevel too
 */
AREXPORT ArBatteryConnector::ArBatteryConnector (
  ArArgumentParser *parser, ArRobot *robot,
  ArRobotConnector *robotConnector, bool autoParseArgs,
  ArLog::LogLevel infoLogLevel) :
	myParseArgsCB (this, &ArBatteryConnector::parseArgs),
	myLogOptionsCB (this, &ArBatteryConnector::logOptions)
{
	myParser = parser;
	myOwnParser = false;
	myRobot = robot;
	myRobotConnector = robotConnector;
	myAutoParseArgs = autoParseArgs;
	myParsedArgs = false;
  myBatteryLogPacketsReceived = false;
  myBatteryLogPacketsSent = false;
	myInfoLogLevel = infoLogLevel;
	myParseArgsCB.setName ("ArBatteryConnector");
	Aria::addParseArgsCB (&myParseArgsCB, 60);
	myLogOptionsCB.setName ("ArBatteryConnector");
	Aria::addLogOptionsCB (&myLogOptionsCB, 80);
}
AREXPORT ArBatteryConnector::~ArBatteryConnector (void)
{
//  Aria::remLogOptionsCB(&myLogOptionsCB);
//  Aria::remParseArgsCB(&myParseArgsCB);
}
/**
 * Parse command line arguments using the ArArgumentParser given in the ArBatteryConnector constructor.
 *
 * See parseArgs(ArArgumentParser*) for details about argument parsing.
 *
  @return true if the arguments were parsed successfully false if not
 **/
AREXPORT bool ArBatteryConnector::parseArgs (void)
{
	return parseArgs (myParser);
}
/**
 * Parse command line arguments held by the given ArArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

  The following arguments are accepted for battery connections.  A program may request support for more than one battery
  using setMaxNumBatteries(); if multi-battery support is enabled in this way, then these arguments must have the battery index
  number appended. For example, "-batteryPort" for battery 1 would instead by "-batteryPort1", and for battery 2 it would be
  "-batteryPort2".
  <dl>
    <dt>-batteryPort <i>port</i></dt>
    <dt>-bp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a battery. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    </dd>
    <dt>-doNotConnectBattery</dt>
    <dt>-dncb</dt>
  </dl>
 **/
AREXPORT bool ArBatteryConnector::parseArgs (ArArgumentParser *parser)
{

	if (myParsedArgs)
		return true;
	myParsedArgs = true;
	bool typeReallySet;
	const char *type;
	char buf[1024];
	int i;
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData;
	bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
	parser->setWasReallySetOnlyTrue (true);

	for (i = 1; i <= Aria::getMaxNumBatteries(); i++) {
		if (i == 1)
			buf[0] = '\0';
		else
			sprintf (buf, "%d", i);
		typeReallySet = false;
		// see if the battery is being added from the command line
		if (!parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		    "-batteryType%s", buf) ||
		    !parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		        "-bt%s", buf)) {
			ArLog::log (ArLog::Normal,
			            "ArBatteryConnector: Bad battery type given for battery number %d",
			            i);
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);

			return false;
		}
		// if we didn't have an argument then just return
		if (!typeReallySet)
			continue;
		if ( (it = myBatteries.find (i)) != myBatteries.end()) {
			ArLog::log (ArLog::Normal, "ArBatteryConnector: A battery already exists for battery number %d, replacing it with a new one of type %s",
			            i, type);
			batteryData = (*it).second;
			delete batteryData;
			myBatteries.erase (i);
		}
		if (typeReallySet && type != NULL) {
			ArBatteryMTX *battery = NULL;
			if ( (battery = Aria::batteryCreate (type, i, "ArBatteryConnector: ")) != NULL) {
				ArLog::log (myInfoLogLevel,
				            "ArBatteryConnector: Created %s as battery %d from arguments",
				            battery->getName(), i);
				myBatteries[i] = new BatteryData (i, battery);
				battery->setInfoLogLevel (myInfoLogLevel);
			} else {
				ArLog::log (ArLog::Normal,
				            "Unknown battery type %s for battery %d, choices are %s",
				            type, i, Aria::batteryGetTypes());
				parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
				return false;
			}
		}
	}

  if (parser->checkArgument("-batteryLogPacketsReceived") || 
      parser->checkArgument("-blpr")) 
    myBatteryLogPacketsReceived = true;
  
  if (parser->checkArgument("-batteryLogPacketsSent") || 
      parser->checkArgument("-blps"))      
    myBatteryLogPacketsSent = true;

	// go through the robot param list and add the batteries defined
	// in the parameter file.
	const ArRobotParams *params = NULL;
	if (myRobot != NULL) {
		params = myRobot->getRobotParams();
		if (params != NULL) {


			for (i = 1; i <= Aria::getMaxNumBatteries(); i++) {
				// if we already have a battery for this then don't add one from
				// the param file, since it was added either explicitly by a
				// program or from the command line
				if (myBatteries.find (i) != myBatteries.end())
					continue;
				type = params->getBatteryMTXBoardType (i);

				// if we don't have a battery type for that number continue
				if (type == NULL || type[0] == '\0')
					continue;

				int baud = params->getBatteryMTXBoardBaud(i);

				if (baud == 0)
					continue;

				ArBatteryMTX *battery = NULL;
				if ( (battery =
				        Aria::batteryCreate (type, i, "ArBatteryConnector: ")) != NULL) {
					ArLog::log (myInfoLogLevel,
					            "ArBatteryConnector::parseArgs() Created %s as battery %d from parameter file",
					            battery->getName(), i);
					myBatteries[i] = new BatteryData (i, battery);
					battery->setInfoLogLevel (myInfoLogLevel);
				} else {
					ArLog::log (ArLog::Normal,
					            "ArBatteryConnector::parseArgs() Unknown battery type %s for battery %d from the .p file, choices are %s",
					            type, i, Aria::batteryGetTypes());
					parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
					return false;
				}
			}
		} else {
			ArLog::log (ArLog::Normal, "ArBatteryConnector::parseArgs() Have robot, but robot has NULL params, so cannot configure its battery");
		}
	}
	// now go through and parse the args for any battery that we have


	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
		batteryData = (*it).second;
		if (!parseBatteryArgs (parser, batteryData)) {
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
	}
	parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
	return true;

} // end parseArgs

AREXPORT bool ArBatteryConnector::parseBatteryArgs (ArArgumentParser *parser,
    BatteryData *batteryData)
{
	char buf[512];
	if (batteryData == NULL) {
		ArLog::log (ArLog::Terse, "ArBatteryConnector::parseBatteryArgs() Was given NULL battery");
		return false;
	}
	if (batteryData->myBattery == NULL) {
		ArLog::log (ArLog::Normal,
		            "ArBatteryConnector::parseBatteryArgs() There is no battery for battery number %d but there should be",
		            batteryData->myNumber);
		return false;
	}
	ArBatteryMTX *battery = batteryData->myBattery;
	if (batteryData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", batteryData->myNumber);

#if 0
	// see if we want to connect to the battery automatically
	if (parser->checkArgumentVar ("-connectBattery%s", buf) ||
	    parser->checkArgumentVar ("-cb%s", buf)) {
		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}
#endif

	// see if we do not want to connect to the battery automatically
	if (parser->checkArgumentVar ("-doNotConnectBattery%s", buf) ||
	    parser->checkArgumentVar ("-dncb%s", buf)) {
		batteryData->myConnect = false;
		batteryData->myConnectReallySet = true;
	}
	if (!parser->checkParameterArgumentStringVar (NULL, &batteryData->myPort,
	    "-batteryPort%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPort,
	        "-bp%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPortType,
	        "-batteryPortType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPortType,
	        "-bpt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myType,
	        "-batteryType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myType,
	        "-bt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myBaud,
	        "-batteryBaud%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myBaud,
	        "-bb%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myAutoConn,
	        "-batteryAutoConn%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myAutoConn,
	        "-bac%s", buf))
		{
		return false;
	}
	// PS - command line needs to set this to true
#if 0
	if (strcasecmp(batteryData->myAutoConn, "true") == 0) {
		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}
#endif
	return internalConfigureBattery (batteryData);

} // end parseBatteryArgs


bool ArBatteryConnector::internalConfigureBattery (
  BatteryData *batteryData)
{
	if(batteryData->myConnectReallySet && ! batteryData->myConnect)
	{
		ArLog::log(ArLog::Terse, "ArBatteryConnector::internalConfigure: Warning: connection to battery %d explicitly disabled by options", batteryData->myNumber);
		return true;
	}
	ArBatteryMTX *battery = batteryData->myBattery;
	if (battery == NULL) {
		ArLog::log (ArLog::Terse, "ArBatteryConnector::internalConfigureBattery() No battery for number %d",
		            batteryData->myNumber);
		return false;
	}
	// the rest handles all the connection stuff
	const ArRobotParams *params;
	if (batteryData->myBattery == NULL) {
		ArLog::log (ArLog::Terse, "ArBatteryConnector::internalConfigureBattery() There is no battery, cannot connect");
		return false;
	}
	char portBuf[8];
	strncpy(portBuf, batteryData->myBattery->getDefaultTcpPort(), 7); portBuf[8] = 0;
	if (myRobotConnector == NULL) {
		ArLog::log (ArLog::Terse, "ArBatteryConnector::internalConfigureBattery() No ArRobotConnector is passed in so simulators and remote hosts will not work correctly");
	}
	if ( (batteryData->myPort == NULL || strlen (batteryData->myPort) == 0) &&
	     (batteryData->myPortType != NULL && strlen (batteryData->myPortType) > 0)) {
		ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() There is a battery port type given ('%s') for battery %d (%s), but no battery port given, cannot configure battery",
		            batteryData->myPortType, batteryData->myNumber, battery->getName());
		return false;
	}

	ArLog::log (ArLog::Verbose, "ArBatteryConnector::internalConfigureBattery() command line battery #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							batteryData->myNumber, 
							batteryData->myType,
							batteryData->myPort,
							batteryData->myPortType,
							batteryData->myBaud,
							batteryData->myAutoConn,
							batteryData->myConnect);


	if ( (batteryData->myPort != NULL && strlen (batteryData->myPort) > 0) &&
	     (batteryData->myPortType != NULL && strlen (batteryData->myPortType) > 0)) {
		ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() Connection type and port given for battery %d (%s), so overriding everything and using that information",
		            batteryData->myNumber, battery->getName());
		
		if ( (batteryData->myConn = Aria::deviceConnectionCreate (
		                              batteryData->myPortType, batteryData->myPort, portBuf,
		                              "ArBatteryConnector:")) == NULL) {
			return false;
		}
		battery->setDeviceConnection (batteryData->myConn);
		return true;
	}
	if ( (batteryData->myPort != NULL && strlen (batteryData->myPort) > 0) &&
	     (batteryData->myPortType == NULL || strlen (batteryData->myPortType) == 0)) {
		if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL) {
			if (params->getBatteryMTXBoardPortType (batteryData->myNumber) != NULL &&
			    params->getBatteryMTXBoardPortType (batteryData->myNumber) [0] != '\0') {
				ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() There is a port given, but no port type given so using the robot parameters port type");
				if ( (batteryData->myConn = Aria::deviceConnectionCreate (
				                              params->getBatteryMTXBoardPortType (batteryData->myNumber),
				                              batteryData->myPort, portBuf,
				                              "ArBatteryConnector: ")) == NULL) {
					return false;
				}
			} else if (battery->getDefaultPortType() != NULL &&
			           battery->getDefaultPortType() [0] != '\0') {
				ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() There is a port given for battery %d (%s), but no port type given and no robot parameters port type so using the battery's default port type", batteryData->myNumber, battery->getName());
				if ( (batteryData->myConn = Aria::deviceConnectionCreate (
				                              battery->getDefaultPortType(),
				                              batteryData->myPort, portBuf,
				                              "ArBatteryConnector: ")) == NULL) {
					return false;
				}
			} else {
				ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() There is a port given for battery %d (%s), but no port type given, no robot parameters port type, and no battery default port type, so using serial",
				            batteryData->myNumber, battery->getName());
				if ( (batteryData->myConn = Aria::deviceConnectionCreate (
				                              "serial",
				                              batteryData->myPort, portBuf,
				                              "ArBatteryConnector: ")) == NULL) {
					return false;
				}
			}
			battery->setDeviceConnection (batteryData->myConn);
			return true;
		} else {
			ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() There is a battery port given ('%s') for battery %d (%s), but no battery port type given and there are no robot params to find the information in, so assuming serial",
			            batteryData->myPort, batteryData->myNumber, battery->getName());
			if ( (batteryData->myConn = Aria::deviceConnectionCreate (
			                              batteryData->myPortType, batteryData->myPort, portBuf,
			                              "ArBatteryConnector: ")) == NULL) {
				return false;
			}
			battery->setDeviceConnection (batteryData->myConn);
			return true;
		}
	}
	// if we get down here there was no information provided by the command line or in a battery connector, so see if we have params... if not then fail, if so then use those
	if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL) {
		ArLog::log (ArLog::Normal, "ArBatteryConnector::internalConfigureBattery() No robot params are available, and no command line information given on how to connect to the battery %d (%s), so cannot connect", batteryData->myNumber, battery->getName());
		return false;
	}

	ArLog::log (ArLog::Verbose, "ArBatteryConnector::internalConfigureBattery() .p battery #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							batteryData->myNumber, 
							params->getBatteryMTXBoardType (batteryData->myNumber),
							params->getBatteryMTXBoardPort (batteryData->myNumber),
							params->getBatteryMTXBoardPortType (batteryData->myNumber),
							params->getBatteryMTXBoardBaud (batteryData->myNumber),
							params->getBatteryMTXBoardAutoConn (batteryData->myNumber));

	// see if auto connect is on
	if (params->getBatteryMTXBoardAutoConn (batteryData->myNumber)) {

		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}

	ArLog::log (ArLog::Verbose, "ArBatteryConnector: Using robot params for connecting to battery %d (%s)", batteryData->myNumber, battery->getName());

	if ( (batteryData->myConn = Aria::deviceConnectionCreate (
	                              params->getBatteryMTXBoardPortType (batteryData->myNumber),
	                              params->getBatteryMTXBoardPort (batteryData->myNumber), portBuf,
	                              "ArBatteryConnector: ")) == NULL) {
		return false;
	}
	battery->setDeviceConnection (batteryData->myConn);
	return true;
}
AREXPORT void ArBatteryConnector::logOptions (void) const
{
	ArLog::log (ArLog::Terse, "Options for ArBatteryConnector:");
  ArLog::log(ArLog::Terse, "-batteryLogPacketsReceived");
  ArLog::log(ArLog::Terse, "-blpr");
  ArLog::log(ArLog::Terse, "-batteryLogPacketsSent");
  ArLog::log(ArLog::Terse, "-blps");
	ArLog::log (ArLog::Terse, "\nOptions shown are for currently set up batteries.  Activate batteries with -batteryType<N> option");
	ArLog::log (ArLog::Terse, "to see options for that battery (e.g. \"-help -batteryType1 batteryMTX\").");
	ArLog::log (ArLog::Terse, "Valid battery types are: %s", Aria::batteryGetTypes());
	ArLog::log (ArLog::Terse, "\nSee docs for details.");
	std::map<int, BatteryData *>::const_iterator it;
	BatteryData *batteryData;
	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
		batteryData = (*it).second;
		logBatteryOptions (batteryData);
	}
}
AREXPORT void ArBatteryConnector::logBatteryOptions (
  BatteryData *batteryData, bool header, bool metaOpts) const
{
	char buf[512];
	if (batteryData == NULL) {
		ArLog::log (ArLog::Normal,
		            "Tried to log battery options with NULL battery data");
		return;
	}
	if (batteryData->myBattery == NULL) {
		ArLog::log (ArLog::Normal,
		            "ArBatteryConnector: There is no battery for battery number %d but there should be",
		            batteryData->myNumber);
		return;
	}
	ArBatteryMTX *battery = batteryData->myBattery;
	if (batteryData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", batteryData->myNumber);
	if (header) {
		ArLog::log (ArLog::Terse, "");
		ArLog::log (ArLog::Terse, "Battery%s: (\"%s\")", buf, battery->getName());
	}
	if (metaOpts) {
		ArLog::log (ArLog::Terse, "-batteryType%s <%s>", buf, Aria::batteryGetTypes());
		ArLog::log (ArLog::Terse, "-bt%s <%s>", buf, Aria::batteryGetTypes());
		ArLog::log (ArLog::Terse, "-doNotConnectBattery%s", buf);
		ArLog::log (ArLog::Terse, "-dncb%s", buf);
	}
	ArLog::log (ArLog::Terse, "-batteryPort%s <batteryPort>", buf);
	ArLog::log (ArLog::Terse, "-bp%s <batteryPort>", buf);
	ArLog::log (ArLog::Terse, "-batteryPortType%s <%s>", buf, Aria::deviceConnectionGetTypes());
	ArLog::log (ArLog::Terse, "-bpt%s <%s>", buf, Aria::deviceConnectionGetTypes());
	ArLog::log (ArLog::Terse, "-remoteBatteryTcpPort%s <remoteBatteryTcpPort>", buf);
	ArLog::log (ArLog::Terse, "-rbtp%s <remoteBatteryTcpPort>", buf);
}
/**
   Normally adding batteries is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArBatteryConnector::connectBatteries.()
   @internal
**/
AREXPORT bool ArBatteryConnector::addBattery (
  ArBatteryMTX *battery, int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData != NULL) {
		if (batteryData->myBattery != NULL)
			ArLog::log (ArLog::Terse,
			            "ArBatteryConnector::addBattery: Already have battery for number #%d of type %s but a replacement battery of type %s was passed in",
			            batteryNumber, batteryData->myBattery->getName(), battery->getName());
		else
			ArLog::log (ArLog::Terse,
			            "ArBatteryConnector::addBattery: Already have battery for number #%d but a replacement battery of type %s was passed in",
			            batteryNumber, battery->getName());
		delete batteryData;
		myBatteries.erase (batteryNumber);
	}
	myBatteries[batteryNumber] = new BatteryData (batteryNumber, battery);
	return true;
}
AREXPORT ArBatteryMTX *ArBatteryConnector::getBattery (int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	// if we have no battery, we can't get it so just return
	if (batteryData == NULL)
		return NULL;
	// otherwise, return the battery
	return batteryData->myBattery;
}
AREXPORT bool ArBatteryConnector::replaceBattery (
  ArBatteryMTX *battery, int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	// if we have no battery, we can't replace it so just return
	if (batteryData == NULL)
		return false;
	if (batteryData->myBattery != NULL)
		ArLog::log (myInfoLogLevel,
		            "ArBatteryConnector::replaceBattery: Already have battery for number #%d of type %s but a replacement battery of type %s was passed in",
		            batteryNumber, batteryData->myBattery->getName(), battery->getName());
	else
		ArLog::log (ArLog::Normal,
		            "ArBatteryConnector::replaceBattery: Replacing a non existant battery number #%d with a battery of type %s passed in",
		            batteryNumber, battery->getName());
	batteryData->myBattery = battery;
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArBatteryConnector::connectBatteries().
   @internal
**/
AREXPORT bool ArBatteryConnector::setupBattery (ArBatteryMTX *battery,
    int batteryNumber)
{
	if (myRobot == NULL && myRobotConnector != NULL)
		myRobot = myRobotConnector->getRobot();
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	//const ArRobotParams *params;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData == NULL && battery == NULL) {
		ArLog::log (ArLog::Terse, "ArBatteryConnector::setupBattery: Do not have battery #%d", batteryNumber) ;
		return false;
	}
	if (batteryData != NULL && battery != NULL &&
	    batteryData->myBattery != battery) {
		if (batteryData->myBattery != NULL)
			ArLog::log (ArLog::Terse, "ArBatteryConnector::setupBattery: Already have battery for number #%d (%s) but a replacement battery (%s) was passed in, this will replace all of the command line arguments for that battery",
			            batteryNumber, batteryData->myBattery->getName(), battery->getName());
		else
			ArLog::log (ArLog::Terse, "ArBatteryConnector::setupBattery: Already have battery for number #%d but a replacement battery (%s) was passed in, this will replace all of the command line arguments for that battery",
			            batteryNumber, battery->getName());
		delete batteryData;
		myBatteries.erase (batteryNumber);
		myBatteries[batteryNumber] = new BatteryData (batteryNumber, battery);
	}
	if (batteryData == NULL && battery != NULL) {
		batteryData = new BatteryData (batteryNumber, battery);
		myBatteries[batteryNumber] = batteryData;
		if (myAutoParseArgs && !parseBatteryArgs (myParser, batteryData)) {
			ArLog::log (ArLog::Verbose, "ArBatteryConnector: Auto parsing args for battery %s (num %d)", batteryData->myBattery->getName(), batteryNumber);
			return false;
		}
	}
	// see if there is no battery (ie if it was a sick done in the old
	// style), or if the battery passed in doesn't match the one this
	// class created (I don't know how it'd happen, but...)... and then
	// configure it
	if ( (batteryData->myBattery == NULL || batteryData->myBattery != battery)) {
		if (!internalConfigureBattery (batteryData))
			return false;
	}
	// setupBattery automatically adds this to the robot, since the
	// connectbattery stuff is the newer more supported way and is more
	// configurable.. it only adds it as a battery since the legacy code
	// won't add it that way, but will add it as a range device
	if (myRobot != NULL) {
		myRobot->addBattery (battery, batteryNumber);
		//myRobot->addRangeDevice(battery);
	} else {
		ArLog::log (ArLog::Normal, "ArBatteryConnector::setupBattery: No robot, so battery cannot be added to robot");
	}
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArBatteryConnector::connectBatteries().
   @internal
**/
AREXPORT bool ArBatteryConnector::connectBattery (ArBatteryMTX *battery,
    int batteryNumber,
    bool forceConnection)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	battery->lockDevice();
	// set up the battery regardless
	if (!setupBattery (battery, batteryNumber)) {
		battery->unlockDevice();
		return false;
	}
	battery->unlockDevice();
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData == NULL) {
		ArLog::log (ArLog::Normal, "ArBatteryConnector::connectBattery: Some horrendous error in connectBattery with battery number %d", batteryNumber);
		return false;
	}
	// see if we want to connect
	if (!forceConnection && !batteryData->myConnect)
		return true;
	else
		return battery->blockingConnect(myBatteryLogPacketsSent, myBatteryLogPacketsReceived);
}

AREXPORT bool ArBatteryConnector::connectBatteries (
  bool continueOnFailedConnect, bool addConnectedBatteriesToRobot,
  bool addAllBatteriesToRobot, bool turnOnBatteries,
  bool powerCycleBatteryOnFailedConnect)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	ArLog::log (myInfoLogLevel,
	            "ArBatteryConnector::connectBatteries() Connecting batteries");
	if (myAutoParseArgs && !myParsedArgs) {
		ArLog::log (ArLog::Verbose,
		            "ArBatteryConnector::connectBatteries() Auto parsing args for batteries");
		if (!parseArgs()) {
			return false;
		}
	}
	if (addAllBatteriesToRobot) {

			ArLog::log (ArLog::Verbose,
		            "ArBatteryConnector::connectBatteries() addAllBatteriesToRobot");

		if (myRobot != NULL) {

			for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
				batteryData = (*it).second;
				myRobot->addBattery (batteryData->myBattery, batteryData->myNumber);
				ArLog::log (ArLog::Verbose,
				            "ArBatteryConnector::connectBatteries: Added %s to robot as battery %d",
				            batteryData->myBattery->getName(), batteryData->myNumber);
			}
		} else {
			ArLog::log (ArLog::Normal, "ArBatteryConnector::connectBatteries: Supposed to add all batteries to robot, but there is no robot");
			return false;
		}
	}
	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {

		batteryData = (*it).second;

		if (batteryData == NULL)
			continue;

		if (batteryData->myConnectReallySet && batteryData->myConnect) {
			ArLog::log (myInfoLogLevel,
			            "ArBatteryConnector::connectBatteries: Connecting %s",
			            batteryData->myBattery->getName());
			batteryData->myBattery->setRobot (myRobot);
			bool connected = false;
			connected = batteryData->myBattery->blockingConnect(myBatteryLogPacketsSent, myBatteryLogPacketsReceived);
			if (connected) {
				if (!addAllBatteriesToRobot && addConnectedBatteriesToRobot) {
					if (myRobot != NULL) {
						myRobot->addBattery (batteryData->myBattery, batteryData->myNumber);
						//myRobot->addRangeDevice(batteryData->myBattery);
						ArLog::log (ArLog::Verbose,
						            "ArBatteryConnector::connectBatteries: Added %s to robot",
						            batteryData->myBattery->getName());
					} else {
						ArLog::log (ArLog::Normal,
						            "ArBatteryConnector::connectBatteries: Could not add %s to robot, since there is no robot",
						            batteryData->myBattery->getName());
					}
				} else if (addAllBatteriesToRobot && myRobot != NULL) {
					ArLog::log (ArLog::Verbose,
					            "ArBatteryConnector::connectBatteries: %s already added to robot)",
					            batteryData->myBattery->getName());
				} else if (myRobot != NULL) {
					ArLog::log (ArLog::Verbose,
					            "ArBatteryConnector::connectBatteries: Did not add %s to robot",
					            batteryData->myBattery->getName());
				}
			} else {
				if (!continueOnFailedConnect) {
					ArLog::log (ArLog::Normal,
					            "ArBatteryConnector::connectBatteries: Could not connect %s, stopping",
					            batteryData->myBattery->getName());
					return false;
				} else
					ArLog::log (ArLog::Normal,
					            "ArBatteryConnector::connectBatteries: Could not connect %s, continuing with remainder of batteries",
					            batteryData->myBattery->getName());
			}
		}
	}
	ArLog::log (myInfoLogLevel,
	            "ArBatteryConnector: Done connecting batteries");
	return true;
}

AREXPORT bool ArBatteryConnector::disconnectBatteries()
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	ArLog::log (myInfoLogLevel, "ArBatteryConnector: Disconnecting from batteries");
  for (it = myBatteries.begin(); it != myBatteries.end(); it++) 
  {
    batteryData = (*it).second;
    if(batteryData)
    {
      if(myRobot)
        myRobot->remBattery(batteryData->myBattery);
      if(batteryData->myBattery)
        batteryData->myBattery->disconnect();
    }
  }

  return true;
}

