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

int main(void)
{

  Aria::init();


  ArLog::log(ArLog::Normal, "\nTesting ArTime with large values:");
  ArTime startLarge, testlarge;
  
  ArLog::log(ArLog::Normal, "Setting an ArTime object \"startLarge\" to now...");
  startLarge.setToNow();
  ArLog::log(ArLog::Normal, "mSecSince %ld secSince %ld", 
	     startLarge.mSecSince(), startLarge.secSince());

  long adding=pow(2,31) - 10 * 1000;
  ArLog::log(ArLog::Normal, "Adding %d milliseconds", adding);
  startLarge.addMSec(adding);
  ArLog::log(ArLog::Normal, "mSecSince %ld secSince %ld", 
	     startLarge.mSecSince(), startLarge.secSince());
  ArLog::log(ArLog::Normal, "mSecSinceLL %lld secSinceLL %lld", 
	     startLarge.mSecSinceLL(), startLarge.secSinceLL());


  ArLog::log(ArLog::Normal, "Adding %d milliseconds", adding);
  startLarge.addMSec(adding);
  ArLog::log(ArLog::Normal, "mSecSince %ld secSince %ld", 
	     startLarge.mSecSince(), startLarge.secSince());
  ArLog::log(ArLog::Normal, "mSecSinceLL %lld secSinceLL %lld", 
	     startLarge.mSecSinceLL(), startLarge.secSinceLL());

  ArLog::log(ArLog::Normal, "\nTesting platform localtime (broken-down) struct:");
  struct tm t;
  ArUtil::localtime(&t);
  ArLog::log(ArLog::Normal, "ArUtil::localtime() returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d",
    t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  time_t yesterday = time(NULL) - (24*60*60) ;
  ArUtil::localtime(&yesterday, &t);
  ArLog::log(ArLog::Normal, "ArUtil::localtime(time(NULL) - 24hours, struct tm*) returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d",
    t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

  ArLog::log(ArLog::Normal, "\nCurrent time as strings:");
  char year[5], month[3], day[3], hour[3], min[3], sec[3];
  ArUtil::putCurrentYearInString(year, 5);
  ArUtil::putCurrentMonthInString(month, 3);
  ArUtil::putCurrentDayInString(day, 3);
  ArUtil::putCurrentHourInString(hour, 3);
  ArUtil::putCurrentMinuteInString(min, 3);
  ArUtil::putCurrentSecondInString(sec, 3);
  ArLog::log(ArLog::Normal, "  Year:%s, Month:%s, Day:%s, Hour:%s, Min:%s, Sec:%s",
      year, month, day, hour, min, sec);

  ArLog::log(ArLog::Normal, "\nTesting ArTime:");
  ArTime start, test;
  
  ArLog::log(ArLog::Normal, "Setting an ArTime object \"start\" to now...");
  start.setToNow();
  start.log();
  ArLog::log(ArLog::Normal, "Sleeping 4 secs");
  ArUtil::sleep(4000);
  ArLog::log(ArLog::Normal, "Setting an ArTime object \"test\" to now...");
  test.setToNow();
  test.log();

  ArLog::log(ArLog::Normal, "ms of \"test\" since start %ld", test.mSecSince(start));
  ArLog::log(ArLog::Normal, "seconds \"test\" since start %ld", test.secSince(start));
  ArLog::log(ArLog::Normal, "ms of start since \"test\" %ld", start.mSecSince(test));
  ArLog::log(ArLog::Normal, "seconds \"start\" test %ld", start.secSince(test));
  ArLog::log(ArLog::Normal, "\"start\" is before \"test\"? %d", test.isBefore(start));
  ArLog::log(ArLog::Normal, "\"start\" is after \"test\"? %d", test.isAfter(start));
  ArLog::log(ArLog::Normal, "\"test\" is before \"start\"? %d", start.isBefore(test));
  ArLog::log(ArLog::Normal, "\"test\" is after \"start\"? %d", start.isAfter(test));
  ArLog::log(ArLog::Normal, "ms from \"start\" to now %ld", start.mSecTo());
  ArLog::log(ArLog::Normal, "s from \"start\" to now %ld", start.secTo());
  ArLog::log(ArLog::Normal, "ms since \"start\" %ld", start.mSecSince());
  ArLog::log(ArLog::Normal, "s since \"start\" %ld", start.secSince());
  ArLog::log(ArLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  ArLog::log(ArLog::Normal, "s from \"test\" stamp to now %ld", test.secTo());

  ArLog::log(ArLog::Normal, "Testing addMSec, adding 200 mSec");
  test.addMSec(200);
  ArLog::log(ArLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  ArLog::log(ArLog::Normal, "Testing addMSec, subtracting 300 mSec");
  test.addMSec(-300);
  ArLog::log(ArLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  ArLog::log(ArLog::Normal, "Testing addMSec, adding 20.999 seconds");
  test.addMSec(20999);
  ArLog::log(ArLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  ArLog::log(ArLog::Normal, "Testing addMSec, subtracting 23.5 seconds");
  test.addMSec(-23500);
  ArLog::log(ArLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  
  ArTime timeDone;
  ArLog::log(ArLog::Normal, "Setting ArTime object \"done\" to now.");
  timeDone.setToNow();
  timeDone.addMSec(1000);
  ArLog::log(ArLog::Normal, "Making sure the add works in the right direction, adding a second to a timestamp set now");
  ArLog::log(ArLog::Normal, "Reading: %ld", timeDone.mSecTo());
  ArLog::log(ArLog::Normal, "Sleeping 20 ms");
  ArUtil::sleep(20);
  ArLog::log(ArLog::Normal, "Reading: %ld", timeDone.mSecTo());
  ArLog::log(ArLog::Normal, "Sleeping 2 seconds");
  ArUtil::sleep(2000);
  ArLog::log(ArLog::Normal, "Reading: %ld", timeDone.mSecTo());

  /*
  puts("\nslamming ArUtil::localtime() from a bunch of threads with the same input time...");
  time_t now = time(NULL);
  class LocaltimeTestThread : public virtual ArASyncTask 
  {
  private:
    time_t time;
  public:
    LocaltimeTestThread(time_t t) : time(t) {}
    virtual void *runThread(void *) {
      struct tm t;
      ArUtil::localtime(&time, &t);
      ArLog::log(ArLog::Normal, "ArUtil::localtime() returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
      return 0;
    }
  };

  for(int i = 0; i < 200; ++i)
    (new LocaltimeTestThread(now))->runAsync();
  ArUtil::sleep(5000);
  */

  ArLog::log(ArLog::Normal, "test is done.");

}
