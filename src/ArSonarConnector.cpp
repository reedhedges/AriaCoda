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
#include "ArSonarConnector.h"
#include "ArRobot.h"
#include "ArSonarMTX.h"
#include "ariaInternal.h"
#include "ArCommands.h"
#include "ArRobotConfigPacketReader.h"

/** @warning do not delete @a parser during the lifetime of this
 ArSonarConnector, which may need to access its contents later.
 @param parser the parser with the arguments to parse
 @param robot the robot these sonars are attached to (or NULL for none)
 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)
 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly
 @param infoLogLevel The log level for information about creating
 sonars and such, this is also passed to all the sonars created as
 their infoLogLevel too
 @param turnOnPowerOutputCB If not NULL, invoke this functor when enabling sonar
(used to switch on power to sonar devices on some robots)
@param turnOffPowerOutputCB If not NULL, invoke this functor to turn off sonar
power if neccesary.
 */
AREXPORT ArSonarConnector::ArSonarConnector (
  ArArgumentParser *parser, ArRobot *robot,
  ArRobotConnector *robotConnector, bool autoParseArgs,
  ArLog::LogLevel infoLogLevel,
  ArRetFunctor1<bool, const char *> *turnOnPowerOutputCB,
  ArRetFunctor1<bool, const char *> *turnOffPowerOutputCB) :
	myParseArgsCB (this, &ArSonarConnector::parseArgs),
	myLogOptionsCB (this, &ArSonarConnector::logOptions)
{
	myParser = parser;
	myOwnParser = false;
	myRobot = robot;
  mySonarLogPacketsReceived = false;
  mySonarLogPacketsSent = false;
	myRobotConnector = robotConnector;
	myAutoParseArgs = autoParseArgs;
	myParsedArgs = false;
	myInfoLogLevel = infoLogLevel;

  myTurnOnPowerOutputCB = turnOnPowerOutputCB;
  myTurnOffPowerOutputCB = turnOffPowerOutputCB;

	myParseArgsCB.setName ("ArSonarConnector");
	Aria::addParseArgsCB (&myParseArgsCB, 60);
	myLogOptionsCB.setName ("ArSonarConnector");
	Aria::addLogOptionsCB (&myLogOptionsCB, 80);
}
AREXPORT ArSonarConnector::~ArSonarConnector (void)
{
//  Aria::remParseArgsCB(&myParseArgsCB);
//  Aria::remLogOptionsCB(&myLogOptionsCB);
}
/**
 * Parse command line arguments using the ArArgumentParser given in the ArSonarConnector constructor.
 *
 * See parseArgs(ArArgumentParser*) for details about argument parsing.
 *
  @return true if the arguments were parsed successfully false if not
 **/
AREXPORT bool ArSonarConnector::parseArgs (void)
{
	return parseArgs (myParser);
}
/**
 * Parse command line arguments held by the given ArArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

  The following arguments are accepted for sonar connections.  A program may request support for more than one sonar
  using setMaxNumSonarBoards(); if multi-sonar support is enabled in this way, then these arguments must have the sonar index
  number appended. For example, "-sonarPort" for sonar 1 would instead by "-sonarPort1", and for sonar 2 it would be
  "-sonarPort2".
  <dl>
    <dt>-sonarPort <i>port</i></dt>
    <dt>-sp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a sonar. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
      </dd>
    <dt>-doNotConnectSonar</dt>
    <dt>-dncs</dt>
  </dl>
 **/
AREXPORT bool ArSonarConnector::parseArgs (ArArgumentParser *parser)
{

	if (myParsedArgs)
		return true;
	myParsedArgs = true;
	bool typeReallySet;
	const char *type;
	char buf[1024];
	int i;
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData;
	bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
	parser->setWasReallySetOnlyTrue (true);

	for (i = 1; i <= Aria::getMaxNumSonarBoards(); i++) {
		if (i == 1)
			buf[0] = '\0';
		else
			sprintf (buf, "%d", i);
		typeReallySet = false;
		// see if the sonar is being added from the command line
		if (!parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		    "-sonarType%s", buf) ||
		    !parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		        "-st%s", buf)) {
			ArLog::log (ArLog::Normal,
			            "ArSonarConnector: Bad sonar type given for sonar number %d",
			            i);
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
		// if we didn't have an argument then just return
		if (!typeReallySet)
			continue;
		if ( (it = mySonars.find (i)) != mySonars.end()) {
			ArLog::log (ArLog::Normal, "ArSonarConnector: A sonar already exists for sonar number %d, replacing it with a new one of type %s",
			            i, type);
			sonarData = (*it).second;
			delete sonarData;
			mySonars.erase (i);
		}
		if (typeReallySet && type != NULL) {
			ArSonarMTX *sonar = NULL;
			if ( (sonar = Aria::sonarCreate (type, i, "ArSonarConnector: ")) != NULL) {
				ArLog::log (myInfoLogLevel,
				            "ArSonarConnector: Created %s as sonar %d from arguments",
				            sonar->getName(), i);
				mySonars[i] = new SonarData (i, sonar);
				sonar->setInfoLogLevel (myInfoLogLevel);
			} else {
				ArLog::log (ArLog::Normal,
				            "Unknown sonar type %s for sonar %d, choices are %s",
				            type, i, Aria::sonarGetTypes());
				parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
				return false;
			}
		}
	}

  if (parser->checkArgument("-sonarLogPacketsReceived") || 
      parser->checkArgument("-slpr")) 
    mySonarLogPacketsReceived = true;
  
  if (parser->checkArgument("-sonarLogPacketsSent") || 
      parser->checkArgument("-slps"))      
    mySonarLogPacketsSent = true;


	// go through the robot param list and add the sonars defined
	// in the parameter file.
	const ArRobotParams *params = NULL;
	if (myRobot != NULL) {
		params = myRobot->getRobotParams();
		if (params != NULL) {


			for (i = 1; i <= Aria::getMaxNumSonarBoards(); i++) {
				// if we already have a sonar for this then don't add one from
				// the param file, since it was added either explicitly by a
				// program or from the command line
				if (mySonars.find (i) != mySonars.end())
					continue;
				type = params->getSonarMTXBoardType (i);
				// if we don't have a sonar type for that number continue
				if (type == NULL || type[0] == '\0')
					continue;

				int baud = params->getSonarMTXBoardBaud(i);
				if (baud == 0)
					continue;

				ArSonarMTX *sonar = NULL;
				if ( (sonar =
				        Aria::sonarCreate (type, i, "ArSonarConnector: ")) != NULL) {
					ArLog::log (myInfoLogLevel,
					            "ArSonarConnector::parseArgs() Created %s as sonar %d from parameter file",
					            sonar->getName(), i);
					mySonars[i] = new SonarData (i, sonar);
					sonar->setInfoLogLevel (myInfoLogLevel);
				} else {
					ArLog::log (ArLog::Normal,
					            "ArSonarConnector::parseArgs() Unknown sonar type %s for sonar %d from the .p file, choices are %s",
					            type, i, Aria::sonarGetTypes());
					parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
					return false;
				}
			}
		} else {
			ArLog::log (ArLog::Normal, "ArSonarConnector::parseArgs() Have robot, but robot has NULL params, so cannot configure its sonar");
		}
	}
	// now go through and parse the args for any sonar that we have


	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		if (!parseSonarArgs (parser, sonarData)) {
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
	}
	parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
	return true;

} // end parseArgs

AREXPORT bool ArSonarConnector::parseSonarArgs (ArArgumentParser *parser,
    SonarData *sonarData)
{
	char buf[512];
	if (sonarData == NULL) {
		ArLog::log (ArLog::Terse, "ArSonarConnector::parseSonarArgs() Was given NULL sonar");
		return false;
	}
	if (sonarData->mySonar == NULL) {
		ArLog::log (ArLog::Normal,
		            "ArSonarConnector::parseSonarArgs() There is no sonar for sonar number %d but there should be",
		            sonarData->myNumber);
		return false;
	}
	ArSonarMTX *sonar = sonarData->mySonar;
	if (sonarData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", sonarData->myNumber);

#if 0
	// see if we want to connect to the sonar automatically
	if (parser->checkArgumentVar ("-connectSonar%s", buf) ||
	    parser->checkArgumentVar ("-cb%s", buf)) {
		sonarData->myConnect = true;
		sonarData->myConnectReallySet = true;
	}
#endif

	// see if we do not want to connect to the sonar automatically
	if (parser->checkArgumentVar ("-doNotConnectSonar%s", buf) ||
	    parser->checkArgumentVar ("-dncs%s", buf)) {
		sonarData->myConnect = false;
		sonarData->myConnectReallySet = true;
	}
	if (!parser->checkParameterArgumentStringVar (NULL, &sonarData->myPort,
	    "-sonarPort%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPort,
	        "-sp%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPortType,
	        "-sonarPortType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPortType,
	        "-spt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myType,
	        "-sonarType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myType,
	        "-st%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myBaud,
	        "-sonarBaud%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myBaud,
	        "-sb%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myAutoConn,
	        "-sonarAutoConn%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myAutoConn,
	        "-sac%s", buf))
		{
		return false;
	}
	// PS - command line needs to set this to true
#if 0
	if (strcasecmp(sonarData->myAutoConn, "true") == 0) {
		sonarData->myConnect = true;
		sonarData->myConnectReallySet = true;
	}
#endif
	return internalConfigureSonar (sonarData);

} // end parseSonarArgs


bool ArSonarConnector::internalConfigureSonar (
  SonarData *sonarData)
{
	if(sonarData->myConnectReallySet && ! sonarData->myConnect)
	{
		ArLog::log(ArLog::Terse, "ArSonarConnector: Warning: connection to sonar %d explicitly disabled by opion", sonarData->myNumber);
		return true;
	}
	ArSonarMTX *sonar = sonarData->mySonar;
	if (sonar == NULL) {
		ArLog::log (ArLog::Terse, "ArSonarConnector::internalConfigureSonar() No sonar for number %d",
		            sonarData->myNumber);
		return false;
	}
	// the rest handles all the connection stuff
	const ArRobotParams *params;
	char portBuf[1024];
	if (sonarData->mySonar == NULL) {
		ArLog::log (ArLog::Terse, "ArSonarConnector::internalConfigureSonar() There is no sonar, cannot connect");
		return false;
	}
	sprintf (portBuf, "%s", sonarData->mySonar->getDefaultTcpPort());
	if (myRobotConnector == NULL) {
		ArLog::log (ArLog::Terse, "ArSonarConnector::internalConfigureSonar() No ArRobotConnector is passed in so simulators and remote hosts will not work correctly");
	}
	if ( (sonarData->myPort == NULL || strlen (sonarData->myPort) == 0) &&
	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {
		ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() There is a sonar port type given ('%s') for sonar %d (%s), but no sonar port given, cannot configure sonar",
		            sonarData->myPortType, sonarData->myNumber, sonar->getName());
		return false;
	}

	ArLog::log (ArLog::Verbose, "ArSonarConnector::internalConfigureSonar() command line sonar #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							sonarData->myNumber, 
							sonarData->myType,
							sonarData->myPort,
							sonarData->myPortType,
							sonarData->myBaud,
							sonarData->myAutoConn);


	if ( (sonarData->myPort != NULL && strlen (sonarData->myPort) > 0) &&
	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {
		ArLog::log (ArLog::Verbose, "ArSonarConnector::internalConfigureSonar() Connection type and port given for sonar %d (%s), so overriding everything and using that information",
		            sonarData->myNumber, sonar->getName());
		
		if ( (sonarData->myConn = Aria::deviceConnectionCreate (
		                              sonarData->myPortType, sonarData->myPort, portBuf,
		                              "ArSonarConnector:")) == NULL) {
			return false;
		}
		sonar->setDeviceConnection (sonarData->myConn);
		return true;
	}

	if ( (sonarData->myPort != NULL && strlen (sonarData->myPort) > 0) &&
	     (sonarData->myPortType == NULL || strlen (sonarData->myPortType) == 0)) {
		if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL) {
			if (params->getSonarMTXBoardPortType (sonarData->myNumber) != NULL &&
			    params->getSonarMTXBoardPortType (sonarData->myNumber) [0] != '\0') {
				ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() There is a port given, but no port type given so using the robot parameters port type");
				if ( (sonarData->myConn = Aria::deviceConnectionCreate (
				                              params->getSonarMTXBoardPortType (sonarData->myNumber),
				                              sonarData->myPort, portBuf,
				                              "ArSonarConnector: ")) == NULL) {
					return false;
				}
			} else if (sonar->getDefaultPortType() != NULL &&
			           sonar->getDefaultPortType() [0] != '\0') {
				ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() There is a port given for sonar %d (%s), but no port type given and no robot parameters port type so using the sonar's default port type", sonarData->myNumber, sonar->getName());
				if ( (sonarData->myConn = Aria::deviceConnectionCreate (
				                              sonar->getDefaultPortType(),
				                              sonarData->myPort, portBuf,
				                              "ArSonarConnector: ")) == NULL) {
					return false;
				}
			} else {
				ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() There is a port given for sonar %d (%s), but no port type given, no robot parameters port type, and no sonar default port type, so using serial",
				            sonarData->myNumber, sonar->getName());
				if ( (sonarData->myConn = Aria::deviceConnectionCreate (
				                              "serial",
				                              sonarData->myPort, portBuf,
				                              "ArSonarConnector: ")) == NULL) {
					return false;
				}
			}
			sonar->setDeviceConnection (sonarData->myConn);
			return true;
		} else {
			ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() There is a sonar port given ('%s') for sonar %d (%s), but no sonar port type given and there are no robot params to find the information in, so assuming serial",
			            sonarData->myPort, sonarData->myNumber, sonar->getName());
			if ( (sonarData->myConn = Aria::deviceConnectionCreate (
			                              sonarData->myPortType, sonarData->myPort, portBuf,
			                              "ArSonarConnector: ")) == NULL) {
				return false;
			}
			sonar->setDeviceConnection (sonarData->myConn);
			return true;
		}
	}


	// if we get down here there was no information provided by the command line or in a sonar connector, so see if we have params... if not then fail, if so then use those
	if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL) {
		ArLog::log (ArLog::Normal, "ArSonarConnector::internalConfigureSonar() No robot params are available, and no command line information given on how to connect to the sonar %d (%s), so cannot connect", sonarData->myNumber, sonar->getName());
		return false;
	}

  int i = sonarData->myNumber;
	ArLog::log (ArLog::Verbose, "ArSonarConnector::internalConfigureSonar() .p sonar #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							i,
							params->getSonarMTXBoardType (i),
							params->getSonarMTXBoardPort (i),
							params->getSonarMTXBoardPortType (i),
							params->getSonarMTXBoardBaud(i),
							params->getSonarMTXBoardAutoConn (i));
	
  sonarData->myType = params->getSonarMTXBoardType(i);
  sonarData->myPort = params->getSonarMTXBoardPort(i);
  sonarData->myPortType = params->getSonarMTXBoardPortType(i);
  sonarData->setBaud(params->getSonarMTXBoardBaud(i));
  sonarData->setAutoConn(params->getSonarMTXBoardAutoConn(i));
  if(params->getSonarMTXBoardAutoConn(i))
  {
    sonarData->myConnect = true;
    sonarData->myConnectReallySet = true;
  }

	ArLog::log (ArLog::Verbose, "ArSonarConnector::internalConfigureSonar(): Using robot params for connecting to sonar %d (%s) (port=%s, portType=%s)", sonarData->myNumber, sonar->getName(), sonarData->myPort, sonarData->myPortType);

  sonarData->myConn = Aria::deviceConnectionCreate(sonarData->myPortType, sonarData->myPort, portBuf, "ArSonarConnector: ");

	if (sonarData->myConn == NULL)
  {
    ArLog::log(ArLog::Terse, "ArSonarConnector::internalConfigureSonar(): Error creating device connection.");
		return false;
	}

	sonar->setDeviceConnection (sonarData->myConn);

	return true;
}
AREXPORT void ArSonarConnector::logOptions (void) const
{
	ArLog::log (ArLog::Terse, "Options for ArSonarConnector:");
  ArLog::log(ArLog::Terse, "-sonarLogPacketsReceived");
  ArLog::log(ArLog::Terse, "-slpr");
  ArLog::log(ArLog::Terse, "-sonarLogPacketsSent");
  ArLog::log(ArLog::Terse, "-slps");
	ArLog::log (ArLog::Terse, "\nOptions shown are for currently set up sonars.  Activate sonars with -sonarType<N> option");
	ArLog::log (ArLog::Terse, "to see options for that sonar (e.g. \"-help -sonarType1 sonarMTX\").");
	ArLog::log (ArLog::Terse, "Valid sonar types are: %s", Aria::sonarGetTypes());
	ArLog::log (ArLog::Terse, "\nSee docs for details.");
	std::map<int, SonarData *>::const_iterator it;
	SonarData *sonarData;
	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		logSonarOptions (sonarData);
	}
}
AREXPORT void ArSonarConnector::logSonarOptions (
  SonarData *sonarData, bool header, bool metaOpts) const
{
	char buf[512];
	if (sonarData == NULL) {
		ArLog::log (ArLog::Normal,
		            "Tried to log sonar options with NULL sonar data");
		return;
	}
	if (sonarData->mySonar == NULL) {
		ArLog::log (ArLog::Normal,
		            "ArSonarConnector: There is no sonar for sonar number %d but there should be",
		            sonarData->myNumber);
		return;
	}
	ArSonarMTX *sonar = sonarData->mySonar;
	if (sonarData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", sonarData->myNumber);
	if (header) {
		ArLog::log (ArLog::Terse, "");
		ArLog::log (ArLog::Terse, "Sonar%s: (\"%s\")", buf, sonar->getName());
	}
	if (metaOpts) {
		ArLog::log (ArLog::Terse, "-sonarType%s <%s>", buf, Aria::sonarGetTypes());
		ArLog::log (ArLog::Terse, "-st%s <%s>", buf, Aria::sonarGetTypes());
		ArLog::log (ArLog::Terse, "-doNotConnectSonar%s", buf);
		ArLog::log (ArLog::Terse, "-dncs%s", buf);
	}
	ArLog::log (ArLog::Terse, "-sonarPort%s <sonarPort>", buf);
	ArLog::log (ArLog::Terse, "-sp%s <sonarPort>", buf);
	ArLog::log (ArLog::Terse, "-sonarPortType%s <%s>", buf, Aria::deviceConnectionGetTypes());
	ArLog::log (ArLog::Terse, "-spt%s <%s>", buf, Aria::deviceConnectionGetTypes());
	ArLog::log (ArLog::Terse, "-remoteSonarTcpPort%s <remoteSonarTcpPort>", buf);
	ArLog::log (ArLog::Terse, "-rstp%s <remoteSonarTcpPort>", buf);
}
/**
   Normally adding sonars is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArSonarConnector::connectSonars.()
   @internal
**/
AREXPORT bool ArSonarConnector::addSonar (
  ArSonarMTX *sonar, int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData != NULL) {
		if (sonarData->mySonar != NULL)
			ArLog::log (ArLog::Terse,
			            "ArSonarConnector::addSonar: Already have sonar for number #%d of type %s but a replacement sonar of type %s was passed in",
			            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
		else
			ArLog::log (ArLog::Terse,
			            "ArSonarConnector::addSonar: Already have sonar for number #%d but a replacement sonar of type %s was passed in",
			            sonarNumber, sonar->getName());
		delete sonarData;
		mySonars.erase (sonarNumber);
	}
	mySonars[sonarNumber] = new SonarData (sonarNumber, sonar);
	return true;
}
AREXPORT ArSonarMTX *ArSonarConnector::getSonar (int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	// if we have no sonar, we can't get it so just return
	if (sonarData == NULL)
		return NULL;
	// otherwise, return the sonar
	return sonarData->mySonar;
}
AREXPORT bool ArSonarConnector::replaceSonar (
  ArSonarMTX *sonar, int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	// if we have no sonar, we can't replace it so just return
	if (sonarData == NULL)
		return false;
	if (sonarData->mySonar != NULL)
		ArLog::log (myInfoLogLevel,
		            "ArSonarConnector::replaceSonar: Already have sonar for number #%d of type %s but a replacement sonar of type %s was passed in",
		            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
	else
		ArLog::log (ArLog::Normal,
		            "ArSonarConnector::replaceSonar: Replacing a non existant sonar number #%d with a sonar of type %s passed in",
		            sonarNumber, sonar->getName());
	sonarData->mySonar = sonar;
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArSonarConnector::connectSonars().
   @internal
**/
AREXPORT bool ArSonarConnector::setupSonar (ArSonarMTX *sonar,
    int sonarNumber)
{
	if (myRobot == NULL && myRobotConnector != NULL)
		myRobot = myRobotConnector->getRobot();
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	//const ArRobotParams *params;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData == NULL && sonar == NULL) {
		ArLog::log (ArLog::Terse, "ArSonarConnector::setupSonar: Do not have sonar #%d", sonarNumber) ;
		return false;
	}
	if (sonarData != NULL && sonar != NULL &&
	    sonarData->mySonar != sonar) {
		if (sonarData->mySonar != NULL)
			ArLog::log (ArLog::Terse, "ArSonarConnector::setupSonar: Already have sonar for number #%d (%s) but a replacement sonar (%s) was passed in, this will replace all of the command line arguments for that sonar",
			            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
		else
			ArLog::log (ArLog::Terse, "ArSonarConnector::setupSonar: Already have sonar for number #%d but a replacement sonar (%s) was passed in, this will replace all of the command line arguments for that sonar",
			            sonarNumber, sonar->getName());
		delete sonarData;
		mySonars.erase (sonarNumber);
		mySonars[sonarNumber] = new SonarData (sonarNumber, sonar);
	}
	if (sonarData == NULL && sonar != NULL) {
		sonarData = new SonarData (sonarNumber, sonar);
		mySonars[sonarNumber] = sonarData;
		if (myAutoParseArgs && !parseSonarArgs (myParser, sonarData)) {
			ArLog::log (ArLog::Terse, "ArSonarConnector: Error Auto parsing args for sonar %s (num %d)", sonarData->mySonar->getName(), sonarNumber);
			return false;
		}
	}
	// see if there is no sonar (ie if it was a sick done in the old
	// style), or if the sonar passed in doesn't match the one this
	// class created (I don't know how it'd happen, but...)... and then
	// configure it
	if ( (sonarData->mySonar == NULL || sonarData->mySonar != sonar)) {
		if (!internalConfigureSonar (sonarData))
			return false;
	}
	// setupSonar automatically adds this to the robot, since the
	// connectsonar stuff is the newer more supported way and is more
	// configurable.. it only adds it as a sonar since the legacy code
	// won't add it that way, but will add it as a range device
	if (myRobot != NULL) {
		myRobot->addSonar (sonar, sonarNumber);
		//myRobot->addRangeDevice(sonar);
	} else {
		ArLog::log (ArLog::Normal, "ArSonarConnector::setupSonar: No robot, so sonar cannot be added to robot");
	}
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArSonarConnector::connectSonars().
   @internal
**/
AREXPORT bool ArSonarConnector::connectSonar (ArSonarMTX *sonar,
    int sonarNumber,
    bool forceConnection)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	sonar->lockDevice();
	// set up the sonar regardless
	if (!setupSonar (sonar, sonarNumber)) {
		sonar->unlockDevice();  
		return false;
	}
	sonar->unlockDevice();
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData == NULL) {
		ArLog::log (ArLog::Normal, "ArSonarConnector::connectSonar: Some horrendous error in connectSonar with sonar number %d", sonarNumber);
		return false;
	}
	// see if we want to connect
	if (!forceConnection && !sonarData->myConnect)
		return true;
	else
		return sonar->blockingConnect(mySonarLogPacketsSent, mySonarLogPacketsReceived);
}


AREXPORT bool ArSonarConnector::connectSonars (
  bool continueOnFailedConnect, bool addConnectedSonarsToRobot,
  bool addAllSonarsToRobot, bool turnOnSonars,
  bool powerCycleSonarOnFailedConnect)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	ArLog::log (myInfoLogLevel,
	            "ArSonarConnector::connectSonars() Connecting sonars... myAutoParseArgs=%d myParsedArgs=%d addAllSonarsToRobot=%d", myAutoParseArgs, myParsedArgs, addAllSonarsToRobot);

	if (myAutoParseArgs && !myParsedArgs) {
		ArLog::log (myInfoLogLevel,
		            "ArSonarConnector::connectSonars() Auto parsing args for sonars");
		if (!parseArgs()) {
			return false;
		}
	}

	if (addAllSonarsToRobot) {
		ArLog::log (myInfoLogLevel,
		            "ArSonarConnector::connectSonars() addAllSonarsToRobot");
		if (myRobot != NULL) {
			for (it = mySonars.begin(); it != mySonars.end(); it++) {
				sonarData = (*it).second;
				myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
				ArLog::log (ArLog::Verbose,
				            "ArSonarConnector::connectSonars: Added %s to robot as sonar %d",
				            sonarData->mySonar->getName(), sonarData->myNumber);
			}
		} else {
			ArLog::log (ArLog::Normal, "ArSonarConnector::connectSonars: Error: Supposed to add all sonars to robot, but there is no robot");
			return false;
		}
	}

  ArLog::log(myInfoLogLevel, "ArSonarConnector::connectSonars(), finally connecting to each sonar...");
	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		if ( (sonarData == NULL) || (myRobot == NULL))
			continue;
//	if ( (sonarData->myPort == NULL || strlen (sonarData->myPort) == 0) &&
//	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {

		ArLog::log (myInfoLogLevel, "ArSonarConnector::connectSonars() sonar #%d type= %s port=%s portType=%s baud=%s autoconnect=%s, connect=%d",
		            sonarData->myNumber,
		            sonarData->myType,
		            sonarData->myPort,
		            sonarData->myPortType,
		            sonarData->myBaud,
		            sonarData->myAutoConn,
                sonarData->myConnect && sonarData->myConnectReallySet
    );
//}

		bool connected = false;

		if (sonarData->myConnectReallySet && sonarData->myConnect) {
      if (!turnOnPower(sonarData))
        ArLog::log(ArLog::Normal, "ArSonarConnector: Warning: unable to turn on sonar power. Continuing anyway...");
			ArLog::log (myInfoLogLevel,
			            "ArSonarConnector::connectSonars() Connecting %s",
			            sonarData->mySonar->getName());
			sonarData->mySonar->setRobot (myRobot);
			// to turn on packet tracing - uncomment
			//connected = sonarData->mySonar->blockingConnect(true, true);
			connected = sonarData->mySonar->blockingConnect (mySonarLogPacketsSent, mySonarLogPacketsReceived);
		

		if (connected) {
			if (!addAllSonarsToRobot && addConnectedSonarsToRobot) {
				if (myRobot != NULL) {
					myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
					//myRobot->addRangeDevice(sonarData->mySonar);
					ArLog::log (myInfoLogLevel,
					            //ArLog::log (ArLog::Verbose,
					            "ArSonarConnector::connectSonars() Added %s to robot",
					            sonarData->mySonar->getName());
				} else {
					ArLog::log (ArLog::Normal,
					            "ArSonarConnector::connectSonars() Could not add %s to robot, since there is no robot",
					            sonarData->mySonar->getName());
				}
			} else if (addAllSonarsToRobot && myRobot != NULL) {
				ArLog::log (ArLog::Normal,
//					ArLog::log (ArLog::Verbose,
				            "ArSonarConnector::connectSonars() %s already added to robot)",
				            sonarData->mySonar->getName());
			} else if (myRobot != NULL) {
				ArLog::log (ArLog::Normal,
//					ArLog::log (ArLog::Verbose,
				            "ArSonarConnector::connectSonars() Did not add %s to robot",
				            sonarData->mySonar->getName());
			}
		} else {
			if (!continueOnFailedConnect) {
				ArLog::log (ArLog::Normal,
				            "ArSonarConnector::connectSonars() Could not connect %s, stopping",
				            sonarData->mySonar->getName());
				return false;
			} else
				ArLog::log (ArLog::Normal,
				            "ArSonarConnector::connectSonars() Could not connect %s, continuing with remainder of sonars",
				            sonarData->mySonar->getName());
		}
	}
	}

	ArLog::log (myInfoLogLevel,
            "ArSonarConnector() Done connecting sonars");
	return true;
}

AREXPORT bool ArSonarConnector::turnOnPower(SonarData *sonarData)

{
  /// MPL the new way
  if (myTurnOnPowerOutputCB != NULL)
  {
    if (myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
		sonarData->myNumber) == NULL ||
					myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
		sonarData->myNumber)[0] == '\0')
    {
      ArLog::log(ArLog::Normal, 
										"ArSonarConnector::connectSonars: Sonar %d has no power output set so can't be turned on (things may still work).",
										sonarData->myNumber);
			return false;
    }
    else
    {
      if (myTurnOnPowerOutputCB->invokeR(
		  myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
			  sonarData->myNumber)))
      {
			ArLog::log(myInfoLogLevel, 
					"ArSonarConnector::connectSonars: Turned on power output %s for sonar %d",

			myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
								sonarData->myNumber),
								sonarData->myNumber);
			return true;
      }
      else
      {
			ArLog::log(ArLog::Normal, 
					"ArSonarConnector::connectSonars: Could not turn on power output %s for sonar %d (things may still work).",
			myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
									sonarData->myNumber),
									sonarData->myNumber);
			return false;
      }
    }
  }
  
  return true;
}

AREXPORT bool ArSonarConnector::connectReplaySonars(
  bool continueOnFailedConnect, bool addConnectedSonarsToRobot,
  bool addAllSonarsToRobot, bool turnOnSonars,
  bool powerCycleSonarOnFailedConnect)

{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	ArLog::log (myInfoLogLevel,
	            "ArSonarConnector::connectReplaySonars() Connecting sonars %d %d", myAutoParseArgs, myParsedArgs);
	if (myAutoParseArgs && !myParsedArgs) {
		ArLog::log (ArLog::Verbose,
		            "ArSonarConnector::connectReplaySonars() Auto parsing args for sonars");
		if (!parseArgs()) {
			return false;
		}
	}

	if (addAllSonarsToRobot) {
			ArLog::log (ArLog::Verbose,
		            "ArSonarConnector::connectReplaySonars() addAllSonarsToRobot");

		if (myRobot != NULL) {

			for (it = mySonars.begin(); it != mySonars.end(); it++) {
				sonarData = (*it).second;
				myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
				ArLog::log (ArLog::Verbose,
				            "ArSonarConnector::connectReplaySonars: Added %s to robot as sonar %d",
				            sonarData->mySonar->getName(), sonarData->myNumber);
			}
		} else {
			ArLog::log (ArLog::Normal, "ArSonarConnector::connectReplaySonars: Supposed to add all sonars to robot, but there is no robot");
			return false;
		}
	}
	for (it = mySonars.begin(); it != mySonars.end(); it++) {

		sonarData = (*it).second;

		if ((sonarData == NULL) || (myRobot == NULL))
			continue;

		ArLog::log (ArLog::Verbose, "ArSonarConnector::connectReplaySonars() sonar #%d type= %s port=%s portType=%s baud=%s autoconnect=%s ",
							sonarData->myNumber, 
							sonarData->myType,
							sonarData->myPort,
							sonarData->myPortType,
							sonarData->myBaud,
							sonarData->myAutoConn);

		sonarData->mySonar->setRobot (myRobot);

		if (!sonarData->mySonar->fakeConnect())
		  return false;
	}
  return true;
}

AREXPORT bool ArSonarConnector::disconnectSonars()
{
  
	for(std::map<int, SonarData *>::iterator it = mySonars.begin();
      it != mySonars.end();
      ++it)
  {
	  SonarData *sonarData = (*it).second;
    if(sonarData)
    {
      if(myRobot)
        myRobot->remSonar(sonarData->mySonar);
      if(sonarData->mySonar)
      {
        sonarData->mySonar->disconnect();
      }
    }
  }
  return true;
}

