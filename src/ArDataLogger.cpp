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

#include "ariaOSDef.h"
#include "ArExport.h"
#include "ArRobot.h"
#include "ArConfig.h"
#include "ArDataLogger.h"
#include "ArRobotBatteryPacketReader.h"
#include "ArCommands.h"
#include <vector>
#include <map>
#include <string>
#include <algorithm>

/**
   @param robot the robot to log information from
   
   @param fileName if NULL then the file name is detered by the
   config, if not NULL then the file name data is put into, if the
   filename is used then the data log file is opened in append mode
   not in write mode, so it'll append to whats there (if you want to
   separate this by runs you need to do it on your own by removing or
   moving the file)

 **/
AREXPORT ArDataLogger::ArDataLogger(ArRobot *robot, const char *fileName) :
  myAddStringFunctor(this, &ArDataLogger::addString),
  myWriteCommentFunctor(this, &ArDataLogger::writeComment),
  myConnectCB(this, &ArDataLogger::connectCallback),
  myProcessFileCB(this, &ArDataLogger::processFile),
  myUserTaskCB(this, &ArDataLogger::userTask),
  myCommandMonitorCB(this, &ArDataLogger::robotCommandMonitor),
  myClearLogFunctor(this, &ArDataLogger::clearLog),
  myStartLogFunctor(this, &ArDataLogger::startLogging),
  myStopLogFunctor(this, &ArDataLogger::stopLogging),
  mySaveCopyWithArgsFunctor(this, &ArDataLogger::saveCopyArgs),
  mySaveCopyFunctor(this, &ArDataLogger::saveCopy),
  myConfigLogFormat("TSV"), myLogFormat(TSV), myLogSep('\t')
{
  myMutex.setLogName("ArDataLogger::myMutex");
  myRobot = robot;
  if (fileName == NULL || fileName[0] == '\0')
    myPermanentFileName = "";
  else
    myPermanentFileName = fileName;
  myRobot->addUserTask("DataLogger", 50, &myUserTaskCB);
  myRobot->requestIOPackets();
  myConfig = NULL;
  myAddToConfigAtConnect = false;
  myAddedToConfig = false;
  myConfigLogging = false;
  myConfigLogInterval = 0;
  myConfigFileName[0] = '\0';
  myOpenedFileName[0] = '\0';
  myAnalogCount = 0;
  myAnalogEnabled = NULL;
  myAnalogVoltageCount = 0;
  myAnalogVoltageEnabled = NULL;
  myDigInCount = 0;
  myDigInEnabled = NULL;
  myDigOutCount = 0;
  myDigOutEnabled = NULL;
  myStringsCount = 0;
  //myStringsEnabled = NULL;

  myLogVoltage = false;
  myLogStateOfCharge = false;
  myLogLeftVel = false;
  myLogRightVel = false;
  myLogTransVel = false;
  myLogRotVel = false;
  myLogLatVel = false;
  myLogLeftStalled = false;
  myLogRightStalled = false;
  myLogStallBits = false;
  myLogFlags = false;
  myLogFaultFlags = false;
  myLogPose = false;
  myLogEncoderPose = false;
  myLogCorrectedEncoderPose = false;
  myLogEncoders = false;
  myLogChargeState = false;
  myLogBatteryInfo = false;
  myLogMovementSent = false;

  myFile = NULL;

  myMovementCommands[ArCommands::VEL] = Cmd(ArCommands::VEL, "Vel");
  myMovementCommands[ArCommands::RVEL] = Cmd(ArCommands::RVEL, "RotVel");
  myMovementCommands[ArCommands::LATVEL] = Cmd(ArCommands::LATVEL, "LatVel");
  myMovementCommands[ArCommands::SETA] = Cmd(ArCommands::SETA, "SetAcc");
  myMovementCommands[ArCommands::SETRA] = Cmd(ArCommands::SETRA, "SetRotAcc");
  // TODO need decelerations too
  myMovementCommands[ArCommands::LATACCEL] = Cmd(ArCommands::LATACCEL, "SetLatAcc");
  myMovementCommands[ArCommands::VEL2] = Cmd(ArCommands::VEL2, "VEL2");
  myMovementCommands[ArCommands::HEAD] = Cmd(ArCommands::HEAD, "HEAD");
  myMovementCommands[ArCommands::DHEAD] = Cmd(ArCommands::DHEAD, "DHEAD");
  myMovementCommands[ArCommands::MOVE] = Cmd(ArCommands::MOVE, "MOVE");
  myMovementCommands[ArCommands::SETV] = Cmd(ArCommands::SETV, "SetV");
  myMovementCommands[ArCommands::SETRV] = Cmd(ArCommands::SETRV, "SetRV");
  myMovementCommands[ArCommands::SETLATV] = Cmd(ArCommands::SETLATV, "SetLV");
}

AREXPORT ArDataLogger::~ArDataLogger(void)
{
  // assumes we are the only command monitor/packet sent callback...
  if(myRobot && myRobot->getPacketSender())
    myRobot->getPacketSender()->setCommandMonitor(NULL);
    //myRobot->getPacketSender()->setPacketSentCallback(NULL);
}

AREXPORT void ArDataLogger::addToConfig(ArConfig *config)
{
  if (config == NULL || myAddedToConfig)
    return;
  myConfig = config;
  if (!myRobot->isConnected())
  {
    myAddToConfigAtConnect = true;
    myRobot->addConnectCB(&myConnectCB);
    return;
  }
  else
  {
    connectCallback();
  }

  myAddedToConfig = true;
  ArLog::log(ArLog::Verbose, "ArDataLogger::addToConfig");
  std::string section;
  char name[512];
  char desc[512];
  section = "Data logging";
  // add everything to the config
  myConfig->addParam(
	  ArConfigArg("DataLog", &myConfigLogging, "True to log data, false not to"),
	  section.c_str(), ArPriority::NORMAL);

  myConfig->addParam(
	  ArConfigArg("DataLogInterval", &myConfigLogInterval, "Seconds between logs, or 0 for every ArRobot cycle.", 0),
	  section.c_str(), ArPriority::NORMAL);

  if (myPermanentFileName.size() == 0)
    myConfig->addParam(
	    ArConfigArg("DataLogFileName", myConfigFileName, 
			"File to log data into", sizeof(myConfigFileName)),
	    section.c_str(), ArPriority::NORMAL);

  ArConfigArg fmtArg("DataLogFormat", &myConfigLogFormat,
      "If TSV or CSV, use simple tab or comma-separated-value format (best for import into "
      "plotting and analysis tools). If Fixed, format data in fixed size (but "
      "also tab-separated) text columns (best for viewing log file manually). "
      "You may want to clear and restart the log after changing this.");
  fmtArg.setDisplayHint("Choices:TSV;;CSV;;Fixed");
  myConfig->addParam(fmtArg, section.c_str(), ArPriority::DETAILED);

  for (int i = 0; i < myStringsCount; ++i)
  {
    if(!myStringsAddedToConfig[i])
    {
      snprintf(name, sizeof(name), "DataLog%s", myStrings[i]->getName());
      snprintf(desc, sizeof(desc), "Logs %s", myStrings[i]->getName());
      myConfig->addParam(
        ArConfigArg(name, myStringsEnabled[i], desc),
        section.c_str(), ArPriority::DETAILED);
    }
    myStringsAddedToConfig[i] = true;
  }

  myConfig->addParam(
	  ArConfigArg("DataLogBatteryVoltage", &myLogVoltage, "True to log battery voltage"),
	  section.c_str(), ArPriority::DETAILED);
  if (myRobot->haveStateOfCharge())
    myConfig->addParam(
	    ArConfigArg("DataLogStateOfCharge", &myLogStateOfCharge, "True to log state of charge"),
	    section.c_str(), ArPriority::DETAILED);
  
  myConfig->addParam(
	  ArConfigArg("DataLogChargeState", &myLogChargeState, 
		      "True to log charge state"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogBatteryInfo", &myLogBatteryInfo, 
		      "True to log battery info (if available)"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogPose", &myLogPose, "True to log robot's pose"),
	  section.c_str(), ArPriority::NORMAL);
  myConfig->addParam(
	  ArConfigArg("DataLogEncoderPose", &myLogEncoderPose, "True to log robot's raw encoder pose"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogCorrectedEncoderPose", &myLogCorrectedEncoderPose, "True to log robot's corrected (by gyro, etc) encoder pose"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogEncoders", &myLogEncoders, "True to log the raw encoder readings"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogLeftVel", &myLogLeftVel, "True to log left wheel velocity"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogRightVel", &myLogRightVel, "True to log right wheel velocity"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogTransVel", &myLogTransVel, "True to log translational velocity"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogRotVel", &myLogRotVel, "True to log rotational velocity"),
	  section.c_str(), ArPriority::DETAILED);
  if (myRobot->hasLatVel())
    myConfig->addParam(
	    ArConfigArg("DataLogLatVel", &myLogRotVel, "True to log lateral velocity"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogLeftStalled", &myLogLeftStalled, "True to log whether the left wheel is stalled"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogRightStalled", &myLogRightStalled, "True to log whether the right wheel is stalled"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogStallBits", &myLogStallBits, "True to log all the stall bits"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogFlags", &myLogFlags, "True to log all the flags"),
	  section.c_str(), ArPriority::DETAILED);
  myConfig->addParam(
	  ArConfigArg("DataLogFaultFlags", &myLogFaultFlags, "True to log all the fault flags"),
	  section.c_str(), ArPriority::DETAILED);

  for (int i = 0; i < myAnalogCount; ++i)
  {
    snprintf(name, sizeof(name), "DataLogAnalog%d", i);
    snprintf(desc, sizeof(desc), 
	     "Logs the value of analog %d as a 10 bit (0-1024) value",
	     i);
    myConfig->addParam(
	    ArConfigArg(name, &myAnalogEnabled[i], desc),
	    section.c_str(), ArPriority::DETAILED);
  }
  for (int i = 0; i < myAnalogVoltageCount; ++i)
  {
    snprintf(name, sizeof(name), "DataLogAnalogVoltage%d", i);
    snprintf(desc, sizeof(desc), 
	     "Logs the value of analog %d as voltage from 0 to 5",
	     i);
    myConfig->addParam(
	    ArConfigArg(name, &myAnalogVoltageEnabled[i], desc),
	    section.c_str(), ArPriority::DETAILED);
  }
  for (int i = 0; i < myDigInCount; ++i)
  {
    snprintf(name, sizeof(name), "DataLogDigIn%d", i);
    snprintf(desc, sizeof(desc), "Logs digital in %d", i);
    myConfig->addParam(
	    ArConfigArg(name, &myDigInEnabled[i], desc),
	    section.c_str(), ArPriority::DETAILED);
  }
  for (int i = 0; i < myDigOutCount; ++i)
  {
    snprintf(name, sizeof(name), "DataLogDigOut%d", i);
    snprintf(desc, sizeof(desc), "Logs digital out %d", i);
    myConfig->addParam(
	    ArConfigArg(name, &myDigOutEnabled[i], desc),
	    section.c_str(), ArPriority::DETAILED);
  }
  myConfig->addParam(
	  ArConfigArg("DataLogMovementCommandsSent", &myLogMovementSent, "True to log movement-related commands sent to the robot in the last ArRobot cycle"),
	  section.c_str(), ArPriority::DETAILED);
  myProcessFileCB.setName("ArDataLogger");
  myConfig->addProcessFileWithErrorCB(&myProcessFileCB, 100);
}

void ArDataLogger::connectCallback(void)
{
  ArLog::log(ArLog::Verbose, "ArDataLogger::connectCallback");
  // out with the old memory
  if (myAnalogEnabled != NULL)
  {
    delete myAnalogEnabled;
    myAnalogEnabled = NULL;
  }
  if (myAnalogVoltageEnabled != NULL)
  {
    delete myAnalogVoltageEnabled;
    myAnalogVoltageEnabled = NULL;
  }
  if (myDigInEnabled != NULL)
  {
    delete myDigInEnabled;
    myDigInEnabled = NULL;
  }
  if (myDigOutEnabled != NULL)
  {
    delete myDigOutEnabled;
    myDigOutEnabled = NULL;
  }
  /*
  if (myStringsEnabled != NULL)
  {
    delete myStringsEnabled;
    myStringsEnabled = NULL;
  }
  */
  // see how much memory we need
  myAnalogCount = myRobot->getIOAnalogSize();
  myAnalogVoltageCount = myRobot->getIOAnalogSize();
  myDigInCount = myRobot->getIODigInSize();
  myDigOutCount = myRobot->getIODigOutSize();
  //myStringsCount = myStrings.size();
  // make and initialize the new memory
  if (myAnalogCount > 0)
  {
    myAnalogEnabled = new bool[myAnalogCount];
    for (int i = 0; i < myAnalogCount; ++i)
    {
      myAnalogEnabled[i] = false;
    }
  }
  if (myAnalogVoltageCount > 0)
  {
    myAnalogVoltageEnabled = new bool[myAnalogVoltageCount];
    for (int i = 0; i < myAnalogVoltageCount; ++i)
      myAnalogVoltageEnabled[i] = false;
  }
  if (myDigInCount > 0)
  {
    myDigInEnabled = new bool[myDigInCount];
    for (int i = 0; i < myDigInCount; ++i)
      myDigInEnabled[i] = false;
  }
  if (myDigOutCount > 0)
  {
    myDigOutEnabled = new bool[myDigOutCount];
    for (int i = 0; i < myDigOutCount; ++i)
      myDigOutEnabled[i] = false;
  }

  
  // If called from robot connect callback and not
  // from addToConfig, call addToConfig. 
  // (prevent infinite recursive calls)
  if (myAddToConfigAtConnect && !myAddedToConfig)
  {
    myAddToConfigAtConnect = false;
    addToConfig(myConfig);
  }


}

bool ArDataLogger::processFile(char *errorBuffer, 
					size_t errorBufferLen)
{
  myMutex.lock();


  if(myConfigLogFormat == "TSV")
  {
    if(myLogFormat != TSV) ArLog::log(ArLog::Normal, "ArDataLogger: Changing format to TSV");
    myLogFormat = TSV;
    myLogSep = '\t';
  } 
  else if(myConfigLogFormat == "CSV")
  {
    if(myLogFormat != CSV) ArLog::log(ArLog::Normal, "ArDataLogger: Changing format to CSV");
    myLogFormat = CSV;
    myLogSep = ',';
  }
  else if(myConfigLogFormat == "Fixed")
  {
    if(myLogFormat != Fixed) ArLog::log(ArLog::Normal, "ArDataLogger: Changing format to Fixed columns");
    myLogFormat = Fixed;
    myLogSep = '\t';
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArDataLogger: Error: Unrecognized log format type \"%s\", expected TSV, CSV or Fixed. Not changing. (Current format is %s)", myConfigLogFormat.c_str());
  }

  // if our file name is different and we're not using a permanent
  // file name or if we're disabled close the old one
  if ((strcmp(myOpenedFileName, myConfigFileName) != 0 && myFile != NULL && 
       myPermanentFileName.size() == 0) ||
       (myFile != NULL && !myConfigLogging))
  {
    ArLog::log(ArLog::Normal, "ArDataLogger: Closed data log file '%s'", myOpenedFileName);
    fclose(myFile);
    myFile = NULL;
  }
  // try to open the file
  if (myConfigLogging && myFile == NULL)
  {
    if (myPermanentFileName.size() == 0  && strlen(myConfigFileName) == 0)
    {
      ArLog::log(ArLog::Verbose, "ArDataLogger: no log file to open");
      myMutex.unlock();
      return true;
    }
    std::string fileName;
    if (myPermanentFileName.size() > 0)
    {
      if ((myFile = ArUtil::fopen(myPermanentFileName.c_str(), "a")) != NULL)
      {
	ArLog::log(ArLog::Normal, "ArDataLogger: Opened data log file '%s'", 
		   myPermanentFileName.c_str());
        strcpy(myOpenedFileName, myPermanentFileName.c_str());
      }
      else
      {
	ArLog::log(ArLog::Normal, "ArDataLogger: Error: Could not open data log file '%s'", 
		   myPermanentFileName.c_str());
	myMutex.unlock();
	return true;
      }
    }
    else
    {
      // if we couldn't open it fail
      if ((myFile = ArUtil::fopen(myConfigFileName, "w")) != NULL)
      {
	strcpy(myOpenedFileName, myConfigFileName);
	ArLog::log(ArLog::Normal, "ArDataLogger: Opened data log file '%s'", 
		   myOpenedFileName);
      }
      else
      {
	ArLog::log(ArLog::Normal, "ArDataLogger: Error: Could not open data log file '%s'", 
		   myConfigFileName);
	myMutex.unlock();
	if (errorBuffer != NULL)
	  snprintf(errorBuffer, errorBufferLen, "DataLogFileName of '%s' cannot be opened", myConfigFileName);
	return false;
      }
    }
  }
  else if (!myConfigLogging)
  {
    myMutex.unlock();
    return true;
  }

  if (myLogBatteryInfo && myRobot->getBatteryPacketReader() != NULL)
  {
    myRobot->getBatteryPacketReader()->requestContinuousPackets();
  }

  if(myLogEncoders)
    myRobot->requestEncoderPackets();

  if(myLogMovementSent)
  {
    // assumes we are the only command/packet monitor...
    myRobot->lock();
    myRobot->getPacketSender()->setCommandMonitor(&myCommandMonitorCB);
    myRobot->unlock();
    //myRobot->getPacketSender()->setPacketSentCallback(&myCommandPacketCB);
  }
  else
  {
    myRobot->lock();
    myRobot->getPacketSender()->setCommandMonitor(NULL);
    //myRobot->getPacketSender()->setPacketSentCallback(NULL);
    myRobot->unlock();
  }

  // if we could then dump in the header
  writeHeader();


  myMutex.unlock();
  return true;
}

void ArDataLogger::writeHeader()
{
  fprintf(myFile, (myLogFormat == Fixed)?"; %-12s":";%s", "Time");
  std::map<std::string, bool *, ArStrCaseCmpOp>::iterator it;
  for (int i = 0; i < myStringsCount; ++i)
  {
    if (*(myStringsEnabled[i]))
    {
      char formatBuf[64];
      if(myLogFormat == Fixed)
        snprintf(formatBuf, 64, "%c%%-0%ds", myLogSep, myStrings[i]->getMaxLength());
      else
        snprintf(formatBuf, 64, "%c%%s", myLogSep);
      fprintf(myFile, formatBuf, myStrings[i]->getName());
    }
  }
  if (myLogVoltage)
    fprintf(myFile, "%cVolt", myLogSep);
  if (myLogStateOfCharge)
    fprintf(myFile, "%cSoC", myLogSep);
  if (myLogChargeState)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%s":"%c%-15s%c%-5s", myLogSep, "ChargeStateName", myLogSep, "csNum");

  if (myLogBatteryInfo && myRobot->getBatteryPacketReader() != NULL)
  {
    int battery;
    for (battery = 1; 
	 battery <= myRobot->getBatteryPacketReader()->getNumBatteries();
	 battery++)
    {
      fprintf(myFile, "%cbat%02dflags1%cbat%02dflags2%cbat%02dflags3%cbat%02drelsoc%cbat%02dabssoc", 
        myLogSep, battery,
        myLogSep, battery,
        myLogSep, battery,
        myLogSep, battery,
        myLogSep, battery
      );
    }
  }
  if (myLogPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%s%c%s":"%c%-10s%c%-10s%c%-10s", 
      myLogSep, "X", myLogSep, "Y", myLogSep, "Th");
  if (myLogEncoderPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%s%c%s":"%c%-10s%c%-10s%c%-10s", 
      myLogSep, "encX", myLogSep, "encY", myLogSep, "encTh");
  if (myLogCorrectedEncoderPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%s%c%s":"%c%-10s%c%-10s%c%-10s", 
	    myLogSep, "corrEncX", myLogSep,  "corrEncY", myLogSep, "corrEncTh");
  if (myLogEncoders)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%s":"%c%-10s%c%-10s", myLogSep, "encL", myLogSep, "encR");
  if (myLogLeftVel)
    fprintf(myFile, "%cLeftV", myLogSep);
  if (myLogRightVel)
    fprintf(myFile, "%cRightV", myLogSep);
  if (myLogTransVel)
    fprintf(myFile, "%cTransV", myLogSep);
  if (myLogRotVel)
    fprintf(myFile, "%cRotV", myLogSep);
  if (myLogLatVel)
    fprintf(myFile, "%cLatV", myLogSep);
  if (myLogLeftStalled)
    fprintf(myFile, "%cLStall", myLogSep);
  if (myLogRightStalled)
    fprintf(myFile, "%cRStall", myLogSep);
  if (myLogStallBits)
    fprintf(myFile, "%cStllBts%16s",  myLogSep, "");
  if (myLogFlags)
    fprintf(myFile, "%cFlags%16s",  myLogSep, "");
  if (myLogFaultFlags)
    fprintf(myFile, "%cFault Flags%10s",  myLogSep, "");
  for (int i = 0; i < myAnalogCount; ++i)
  {
    if (myAnalogEnabled[i])
      fprintf(myFile, "%cAn%d", myLogSep, i);
  }
  for (int i = 0; i < myAnalogVoltageCount; ++i)
  {
    if (myAnalogVoltageEnabled[i])
      fprintf(myFile, "%cAnV%d", myLogSep, i);
  }
  for (int i = 0; i < myDigInCount; ++i)
  {
    if (myDigInEnabled[i])
      fprintf(myFile, "%cDigIn%d%8s", myLogSep, i, "");
  }
  for (int i = 0; i < myDigOutCount; ++i)
  {
    if (myDigOutEnabled[i])
      fprintf(myFile, "%cDigOut%d%8s", myLogSep, i, "");
  }
  if(myLogMovementSent)
  { 
    myMovementCommandsMutex.lock();
    for(CmdMap::const_iterator i = myMovementCommands.begin(); i != myMovementCommands.end(); ++i)
    {
      fprintf(myFile, "%cCmd%s", myLogSep, (*i).second.name.c_str());
    }
    myMovementCommandsMutex.unlock();
  }

  fprintf(myFile, "\n");
  fflush(myFile);
}

void ArDataLogger::userTask(void)
{
  myMutex.lock();
  // if we don't need to do anything just return
  if (myFile == NULL || (myConfigLogInterval > 0 && myLastLogged.secSince() < myConfigLogInterval))
  {
    myMutex.unlock();
    return;
  }

  fprintf(myFile, "%.4f", ArUtil::getTime()/1000.0);

  char *buf;
  buf = new char[myMaxMaxLength];
  ArStringInfoHolder *infoHolder;
  for (int i = 0; i < myStringsCount; ++i)
  {
    if (*(myStringsEnabled[i]))
    {
      char formatBuf[64];
      infoHolder = myStrings[i];
      if(myLogFormat == Fixed)
        snprintf(formatBuf, 64, "%c%%-0%ds", myLogSep, myStrings[i]->getMaxLength());
      else
        snprintf(formatBuf, 64, "%c%%s", myLogSep);
      infoHolder->getFunctor()->invoke(buf, infoHolder->getMaxLength());
      /*
      ArLog::log(ArLog::Normal, "For '%s' want to log with format '%s' and buf '%s' (max %d) (max max %d)",
		 infoHolder->getName(), formatBuf, buf, 
		 infoHolder->getMaxLength(), 
		 myMaxMaxLength);
      */
      std::string s(buf);
      std::replace(s.begin(), s.end(), myLogSep, ' ');
      fprintf(myFile, formatBuf, s.c_str());
    }
  }
  delete buf;

  if (myLogVoltage)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%.2f":"%c%-.2f", myLogSep, myRobot->getRealBatteryVoltageNow());
  if (myLogStateOfCharge)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%.1f":"%c%-.1f", myLogSep, myRobot->getStateOfCharge());
  if (myLogChargeState)
  {  
    ArRobot::ChargeState chargeState = myRobot->getChargeState();
    std::string chargeString;
    if (chargeState == ArRobot::CHARGING_UNKNOWN)
      chargeString = "Unknowable";
    else if (chargeState == ArRobot::CHARGING_NOT)
      chargeString = "Not";
    else if (chargeState == ArRobot::CHARGING_BULK)
      chargeString = "Bulk";
    else if (chargeState == ArRobot::CHARGING_OVERCHARGE)
      chargeString = "Overcharge";
    else if (chargeState == ArRobot::CHARGING_FLOAT)
      chargeString = "Float";
    else if (chargeState == ArRobot::CHARGING_BALANCE)
      chargeString = "Balance";
    else
      chargeString = "Unknown";
    fprintf(myFile, (myLogFormat != Fixed)?"%c%s%c%d":"%c%-15s%c%-5d", myLogSep, chargeString.c_str(), myLogSep, chargeState);
  }

  if (myLogBatteryInfo && myRobot->getBatteryPacketReader() != NULL)
  {
    int battery;
    int flags;
    for (battery = 1; 
       battery <= myRobot->getBatteryPacketReader()->getNumBatteries();
       battery++)
    {
      fprintf(myFile, "%c", myLogSep);
      flags = myRobot->getBatteryPacketReader()->getFlags1(battery);
      for (int i = 0, val = 1; i < 8; ++i, val *= 2)
        fprintf(myFile, "%d", (bool) (flags & val));
      if(!(myLogFormat != Fixed)) fprintf(myFile, "   ");

      fprintf(myFile, "%c", myLogSep);
      flags = myRobot->getBatteryPacketReader()->getFlags2(battery);
      for (int i = 0, val = 1; i < 8; ++i, val *= 2)
        fprintf(myFile, "%d", (bool) (flags & val));
      if(!(myLogFormat != Fixed)) fprintf(myFile, "   ");

      fprintf(myFile, "%c", myLogSep);
      flags = myRobot->getBatteryPacketReader()->getFlags3(battery);
      for (int i = 0, val = 1; i < 8; ++i, val *= 2)
        fprintf(myFile, "%d", (bool) (flags & val));
      if(!(myLogFormat != Fixed)) fprintf(myFile, "   ");
      
      fprintf(myFile, (myLogFormat != Fixed)?"%c%d%c%d":"%c%-11d%c%-11d", 
	      myLogSep, myRobot->getBatteryPacketReader()->getRelSOC(battery),
	      myLogSep, myRobot->getBatteryPacketReader()->getAbsSOC(battery));      
    }
  }

  if (myLogPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%.0f%c%.0f%c%.0f":"%c%-10.0f%c%-10.0f%c%-10.0f", 
      myLogSep, myRobot->getX(), myLogSep, myRobot->getY(), myLogSep, myRobot->getTh());
  if (myLogEncoderPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%.0f%c%.0f%c%.0f":"%c%-10.0f%c%-10.0f%c%-10.0f",
	    myLogSep, myRobot->getRawEncoderPose().getX(), 
	    myLogSep, myRobot->getRawEncoderPose().getY(), 
	    myLogSep, myRobot->getRawEncoderPose().getTh());
  if (myLogCorrectedEncoderPose)
    fprintf(myFile, (myLogFormat != Fixed)?"%c%.0f%c%.0f%c%.0f":"%c%-10.0f%c%-10.0f%c%-10.0f", 
	    myLogSep, myRobot->getEncoderPose().getX(), 
	    myLogSep, myRobot->getEncoderPose().getY(), 
	    myLogSep, myRobot->getEncoderPose().getTh());
  if (myLogEncoders)
    fprintf(myFile, (myLogFormat != Fixed)?":%c%ld%c%ld":"%c%-10ld%c%-10ld", 
	    myLogSep, myRobot->getLeftEncoder(), myLogSep, myRobot->getRightEncoder());
  if (myLogLeftVel)
    fprintf(myFile, "%c%.0f", myLogSep, myRobot->getLeftVel());
  if (myLogRightVel)
    fprintf(myFile, "%c%.0f", myLogSep, myRobot->getRightVel());
  if (myLogTransVel)
    fprintf(myFile, "%c%.0f", myLogSep, myRobot->getVel());
  if (myLogRotVel)
    fprintf(myFile, "%c%.0f", myLogSep, myRobot->getRotVel());
  if (myLogLatVel)
    fprintf(myFile, "%c%.0f", myLogSep, myRobot->getLatVel());
  if (myLogLeftStalled)
    fprintf(myFile, "%c%d", myLogSep, (bool)myRobot->isLeftMotorStalled());
  if (myLogRightStalled)
    fprintf(myFile, "%c%d", myLogSep, (bool)myRobot->isRightMotorStalled());
  if (myLogStallBits)
  {
    fprintf(myFile, "%c", myLogSep);
    for (int i = 0, val = 1; i < 16; ++i, val *= 2)
      fprintf(myFile, "%d", (bool)(myRobot->getStallValue() & val));
  }
  if (myLogFlags)
  {
    fprintf(myFile, "%c", myLogSep);
    for (int i = 0, val = 1; i < 16; ++i, val *= 2)
      fprintf(myFile, "%d", (bool)(myRobot->getFlags() & val));
  }
  if (myLogFaultFlags)
  {
    fprintf(myFile, "%c", myLogSep);
    for (int i = 0, val = 1; i < 16; ++i, val *= 2)
      fprintf(myFile, "%d", (bool)(myRobot->getFaultFlags() & val));
  }
  for (int i = 0; i < myAnalogCount; ++i)
  {
    if (myAnalogEnabled[i])
      fprintf(myFile, "%c%d", myLogSep, myRobot->getIOAnalog(i));
  }
  for (int i = 0; i < myAnalogVoltageCount; ++i)
  {
    if (myAnalogVoltageEnabled[i])
      fprintf(myFile, "%c%.2f", myLogSep, myRobot->getIOAnalogVoltage(i));
  }
  for (int i = 0; i < myDigInCount; ++i)
  {
    if (myDigInEnabled[i])
    {
      fprintf(myFile, "%c", myLogSep);
      for (int j = 0, val = 1; j < 8; ++j, val *= 2)
        fprintf(myFile, "%d", (bool)(myRobot->getIODigIn(i) & val));
    }
  }
  for (int i = 0; i < myDigOutCount; ++i)
  {
    if (myDigOutEnabled[i])
    {
      fprintf(myFile, "%c", myLogSep);
      for (int j = 0, val = 1; j < 8; ++j, val *= 2)
        fprintf(myFile, "%d", (bool)(myRobot->getIODigOut(i) & val));
    }
  }

  if(myLogMovementSent)
  {
    myMovementCommandsMutex.lock();
    for(CmdMap::iterator i = myMovementCommands.begin(); i != myMovementCommands.end(); ++i)
    {
      Cmd &c = (*i).second;
      if(c.sent) fprintf(myFile, "%c%d", myLogSep, c.arg);
      else fprintf(myFile, "%cna", myLogSep);
      c.sent = false; // reset for next iteration
    }
    myMovementCommandsMutex.unlock();
  }
  
  fprintf(myFile, "\n");
  fflush(myFile);
  myLastLogged.setToNow();
  myMutex.unlock();
}


// todo omit units from config name? (ie separate name and units here, concat) 
AREXPORT void ArDataLogger::addString(
	const char *orig_name, ArTypes::UByte2 maxLength,
	ArFunctor2<char *, ArTypes::UByte2> *functor)
{
  ArTypes::UByte2 len;

  // todo maybe this should be an option:
  std::string name(orig_name);
  std::replace(name.begin(), name.end(), ' ', '_');
  std::replace(name.begin(), name.end(), '\t', '_');
  std::replace(name.begin(), name.end(), ',', '_');
  std::replace(name.begin(), name.end(), ';', '_');


  myMutex.lock();

  // TODO check myStrings and warn if we already have one of this name

  if (maxLength < name.length())
    len = (ArTypes::UByte2) name.length();
  else
    len = maxLength;  
  if (myMaxMaxLength < len)
    myMaxMaxLength = len;

  myStrings.push_back(new ArStringInfoHolder(name.c_str(), len, functor));
  bool *boolPtr;
  boolPtr = new bool;
  // if we've added to config we default to true
  if (myAddedToConfig)
    *boolPtr = true;
  else
    *boolPtr = false;
  myStringsEnabled.push_back(boolPtr);
  myStringsCount++;
  myMutex.unlock();
  if (myAddedToConfig)
  {
    char pname[512];
    char pdesc[512];
    snprintf(pname, sizeof(pname), "DataLog%s", name.c_str());
    snprintf(pdesc, sizeof(pdesc), "Logs %s", orig_name);
    myConfig->addParam(ArConfigArg(pname, boolPtr, pdesc), "Data logging", ArPriority::DETAILED);
    myStringsAddedToConfig.push_back(true);
    processFile(NULL, 0);
  }
  else
  {
    // if myAddedToConfig==false, all items including this new one will be added when addToConfig() is called.
    myStringsAddedToConfig.push_back(false);
  }
}


void ArDataLogger::robotCommandMonitor(unsigned char cmd, short int arg)
{
  myMovementCommandsMutex.lock();
  CmdMap::iterator i = myMovementCommands.find(cmd);
  if(i != myMovementCommands.end())
  {
    (*i).second.sent = true;
    (*i).second.arg = arg;
  }
  myMovementCommandsMutex.unlock();
}


AREXPORT void ArDataLogger::writeComment(const char *str)
{
  myMutex.lock();
  if(!myFile)
  {
    myMutex.unlock();
    return;
  }
  ArLog::log(ArLog::Normal, "ArDataLogger: User comment: %s", str);
  fprintf(myFile, "; %ld %s\n", time(NULL), str);
  fflush(myFile);
  myMutex.unlock();
}
 

AREXPORT void ArDataLogger::writeComment(ArArgumentBuilder *ab)
{
  writeComment(ab->getFullString());
}

AREXPORT void ArDataLogger::clearLog()
{
  myMutex.lock();
  if(!myFile || strlen(myOpenedFileName) == 0)
  {
    myMutex.unlock();
    return;
  }
  ArLog::log(ArLog::Normal, "ArDataLogger: Warning: Clearing log file!");
  //rewind(myFile);
  if(myFile) fclose(myFile);
  myFile = ArUtil::fopen(myOpenedFileName, "w");
  if(myFile)
    writeHeader();
  else
    ArLog::logErrorFromOS(ArLog::Normal, "ArDataLogger: Error reopening log file %s", myOpenedFileName);
  
  myMutex.unlock();
}


AREXPORT std::string ArDataLogger::getStatus() 
{
  if(myConfigLogging && myFile)
    return std::string("Logging to ") + getOpenLogFileName();
  else
    return "Not logging";
}

AREXPORT void ArDataLogger::getStatus(char *buf, ArTypes::UByte2 buflen)
{
  strncpy(buf, getStatus().c_str(), buflen);
}


AREXPORT void ArDataLogger::saveCopyAs(const char *name_s)
{
  std::string filename;
  if(name_s && strlen(name_s) > 0)
  {
    filename = name_s;
  }
  else
  {
    filename = getOpenLogFileName();
    char timesuffix[21];
    struct tm t;
    ArUtil::localtime(&t);
    snprintf(timesuffix, 21, "-%4d-%02d-%02d-%02d-%02d:%02d", 
      1900 + t.tm_year,
      t.tm_mon + 1,
      t.tm_mday,
      t.tm_hour,
      t.tm_min,
      t.tm_sec
    );
    size_t dotpos = filename.rfind(".");
    if(dotpos == filename.npos)
      filename.append(timesuffix);
    else
      filename.insert(dotpos, timesuffix);
  }
  myMutex.lock();
  ArLog::log(ArLog::Normal, "ArDataLogger: Making copy of log file \"%s\" as \"%s\" on system...", getOpenLogFileName(), filename.c_str());
  char cmd[512];
#ifdef WIN32
  const char *copycommand = "copy";
#else
  const char *copycommand = "cp";
#endif
  snprintf(cmd, 512, "%s \"%s\" \"%s\"", copycommand, getOpenLogFileName(), filename.c_str());
  if(system(cmd) != 0)
    ArLog::log(ArLog::Terse, "ArDataLogger: Warning: Error copying log file \"%s\" to \"%s\" on system", getOpenLogFileName(), filename.c_str());
  myMutex.unlock();
}

AREXPORT void ArDataLogger::saveCopyArgs(ArArgumentBuilder *ab)
{
  saveCopyAs(ab->getFullString());
}

AREXPORT void ArDataLogger::startLogging(int interval)
{
  myConfigLogging = true;
  if (interval != -1) myConfigLogInterval = interval;
  processFile(NULL, 0);
}

AREXPORT void ArDataLogger::stopLogging()
{
  myConfigLogging = false;
  processFile(NULL, 0);
}
