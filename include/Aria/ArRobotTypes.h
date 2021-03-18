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


*/

#ifndef ARIA_WRAPPER

#ifndef ARROBOTTYPES_H
#define ARROBOTTYPES_H


#include "Aria/ariaUtil.h"
#include "Aria/ArRobotParams.h"

/** @cond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

// note these are just type names

class ArRobotGeneric : public ArRobotParams
{
public:
  AREXPORT ArRobotGeneric();
  AREXPORT virtual ~ArRobotGeneric() = default;
};

class ArRobotAmigo : public ArRobotParams
{
public:

  AREXPORT ArRobotAmigo();
  AREXPORT virtual ~ArRobotAmigo() = default;
};

class ArRobotAmigoSh : public ArRobotParams
{
public:

  AREXPORT ArRobotAmigoSh();
  AREXPORT virtual ~ArRobotAmigoSh() = default;
};


class ArRobotAmigoShTim5xxWibox : public ArRobotAmigoSh
{
public:
  AREXPORT ArRobotAmigoShTim5xxWibox();
  AREXPORT virtual ~ArRobotAmigoShTim5xxWibox() = default;
};

class ArRobotP2AT : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT();
  AREXPORT virtual ~ArRobotP2AT() = default;
};

class ArRobotP2AT8 : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT8();
  AREXPORT virtual ~ArRobotP2AT8() = default;
};

class ArRobotP2AT8Plus : public ArRobotParams
{
public:
  AREXPORT ArRobotP2AT8Plus();
  AREXPORT virtual ~ArRobotP2AT8Plus() = default;
};

class ArRobotP2IT : public ArRobotParams
{
public:
  AREXPORT ArRobotP2IT();
  AREXPORT virtual ~ArRobotP2IT() = default;
};

class ArRobotP2DX : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DX();
  AREXPORT virtual ~ArRobotP2DX() = default;
};

class ArRobotP2DXe : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DXe();
  AREXPORT virtual ~ArRobotP2DXe() = default;
};

class ArRobotP2DF : public ArRobotParams
{
public:
  AREXPORT ArRobotP2DF();
  AREXPORT virtual ~ArRobotP2DF() = default;
};

class ArRobotP2D8 : public ArRobotParams
{
public:
  AREXPORT ArRobotP2D8();
  AREXPORT virtual ~ArRobotP2D8() = default;
};

class ArRobotP2D8Plus : public ArRobotParams
{
public:
  AREXPORT ArRobotP2D8Plus();
  AREXPORT virtual ~ArRobotP2D8Plus() = default;
};

class ArRobotP2CE : public ArRobotParams
{
public:
  AREXPORT ArRobotP2CE();
  AREXPORT virtual ~ArRobotP2CE() = default;
};

class ArRobotP2PP : public ArRobotParams
{
public:
  AREXPORT ArRobotP2PP();
  AREXPORT virtual ~ArRobotP2PP() = default;
};

class ArRobotP2PB : public ArRobotParams
{
public:
  AREXPORT ArRobotP2PB();
  AREXPORT virtual ~ArRobotP2PB() = default;
};


class ArRobotP3AT : public ArRobotParams
{
public:
  AREXPORT ArRobotP3AT();
  AREXPORT virtual ~ArRobotP3AT() = default;
};


class ArRobotP3DX : public ArRobotParams
{
public:
  AREXPORT ArRobotP3DX();
  AREXPORT virtual ~ArRobotP3DX() = default;
};

class ArRobotPerfPB : public ArRobotParams
{
public:
  AREXPORT ArRobotPerfPB();
  AREXPORT virtual ~ArRobotPerfPB() = default;
};

class ArRobotPerfPBPlus : public ArRobotParams
{
public:
  AREXPORT ArRobotPerfPBPlus();
  AREXPORT virtual ~ArRobotPerfPBPlus() = default;
};

class ArRobotPion1M : public ArRobotParams
{
public:
  AREXPORT ArRobotPion1M();
  AREXPORT virtual ~ArRobotPion1M() = default;
};

class ArRobotPion1X : public ArRobotParams
{
public:
  AREXPORT ArRobotPion1X();
  AREXPORT virtual ~ArRobotPion1X() = default;
};

class ArRobotPsos1M : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos1M();
  AREXPORT virtual ~ArRobotPsos1M() = default;
};

class ArRobotPsos43M : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos43M();
  AREXPORT virtual ~ArRobotPsos43M() = default;
};

class ArRobotPsos1X : public ArRobotParams
{
public:
  AREXPORT ArRobotPsos1X();
  AREXPORT virtual ~ArRobotPsos1X() = default;
};

class ArRobotPionAT : public ArRobotParams
{
public:
  AREXPORT ArRobotPionAT();
  AREXPORT virtual ~ArRobotPionAT() = default;
};

class ArRobotMapper : public ArRobotParams
{
public:
  AREXPORT ArRobotMapper();
  AREXPORT virtual ~ArRobotMapper() = default;
};

class ArRobotPowerBot : public ArRobotParams
{
public:
  AREXPORT ArRobotPowerBot();
  AREXPORT virtual ~ArRobotPowerBot() = default;
};

class ArRobotP3DXSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3DXSH();
  AREXPORT virtual ~ArRobotP3DXSH() = default;
};

class ArRobotP3ATSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3ATSH();
  AREXPORT virtual ~ArRobotP3ATSH() = default;
};

class ArRobotP3ATIWSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotP3ATIWSH();
  AREXPORT virtual ~ArRobotP3ATIWSH() = default;
};

class ArRobotPatrolBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPatrolBotSH();
  AREXPORT virtual ~ArRobotPatrolBotSH() = default;
};

class ArRobotPeopleBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPeopleBotSH();
  AREXPORT virtual ~ArRobotPeopleBotSH() = default;
};

class ArRobotPowerBotSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotPowerBotSH();
  AREXPORT virtual ~ArRobotPowerBotSH() = default;
};

class ArRobotWheelchairSH : public ArRobotParams
{
 public:
  AREXPORT ArRobotWheelchairSH();
  AREXPORT virtual ~ArRobotWheelchairSH() = default;
};

class ArRobotPowerBotSHuARCS : public ArRobotParams
{
 public:
  AREXPORT ArRobotPowerBotSHuARCS();
  AREXPORT virtual ~ArRobotPowerBotSHuARCS() = default;
};

class ArRobotSeekur : public ArRobotParams
{
 public:
  AREXPORT ArRobotSeekur();
  AREXPORT virtual ~ArRobotSeekur() = default;
};

/// @since Aria 2.7.2
class ArRobotMT400 : public ArRobotParams
{
 public:
  AREXPORT ArRobotMT400();
  AREXPORT virtual ~ArRobotMT400() = default;
};

/// @since Aria 2.7.2
class ArRobotResearchPB : public ArRobotParams
{
 public:
  AREXPORT ArRobotResearchPB();
  AREXPORT virtual ~ArRobotResearchPB() = default;
};

/// @since Aria 2.7.2
class ArRobotSeekurJr : public ArRobotParams
{
 public:
  AREXPORT ArRobotSeekurJr();
  AREXPORT virtual ~ArRobotSeekurJr() = default;
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
  AREXPORT virtual ~ArRobotPioneerLX() = default;
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