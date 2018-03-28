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

#ifndef ARSYSTEMSTATUS_H
#define ARSYSTEMSTATUS_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArMutex.h"
#include <string>

class ArSystemStatusRefreshThread;

/** @brief Utility to get statistics about the  host operating system
 *  (CPU usage, wireless link data, etc).
 *
 *  Normally, calling any accessor to read a value will query the operating
 *  system to get the most recent value.  However, if you will be accessing
 *  data very frequently and want those calls to be faster, you can start 
 *  a thread by calling startPeriodicUpdate() which will periodically query
 *  new values from the operating system and cache them for accessors to 
 *  return.
 *
 *  This class is only implemented for Linux; on Windows you will get invalid
 *  information.
 *  @todo Add a function and functor that formats uptime like "X years, 
 *  X months, X days, X hours, X min, X sec." (omitting 0 values).

  @ingroup UtilityClasses
 */
class ArSystemStatus {
public:

  /** Create a new thread which periodically invalidates cached data,
   *  causing it to be recalculated when next accessed.  Starting
   *  this thread is optional; start it if you 
   *  will be accessing the data frequently, so that is doesn't need to
   *  be re-read and re-calculated on each access. If you will only be 
   *  accessing the data occasionally, you do not need to start the update
   *  thread, it will be updated each time you read a value.
   */
  AREXPORT static void startPeriodicUpdate(int refreshFrequency = 5000, ArLog::LogLevel logLevel = ArLog::Verbose);

  /** Stop periodic update thread. Henceforth any access of data will
   *  cause it to be re-read and recalculated. */
  AREXPORT static void stopPeriodicUpdate();

  /** @deprecated use startPeriodicUpdate() which has a better name. */
  AREXPORT static void runRefreshThread(int refreshFrequency = 5000) {
    startPeriodicUpdate(refreshFrequency);
  }

  /** Get CPU work to idle ratio since last refresh.
   *  This is a value ranging from (0 .. 1) X (Num. CPUs). (Therefore
   *  if you have two CPUs, the maximum value will be 2.0, or 200%.)
   *  This value is calculated as the percentage 
   *  of time the CPU spent doing work (not in "idle" state) since the 
   *  previous calculation.
   *  @return CPU usage value, or -1 if unable to determine
   */
  AREXPORT static double getCPU();

  /** Get CPU usage as percentage since last refresh. This is a value ranging from
   *  (0..100) X (Num. CPUs). (Therefore if you have two CPUs, the maximum value
   *  will be 200%).
   *  @sa getCPU()
   *  @return CPU usage as percentage, or -1 if not able to determine
   */
  AREXPORT static double getCPUPercent();

  /// Get CPU percentage in a string
  AREXPORT static std::string getCPUPercentAsString();

  /// Get total system uptime (seconds)
  AREXPORT static unsigned long getUptime();

  /// Get program's uptime (seconds)
  AREXPORT static unsigned long getProgramUptime();

  /// Get total system uptime (hours)
  AREXPORT static double getUptimeHours();

  /// Get total system uptime in a string (hours)
  AREXPORT static std::string getUptimeHoursAsString();

  /** @return Pointer to a functor which can be used to retrieve the current CPU percentage */
  AREXPORT static ArRetFunctor<double>* getCPUPercentFunctor();

  /** @return Pointer to a functor which can be used to retrieve the current uptime (hours) */
  AREXPORT static ArRetFunctor<double>* getUptimeHoursFunctor();

  /** @return Pointer to a functor which can be used to retrieve the current uptime (hours) */
  AREXPORT static ArRetFunctor<unsigned long>* getUptimeFunctor();

  /** @return Pointer to a functor which can be used to retrieve the current uptime (hours) */
  AREXPORT static ArRetFunctor<unsigned long>* getProgramUptimeFunctor();



  /** Get wireless network general link quality heuristic (for first configured
   * wireless device). */
  AREXPORT static int getWirelessLinkQuality();

  /** Get wireless netork signal level (for first configured
   * wireless device). */
  AREXPORT static int getWirelessLinkSignal();

  /** Get wireless network noise level (for first configured
   * wireless device). */
  AREXPORT static int getWirelessLinkNoise();

  /** Get wireless network total discarded packets (for first configured
   * wireless device). */
  AREXPORT static int getWirelessDiscardedPackets();

  /** Get wireless network packets discarded because of a conflict with another
   * network (for first configured
   * wireless device). */
  AREXPORT static int getWirelessDiscardedPacketsBecauseNetConflict();

  /** Get if the wireless has a link */
  AREXPORT static int getMTXWirelessLink();

  /** Get wireless network quality (for first configured
   * wireless device). */
  AREXPORT static int getMTXWirelessQuality();

  /** Get wireless network ip address (for first configured
   * wireless device). */
  AREXPORT static int getMTXWirelessIpAddress1();
  AREXPORT static int getMTXWirelessIpAddress2();
  AREXPORT static int getMTXWirelessIpAddress3();
  AREXPORT static int getMTXWirelessIpAddress4();

  /// Gets the wireless IP address as a string
  AREXPORT static const char *getMTXWirelessIpAddressString();



  AREXPORT static ArRetFunctor<int>* getWirelessLinkQualityFunctor();
  AREXPORT static ArRetFunctor<int>* getWirelessLinkNoiseFunctor();
  AREXPORT static ArRetFunctor<int>* getWirelessLinkSignalFunctor();

  AREXPORT static ArRetFunctor<int>* getMTXWirelessLinkFunctor();
  AREXPORT static ArRetFunctor<int>* getMTXWirelessQualityFunctor();


  /** @internal */
  AREXPORT static void invalidate();

  /** @deprecated Calling this function is no longer neccesary. */
  AREXPORT static void refresh() { } 
private:
  

	static ArMutex ourCPUMutex;
	static double ourCPU;
	static unsigned long ourUptime;
	static unsigned long ourFirstUptime;
	static unsigned long ourLastCPUTime;
	static ArTime ourLastCPURefreshTime;
	static ArGlobalRetFunctor<double> ourGetCPUPercentCallback;
	static ArGlobalRetFunctor<double> ourGetUptimeHoursCallback;
	static ArGlobalRetFunctor<unsigned long> ourGetUptimeCallback;
	static ArGlobalRetFunctor<unsigned long> ourGetProgramUptimeCallback;

	static ArMutex ourWirelessMutex;
	static int ourLinkQuality, ourLinkSignal, ourLinkNoise,
		ourDiscardedTotal, ourDiscardedDecrypt, ourDiscardedConflict;
	static ArGlobalRetFunctor<int> ourGetWirelessLinkQualityCallback;
	static ArGlobalRetFunctor<int> ourGetWirelessLinkNoiseCallback;
	static ArGlobalRetFunctor<int> ourGetWirelessLinkSignalCallback;

	static ArMutex ourMTXWirelessMutex;
	static int ourMTXWirelessLink, ourMTXWirelessQuality, ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4;
	static std::string ourMTXIpString;
	static ArGlobalRetFunctor<int> ourGetMTXWirelessLinkCallback;
	static ArGlobalRetFunctor<int> ourGetMTXWirelessQualityCallback;

	static void refreshCPU(); ///< Refresh CPU, if neccesary
	static void refreshWireless(); ///< Refresh Wireless stats, if neccesary

	static void refreshMTXWireless(); ///< Refresh MTX Wireless stats, if neccesary


	static ArSystemStatusRefreshThread* ourPeriodicUpdateThread;
	static bool ourShouldRefreshWireless;
	static bool ourShouldRefreshCPU;

	static bool ourShouldRefreshMTXWireless;

};

#endif
