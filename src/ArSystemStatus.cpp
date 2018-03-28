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
#include "ariaInternal.h"
#include "ariaOSDef.h"
#include "ariaUtil.h"
#include "ArSystemStatus.h"
#include "ArASyncTask.h"
#include <stdio.h>


double ArSystemStatus::ourCPU = -1.0;
unsigned long ArSystemStatus::ourUptime = 0;
unsigned long ArSystemStatus::ourFirstUptime = 0;
unsigned long ArSystemStatus::ourLastCPUTime = 0;
ArTime ArSystemStatus::ourLastCPURefreshTime;
ArGlobalRetFunctor<double> ArSystemStatus::ourGetCPUPercentCallback(&ArSystemStatus::getCPUPercent);
ArGlobalRetFunctor<double> ArSystemStatus::ourGetUptimeHoursCallback(&ArSystemStatus::getUptimeHours);
ArGlobalRetFunctor<unsigned long> ArSystemStatus::ourGetUptimeCallback(&ArSystemStatus::getUptime);
ArGlobalRetFunctor<unsigned long> ArSystemStatus::ourGetProgramUptimeCallback(&ArSystemStatus::getProgramUptime);

int ArSystemStatus::ourLinkQuality = -1;
int ArSystemStatus::ourLinkSignal = -1;
int ArSystemStatus::ourLinkNoise = -1;
int ArSystemStatus::ourDiscardedTotal = -1;
int ArSystemStatus::ourDiscardedConflict = -1;
int ArSystemStatus::ourDiscardedDecrypt = -1;
int ArSystemStatus::ourMTXWirelessLink = -1;
int ArSystemStatus::ourMTXWirelessQuality = -1;
int ArSystemStatus::ourMTXIp1 = -1;
int ArSystemStatus::ourMTXIp2 = -1;
int ArSystemStatus::ourMTXIp3 = -1;
int ArSystemStatus::ourMTXIp4 = -1;
std::string ArSystemStatus::ourMTXIpString = "";
ArMutex ArSystemStatus::ourCPUMutex;
ArMutex ArSystemStatus::ourWirelessMutex;
ArMutex ArSystemStatus::ourMTXWirelessMutex;
ArGlobalRetFunctor<int> ArSystemStatus::ourGetWirelessLinkQualityCallback(&ArSystemStatus::getWirelessLinkQuality);
ArGlobalRetFunctor<int> ArSystemStatus::ourGetWirelessLinkNoiseCallback(&ArSystemStatus::getWirelessLinkNoise);
ArGlobalRetFunctor<int> ArSystemStatus::ourGetWirelessLinkSignalCallback(&ArSystemStatus::getWirelessLinkSignal);
ArGlobalRetFunctor<int> ArSystemStatus::ourGetMTXWirelessLinkCallback(&ArSystemStatus::getMTXWirelessLink);
ArGlobalRetFunctor<int> ArSystemStatus::ourGetMTXWirelessQualityCallback(&ArSystemStatus::getMTXWirelessQuality);
ArSystemStatusRefreshThread* ArSystemStatus::ourPeriodicUpdateThread = 0;
bool ArSystemStatus::ourShouldRefreshWireless = true;
bool ArSystemStatus::ourShouldRefreshMTXWireless = true;
bool ArSystemStatus::ourShouldRefreshCPU = true;



void ArSystemStatus::refreshCPU()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshCPU) return;
	unsigned long interval = ourLastCPURefreshTime.mSecSince();
	FILE* statfp = ArUtil::fopen("/proc/stat", "r");
	FILE* uptimefp = ArUtil::fopen("/proc/uptime", "r");
	if (!statfp) {
		ArLog::log(ArLog::Terse, "ArSystemStatus: Error: Failed to open /proc/stat!");
	}
	if (!uptimefp) {
		ArLog::log(ArLog::Terse, "ArSystemStatus: Error: Failed to open /proc/uptime!");
	}
	if (!statfp || !uptimefp)
	{
		ourCPU = -1.0;
		ourLastCPUTime = ourUptime = 0;
		ourShouldRefreshCPU = false;
		if (statfp)
			fclose(statfp);
		if (uptimefp)
			fclose(uptimefp);
		return;
	}
	//char line[512];
	//fgets(line,  512, uptimefp);
	//printf("read uptime file: %s\n", line);
	//double uptime = 0, idle_uptime = 0;
	//fscanf(uptimefp, "%lf %lf", &uptime, &idle_uptime);
	//ourUptime = (unsigned long)uptime;
	unsigned long uptime;
	fscanf(uptimefp, "%ld", &uptime);
	ourUptime = uptime;
	fclose(uptimefp);

	if (ourFirstUptime == 0)
		ourFirstUptime = ourUptime;

	unsigned long user, nice, sys, idle, total;
	char tag[32];
	fscanf(statfp, "%s %lu %lu %lu %lu", tag, &user, &nice, &sys, &idle);
	fclose(statfp);
	total = user + nice + sys; // total non-idle cpu time in 100ths of a sec
	if (ourLastCPUTime == 0 || interval == 0)
	{
		// no time has past since last refresh
		ourLastCPUTime = total;
		ourShouldRefreshCPU = false;
		return;
	}
	ourCPU = (double)(total - ourLastCPUTime) / ((double)interval / 10.0); // convert 'interval' to 1/100 sec units
	ourLastCPUTime = total;
	ourLastCPURefreshTime.setToNow();
	ourShouldRefreshCPU = false;
#endif // WIN32
}



/** @cond INTERNAL_CLASSES */
class ArSystemStatusRefreshThread : public virtual ArASyncTask {
public:
	ArSystemStatusRefreshThread(int refreshFrequency) :
		myRefreshFrequency(refreshFrequency)
	{
		setThreadName("ArSystemStatusRefreshThread");
	}
	void runAsync() { create(false); }
	void setRefreshFreq(int freq) { myRefreshFrequency = freq; }
private:
	int myRefreshFrequency;
	virtual void* runThread(void* arg)
	{
		threadStarted();
		while (Aria::getRunning() && getRunning())
		{
			ArSystemStatus::invalidate();
			ArUtil::sleep(myRefreshFrequency);
		}
		threadFinished();
		return NULL;
	}
};
/** @endcond INTERNAL_CLASSES */


AREXPORT void ArSystemStatus::startPeriodicUpdate(int refreshFrequency, ArLog::LogLevel logLevel)
{
	ourCPUMutex.setLogName("ArSystemStatusRefreshThread::ourCPUMutex");
	ourWirelessMutex.setLogName("ArSystemStatusRefreshThread::ourWirelessMutex");

	if (ourPeriodicUpdateThread) {
		// If we already have a thread, just change its refresh frequency
		ourPeriodicUpdateThread->setRefreshFreq(refreshFrequency);
		ourPeriodicUpdateThread->setLogLevel(logLevel);
		return;
	}
	// Otherwise, start a new thread, with the desired refresh frequency
	ourPeriodicUpdateThread = new ArSystemStatusRefreshThread(refreshFrequency);
	ourPeriodicUpdateThread->setLogLevel(logLevel);
	ourPeriodicUpdateThread->runAsync();
}

AREXPORT void ArSystemStatus::stopPeriodicUpdate()
{
	if (!ourPeriodicUpdateThread) return;
	ourPeriodicUpdateThread->stopRunning();
	delete ourPeriodicUpdateThread;
	ourPeriodicUpdateThread = 0;
}



AREXPORT double ArSystemStatus::getCPU() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourCPU;
}

AREXPORT double ArSystemStatus::getCPUPercent() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	if (ourCPU < 0)
	{
		return ourCPU;  // invalid value indicator
	}
	return ourCPU * 100.0;
}

// Get CPU percentage in a string
AREXPORT std::string ArSystemStatus::getCPUPercentAsString() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	if (ourCPU < 0)
	{
		return std::string("n/a");
	}
	char tmp[32];
	snprintf(tmp, 31, "%.2f", getCPUPercent());
	return std::string(tmp);
}

// Get total system uptime (seconds)
AREXPORT unsigned long ArSystemStatus::getUptime() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime;
}

// Get total system uptime (hours)
AREXPORT double ArSystemStatus::getUptimeHours() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime / 3600.0;
}

// Get total system uptime (seconds)
AREXPORT unsigned long ArSystemStatus::getProgramUptime() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime - ourFirstUptime;
}

// Get total system uptime in a string (hours)
AREXPORT std::string ArSystemStatus::getUptimeHoursAsString() {
	ArScopedLock lock(ourCPUMutex);
	refreshCPU();
	char tmp[32];
	snprintf(tmp, 31, "%.2f", getUptimeHours());
	return std::string(tmp);
}

// return Pointer to a functor which can be used to retrieve the current CPU percentage
AREXPORT ArRetFunctor<double>* ArSystemStatus::getCPUPercentFunctor() {
	return &ourGetCPUPercentCallback;
}

// return Pointer to a functor which can be used to retrieve the current uptime (hours)
AREXPORT ArRetFunctor<double>* ArSystemStatus::getUptimeHoursFunctor() {
	return &ourGetUptimeHoursCallback;
}

// return Pointer to a functor which can be used to retrieve the current uptime (seconds)
AREXPORT ArRetFunctor<unsigned long>* ArSystemStatus::getUptimeFunctor() {
	return &ourGetUptimeCallback;
}

// return Pointer to a functor which can be used to retrieve the current program uptime (seconds)
AREXPORT ArRetFunctor<unsigned long>* ArSystemStatus::getProgramUptimeFunctor() {
	return &ourGetProgramUptimeCallback;
}

AREXPORT ArRetFunctor<int>* ArSystemStatus::getWirelessLinkQualityFunctor() {
	return &ourGetWirelessLinkQualityCallback;
}
AREXPORT ArRetFunctor<int>* ArSystemStatus::getWirelessLinkNoiseFunctor() {
	return &ourGetWirelessLinkNoiseCallback;
}
AREXPORT ArRetFunctor<int>* ArSystemStatus::getWirelessLinkSignalFunctor() {
	return &ourGetWirelessLinkSignalCallback;
}

AREXPORT ArRetFunctor<int>* ArSystemStatus::getMTXWirelessLinkFunctor() {
	return &ourGetMTXWirelessLinkCallback;
}
AREXPORT ArRetFunctor<int>* ArSystemStatus::getMTXWirelessQualityFunctor() {
	return &ourGetMTXWirelessQualityCallback;
}

// Get wireless stats from /proc/net/wireless:

void ArSystemStatus::refreshWireless()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshWireless) return;
	FILE* fp = ArUtil::fopen("/proc/net/wireless", "r");
	if (!fp)
	{
		ArLog::log(ArLog::Terse, "ArSystemStatus: Error: Failed to open /proc/net/wireless!");
		ourShouldRefreshWireless = false;
		return;
	}

	// first two lines are header info
	char line[256];
	if (!(fgets(line, 256, fp) && fgets(line, 256, fp)))
	{
		fclose(fp);
		ourLinkQuality = ourLinkSignal = ourLinkNoise =
			ourDiscardedTotal = ourDiscardedDecrypt = -1;
		ourShouldRefreshWireless = false;
		return;
	}


	// next line is info for first device
	char id[32];
	unsigned int stat;
	int disc_frag, disc_retry, disc_misc, missed;
	disc_frag = disc_retry = disc_misc = missed = 0;
	int r = fscanf(fp, "%31s %x %d. %d. %d. %d %d %d %d %d %d",
		id, &stat,
		&ourLinkQuality, &ourLinkSignal, &ourLinkNoise,
		&ourDiscardedConflict, &ourDiscardedDecrypt,
		&disc_frag, &disc_retry, &disc_misc, &missed);
	fclose(fp);
	if (r < 11)
		ArLog::log(ArLog::Verbose, "ArSystemStatus: Warning: Failed to parse /proc/net/wireless (only %d out of 11 values parsed).", r);
	if (ourDiscardedConflict == -1 || ourDiscardedDecrypt == -1)
		ourDiscardedTotal = -1;
	else
		ourDiscardedTotal = ourDiscardedConflict + ourDiscardedDecrypt
		+ disc_frag + disc_retry + disc_misc;
	ourShouldRefreshWireless = false;
#endif // WIN32
}


// Get wireless stats from /var/robot/status/network/wireless

void ArSystemStatus::refreshMTXWireless()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshMTXWireless) return;
	FILE* fpIp = ArUtil::fopen("/mnt/status/network/wireless/ip", "r");
	FILE* fpLink = ArUtil::fopen("/mnt/status/network/wireless/link", "r");
	FILE* fpQuality = ArUtil::fopen("/mnt/status/network/wireless/quality", "r");

	if ((!fpIp) || (!fpLink) || (!fpQuality))
	{
		ArLog::log(ArLog::Terse, "ArSystemStatus: Error: Failed to open /mnt/status/network/wireless/ files!");
		ourShouldRefreshMTXWireless = false;
		return;
	}

#if 0
	// grab the data from each file
	char lineIp[256];
	if (!(fgets(lineIp, 256, fpIp)))
	{
		fclose(fpIp);
		ourMTXIP1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessLink = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}
#endif


	// ?? - need to store IP somewhere don't know if we need it

	if (!(fscanf(fpLink, "%d", &ourMTXWirelessLink)))
	{
		fclose(fpLink);
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessLink = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}


	if (!((fscanf(fpQuality, "%d", &ourMTXWirelessQuality))))
	{
		fclose(fpQuality);
		fclose(fpLink);
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}

	if (!(fscanf(fpIp, "%d.%d.%d.%d", &ourMTXIp1, &ourMTXIp2, &ourMTXIp3, &ourMTXIp4)))
	{
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}


	/*
	ArLog::log(ArLog::Normal, "ArSystemStatus: %d.%d.%d.%d %d %d ",
	ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4, ourMTXLinkQuality, ourMTXLinkSignal);
	*/

	char buf[1024];
	sprintf(buf, "%d.%d.%d.%d", ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4);
	ourMTXIpString = buf;

	fclose(fpQuality);
	fclose(fpLink);
	fclose(fpIp);
	ourShouldRefreshMTXWireless = false;
#endif // WIN32
}

AREXPORT int ArSystemStatus::getWirelessLinkQuality() {
	ArScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkQuality;
}

AREXPORT int ArSystemStatus::getWirelessLinkSignal() {
	ArScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkSignal;
}

AREXPORT int ArSystemStatus::getWirelessLinkNoise() {
	ArScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkNoise;
}

AREXPORT int ArSystemStatus::getWirelessDiscardedPackets() {
	ArScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourDiscardedTotal;
}

AREXPORT int ArSystemStatus::getWirelessDiscardedPacketsBecauseNetConflict() {
	ArScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourDiscardedConflict;
}

AREXPORT int ArSystemStatus::getMTXWirelessLink() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXWirelessLink;
}

AREXPORT int ArSystemStatus::getMTXWirelessQuality() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXWirelessQuality;
}

AREXPORT int ArSystemStatus::getMTXWirelessIpAddress1() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp1;
}

AREXPORT int ArSystemStatus::getMTXWirelessIpAddress2() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp2;
}
AREXPORT int ArSystemStatus::getMTXWirelessIpAddress3() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp3;
}

AREXPORT int ArSystemStatus::getMTXWirelessIpAddress4() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp4;
}

AREXPORT const char * ArSystemStatus::getMTXWirelessIpAddressString() {
	ArScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIpString.c_str();
}

AREXPORT void ArSystemStatus::invalidate()
{
	ArScopedLock lockc(ourCPUMutex);
	ArScopedLock lockw(ourWirelessMutex);
	ourShouldRefreshCPU = true;
	ourShouldRefreshWireless = true;
	ourShouldRefreshMTXWireless = true;
}

