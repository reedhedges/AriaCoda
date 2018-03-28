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
#include "ArActionDesired.h"

/* Need to export symbols but can't prior to VS 2010 (The VS 2003 _MSC_VER is version 1310, 2005 is 14xx, 2008 is 15xx, 2010 is 1600)*/
//#if (_MSC_VER < 1600)
//#define STATIC_CONST_AREXPORT // defined to nothing
//#else
//#define STATIC_CONST_AREXPORT AREXPORT
//#endif

AREXPORT const double ArActionDesired::NO_STRENGTH = 0.0;
AREXPORT const double ArActionDesired::MIN_STRENGTH = .000001;
AREXPORT const double ArActionDesired::MAX_STRENGTH = 1.0;

AREXPORT const double ArActionDesiredChannel::NO_STRENGTH =
                                                 ArActionDesired::NO_STRENGTH;
AREXPORT const double ArActionDesiredChannel::MIN_STRENGTH = 
                                                ArActionDesired::MIN_STRENGTH;
AREXPORT const double ArActionDesiredChannel::MAX_STRENGTH = 
                                                ArActionDesired::MAX_STRENGTH;


AREXPORT void ArActionDesired::log(void) const
{
  // all those maxes and movement parameters
  if (getMaxVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "\tMaxTransVel %.0f", getMaxVel());
  if (getMaxNegVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "\tMaxTransNegVel %.0f", 
	       getMaxNegVel());
  if (getTransAccelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "\tTransAccel %.0f", getTransAccel());
  if (getTransDecelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "\tTransDecel %.0f", getTransDecel());

  if (getMaxRotVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tMaxRotVel %.0f", "",
	       getMaxRotVel());
  if (getMaxRotVelPosStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tMaxRotVelPos %.0f", "",
	       getMaxRotVelPos());
  if (getMaxRotVelNegStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tMaxRotVelNeg %.0f", "",
	       getMaxRotVelNeg());
  if (getRotAccelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tRotAccel %.0f", "",
	       getRotAccel());
  if (getRotDecelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tRotDecel %.0f", "",
	       getRotDecel());

  if (getMaxLeftLatVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%12s\tMaxLeftLatVel %.0f", "",
	       getMaxLeftLatVel());
  if (getMaxRightLatVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%12s\tMaxRightLatVel %.0f", "",
	       getMaxRightLatVel());
  if (getLatAccelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%12s\tLatAccel %.0f", "",
	       getLatAccel());
  if (getLatDecelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%12s\tLatDecel %.0f", "",
	       getLatDecel());
  
  // the actual movement part
  if (getVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "\tVel %.0f", getVel());
  if (getHeadingStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tHeading %.0f", "", 
	       getHeading());
  if (getDeltaHeadingStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tDeltaHeading %.0f", "", 
	       getDeltaHeading());
  if (getRotVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%25s\tRotVel %.0f", "", 
	       getRotVel());
  if (getLatVelStrength() >= ArActionDesired::MIN_STRENGTH)
    ArLog::log(ArLog::Normal, "%12s\tLatVel %.0f", "", 
	       getLatVel());
}


AREXPORT bool ArActionDesired::isAnythingDesired(void) const
{
  if (getVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxNegVelStrength() >= ArActionDesired::MIN_STRENGTH || 
      getTransAccelStrength() >= ArActionDesired::MIN_STRENGTH || 
      getTransDecelStrength() >= ArActionDesired::MIN_STRENGTH ||

      getHeadingStrength() >= ArActionDesired::MIN_STRENGTH ||
      getDeltaHeadingStrength() >= ArActionDesired::MIN_STRENGTH ||
      getRotVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxRotVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxRotVelPosStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxRotVelNegStrength() >= ArActionDesired::MIN_STRENGTH ||
      getRotAccelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getRotDecelStrength() >= ArActionDesired::MIN_STRENGTH ||

      getMaxLeftLatVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getMaxRightLatVelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getLatAccelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getLatDecelStrength() >= ArActionDesired::MIN_STRENGTH ||
      getLatVelStrength() >= ArActionDesired::MIN_STRENGTH)
    return true;
  else
    return false;
}

AREXPORT void ArActionDesired::sanityCheck(const char *actionName)
{
  myMaxVelDes.checkLowerBound(actionName, "TransMaxVel", 0);
  myMaxNegVelDes.checkUpperBound(actionName, "TransMaxNegVel", 0);

  myTransAccelDes.checkLowerBound(actionName, "TransAccel", 1);
  myTransDecelDes.checkLowerBound(actionName, "TransDecel", 1);

  if (myMaxRotVelDes.getStrength() >= ArActionDesired::MIN_STRENGTH && 
      ArMath::roundInt(myMaxRotVelDes.getDesired()) == 0)
    ArLog::log(ArLog::Normal, 
	  "ActionSanityChecking: '%s' setting %s to %g which winds up as 0 (this is just a warning)",
	       actionName, "MaxRotVel", myMaxRotVelDes.getDesired());
  myMaxRotVelDes.checkLowerBound(actionName, "MaxRotVel", 0);


  myMaxRotVelPosDes.checkLowerBound(actionName, "MaxRotVelPos", 1); 
  myMaxRotVelNegDes.checkLowerBound(actionName, "MaxRotVelNeg", 1);

  myRotAccelDes.checkLowerBound(actionName, "RotAccel", 1);
  myRotDecelDes.checkLowerBound(actionName, "RotDecel", 1);

  myMaxLeftLatVelDes.checkLowerBound(actionName, "MaxLeftLatVel", 0);
  myMaxRightLatVelDes.checkLowerBound(actionName, "MaxRightLatVel", 0);

  myLatAccelDes.checkLowerBound(actionName, "LatAccel", 1);
  myLatDecelDes.checkLowerBound(actionName, "LatDecel", 1);
};
