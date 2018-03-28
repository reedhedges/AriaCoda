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

#include "Aria.h"
#include <assert.h>

struct VelParams {
  int transVel, transNegVel, transAccel, transDecel, rotVel, rotAccel, rotDecel, latVel, latAccel, latDecel;
  void assertEqual(const VelParams &p)
  {
    assert(p.transVel == transVel);
    assert(p.transNegVel == transNegVel);
    assert(p.transAccel == transAccel);
    assert(p.transDecel == transDecel);
    assert(p.rotVel == rotVel);
    assert(p.rotAccel == rotAccel);
    assert(p.rotDecel == rotDecel);
    assert(p.latVel == latVel);
    assert(p.latAccel == latAccel);
    assert(p.latDecel == latDecel);
  }
};

VelParams SentMax, RecdMax;


bool handleAbsoluteMaxesReplyPacket(ArRobotPacket *pkt)
{
  if(pkt->getID() != 213) return false;
  ArLog::log(ArLog::Normal, "Received ABSOLUTE_MAXES reply with:");
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransVel=%d (SentMax %d)", RecdMax.transVel= pkt->bufToByte2(), SentMax.transVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransNegVel=%d (SentMax %d)", RecdMax.transNegVel = pkt->bufToByte2(), SentMax.transNegVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransAccel=%d (SentMax %d)", RecdMax.transAccel = pkt->bufToByte2(), SentMax.transAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransDecel=%d (SentMax %d)", RecdMax.transDecel = pkt->bufToByte2(), SentMax.transDecel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotVel=%d (SentMax %d)", RecdMax.rotVel = pkt->bufToByte2(), SentMax.rotVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotAccel=%d (SentMax %d)", RecdMax.rotAccel = pkt->bufToByte2(), SentMax.rotAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotDecel=%d (SentMax %d)", RecdMax.rotDecel = pkt->bufToByte2(), SentMax.rotDecel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatVel=%d (SentMax %d)", RecdMax.latVel = pkt->bufToByte2(), SentMax.latVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatAccel=%d (SentMax %d)", RecdMax.latAccel = pkt->bufToByte2(), SentMax.latAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatDecel=%d (SentMax %d)", RecdMax.latDecel = pkt->bufToByte2(), SentMax.latDecel);
  SentMax.assertEqual(RecdMax);
  return true;
}

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "testAbsoluteMaxesCommand: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "testAbsoluteMaxesCommand: Connected.");

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  ArUtil::sleep(500);
  ArLog::log(ArLog::Normal, "Initial ARIA defaults for absolute maximums:");
  robot.lock();
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransVel=%d", robot.getAbsoluteMaxTransVel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransNegVel=%d", robot.getAbsoluteMaxTransNegVel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransAccel=%d", robot.getAbsoluteMaxTransAccel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransDecel=%d", robot.getAbsoluteMaxTransDecel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotVel=%d", robot.getAbsoluteMaxRotVel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotAccel=%d", robot.getAbsoluteMaxRotAccel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotDecel=%d", robot.getAbsoluteMaxRotDecel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatVel=%d", robot.getAbsoluteMaxLatVel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatAccel=%d", robot.getAbsoluteMaxLatAccel());
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatDecel=%d", robot.getAbsoluteMaxLatDecel());
  robot.unlock();

  // Send ABSOLUTE_MAXES
  ArRobotPacket pkt;
  pkt.setID(ArCommands::ABSOLUTE_MAXES);
  pkt.byte2ToBuf(SentMax.transVel = 500);  
  pkt.byte2ToBuf(SentMax.transNegVel = 500);
  pkt.byte2ToBuf(SentMax.transAccel = 200);
  pkt.byte2ToBuf(SentMax.transDecel = 200);
  pkt.byte2ToBuf(SentMax.rotVel = 50);
  pkt.byte2ToBuf(SentMax.rotAccel = 20);
  pkt.byte2ToBuf(SentMax.rotDecel = 20);
  pkt.byte2ToBuf(SentMax.latVel = 500);
  pkt.byte2ToBuf(SentMax.latAccel = 200);
  pkt.byte2ToBuf(SentMax.latDecel = 200);
  pkt.finalizePacket();
  ArLog::log(ArLog::Normal, "Sending ABSOLUTE_MAXES command with:");
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransVel=%d", SentMax.transVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransNegVel=%d", SentMax.transNegVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransAccel=%d", SentMax.transAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxTransDecel=%d", SentMax.transDecel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotVel=%d", SentMax.rotVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotAccel=%d", SentMax.rotAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxRotDecel=%d", SentMax.rotDecel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatVel=%d", SentMax.latVel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatAccel=%d", SentMax.latAccel);
  ArLog::log(ArLog::Normal, "\tAbsoluteMaxLatDecel=%d", SentMax.latDecel);
  robot.lock();
  robot.getPacketSender()->sendPacket(&pkt);
  robot.unlock();



  ArLog::log(ArLog::Normal, "testAbsoluteMaxesCommand: Waiting for robot disconnect...");
  robot.waitForRunExit();
  ArLog::log(ArLog::Normal, "testAbsoluteMaxesCommand: Exiting.");
  Aria::exit(0);
  return 0;
}
