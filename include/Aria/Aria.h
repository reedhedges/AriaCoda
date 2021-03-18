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

#ifndef ARIA_H
#define ARIA_H


// This file includes all headers defining the ARIA public interface. 
// Internal classes should be omitted.

#include "Aria/ariaOSDef.h"
#include "Aria/ariaTypedefs.h"
#include "Aria/ArSerialConnection.h"
#include "Aria/ArTcpConnection.h"
#include "Aria/ArLog.h"
//#include "Aria/ArRobotPacket.h"
//#include "Aria/ArRobotPacketSender.h"
//#include "Aria/ArRobotPacketReceiver.h"
//#include "Aria/ArRobotConfigPacketReader.h"
//#include "Aria/ArRobotTypes.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArArgumentBuilder.h"
#include "Aria/ArArgumentParser.h"
#include "Aria/ArFileParser.h"
#include "Aria/ArConfig.h"
#include "Aria/ArConfigArg.h"
#include "Aria/ArConfigGroup.h"
#include "Aria/ArRobot.h"
#include "Aria/ArCommands.h"
#include "Aria/ArJoyHandler.h"
#include "Aria/ArSyncTask.h"
#include "Aria/ArTaskState.h"
#include "Aria/ariaInternal.h"
#include "Aria/ArSonarDevice.h"
#include "Aria/ArPriorityResolver.h"
#include "Aria/ArAction.h"
#include "Aria/ArActionGroup.h"
#include "Aria/ArActionGroups.h"
#include "Aria/ArActionDeceleratingLimiter.h"
#include "Aria/ArActionLimiterForwards.h"
#include "Aria/ArActionLimiterBackwards.h"
#include "Aria/ArActionLimiterTableSensor.h"
#include "Aria/ArActionBumpers.h"
#include "Aria/ArActionIRs.h"
#include "Aria/ArActionStallRecover.h"
#include "Aria/ArActionAvoidFront.h"
#include "Aria/ArActionAvoidSide.h"
#include "Aria/ArActionConstantVelocity.h"
#include "Aria/ArActionInput.h"
#include "Aria/ArActionRobotJoydrive.h"
#include "Aria/ArActionJoydrive.h"
#include "Aria/ArActionKeydrive.h"
#include "Aria/ArActionTriangleDriveTo.h"
#include "Aria/ArActionTurn.h"
#include "Aria/ArActionRatioInput.h"
#include "Aria/ArActionStop.h"
#include "Aria/ArActionGoto.h"
#include "Aria/ArRecurrentTask.h"
#include "Aria/ArInterpolation.h"
#include "Aria/ArGripper.h"
#include "Aria/ArRVisionPTZ.h"
#include "Aria/ArLaserLogger.h"
#include "Aria/ArKeyHandler.h"
#include "Aria/ArDPPTU.h"
#include "Aria/ArVCC4.h"
#include "Aria/ArSignalHandler.h"
#include "Aria/ArAnalogGyro.h"
#include "Aria/ArMapInterface.h"
#include "Aria/ArMapObject.h"
#include "Aria/ArMap.h"
#include "Aria/ArLineFinder.h"
#include "Aria/ArBumpers.h"
#include "Aria/ArIRs.h"
#include "Aria/ArDrawingData.h"
#include "Aria/ArForbiddenRangeDevice.h"
//#include "Aria/ArTCM2.h"
#if !defined(WIN32) && !defined(SWIGWIN)
#include "Aria/ArMTXIO.h"
#endif
#include "Aria/ArActionGotoStraight.h"
#include "Aria/ArDataLogger.h"
#include "Aria/ArRobotJoyHandler.h"
#include "Aria/ArRatioInputKeydrive.h"
#include "Aria/ArRatioInputJoydrive.h"
#include "Aria/ArRatioInputRobotJoydrive.h"
#include "Aria/ArActionMovementParameters.h"
#include "Aria/ArSoundPlayer.h"
#include "Aria/ArSoundsQueue.h"
//#include "Aria/ArCameraCollection.h"
//#include "Aria/ArCameraCommands.h"
#include "Aria/ArStringInfoGroup.h"
#include "Aria/ArSonarAutoDisabler.h"
#include "Aria/ArActionDriveDistance.h"
//#include "Aria/ArLaserReflectorDevice.h"
//#include "Aria/ArRobotConfig.h"
//#include "Aria/ArTCMCompassRobot.h"
//#include "Aria/ArTCMCompassDirect.h"
#include "Aria/ArLaserFilter.h"
#include "Aria/ArUrg.h"
#include "Aria/ArGPS.h"
#include "Aria/ArTrimbleGPS.h"
#include "Aria/ArNovatelGPS.h"
#include "Aria/ArGPSCoords.h"
#include "Aria/ArLaser.h"
#include "Aria/ArRobotConnector.h"
#include "Aria/ArLaserConnector.h"
#include "Aria/ArSonarConnector.h"
#include "Aria/ArBatteryConnector.h"
#include "Aria/ArLCDConnector.h"
#include "Aria/ArSonarMTX.h"
#include "Aria/ArBatteryMTX.h"
#include "Aria/ArLCDMTX.h"
#include "Aria/ArSimulatedLaser.h"
#include "Aria/ArExitErrorSource.h"
#include "Aria/ArActionLimiterRot.h"
#include "Aria/ArRobotBatteryPacketReader.h"
#include "Aria/ArLMS1XX.h"
#include "Aria/ArUrg_2_0.h"
#include "Aria/ArActionMovementParametersDebugging.h"
//#include "Aria/ArZippable.h"
#include "Aria/ArS3Series.h"
#include "Aria/ArSZSeries.h"
//#include "Aria/ArRobotPacketReaderThread.h"
//#include "Aria/ArHasFileName.h"

#endif // ARIA_H
