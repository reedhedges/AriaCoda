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
#include "ArActionMovementParameters.h"
#include "ArConfig.h"
#include "ArRobot.h"

/**
   @param name the name of this instance of the action

   @param overrideFaster if this is set to true and a fast max vel is
   set and our max vel is lower we'll set the vel with all our
   strength (since hopefully that faster setting was set with minimum
   strength)

   @param addLatVelIfAvailable If true, include the LatVel parameters in the
   ArConfig if the robot supports lateral motion.
 **/
AREXPORT ArActionMovementParameters::ArActionMovementParameters(
	const char *name,
	bool overrideFaster,
	bool addLatVelIfAvailable) : 
  ArAction(name, "Sets all the max vel and accels/decels")
{
  myOverrideFaster = overrideFaster;
  myAddLatVelIfAvailable = addLatVelIfAvailable;
  myEnabled = true;
  myEnableOnce = false;
  setParameters();
}

AREXPORT ArActionMovementParameters::~ArActionMovementParameters()
{

}

AREXPORT void ArActionMovementParameters::setParameters(double maxVel, 
							double maxNegVel,
							double transAccel,
							double transDecel,
							double rotVelMax, 
							double rotAccel,
							double rotDecel,
							double latVelMax, 
							double latAccel,
							double latDecel)
{
  myMaxVel = maxVel;
  myMaxNegVel = maxNegVel;
  myTransAccel = transAccel;
  myTransDecel = transDecel;
  myMaxRotVel = rotVelMax;
  myRotAccel = rotAccel;
  myRotDecel = rotDecel;
  myMaxLatVel = latVelMax;
  myLatAccel = latAccel;
  myLatDecel = latDecel;
}

AREXPORT void ArActionMovementParameters::addToConfig(ArConfig *config, 
						      const char *section, 
						      const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::DETAILED);
  name = strPrefix;
  name += "TransVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxVel, 
		      "Maximum forward translational velocity (0 means use default)", 
		      0),
		      //myRobot->getAbsoluteMaxTransVel()),
	  section, ArPriority::DETAILED);


  name = strPrefix;
  name += "TransNegVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxNegVel, 
		      "Maximum backwards translational velocity (0 means use default)", 
		      0),
	  //myRobot->getAbsoluteMaxTransVel()),
	  section, ArPriority::DETAILED);

  name = strPrefix;
  name += "TransAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myTransAccel, 
		      "Translational acceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxTransAccel()),		      
	  section, ArPriority::DETAILED);

  name = strPrefix;
  name += "TransDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myTransDecel, 
		      "Translational deceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxTransDecel()),		      
	  section, ArPriority::DETAILED);

  name = strPrefix;
  name += "RotVelMax";
  config->addParam(
	  ArConfigArg(name.c_str(), &myMaxRotVel, 
		      "Maximum rotational velocity (0 means use default)", 
		      0), // myRobot->getAbsoluteMaxRotVel()),
	  section, ArPriority::DETAILED);

  name = strPrefix;
  name += "RotAccel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myRotAccel, 
		      "Rotational acceleration (0 means use default)", 0),
	  //myRobot->getAbsoluteMaxRotAccel()),
	  section, ArPriority::DETAILED);

  name = strPrefix;
  name += "RotDecel";
  config->addParam(
	  ArConfigArg(name.c_str(), &myRotDecel, 
		      "Rotational deceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxRotDecel()),
	  section, ArPriority::DETAILED);

  if (myAddLatVelIfAvailable && myRobot != NULL && myRobot->hasLatVel())
  {
    name = strPrefix;
    name += "LatVelMax";
    config->addParam(
	    ArConfigArg(name.c_str(), &myMaxLatVel, 
			"Maximum lateral velocity (0 means use default)", 
			0), // myRobot->getAbsoluteMaxLatVel()),
	    section, ArPriority::DETAILED);
    
    name = strPrefix;
    name += "LatAccel";
    config->addParam(
	    ArConfigArg(name.c_str(), &myLatAccel, 
			"Lateral acceleration (0 means use default)", 0),
	    //myRobot->getAbsoluteMaxLatAccel()),
	    section, ArPriority::DETAILED);
    
    name = strPrefix;
    name += "LatDecel";
    config->addParam(
	    ArConfigArg(name.c_str(), &myLatDecel, 
			"Lateral deceleration (0 means use default)", 0),
	    //myRobot->getAbsoluteMaxLatDecel()),
	    section, ArPriority::DETAILED);
  }

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::DETAILED);


}

AREXPORT ArActionDesired *ArActionMovementParameters::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  if (!myEnabled && !myEnableOnce)
    return NULL;
  myEnableOnce = false;

  if (fabs(myMaxVel) >= 1)
    myDesired.setMaxVel(myMaxVel);

  if (fabs(myMaxNegVel) >= 1)
    myDesired.setMaxNegVel(-myMaxNegVel);

  if (fabs(myTransAccel) >= 1)
    myDesired.setTransAccel(myTransAccel);

  if (fabs(myTransDecel) >= 1)
    myDesired.setTransDecel(myTransDecel);

  if (fabs(myMaxRotVel) >= 1)
    myDesired.setMaxRotVel(myMaxRotVel);

  if (fabs(myRotAccel) >= 1)
    myDesired.setRotAccel(myRotAccel);

  if (fabs(myRotDecel) >= 1)
    myDesired.setRotDecel(myRotDecel);

  if (fabs(myMaxLatVel) >= 1)
  {
    myDesired.setMaxLeftLatVel(myMaxLatVel);
    myDesired.setMaxRightLatVel(myMaxLatVel);
  }

  if (fabs(myLatAccel) >= 1)
    myDesired.setLatAccel(myLatAccel);

  if (fabs(myLatDecel) >= 1)
    myDesired.setLatDecel(myLatDecel);
  
  return &myDesired;
}
