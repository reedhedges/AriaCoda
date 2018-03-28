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
#include "ArActionMovementParametersDebugging.h"
#include "ArConfig.h"
#include "ArRobot.h"

/**
   @param name the name of this instance of the action
 **/
AREXPORT ArActionMovementParametersDebugging::ArActionMovementParametersDebugging(
	const char *name) :
  ArAction(name, "Sets all the max vel and accels/decels in a way appropriate for debugging")
{
  myEnabled = true;
  myEnableOnce = false;

  mySetMaxVel = false;
  myMaxVel = 2000;
  mySetMaxNegVel = false;
  myMaxNegVel = 1000;
  mySetTransAccel = false;
  myTransAccel = 500;
  mySetTransDecel = false;
  myTransDecel = 500;
  mySetMaxRotVel = false;
  myMaxRotVel = 250;
  mySetRotAccel = false;
  myRotAccel = 250;
  mySetRotDecel = false;
  myRotDecel = 250;
  mySetMaxLeftLatVel = false;
  myMaxLeftLatVel = 2000;
  mySetMaxRightLatVel = false;
  myMaxRightLatVel = 2000;
  mySetLatAccel = false;
  myLatAccel = 500;
  mySetLatDecel = false;
  myLatDecel = 500;
}

AREXPORT ArActionMovementParametersDebugging::~ArActionMovementParametersDebugging()
{

}

AREXPORT void ArActionMovementParametersDebugging::addToConfig(
	ArConfig *config, const char *section, const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetTransVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetMaxVel, 
		      "True to set the transVelMax"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "TransVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxVel, 
		      "Maximum forward translational velocity (0 means use default)"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetTransNegVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetMaxNegVel, 
		      "True to set the transNegVelMax"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "TransNegVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxNegVel, 
		      "Maximum backwards translational velocity"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetTransAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetTransAccel, 
		      "True to set the transAccel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "TransAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myTransAccel, 
		      "Translational acceleration"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetTransDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetTransDecel, 
		      "True to set the transDecel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "TransDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myTransDecel, 
		      "Translational deceleration"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetRotVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetMaxRotVel, 
		      "True to set the rotVelMax"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "RotVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxRotVel, 
		      "Maximum rotational velocity"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetRotAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetRotAccel, 
		      "True to set rotAccel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "RotAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myRotAccel, 
		      "Rotational acceleration"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetRotDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetRotDecel, 
		      "True to set rotDecel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "RotDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myRotDecel, 
		      "Rotational deceleration"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetLeftLatVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetMaxLeftLatVel, 
		      "True to set leftLatVelMax"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "LeftLatVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxLeftLatVel, 
		      "Maximum left lateral velocity"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "SetRightLatVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetMaxRightLatVel, 
		      "True to set rightLatVelMax"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "RightLatVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxRightLatVel, 
		      "Maximum right lateral velocity"),
	  section, ArPriority::EXPERT);
  
  name = strPrefix;
  name += "SetLatAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetLatAccel, 
		      "True to set latAccel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "LatAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myLatAccel, 
		      "Lateral acceleration"),
	  section, ArPriority::EXPERT);
  
  name = strPrefix;
  name += "SetLatDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &mySetLatDecel, 
		      "True to set latDecel"),
	  section, ArPriority::EXPERT);

  name = strPrefix;
  name += "LatDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myLatDecel, 
		      "Lateral deceleration"),
	  section, ArPriority::EXPERT);
  
  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::EXPERT);


}

AREXPORT ArActionDesired *ArActionMovementParametersDebugging::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  if (!myEnabled && !myEnableOnce)
    return NULL;
  myEnableOnce = false;

  if (mySetMaxVel)
    myDesired.setMaxVel(myMaxVel);

  if (mySetMaxNegVel)
    myDesired.setMaxNegVel(myMaxNegVel);

  if (mySetTransAccel)
    myDesired.setTransAccel(myTransAccel);

  if (mySetTransDecel)
    myDesired.setTransDecel(myTransDecel);

  if (mySetMaxRotVel)
    myDesired.setMaxRotVel(myMaxRotVel);

  if (mySetRotAccel)
    myDesired.setRotAccel(myRotAccel);

  if (mySetRotDecel)
    myDesired.setRotDecel(myRotDecel);

  if (mySetMaxLeftLatVel)
    myDesired.setMaxLeftLatVel(myMaxLeftLatVel);

  if (mySetMaxRightLatVel)
    myDesired.setMaxRightLatVel(myMaxRightLatVel);

  if (mySetLatAccel)
    myDesired.setLatAccel(myLatAccel);

  if (mySetLatDecel)
    myDesired.setLatDecel(myLatDecel);
  
  return &myDesired;
}
