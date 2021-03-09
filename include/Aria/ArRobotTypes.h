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

#ifndef ARIA_WRAPPER

#ifndef ARROBOTTYPES_H
#define ARROBOTTYPES_H


#include "Aria/ariaUtil.h"
#include "Aria/ArRobotParams.h"

/** @cond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

class ArRobotGeneric : public ArRobotParams
{
public:
  AREXPORT ArRobotGeneric();
  AREXPORT virtual ~ArRobotGeneric() {}
};

class ArRobotAmigo : public ArRobotParams
{
public:

  AREXPORT ArRobotAmigo();
  AREXPORT virtual ~ArRobotAmigo() {}
};

class ArRobotAmigoSh : public ArRobotParams
{
public:

  AREXPORT ArRobotAmigoSh();
  AREXPORT virtual ~ArRobotAmigoSh() {}
};


class ArRobotAmigoShTim5xxWibox : public ArRobotAmigoSh
{
public:
  AREXPORT ArRobotAmigoShTim5xxWibox();
  AREXPORT virtual ~ArRobotAmigoShTim5xxWibox() {}
};

class ArRobotP2AT : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT();
  AREXPORT virtual ~ArRobotP2AT() {}
};

class ArRobotP2AT8 : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT8();
  AREXPORT virtual ~ArRobotP2AT8() {}
};

class ArRobotP2AT8Plus : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT8Plus();
  AREXPORT virtual ~ArRobotP2AT8Plus() {}
};

class ArRobotP2IT : public ArRobotParams
{
public:
  AREXPORT ArRobotP2IT();
  AREXPORT virtual ~ArRobotP2IT() {}
};

class ArRobotP2DX : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DX();
  AREXPORT virtual ~ArRobotP2DX() {}
};

class ArRobotP2DXe : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DXe();
  AREXPORT virtual ~ArRobotP2DXe() {}
};

class ArRobotP2DF : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DF();
  AREXPORT virtual ~ArRobotP2DF() {}
};

class ArRobotP2D8 : public ArRobotParams
{
public:
  AREXPORT ArRobotP2D8();
  AREXPORT virtual ~ArRobotP2D8() {}
};

class ArRobotP2D8Plus : public ArRobotParams
{
public:
  AREXPORT ArRobotP2D8Plus();
  AREXPORT virtual ~ArRobotP2D8Plus() {}
};

class ArRobotP2CE : public ArRobotParams
{
public:
  AREXPORT ArRobotP2CE();
  AREXPORT virtual ~ArRobotP2CE() {}
};

class ArRobotP2PP : public ArRobotParams
{
public:
  AREXPORT ArRobotP2PP();
  AREXPORT virtual ~ArRobotP2PP() {}
};

class ArRobotP2PB : public ArRobotParams
{
public:
  AREXPORT ArRobotP2PB();
  AREXPORT virtual ~ArRobotP2PB() {}
};


class ArRobotP3AT : public ArRobotParams
{
public:
  AREXPORT ArRobotP3AT();
  AREXPORT virtual ~ArRobotP3AT() {}
};


class ArRobotP3DX : public ArRobotParams
{
public:
  AREXPORT ArRobotP3DX();
  AREXPORT virtual ~ArRobotP3DX() {}
};

class ArRobotPerfPB : public ArRobotParams
{
public:
  AREXPORT ArRobotPerfPB();
  AREXPORT virtual ~ArRobotPerfPB() {}
};

class ArRobotPerfPBPlus : public ArRobotParams
{
public:
  AREXPORT ArRobotPerfPBPlus();
  AREXPORT virtual ~ArRobotPerfPBPlus() {}
};

class ArRobotPion1M : public ArRobotParams
{
public:
  AREXPORT ArRobotPion1M();
  AREXPORT virtual ~ArRobotPion1M() {}
};

class ArRobotPion1X : public ArRobotParams
{
public:
  AREXPORT ArRobotPion1X();
  AREXPORT virtual ~ArRobotPion1X() {}
};

class ArRobotPsos1M : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos1M();
  AREXPORT virtual ~ArRobotPsos1M() {}
};

class ArRobotPsos43M : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos43M();
  AREXPORT virtual ~ArRobotPsos43M() {}
};

class ArRobotPsos1X : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos1X();
  AREXPORT virtual ~ArRobotPsos1X() {}
};

class ArRobotPionAT : public ArRobotParams
{
public:
  AREXPORT ArRobotPionAT();
  AREXPORT virtual ~ArRobotPionAT() {}
};

class ArRobotMapper : public ArRobotParams
{
public:
  AREXPORT ArRobotMapper();
  AREXPORT virtual ~ArRobotMapper() {}
};

class ArRobotPowerBot : public ArRobotParams
{
public:
  AREXPORT ArRobotPowerBot();
  AREXPORT virtual ~ArRobotPowerBot() {}
};

class ArRobotP3DXSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3DXSH();
  AREXPORT virtual ~ArRobotP3DXSH() {}
};

class ArRobotP3ATSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3ATSH();
  AREXPORT virtual ~ArRobotP3ATSH() {}
};

class ArRobotP3ATIWSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3ATIWSH();
  AREXPORT virtual ~ArRobotP3ATIWSH() {}
};

class ArRobotPatrolBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPatrolBotSH();
  AREXPORT virtual ~ArRobotPatrolBotSH() {}
};

class ArRobotPeopleBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPeopleBotSH();
  AREXPORT virtual ~ArRobotPeopleBotSH() {}
};

class ArRobotPowerBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPowerBotSH();
  AREXPORT virtual ~ArRobotPowerBotSH() {}
};

class ArRobotWheelchairSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotWheelchairSH();
  AREXPORT virtual ~ArRobotWheelchairSH() {}
};

class ArRobotPowerBotSHuARCS : public ArRobotParams
{
 public:
  AREXPORT ArRobotPowerBotSHuARCS();
  AREXPORT virtual ~ArRobotPowerBotSHuARCS() {}
};

class ArRobotSeekur : public ArRobotParams
{
 public:
  AREXPORT ArRobotSeekur();
  AREXPORT virtual ~ArRobotSeekur() {}
};

/// @since Aria 2.7.2
class ArRobotMT400 : public ArRobotParams
{
 public:
  AREXPORT ArRobotMT400();
  AREXPORT virtual ~ArRobotMT400() {}
};

/// @since Aria 2.7.2
class ArRobotResearchPB : public ArRobotParams
{
 public:
  AREXPORT ArRobotResearchPB();
  AREXPORT virtual ~ArRobotResearchPB() {}
};

/// @since Aria 2.7.2
class ArRobotSeekurJr : public ArRobotParams
{
 public:
  AREXPORT ArRobotSeekurJr();
  AREXPORT virtual ~ArRobotSeekurJr() {}
};

/// @since Aria 2.7.4
class ArRobotP3DXSH_lms1xx : public ArRobotP3DXSH
{
public: 
  AREXPORT ArRobotP3DXSH_lms1xx();
};

/// @since Aria 2.7.4
class ArRobotP3ATSH_lms1xx : public ArRobotP3ATSH
{
public: 
  AREXPORT ArRobotP3ATSH_lms1xx();
};

/// @since Aria 2.7.4
class ArRobotPeopleBotSH_lms1xx : public ArRobotPeopleBotSH
{
public: 
  AREXPORT ArRobotPeopleBotSH_lms1xx();
};

/// @since Aria 2.7.4
class ArRobotP3DXSH_lms500 : public ArRobotP3DXSH
{
public: 
  AREXPORT ArRobotP3DXSH_lms500();
};

/// @since Aria 2.7.4
class ArRobotP3ATSH_lms500 : public ArRobotP3ATSH
{
public: 
  AREXPORT ArRobotP3ATSH_lms500();
};

/// @since Aria 2.7.4
class ArRobotPeopleBotSH_lms500 : public ArRobotPeopleBotSH
{
public: 
  AREXPORT ArRobotPeopleBotSH_lms500();
};

/// @since Aria 2.7.4
class ArRobotPowerBotSH_lms500 : public ArRobotPowerBotSH
{
public: 
  AREXPORT ArRobotPowerBotSH_lms500();
};

/// @since Aria 2.7.4
class ArRobotResearchPB_lms500 : public ArRobotResearchPB
{
public: 
  AREXPORT ArRobotResearchPB_lms500();
};

/// @since Aria 2.8
class ArRobotPioneerLX : public ArRobotParams
{
public:
  AREXPORT ArRobotPioneerLX();
  AREXPORT virtual ~ArRobotPioneerLX() {}
};

/// @since Aria 2.9.2
class ArRobotPioneerLX_LD : public ArRobotPioneerLX
{
public:
  AREXPORT ArRobotPioneerLX_LD();
};

/** @endcond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

#endif // ARROBOTTYPES_H
#endif // not ARIA_WRAPPER