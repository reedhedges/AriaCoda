
/*
  This ArRangeDevice subclass simply repeatedly provides a small number of fixed
  readings in a few different subsets that can be turned on/off, and invalidation
  of some readings can be manually triggered, to verify
  correct behavior of how ArRangeDevice and ArRangeBuffer store and process
  readings.
 
  The patterns of readings are:
    1. A line of ten readings in front of 0,0 at Y=10meters, spanning from X=-10m to X=+10m.
    2. The left 5 of those readings are duplicated (normally should be omitted from range buffer.)
    3. A line of ten readings in front of 0,0 at Y=5meters, spanning from X=0 to X=+10m.  These occlude the right 5 readings of the first line.
    4. Two readings very close to 0,0.  If the robot is at 0,0 these would be "inside" the robot and should normally be omitted from range buffer.
    5. A reading directly in front of the robot, at X=0 and Y=(half the robot length plus 5 mm)
    6. Five readings 25 meters away. If the max range is 20 meters, these should not appear.

  This test starts the test range device, logs the readings, turns some off, and
  logs them again after a short wait which is long enough that the old readings
  should have been removed from the buffer.  It cycles through some permutations of the above, waiting several seconds in
  between (so old readings should have been cleared from the buffers), and logs
  the readings.    Invalidation of sets of readings can be manually triggered:
  Any reading more than 5 meters away (should remove first line at 10m), all readings,
  or the line of readings at 5 meters, or the line of readings at 10 meters.

  Note the TestRangeDevice provides a new set of readings every robot cycle (100ms), but results are only logged every 1 second.  
  Current readings are configured to remain for 1 second, current readings for 2 seconds (so cumulative readings should lag behind current readings
  every time data is logged every second)

  Data log is written to basicRangeBufferTest.dat and may be plotted with gnu plot.

  The logs of readings may be compared to basicRangeBufferTestKnownGood.dat as an
  approval/regression test.
*/

#include "Aria/ArRangeDevice.h"
#include "Aria/ArRobot.h"
#include "Aria/ArRobotConnector.h"
#include <cassert>

const size_t CurrentBufferSize = 100;
const size_t CumulativeBufferSize = 500;
const unsigned int MaxRange = 20'000;
const int MaxSecondsCurrent = 1;
const int MaxSecondsCumulative = 2;
const double MaxDistCumulative = 20'000;

class TestRangeDevice : public ArRangeDevice
{
public:
  TestRangeDevice(ArRobot *r) : 
    ArRangeDevice(CurrentBufferSize, CumulativeBufferSize, "TestRangeDevice", MaxRange, MaxSecondsCurrent, MaxSecondsCumulative, MaxDistCumulative),
    robot(r), processTask(this, &TestRangeDevice::processReadings)
  {
    assert(robot);
    robot->addSensorInterpTask("TestRangeDevice", 10, &processTask);
    setRobot(robot);
  }

  void processReadings()
  {
    if(flags.tenMeterLine)
    {
      for(int x = -10; x <= 10; x += 2)
        addReading(x * 1000, 10 * 1000);
    }

    if(flags.duplicates)
    {
      for(int x = -10; x <= 0; x += 2)
        addReading(x * 1000, 10 * 1000);
    }

    if(flags.fiveMeterLine)
    {
      for(int x = 0; x <= 10; x += 1)
        addReading(x * 1000, 5 * 1000);
    }

    if(flags.insideRobot)
    {
      addReading(-125, 125);
      addReading(125, -125);
    }

    if(flags.inFrontOfRobot)
    {
      addReading(0, (robot->getRobotLength() / 2.0) + 5);
    }

    if(flags.farAway)
    {
      for (int x = -5; x <= 5; x += 2)
        addReading(x * 1000, 25 * 1000);
    }
  }

  ~TestRangeDevice()
  {
    robot->remSensorInterpTask(&processTask);
    robot->remRangeDevice(this);
  }

  ArRobot *robot = 0;
  ArFunctorC<TestRangeDevice> processTask;
 
  struct {
    bool tenMeterLine = true;
    bool duplicates = true;
    bool fiveMeterLine = true;
    bool insideRobot = true;
    bool inFrontOfRobot = true;
    bool farAway = true;
  } flags;
};


class Test
{
public:
  Test(ArRobot *r, TestRangeDevice *t) :
    robot(r), testRangeDevice(t),
    testTask(this, &Test::sequence)
  {
    fp = fopen("basicRangeDeviceTest.dat", "w");
    assert(fp);
    assert(robot);
    robot->addUserTask("Test", 20, &testTask);
  }

  ~Test()
  {
    fclose(fp);
    ArLog::info("Wrote data sets to \"basicRangeDeviceTest.dat\"");
    ArLog::info("You can plot each data set by name in gnuplot with the command:\n\tplot \"basicRangeDeviceTest.dat\" index \"TestRangeDevice_Current_N\" w points\n\t...Where N is set number 0, 1, 2, 3 etc. or replace \"Current\" with \"Cumulative\".");
  }

  void logData(const char *comment)
  {
    ArLog::info("basicRangeDeviceTest: %s", comment);
    {
      const ArRangeBuffer &b = testRangeDevice->getCurrentRangeBuffer();
      fprintf(fp, "\n\n# %s_Current_%lu %s ", testRangeDevice->getName(), currentLogCounter++, comment); // no newline
      b.logInternal(fp);
      for (auto i = b.getBegin(); i != b.getEnd(); ++i)
        fprintf(fp, "%.0f\t%.0f\n", i->getX(), i->getY());
    }

    {
      const ArRangeBuffer &b = testRangeDevice->getCumulativeRangeBuffer();
      fprintf(fp, "\n\n# %s_Cumulative_%lu %s ", testRangeDevice->getName(), cumulativeLogCounter++, comment); // no newline
      b.logInternal(fp);

      for (auto i = b.getBegin(); i != b.getEnd(); ++i)
        fprintf(fp, "%.0f\t%.0f\n", i->getX(), i->getY());
    }
  }

  void sequence()
  {

  }

  ArRobot *robot;
  TestRangeDevice *testRangeDevice;
  ArFunctorC<Test> testTask;
  ArTime timer;
  bool first = true;
  unsigned long currentLogCounter = 0;
  unsigned long cumulativeLogCounter = 0;
  FILE *fp = NULL;
};

int main(int argc, char **argv)
{
  Aria::init();
  

  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  TestRangeDevice testRangeDevice(&robot);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "basicRangeDeviceTest: Could not connect to the robot.");
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

  //ArLog::setLogTime(true);


  ArLog::log(ArLog::Normal, "basicRangeDeviceTest: Connected to robot.");

  testRangeDevice.logConfig();

  Test test(&robot, &testRangeDevice);

  robot.runAsync();

  // XXX TODO invalidate specific readings and check sizes.  add logInternals() to ArRangeBuffer to log
  // size etc. of reserved invalidated readings. as well as others. log these to data file in addition to ArLog 
  // to compare against old for approval/regression test.  (does ArLog need an option to write log messages to data file as a comment?)

  ArUtil::sleep(1000);
  test.logData("all readings on");

  testRangeDevice.flags.duplicates = false;
  ArUtil::sleep(1000);
  test.logData("duplicates off");

  testRangeDevice.flags.fiveMeterLine = false;
  ArUtil::sleep(1000);
  test.logData("five meter line off");

  testRangeDevice.flags.insideRobot = false;
  ArUtil::sleep(1000);
  test.logData("inside robot off");

  testRangeDevice.flags.inFrontOfRobot = false;
  ArUtil::sleep(1000);
  test.logData("in front of robot off");

  testRangeDevice.flags.fiveMeterLine = true;
  ArUtil::sleep(1000);
  test.logData("five meter line on");

  testRangeDevice.flags.duplicates = true;
  ArUtil::sleep(1000);
  test.logData("duplicates on");

  ArLog::log(ArLog::Normal, "basicRangeDeviceTest: done");
  Aria::exit(0);
  return 0;
}
