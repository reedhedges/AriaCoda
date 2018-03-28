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

/* tests mobilesim-only commands, and some robot commands that can be 
 * particularly tricky in mobilesim (e.g. HEAD and MOVE).
 */


#include "Aria.h"

#include <assert.h>

bool log_simstat = true;

unsigned int simint, realint, lastint;
int realX, realY, realTh, realZ;
double lat, lon, alt;
double errorx, errory, errorth;

bool handleSimStatPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 0x62) return false;
  if(log_simstat) printf("----------- SIMSTAT pkt received: ------------\n");
  char a = pkt->bufToByte();  // unused byte
  char b = pkt->bufToByte();  // unused byte
  ArTypes::UByte4 flags = pkt->bufToUByte4();
  if(log_simstat) printf("\tFlags=0x%x  (Georef? %s, OdomError? %s)\n", flags, (flags&ArUtil::BIT1)?"yes":"no", (flags&ArUtil::BIT2)?"yes":"no");
  simint = pkt->bufToUByte2();
  realint = pkt->bufToUByte2();
  lastint = pkt->bufToUByte2();
  if(log_simstat) printf("\tSimInterval=%d, RealInterval=%d, LastInterval=%d.\n", simint, realint, lastint);
  realX = pkt->bufToByte4();
  realY = pkt->bufToByte4();
  realZ = pkt->bufToByte4();
  realTh = pkt->bufToByte4();
  if(log_simstat) printf("\tTrue Pose = (%d, %d, %d, %d)\n", realX, realY, realZ, realTh);
  lat = pkt->bufToByte4() / 10e6;
  lon = pkt->bufToByte4() / 10e6;
  alt = pkt->bufToByte4() / 100.0;
  if(log_simstat) printf("\tGeo Pose = (%f, %f, %f)\n", lat, lon, alt);
  /*
    This stuff has been moved to sim device data packet
  unsigned int numDevs = pkt->bufToUByte4();
  printf("\t%d Devices:\n", numDevs);
  for(unsigned int i = 0; i < numDevs; i++)
  {
    memset(s1, 0, 24);
    memset(s2, 0, 24);
    pkt->bufToStr(s1, 23); 
    pkt->bufToStr(s2, 23); 
    unsigned int which = (unsigned int) pkt->bufToUByte();
    unsigned int status = pkt->bufToUByte4();
    printf("\t\t%s is %s #%d (status 0x%x)\n", s1, s2, which, status);
  }
  */
  errorx = pkt->bufToByte4() / 10e6;
  errory = pkt->bufToByte4()  / 10e6;
  errorth = pkt->bufToByte4() / 10e6;
  if(log_simstat) printf("\tOdom Error = (%f, %f, %f)\n", errorx, errory, errorth);
  if(log_simstat) printf("-------------------------------------------------\n");
  return true;
}

bool handleSimDeviceData(ArRobotPacket* pkt)
{
  if(pkt->getID() != 100) return false;
  printf("------ SIM DEVICE DATA packet received: ---------\n");
  char name[24];
  char type[24];
  int idx;
  pkt->bufToStr(name, 23);
  pkt->bufToStr(type, 23);
  idx = pkt->bufToUByte2();
  printf("Data for %s which is #%d of type %s.\n", name, idx, type);
  if(strcmp(type, "gps") == 0)
  {
    double lat, lon, dop;
    lat = (double)pkt->bufToByte4() / (double)10e6;
    lon = (double)pkt->bufToByte4() / (double)10e6;
    dop = (double)pkt->bufToByte2() / 1000.0;
    printf("GPS Lat=%f, Lon=%f, DOP=%f\n", lat, lon, dop);
  }
  printf("-------------------------------------------------\n");
  return true;
}

ArCondition gotSimMapChangedPacketCondition;
bool handleSimMapChangedPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 102) return false;
  printf("------- SIM MAP CHANGED packet received: --------\n");
  char name[256];
  int user = pkt->bufToUByte();
  int really_loaded = pkt->bufToUByte();
  if(user == 1)
    printf("user=%d: Loaded by user GUI\n", user);
  else
  {
    printf("user=%d: Loaded by client program\n", user);
    if(really_loaded)
      printf("loaded=%d: A new map was loaded.\n", really_loaded);
    else
      printf("loaded=%d: Map already loaded, not reloaded.\n", really_loaded);
  }
  pkt->bufToStr(name, 256);
  printf("filename=%s\n", name);
  printf("-------------------------------------------------\n");
  gotSimMapChangedPacketCondition.signal();
  return true;
}
  

ArCondition gotConfigPacketCondition;
bool handleConfigPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 0x20) return false;
  printf("----------- CONFIG pkt received: ------------\n");
  char buf[256];
  pkt->bufToStr(buf, sizeof(buf));
  printf("Type=%s\n", buf);
  pkt->bufToStr(buf, sizeof(buf));
  printf("Subtype=%s\n", buf);
  pkt->bufToStr(buf, sizeof(buf));
  printf("SerialNumber=%s\n", buf);
  printf("unknown=%d\n", pkt->bufToUByte());
  printf("RotVelTop=%d\n", pkt->bufToUByte2());
  printf("TransVelTop=%d\n", pkt->bufToUByte2());
  printf("RotAccelTop=%d\n", pkt->bufToUByte2());
  printf("TransAccelTop=%d\n", pkt->bufToUByte2());
  printf("(PWMMax=%d)\n", pkt->bufToUByte2());
  pkt->bufToStr(buf, sizeof(buf));
  printf("Name=%s\n", buf);
  printf("SipCycleTime=%d\n", pkt->bufToUByte());
  printf("(HostBaud=%d)\n", pkt->bufToUByte());
  printf("(Aux1Baud=%d)\n", pkt->bufToUByte());
  printf("HasGripper=%d\n", pkt->bufToUByte2());
  printf("HasFrontSonar=%d\n", pkt->bufToUByte2());
  printf("HasRearSonar=%d\n", pkt->bufToUByte());
  printf("LowBattery=%d\n", pkt->bufToUByte2());
  printf("(RevCount=%d)\n", pkt->bufToUByte2());
  printf("Watchdog=%d\n", pkt->bufToUByte2());
  printf("NormalMPacs=%d\n", pkt->bufToUByte());
  printf("StallVal=0x%X\n", pkt->bufToUByte2());
  printf("StallCount=%d\n", pkt->bufToUByte2());
  printf("(JoyVel=%d)\n", pkt->bufToUByte2());
  printf("(JoyRotVel=%d)\n", pkt->bufToUByte2());
  printf("RotVelMax=%d\n", pkt->bufToUByte2());
  printf("TransVelMax=%d\n", pkt->bufToUByte2());
  printf("RotAccel=%d\n", pkt->bufToUByte2());
  printf("RotDecel=%d\n", pkt->bufToUByte2());
  printf("(RotKP=%d)\n", pkt->bufToUByte2());
  printf("(RotKV=%d)\n", pkt->bufToUByte2());
  printf("(RotKI=%d)\n", pkt->bufToUByte2());
  printf("TransAccel=%d\n", pkt->bufToUByte2());
  printf("TransDecel=%d\n", pkt->bufToUByte2());
  printf("(TransKP=%d)\n", pkt->bufToUByte2());
  printf("(TransKV=%d)\n", pkt->bufToUByte2());
  printf("(TransKI=%d)\n", pkt->bufToUByte2());
  printf("FrontBumps=%d\n", pkt->bufToUByte());
  printf("RearBumps=%d\n", pkt->bufToUByte());
  printf("HasCharger=%d\n", pkt->bufToUByte());
  printf("SonarCycle=%d\n", pkt->bufToUByte());
  printf("ResetBaud=%d\n", pkt->bufToUByte());
  printf("HasGyro=%d\n", pkt->bufToUByte());
  printf("(DriftFactor=%d)\n", pkt->bufToUByte2());
  printf("(Aux2Baud=%d)\n", pkt->bufToUByte());
  printf("(Aux3Baud=%d)\n", pkt->bufToUByte());
  printf("(TicksPerMM=%d)\n", pkt->bufToUByte2());
  printf("(ShutdownVoltage=%d)\n", pkt->bufToUByte2());
  pkt->bufToStr(buf, sizeof(buf));
  printf("FirmwareVersion=%s\n", buf);
  printf("(ChargeThreshold=%d)\n", pkt->bufToUByte2());
  gotConfigPacketCondition.signal();
  return true;

}

class CheckPositionTask 
{
  ArRobot *robot;
  ArFunctorC<CheckPositionTask> taskCB;
  ArTime lastCheck;
  ArPose oldpose, newpose, corner;
  double expectedDist;
  double speed;
  int time;
  double turndist;
public:
  CheckPositionTask(ArRobot *_robot, double _speed, int _time, double _turndist) :
    robot(_robot), taskCB(this, &CheckPositionTask::checkPosition),
    expectedDist(_speed * ((double)_time/1000.0)), speed(_speed), time(_time), turndist(_turndist)
  {
    oldpose.setPose(realX, realY, realTh);
    corner = oldpose;
    printf("Speed=%.0f m/sec, Check interval=%d ms, Expected distance per check interval=%0.f mm, Sim Interval=%d ms\n", speed, time, expectedDist, realint);
    printf("XPos\tYPos\tOldX\tOldY\tVelRptd\tPosDiff\tVelErr\tPosErr\tLastInt\tSimIntervalErr\n");
  }
  void addTask()
  {
  puts("addTask()");
    robot->addSensorInterpTask("testMobileSim::CheckPositionTask", ArListPos::LAST, &taskCB);
  }
  void checkPosition()
  {
  puts("checkPosition()");
  fflush(stdout);
    if(lastCheck.mSecSince() >= time)
    {
      lastCheck.setToNow();
      newpose.setPose(realX, realY, realTh);
      double dist = newpose.findDistanceTo(oldpose);
      double poserr = fabs( dist - expectedDist );
      printf("%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.1f\t%u\t%.0f", newpose.getX(), newpose.getY(), oldpose.getX(), oldpose.getY(), robot->getVel(), dist, fabs(speed-robot->getVel()), poserr, lastint, fabs((double)realint-(double)lastint));
      if(poserr > 25)
        printf("\t!!");
      if(poserr > 50)
        printf("!!");
      if(poserr > 100)
      {
        printf("!!");
      }
      if(poserr > 250)
      {
        robot->comStr(ArCommands::SIM_MESSAGE, "Warning, client detected big jump (>100)!");
        printf("!!");
      }
      puts(""); fflush(stdout);
      if(poserr > 500)
      {
        puts("Distance error is too much (more than 500 mm)!! Stopping here");
        robot->comStr(ArCommands::SIM_MESSAGE, "Jump was too big (>500)! Stopping here.");
        robot->stop();
        return; 
      }
      oldpose = newpose;
      if(newpose.findDistanceTo(corner) >= turndist)
      {
        puts("   [turning]");
        corner = newpose;
        robot->setDeltaHeading(-90);
      }
    }
  }
};

class CheckRotationTask
{
  void checkRotation()
  {
  }
};


/* * TODO: Register with Aria as an argument parser. * */

void usageerror()
{
  printf("usage: testMobileSim <options>\n"
      "               or\n"\
      "          testMobileSim --all --message <message to send> --message-sequence --newmap <name of mapfile>\n"\
      "testMobileSim options:\n" \
      " --stayconnected --simstat --message <message to send> --setpose <X,Y,TH>\n"
      " --move --vel2 --exit --newmap|--master-newmap <name of mapfile> --config\n"
      " --gps --position --rotation --die-during-sync --slow-sync --crash\n"
      " --sleep-after-newmap --logstate <message to send>\n"
      "Note: --stayconnected, --position and --rotation enable a persistent mode that does not exit.\n\n"
      );
  Aria::logOptions();
  exit(1);
}

int main(int argc, char **argv)
{
  const int test_simstat = 1;
  const int test_message = 1<<1;
  const int test_setpose = 1<<2;
  const int test_move = 1<<3;
  const int test_vel2 = 1<<4;
  const int test_exit = 1<<5;
  const int test_newmap = 1<<6;
  const int test_seta = 1<<7;
  const int test_config = 1<<8;
  const int test_old_broken_setpose = 1<<9;
  const int test_gps = 1<<10;
  const int test_position = 1<<11;
  const int test_rotation = 1<<12;
  const int test_die_during_sync = 1<<13;
  const int test_slow_sync = 1<<14;
  const int test_crash = 1<<15;
  const int test_message_sequence = 1<<16;
  const int test_hang_during_sync = 1<<17;
  const int test_logstate = 1<<18;

  int testmask = 0;
  char* mapfile = NULL;
  bool master_newmap = false;
  char* message = NULL;
  char* logstate_msg = NULL;
  int32_t X = 0;
  int32_t Y = 0;
  int32_t Th = 0;
  bool stayConnected = false;
  bool simdevicepacket = false;
  bool sleep_after_newmap = false;

  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);
  ArArgumentParser parser(&argc, argv);
  ArSimpleConnector connector(&parser);

  if(parser.checkArgument("--help") || parser.checkArgument("-help"))
    usageerror();
  mapfile = parser.checkParameterArgument("--newmap");
  if(mapfile)
  {
    if(strlen(mapfile) == 0)
      usageerror();
    printf("mapfile will be \"%s\"\n", mapfile);
    testmask |= test_newmap;
  }
  master_newmap = parser.checkArgument("--master-newmap");
  sleep_after_newmap =  parser.checkArgument("--sleep-after-newmap");

  message = parser.checkParameterArgument("--message");
  if(message)
  {
    if(strlen(message) == 0)
      usageerror();
    printf("message will be \"%s\"\n", message);
    testmask |= test_message;
  }

  if(parser.checkArgument("--simstat"))
    testmask |= test_simstat;
  char* posestr = parser.checkParameterArgument("--setpose");
  if(posestr)
  {
    if(strlen(posestr) == 0)
      usageerror();
    sscanf(posestr, "%d,%d,%d", &X, &Y, &Th);
    printf("setpose: X=%d, Y=%d, Th=%d.\n", X, Y, Th);
    testmask |= test_setpose;
  }
  if(parser.checkArgument("--move"))
    testmask |= test_move;
  if(parser.checkArgument("--vel2"))
    testmask |= test_vel2;
  if(parser.checkArgument("--exit"))
    testmask |= test_exit;
  if(parser.checkArgument("--seta"))
    testmask |= test_seta;
  if(parser.checkArgument("--config"))
    testmask |= test_config;
  if(parser.checkArgument("--broken-setpose"))
    testmask |= test_old_broken_setpose;
  if(parser.checkArgument("--gps"))
  {
    testmask |= test_gps;
    simdevicepacket = true;
  }
  if(parser.checkArgument("--position"))
  {
    testmask |= test_position;
  }
  if(parser.checkArgument("--rotation"))
  {
    testmask |= test_rotation;
  }
  if(parser.checkArgument("--die-during-sync"))
  {
    testmask |= test_die_during_sync;
  }
  if(parser.checkArgument("--slow-sync"))
  {
    testmask |= test_slow_sync;
  }
  if(parser.checkArgument("--crash"))
  {
    testmask |= test_crash;
  }
  if(parser.checkArgument("--message-sequence"))
  {
    testmask |= test_message_sequence;
  }
  if(parser.checkArgument("--hang-during-sync"))
  {
    testmask |= test_hang_during_sync;
  }

  logstate_msg = parser.checkParameterArgument("--logstate");
  if(logstate_msg)
  {
    testmask |= test_logstate;
  }
  
  if(parser.checkArgument("--all"))
  {
    if(!message || !mapfile)
      usageerror();
    testmask = 0xFF;
  }

  if(parser.checkArgument("--stayconnected"))
    stayConnected = true;
      


  ArRobot robot;
  robot.setLogMovementSent(true);

  ArSick laser;
  robot.addRangeDevice(&laser);

  if (!connector.parseArgs())
  {
    connector.logOptions();
    return 1;
  }


  if(testmask & test_die_during_sync)
  {
    connector.setupRobot(&robot);
    robot.asyncConnect();
    ArUtil::sleep(20);
    abort();
  }
  else if(testmask & test_slow_sync)
  {
    connector.setupRobot(&robot);
    ArDeviceConnection *devCon = robot.getDeviceConnection();
    assert(devCon->openSimple());
    for(int syncSeq = 0; syncSeq < 3; ++syncSeq)
    {
      for(int i = 0; i < 3; ++i)
      {
        printf("Sending SYNC%d packet...\n", syncSeq);
        ArRobotPacket pkt;
        pkt.setID(syncSeq);
        pkt.finalizePacket();
        devCon->writePacket(&pkt);
        ArRobotPacketReceiver rec;
        rec.setDeviceConnection(devCon);
        ArRobotPacket *replyPkt = rec.receivePacket(1000);
        assert(replyPkt);
        printf("Got reply to SYNC%d: SYNC%d.\n", syncSeq, replyPkt->getID());
        if(replyPkt->getID() == 2)
        {
          printf("Got SYNC2 with length %d.\n", replyPkt->getDataLength());
          exit(0);
        }
      }
    }
    puts("Hey, did we never receive SYNC2? ");
    exit(-1);
  }
  else if(testmask & test_hang_during_sync)
  {
    connector.setupRobot(&robot);
    robot.asyncConnect();
    ArUtil::sleep(20);
    puts("Hanging (by locking ArRobot indefinitely and then doing nothing forever)...");
    robot.lock();
    while(true)
      ArUtil::sleep(1000000);
  }
  else
  {
    if (!connector.connectRobot(&robot))
    {
      printf("Could not connect to simulated robot... exiting\n");
      return 2;
    }
  }
  printf("Connected to robot.\n");
  robot.runAsync(true);

  printf("running laser.\n");
  connector.setupLaser(&laser);
  laser.runAsync();
  printf("connecting to laser.\n");
  if(!laser.blockingConnect())
  {
    printf("Could not connect to simulated laser. That's OK, maybe it doesn't have one.\n");
  }

  robot.enableMotors();

  ArGlobalRetFunctor1<bool, ArRobotPacket*> mySimDeviceDataPacketHandler(&handleSimDeviceData);
  if(simdevicepacket) {
    puts("adding simdevicedata packet handler...");
    robot.addPacketHandler(&mySimDeviceDataPacketHandler);
  }

  ArGlobalRetFunctor1<bool, ArRobotPacket*> mySimStatPacketHandler(&handleSimStatPacket);
  if(testmask & test_simstat)
  {
    printf("** SIMSTAT **\n");
    robot.addPacketHandler(&mySimStatPacketHandler);
    printf("-> SIM_STAT 1...\n");
    robot.comInt(ArCommands::SIM_STAT, 1);
    printf("Request sent. sleeping for 4 seconds...\n");
    ArUtil::sleep(4000);
    robot.remPacketHandler(&mySimStatPacketHandler);
  }

  if(testmask & test_gps)
  {
    printf("** GPS request **\n");
    struct gpscmd {
      ArTypes::Byte enable;
      char gpsstr[4];
      ArTypes::UByte2 idx;
    };
    gpscmd c;
    c.enable = 1;
    strcpy(c.gpsstr, "gps");
    c.idx = 0;
    printf("-> 229 %d, \"%s\", %d...\n", c.enable, c.gpsstr, c.idx);
    robot.comDataN(229, (const char*)&c, sizeof(c)); 
  }

  if(testmask & test_message)
  {
    printf("** Message **\n");
    printf("-> SIM_MESSAGE \"%s\"...\n", message);
    robot.comStr(ArCommands::SIM_MESSAGE, message);
    printf("Message command sent (message length %d bytes).\n\n", strlen(message));
  }

  if(testmask & test_message_sequence)
  {
    puts("** Message sequence (1..20)");
    char buf[8];
    for(int i = 1; i <= 20; ++i)
    {
      snprintf(buf, 8, "seq%d", i);
      printf("-> SIM_MESSAGE \"%s\"...\n", buf);
      robot.comStr(ArCommands::SIM_MESSAGE, buf);
      printf("Message command sent (message length %d bytes).\n\n", strlen(buf));
    }
  }

  if(testmask & test_old_broken_setpose)
  {
    /* OLD WAY:*/
    printf("** SIM_SET_POSE with broken argument packing **\n");
    int16_t cmd[6];
    cmd[1] = (int16_t) X;
    cmd[0] = (int16_t) (X >> 16);

    cmd[3] = (int16_t) Y;
    cmd[2] = (int16_t) (Y >> 16);

    cmd[5] = (int16_t) Th;
    cmd[4] = (int16_t) (Th >> 16);
    printf("-> SIM_SET_POSE,\n");
    printf("\t x[0] = %d, x[1] = %d;    y[0] = %d, y[1] = %d;    th[0] = %d, th[1] = %d\n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
    robot.comDataN(ArCommands::SIM_SET_POSE, (char*)cmd, sizeof(cmd));
    printf("Command sent.\n\n");
  }
    
  if(testmask & test_setpose)
  {
    printf("** SIM_SET_POSE **\n");

    ArRobotPacket pkt;
    pkt.setID(ArCommands::SIM_SET_POSE);
    pkt.uByteToBuf(0);
    pkt.byte4ToBuf(X);
    pkt.byte4ToBuf(Y);
    pkt.byte4ToBuf(Th);
    pkt.finalizePacket();
    printf("-> SIM_SET_POSE %d %d %d\n", X, Y, Th);
    robot.getDeviceConnection()->write(pkt.getBuf(), pkt.getLength());
    printf("Command sent.\n\n");
  }

  if(testmask & test_seta)
  {
    puts("** SETA test **");
    if(robot.hasSettableAccsDecs())
      puts("ArRobot thinks this robot has settable acceleration and deceleration.");
    else
      puts("WARNING ArRobot does not think that this robot has settable acceleration and deceleration.");
    //puts("-> SETA 500...");
    //robot.comInt(ArCommands::SETA, 500);
    //puts("Command sent.\n");
    puts("-> SETA 500 (via ArRobot)...");
    robot.setTransAccel(500);
    puts("TransAccel set in ArRobot. Will be sent next state reflection cycle.  Waiting 300ms for cycles to happen...\n");
    ArUtil::sleep(300);
  }

  if(testmask & test_move)
  {
    printf("** MOVE test **\n");
    printf("-> MOVE 1000\n");
    robot.lock();
    robot.move(1000);
    robot.unlock();
    printf("(sleep 4 sec)...\n"); ArUtil::sleep(4000);
    printf("-> MOVE -2000\n");
    robot.move(-2000);
    printf("(sleep 5 sec)...\n"); ArUtil::sleep(5000);
    printf("-> MOVE -2000\n");
    robot.move(-2000);
    printf("(sleep 5 sec)...\n"); ArUtil::sleep(5000);
    printf("-> MOVE 1\n");
    robot.move(1);
    printf("(sleep 2 sec)...\n"); ArUtil::sleep(2000);
    printf("-> MOVE 0\n");
    robot.move(0);
    printf("(sleep 2 sec)...\n"); ArUtil::sleep(2000);
    printf("-> MOVE 500\n");
    robot.move(500);
    printf("(sleep 3 sec)...\n"); ArUtil::sleep(3000);
    printf("-> MOVE 500\n");
    robot.move(500);
    printf("(sleep 3 sec)...\n"); ArUtil::sleep(3000);
    printf("-> MOVE 500\n");
    robot.move(500);
    printf("(sleep 3 sec)...\n"); ArUtil::sleep(3000);
    printf("\n");
  }

  if(testmask & test_vel2)
  {
    printf("** VEL2 Tests **\n");
    printf("-> VEL2 100 100...\n");
    robot.setVel2(100, 100);
    ArUtil::sleep(4000);
    printf("-> VEL2 -100 100...\n");
    robot.setVel2(-100, 100);
    ArUtil::sleep(4000);
    printf("-> VEL2 100 -100...\n");
    robot.setVel2(100, -100);
    ArUtil::sleep(4000);
    printf("-> VEL2 150 100...\n");
    robot.setVel2(200, 100);
    ArUtil::sleep(4000);
    printf("-> VEL2 -200 -200...\n");
    robot.setVel2(-200, -200);
    ArUtil::sleep(4000);
    printf("-> VEL2 0 0...\n");
    robot.setVel2(0, 0);
    ArUtil::sleep(4000);
    printf("-> stop...\n");
    robot.stop();
    printf("\n\n");
  }

  if(testmask & test_newmap)
  {
    ArGlobalRetFunctor1<bool, ArRobotPacket*> mapChanged(&handleSimMapChangedPacket);
    robot.addPacketHandler(&mapChanged);
    ArUtil::sleep(200);
    if(master_newmap)
      printf("** SIM_CTRL 2 (master load new map) **\n");
    else
      printf("** SIM_CTRL 1 (load new map) **\n");
    struct sim_ctrl_map_t {
      ArTypes::Byte2 code;
      ArTypes::UByte2 len;
      char mapfile[128];
    };
    struct sim_ctrl_map_t cmd;
    if(master_newmap)
      cmd.code = 2;
    else
      cmd.code = 1;
    cmd.len = strlen(mapfile);
    memset(cmd.mapfile, 0, 128);
    strncpy(cmd.mapfile, mapfile, cmd.len);
    printf("-> SIM_CTRL %d, %d, \"%s\"...\n", cmd.code, cmd.len, cmd.mapfile);
    robot.comDataN(ArCommands::SIM_CTRL, (const char*)&cmd, sizeof(cmd));
    puts("   waiting for map change reply packet...");
    gotSimMapChangedPacketCondition.wait();
    puts("   got map change reply packet.");
    if(sleep_after_newmap)
    {
      puts("    sleeping for 15 seconds...");
      ArUtil::sleep(15000);
    }
  }

  
  if(testmask & test_config)
  {
    puts("** Config Packet **");
    ArGlobalRetFunctor1<bool, ArRobotPacket*> myConfigPacketHandler(&handleConfigPacket);
    robot.addPacketHandler(&myConfigPacketHandler);
    ArUtil::sleep(300);
    puts("-> CONFIG...");
    robot.com(18);
    gotConfigPacketCondition.wait();
    printf("\nArRobot::getAbsoluteMaxTransVel() == %f.", robot.getAbsoluteMaxTransVel());
  }

  if(testmask & test_crash)
  {
    puts("** SIM_RESET test (crash/abort MobileSim) after 2 sec");
    ArUtil::sleep(2000);
    puts("-> 255...");
    robot.com(255);
  }

  if(testmask & test_exit)
  {
    printf("** SIM_EXIT test **\n");
    printf("-> SIM_EXIT 23...\n");
    robot.comInt(ArCommands::SIM_EXIT, 23);
    // old SRISim command: robot.com(62);
    printf("\n\n");
  }


  // Note: must be last test:
  if(testmask & test_position || testmask & test_rotation)
  {
    printf("** Positions match velocities test **\n");
    int time = 1000;
    log_simstat = false;
    ArGlobalRetFunctor1<bool, ArRobotPacket*> mySimStatPacketHandler(&handleSimStatPacket);
    robot.lock();
    robot.addPacketHandler(&mySimStatPacketHandler);
    printf("-> SIMSTAT 2\n");
    robot.comInt(237, 2);
    printf("(now is the time to move the robot in the sim before the test start...)\n");
    robot.unlock();
    ArUtil::sleep(4000);
    robot.lock();

    if(testmask & test_position)
    {
      double speed = 200;
      int turndist = 20000;
      printf("-> VEL %.0f...\n", speed);
      robot.setVel(speed);
      robot.unlock();
      ArUtil::sleep(4000);  // let it get up to speed, also more time to move the robot or whatever
      robot.lock();
      (new CheckPositionTask(&robot, speed, time, turndist))->addTask();
    }
    else if(testmask & test_rotation)
    {
      double speed = 30;
      printf("-> RVEL %.0f...\n", speed);
      robot.setRotVel(speed);
      robot.unlock();
      ArUtil::sleep(1000);
      robot.lock();
      //(new CheckRotationTask(&robot, speed, time))->addTask();
    }

    robot.unlock();
    stayConnected = true;
  }

  if(testmask & test_logstate)
  {
    puts("** SIM_CTRL 5 (logstate) **");
    struct {
      ArTypes::Byte2 sim_ctrl;
      char logmsg[128];
    } cmd;
    cmd.sim_ctrl = 5;
    //cmd.len = strlen(logstate_msg);
    strncpy(cmd.logmsg, logstate_msg, 128);
    printf("-> SIM_CTRL %d, \"%s\"...\n", cmd.sim_ctrl, cmd.logmsg);
    robot.comDataN(ArCommands::SIM_CTRL, (const char*)&cmd, sizeof(cmd));
  } 
      
  // ------ end of tests -----

  robot.lock();
  if(stayConnected && robot.isRunning())
  {
    robot.unlock();
    puts("waitForRunExit()");
    robot.waitForRunExit();
  }
  else
  {
    robot.unlock();
  }

  printf("** Done with tests. Sleeping for 3 seconds and then stopping ArRobot thread... **\n");
  ArUtil::sleep(3000);
  robot.stopRunning();
  printf("** Exiting. **\n");
  Aria::exit(0);
  return 0;
}

