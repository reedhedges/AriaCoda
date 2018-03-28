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

#define _GNU_SOURCE 1 // for isnormal() and other newer (non-ansi) C functions

#include "ArExport.h"
#include "ariaOSDef.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <clocale>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>  
#include <mmsystem.h> // for timeGetTime()
#include <Winnls.h>
#include <wchar.h>
#else
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#endif


#include "ariaInternal.h"
#include "ariaTypedefs.h"
#include "ariaUtil.h"

#include "ArLMS2xx.h"
#include "ArUrg.h"
#include "ArLMS1XX.h"
#include "ArS3Series.h"
#include "ArUrg_2_0.h"
#include "ArSZSeries.h"
#include "ArSonarMTX.h"
#include "ArBatteryMTX.h"
#include "ArLCDMTX.h"

#include "ArSerialConnection.h"
#include "ArTcpConnection.h"

#ifdef WIN32
#include <io.h>
AREXPORT const char *ArUtil::COM1 = "COM1";
AREXPORT const char *ArUtil::COM2 = "COM2";
AREXPORT const char *ArUtil::COM3 = "COM3";
AREXPORT const char *ArUtil::COM4 = "COM4";
AREXPORT const char *ArUtil::COM5 = "COM5";
AREXPORT const char *ArUtil::COM6 = "COM6";
AREXPORT const char *ArUtil::COM7 = "COM7";
AREXPORT const char *ArUtil::COM8 = "COM8";
AREXPORT const char *ArUtil::COM9 = "COM9";
AREXPORT const char *ArUtil::COM10 = "\\\\.\\COM10";
AREXPORT const char *ArUtil::COM11 = "\\\\.\\COM11";
AREXPORT const char *ArUtil::COM12 = "\\\\.\\COM12";
AREXPORT const char *ArUtil::COM13 = "\\\\.\\COM13";
AREXPORT const char *ArUtil::COM14 = "\\\\.\\COM14";
AREXPORT const char *ArUtil::COM15 = "\\\\.\\COM15";
AREXPORT const char *ArUtil::COM16 = "\\\\.\\COM16";
// See http://support.microsoft.com/kb/115831 for explanation of port names for
// COM10 and up.
#else // ifndef WIN32
AREXPORT const char *ArUtil::COM1 = "/dev/ttyS0";
AREXPORT const char *ArUtil::COM2 = "/dev/ttyS1";
AREXPORT const char *ArUtil::COM3 = "/dev/ttyS2";
AREXPORT const char *ArUtil::COM4 = "/dev/ttyS3";
AREXPORT const char *ArUtil::COM5 = "/dev/ttyS4";
AREXPORT const char *ArUtil::COM6 = "/dev/ttyS5";
AREXPORT const char *ArUtil::COM7 = "/dev/ttyS6";
AREXPORT const char *ArUtil::COM8 = "/dev/ttyS7";
AREXPORT const char *ArUtil::COM9 = "/dev/ttyS8";
AREXPORT const char *ArUtil::COM10 = "/dev/ttyS9";
AREXPORT const char *ArUtil::COM11 = "/dev/ttyS10";
AREXPORT const char *ArUtil::COM12 = "/dev/ttyS11";
AREXPORT const char *ArUtil::COM13 = "/dev/ttyS12";
AREXPORT const char *ArUtil::COM14 = "/dev/ttyS13";
AREXPORT const char *ArUtil::COM15 = "/dev/ttyS14";
AREXPORT const char *ArUtil::COM16 = "/dev/ttyS15";
#endif  // WIN32

AREXPORT const char *ArUtil::TRUESTRING = "true";
AREXPORT const char *ArUtil::FALSESTRING = "false";

// const double eps = std::numeric_limits<double>::epsilon();  
const double ArMath::ourEpsilon = 0.00000001; 

#ifdef WIN32
// max returned by rand()
const long ArMath::ourRandMax = RAND_MAX;
#else
// max returned by lrand48()
const long ArMath::ourRandMax = 2147483648;// 2^31, per lrand48 man page
#endif

#ifdef WIN32
const char  ArUtil::SEPARATOR_CHAR = '\\';
const char *ArUtil::SEPARATOR_STRING = "\\";
const char  ArUtil::OTHER_SEPARATOR_CHAR = '/';
#else
const char  ArUtil::SEPARATOR_CHAR = '/';
const char *ArUtil::SEPARATOR_STRING = "/";
const char  ArUtil::OTHER_SEPARATOR_CHAR = '\\';
#endif

#ifdef WIN32
ArMutex ArUtil::ourLocaltimeMutex;
#endif


/**
  Sleep (do nothing, without continuing the program) for @arg ms miliseconds.
  Use this to add idle time to threads, or in situations such as waiting for
hardware with a known response time.  To perform actions at specific intervals,
however, use the ArTime timer utility instead, or the ArUtil::getTime() method
to check time.
  @note in Linux, it actually calls the system usleep() function with 10 ms less
than the desired sleep time, since usleep() can sleep for about 10 ms. more than
requested (for small sleep times especially, it sleeps for the next highest
multiple of 10.)
   @param ms the number of milliseconds to sleep for
*/
AREXPORT void ArUtil::sleep(unsigned int ms)
{
#ifdef WIN32
  Sleep(ms);
#else // ifndef win32
  if (ms > 10)
    ms -= 10;
  usleep(ms * 1000);
#endif // linux

}

/**
   Get the time in milliseconds, counting from some arbitrary point.
   This time is only valid within this run of the program, it does not represent
"time of day". In other words, values returned by getTime() may be compared to
each other within the same run time of a program.
   If ARIA was compiled on Linux with POSIX monotonic clock functions
available, and the current running Linux kernel supports the monotonic clock,
then the clock_gettime() function is used with the CLOCK_MONOTONIC option 
(this is an accurate clock that is robust against e.g. changes to the
system time, etc.). Otherwise, gettimeofday() is used instead. Programs using ARIA
on Linux must therefore link to the librt library (with -lrt) as well as libAria.  On Windows, the
timeGetTime() function is used from the winmm library -- this means programs
using ARIA on Windows must be linked to the winmm.lib library as well as ARIA.
   @return millisecond time
*/
AREXPORT unsigned int ArUtil::getTime(void)
{
// the good unix way
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK) && !defined(__MACH__)
  struct timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    return tp.tv_nsec / 1000000 + (tp.tv_sec % 1000000)*1000;
// the old unix way as a fallback
#endif // if it isn't the good way
#if !defined(WIN32)
  struct timeval tv;
  if (gettimeofday(&tv,NULL) == 0)
    return tv.tv_usec/1000 + (tv.tv_sec % 1000000)*1000;
  else
    return 0;
#elif defined(WIN32)
  return timeGetTime();
#endif
}

/*
   Takes a string and splits it into a list of words. It appends the words
   to the outList. If there is nothing found, it will not touch the outList.
   @param inString the input string to split
   @param outList the list in which to store the words that are found
*/
/*
AREXPORT void ArUtil::splitString(std::string inString,
				  std::list<std::string> &outList)
{
  const char *start, *end;

  // Strip off leading white space
  for (end=inString.c_str(); *end && isspace(*end); ++end)
    ;
  while (*end)
  {
    // Mark start of the word then find end
    for (start=end; *end && !isspace(*end); ++end)
      ;
    // Store the word
    if (*start && ((*end && isspace(*end)) || !*end))
      outList.push_back(std::string(start, (int)(end-start)));
    for (; *end && isspace(*end); ++end)
      ;
  }
}
*/



#ifdef WIN32

/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
AREXPORT long ArUtil::sizeFile(std::string fileName)
{
  struct stat buf;

  if (ArUtil::filestat(fileName, &buf) < 0)
    return(-1);

  if (!(buf.st_mode | _S_IFREG))
    return(-1);

  return(buf.st_size);
}

/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
AREXPORT long ArUtil::sizeFile(const char * fileName)
{
  struct stat buf;

  if (ArUtil::filestat(fileName, &buf) < 0)
    return(-1);

  if (!(buf.st_mode | _S_IFREG))
    return(-1);

  return(buf.st_size);
}

#else // !WIN32

AREXPORT long ArUtil::sizeFile(std::string fileName)
{
  struct stat buf;

  if (stat(fileName.c_str(), &buf) < 0)
  {
    ArLog::logErrorFromOS(ArLog::Normal, "ArUtil::sizeFile: stat failed");
    return(-1);
  }

  if (!S_ISREG(buf.st_mode))
    return(-1);

  return(buf.st_size);
}


/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
AREXPORT long ArUtil::sizeFile(const char * fileName)
{
  struct stat buf;

  if (stat(fileName, &buf) < 0)
  {
    ArLog::logErrorFromOS(ArLog::Normal, "ArUtil::sizeFile: stat failed");
    return(-1);
  }

  if (!S_ISREG(buf.st_mode))
    return(-1);

  return(buf.st_size);
}

#endif // else !WIN32

/**
   @return true if file is found
   @param fileName name of the file to size
*/
AREXPORT bool ArUtil::findFile(const char *fileName)
{
  FILE *fp;

  if ((fp=ArUtil::fopen(fileName, "r")))
  {
    fclose(fp);
    return(true);
  }
  else
    return(false);
}




/*
   Works for \ and /. Returns true if something was actualy done. Sets
   fileOut to be what ever the answer is.
   @return true if the path contains a file
   @param fileIn input path/fileName
   @param fileOut output fileName
*/
/*AREXPORT bool ArUtil::stripDir(std::string fileIn, std::string &fileOut)
{
  const char *ptr;

  for (ptr=fileIn.c_str(); *ptr; ++ptr)
    ;
  for (--ptr; (ptr > fileIn.c_str()) && (*ptr != '/') && (*ptr != '\\'); --ptr)
    ;
  if ((*ptr == '/') || (*ptr == '\\'))
  {
    fileOut=ptr+1;
    return(true);
  }
  else
  {
    fileOut=fileIn;
    return(false);
  }
}
*/
/*
   Works for \ and /. Returns true if something was actualy done. Sets
   fileOut to be what ever the answer is.
   @return true if the file contains a path
   @param fileIn input path/fileName
   @param fileOut output path
*/
/*
AREXPORT bool ArUtil::stripFile(std::string fileIn, std::string &fileOut)
{
  const char *start, *end;

  for (start=end=fileIn.c_str(); *end; ++end)
  {
    if ((*end == '/') || (*end == '\\'))
    {
      start=end;
      for (; *end && ((*end == '/') || (*end == '\\')); ++end)
        ;
    }
  }

  if (start < end)
  {
    fileOut.assign(fileIn, 0, start-fileIn.c_str());
    return(true);
  }

  fileOut=fileIn;
  return(false);
}
*/
AREXPORT bool ArUtil::stripQuotes(char *dest, const char *src, size_t destLen)
{
  size_t srcLen = strlen(src);
  if (destLen < srcLen + 1)
  {
    ArLog::log(ArLog::Normal, "ArUtil::stripQuotes: destLen isn't long enough to fit copy its %d should be %d", destLen, srcLen + 1);
    return false;
  }
  // if there are no quotes to strip just copy and return
  if (srcLen < 2 || 
      (src[0] != '"' || src[srcLen - 1] != '"'))
  {
    strcpy(dest, src);
    return true;
  }
  // we have quotes so chop of the first and last char
  strncpy(dest, &src[1], srcLen - 1);
  dest[srcLen - 2] = '\0';
  return true;
}



/**
 * This method behaves similarly to the char[] version, except that it modifies
 * the given std::string.  If the first and last characters of the string are 
 * quotation marks ("), then they are removed from the string.  Multiple nested
 * quotation marks are not handled.
 * @param strToStrip a pointer to the std::string to be read/modified; must be 
 * non-NULL
 * @return bool true if the string was successfully processed; false otherwise 
**/  
AREXPORT bool ArUtil::stripQuotes(std::string *strToStrip)
{
  if (strToStrip == NULL) {
    ArLog::log(ArLog::Normal,
               "ArUtil::stripQuotes() NULL string");
    return false;
  }
  
  // If there are no matching quotes to strip, just return
  if ((strToStrip->size() < 2) || 
      (strToStrip->at(0) != '"') || 
      (strToStrip->at(strToStrip->size() - 1) != '"'))
  {
    return true;
  }
  
  // Matching quotes found so chop of the first and last char
  strToStrip->erase(strToStrip->begin());
  strToStrip->resize(strToStrip->size() - 1);

  return true;
               
} // end method stripQuotes


/**
 * This method strips out bad characters
**/  
AREXPORT bool ArUtil::fixBadCharacters(
	std::string *strToStrip, bool removeSpaces, bool fixOtherWhiteSpace)
{
  if (strToStrip == NULL) {
    ArLog::log(ArLog::Normal,
               "ArUtil::fixBadCharacters() NULL string");
    return false;
  }
  
  // The .length() isn't guaranteed to be O(1) (it could be O(n)) but
  // it's not usually O(1).... and hopefully this isn't called that
  // often (the original place was the identifer, which is once at
  // startup)... most of the other solutions I've seen are way more
  // complicated
  for (size_t i = 0; i < (*strToStrip).length(); i++)
  {
    if (!removeSpaces && (*strToStrip)[i] == ' ')
      continue;
    else if (!removeSpaces && fixOtherWhiteSpace && isspace((*strToStrip)[i]))
      (*strToStrip)[i] = ' ';
    else if ((*strToStrip)[i] == '(' || (*strToStrip)[i] == '{' || 
	     (*strToStrip)[i] == '<')
      (*strToStrip)[i] = '[';
    else if ((*strToStrip)[i] == ')' || (*strToStrip)[i] == '}' || 
	     (*strToStrip)[i] == '>')
      (*strToStrip)[i] = ']';
    // the new way that lets us work with utf8
    else if (removeSpaces && isspace((*strToStrip)[i]))
      (*strToStrip)[i] = '-';
    else if ((*strToStrip)[i] == '!' || (*strToStrip)[i] == '@' || 
	     (*strToStrip)[i] == '#' || (*strToStrip)[i] == '$' || 
	     (*strToStrip)[i] == '%' || (*strToStrip)[i] == '^' || 
	     (*strToStrip)[i] == '&' || (*strToStrip)[i] == '*' || 
	     (*strToStrip)[i] == '=' || (*strToStrip)[i] == '|' || 
	     (*strToStrip)[i] == '\\' || (*strToStrip)[i] == '\'' ||
	     (*strToStrip)[i] == '"' || (*strToStrip)[i] == ':' ||
	     (*strToStrip)[i] == ';' || (*strToStrip)[i] == '?' ||
	     (*strToStrip)[i] == '/' || (*strToStrip)[i] == '~' ||
	     (*strToStrip)[i] == '`')
      (*strToStrip)[i] = '-';
    else
      continue;

    /* the old way that caused us issues with utf8
    else if (isalpha((*strToStrip)[i]) || isdigit((*strToStrip)[i]) || 
	     (*strToStrip)[i] == '.' || (*strToStrip)[i] == '_' || 
	     (*strToStrip)[i] == '-' || (*strToStrip)[i] == '+' || 
	     (*strToStrip)[i] == '[' || (*strToStrip)[i] == ']')
      continue;
    else
      (*strToStrip)[i] = '-';
    */
  }

  return true;
               
} // end method fixBadCharacters


/** Append a directory separator character to the given path string, depending on the
 * platform.  On Windows, a backslash ('\\') is added. On other platforms, a 
 * forward slash ('/') is appended. If there is no more allocated space in the
 * path string (as given by the @arg pathLength parameter), no character will be appended.
   @param path the path string to append a slash to
   @param pathLength maximum length allocated for path string
*/
AREXPORT void ArUtil::appendSlash(char *path, size_t pathLength)
{
  // first check boundary
  size_t len;
  len = strlen(path);
  if (len > pathLength - 2)
    return;

  if (len == 0 || (path[len - 1] != '\\' && path[len - 1] != '/'))
  {
#ifdef WIN32
    path[len] = '\\';
#else
    path[len] = '/';
#endif
    path[len + 1] = '\0';
  }
}

/** Append the appropriate directory separator for this platform (a forward
 * slash "/" on Linux, or a backslash "\" on Windows) to @arg path.
 */
AREXPORT void ArUtil::appendSlash(std::string &path)
{
  // first check boundary
  size_t len = path.length();
  if ((len == 0) || 
      (path[len - 1] != SEPARATOR_CHAR && path[len - 1] != OTHER_SEPARATOR_CHAR)) {
    path += SEPARATOR_STRING;
  }

} // end method appendSlash

/**
   Replace in @a path all incorrect directory separators for this platform with
the correct directory separator character (forward slash '/' on Linux, backslash
'\' on Windows).
   @param path the path in which to fix the orientation of the slashes
   @param pathLength the maximum length of path
*/
AREXPORT void ArUtil::fixSlashes(char *path, size_t pathLength)
{
#ifdef WIN32
  fixSlashesBackward(path, pathLength);
#else
  fixSlashesForward(path, pathLength);
#endif
}

/** Replace any forward slash charactars ('/') in @a path with backslashes ('\').
   @param path the path in which to fix the orientation of the slashes
   @param pathLength size of @a path
*/
AREXPORT void ArUtil::fixSlashesBackward(char *path, size_t pathLength)
{
  for (size_t i=0; path[i] != '\0' && i < pathLength; i++)
  {
    if (path[i] == '/')
      path[i]='\\';
  }
}

/** Replace any forward slash charactars ('/') in @a path with backslashes ('\').
   @param path the path in which to fix the orientation of the slashes
   @param pathLength size of @a path
*/
AREXPORT void ArUtil::fixSlashesForward(char *path, size_t pathLength)
{

  for (size_t i=0; path[i] != '\0' && i < pathLength; i++)
  {
    if (path[i] == '\\')
      path[i]='/';
  }
}

/**
   Replace in @a path all incorrect directory separators for this platform with
the correct directory separator character (forward slash '/' on Linux, backslash
'\' on Windows).
   @param path the path in which to fix the orientation of the slashes
*/
AREXPORT void ArUtil::fixSlashes(std::string &path) 
{
  for (size_t i = 0; i < path.length(); i++)
  {
    if (path[i] == OTHER_SEPARATOR_CHAR)
      path[i]= SEPARATOR_CHAR;
  }
}
  
/** What is the appropriate directory path separator character for this
 * platform? */
AREXPORT char ArUtil::getSlash()
{
  return SEPARATOR_CHAR;
}

/**
   This function will take the @a baseDir and add @a insideDir after
   it, separated by appropriate directory path separators for this platform,
   with a final directory separator retained or added after @a inside dir.
   For example on Linux, this results in 'baseDir/insideDir/', whereas
   backslashes are used on Windows instead. The resulting string is 
   placed in the @a dest buffer (the path string will be truncated if
   there is not enough space in @a dest according to @a destLength)

   @param dest the place to put the result
   @param destLength the length available in @a dest
   @param baseDir the directory to start with
   @param insideDir the directory to place after the baseDir 
**/
AREXPORT void ArUtil::addDirectories(char *dest, size_t destLength, 
				     const char *baseDir,
				     const char *insideDir)
{
  // start it off
  strncpy(dest, baseDir, destLength - 1);
  // make sure we have a null term
  dest[destLength - 1] = '\0';
  // toss on that slash
  appendSlash(dest, destLength);
  // put on the inside dir
  strncat(dest, insideDir, destLength - strlen(dest) - 1);
  // now toss on that slash
  appendSlash(dest, destLength);
  // and now fix up all the slashes
  fixSlashes(dest, destLength);
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if @a str  is  found, respectively, to be less than, to
    match, or be greater than @a str2. 
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
AREXPORT int ArUtil::strcmp(const std::string &str, const std::string &str2)
{
  return ::strcmp(str.c_str(), str2.c_str());
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
AREXPORT int ArUtil::strcmp(const std::string &str, const char *str2)
{
  if (str2 != NULL) {
    return ::strcmp(str.c_str(), str2);
  }
  else {
    return 1;
  }
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
AREXPORT int ArUtil::strcmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
AREXPORT int ArUtil::strcmp(const char *str, const char *str2)
{
  if ((str != NULL) && (str2 != NULL)) {
    return ::strcmp(str, str2);
  }
  else if ((str == NULL) && (str2 == NULL)) {
    return 0;
  }
  else if (str == NULL) {
    return -1;
  }
  else { // str2 == NULL
    return 1;
  }
}


/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
AREXPORT int ArUtil::strcasecmp(const std::string &str, 
                                const std::string &str2)
{
  return ::strcasecmp(str.c_str(), str2.c_str());
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
AREXPORT int ArUtil::strcasecmp(const std::string &str, const char *str2)
{
  if (str2 != NULL) {
    return ::strcasecmp(str.c_str(), str2);
  }
  else {
    return 1;
  }
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
AREXPORT int ArUtil::strcasecmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcasecmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
AREXPORT int ArUtil::strcasecmp(const char *str, const char *str2)
{
  if ((str != NULL) && (str2 != NULL)) {
    return ::strcasecmp(str, str2);
  }
  else if ((str == NULL) && (str2 == NULL)) {
    return 0;
  }
  else if (str == NULL) {
    return -1;
  }
  else { // str2 == NULL
    return 1;
  }
}


AREXPORT bool ArUtil::strSuffixCmp(const char *str, const char *suffix)
{
  if (str != NULL && str[0] != '\0' && 
      suffix != NULL && suffix[0] != '\0' &&
      strlen(str) > strlen(suffix) + 1 &&
      strncmp(&str[strlen(str) - strlen(suffix)], 
		  suffix, strlen(suffix)) == 0)
    return true;
  else
    return false;
 
}

AREXPORT bool ArUtil::strSuffixCaseCmp(const char *str, const char *suffix)
{
  if (str != NULL && str[0] != '\0' && 
      suffix != NULL && suffix[0] != '\0' &&
      strlen(str) > strlen(suffix) + 1 &&
      strncasecmp(&str[strlen(str) - strlen(suffix)], 
		  suffix, strlen(suffix)) == 0)
    return true;
  else
    return false;
}


AREXPORT int ArUtil::strcasequotecmp(const std::string &inStr1, 
                                     const std::string &inStr2)
{
std::string str1 = inStr1;
std::string str2 = inStr2;

	int x = 0;
	while (x < str1.length()) {
		if (isalpha(str1[x]) && isupper(str1[x]))
			str1[x] = tolower(str1[x]);
	x++;
	}

	x = 0;
	while (x < str2.length()) {
		if (isalpha(str2[x]) && isupper(str2[x]))
			str2[x] = tolower(str2[x]);
	x++;
	}
  
  int len1 = str1.length();
  size_t pos1 = 0;
  if ((len1 >= 2) && (str1[0] == '\"') && (str1[len1 - 1] == '\"')) {
    pos1 = 1;
  }
  int len2 = str2.length();
  size_t pos2 = 0;
  if ((len2 >= 2) && (str2[0] == '\"') && (str2[len2 - 1] == '\"')) {
    pos2 = 1;
  }

  /* Unfortunately gcc2 does't support the 5 argument version of std::string::compare()... 
   * (Furthermore, note that it's 3-argument compare has the arguments in the wrong order.)
   */
#if defined(__GNUC__) && (__GNUC__ <= 2) && (__GNUC_MINOR__ <= 96)
#warning Using GCC 2.96 or less so must use nonstandard std::string::compare method.
  int cmp = str1.compare(str2.substr(pos2, len2 - 2 * pos2), pos1, len1 - 2 * pos1);
#else
  int cmp = str1.compare(pos1, 
	 	  	 len1 - 2 * pos1,
                         str2,
                         pos2, 
			 len2 - 2 * pos2);
#endif

  return cmp;

} // end method strcasequotecmp

/**
   This copies src into dest but puts a \ before any spaces in src,
   escaping them... its mostly for use with ArArgumentBuilder... 
   make sure you have at least maxLen spaces in the arrays that you're passing 
   as dest... this allocates no memory
**/
AREXPORT void ArUtil::escapeSpaces(char *dest, const char *src, size_t maxLen)
{
  size_t i, adj, len;

  len = strlen(src);
  // walk it, when we find one toss in the slash and incr adj so the
  // next characters go in the right space
  for (i = 0, adj = 0; i < len && i + adj < maxLen; i++)
  {
    if (src[i] == ' ')
    {
      dest[i+adj] = '\\';
      adj++;
    }
    dest[i+adj] = src[i];
  }
  // make sure its null terminated
  dest[i+adj] = '\0';
}

/**
   This copies src into dest but makes it lower case make sure you
   have at least maxLen arrays that you're passing as dest... this
   allocates no memory
**/
AREXPORT void ArUtil::lower(char *dest, const char *src, size_t maxLen)
{
  size_t i;
  size_t len;
  
  len = strlen(src);
  for (i = 0; i < len && i < maxLen; i++)
    dest[i] = tolower(src[i]);
  dest[i] = '\0';

}


AREXPORT bool ArUtil::isOnlyAlphaNumeric(const char *str)
{
  unsigned int ui;
  unsigned int len;
  if (str == NULL)
    return true;
  for (ui = 0, len = strlen(str); ui < len; ui++)
  {
    if (!isalpha(str[ui]) && !isdigit(str[ui]) && str[ui] != '\0' && 
	str[ui] != '+' && str[ui] != '-')
      return false;
  }
  return true;
}

AREXPORT bool ArUtil::isOnlyNumeric(const char *str)
{
  if (str == NULL)
    return true;
  for (unsigned i = 0, len = strlen(str); i < len; i++)
  {
    if (!isdigit(str[i]) && str[i] != '\0' && str[i] != '+' && str[i] != '-')
      return false;
  }
  return true;
}

AREXPORT bool ArUtil::isStrEmpty(const char *str)
{
	if (str == NULL) {
		return true;
	}
	if (str[0] == '\0') {
		return true;
	}
	return false;

} // end method isStrEmpty

  
AREXPORT bool ArUtil::isStrInList(const char *str,
                                  const std::list<std::string> &list,
                                  bool isIgnoreCase)
{
  if (str == NULL) {
    return false;
  }
  for (std::list<std::string>::const_iterator aIter = list.begin();
       aIter != list.end();
       aIter++) {
    if (!isIgnoreCase) {
      if (strcmp((*aIter).c_str(), str) == 0) {
        return true;
      }
    }
    else { // ignore case
      if (strcasecmp((*aIter).c_str(), str) == 0) {
        return true;
      }
    } // end else ignore case
  } // end for each string

  return false;

} // end method isStrInList


AREXPORT const char *ArUtil::convertBool(int val)
{
  if (val)
    return TRUESTRING;
  else
    return FALSESTRING;
}

AREXPORT double ArUtil::atof(const char *nptr)
{
  if (strcasecmp(nptr, "inf") == 0)
    return HUGE_VAL;
  else if (strcasecmp(nptr, "-inf") == 0)
    return -HUGE_VAL;
  else {
	  return ::atof(nptr);
  }
}

 /// Prints the given format string an arguments in a NON-localized fashion.
/** @swigomit */
AREXPORT bool ArUtil::printValue(char *buf, size_t bufLen, const char *formatString, ...)
{
  if ((buf == NULL) || (bufLen <= 0) || (formatString == NULL)) { 
    ArLog::log(ArLog::Normal,
               "ArUtil::printValue() invalid parameters");
    return false;
  }

  // TODO i18n This needs to be investigated.  LC_NUMERIC was causing a
  // crash in the client code.  LC_ALL may cause display problems when
  // the locale is Japanese (not sure).
  std::setlocale(LC_ALL, "C");

  buf[0] = '\0';
  va_list ptr;
  va_start(ptr, formatString);
  //vsprintf(buf, str, ptr);
  vsnprintf(buf, bufLen, formatString, ptr);
  buf[bufLen - 1] = '\0';
  va_end(ptr);

  return true;

}
  
/// Reads a double value from the given input string in a NON-localized fashion.
/** @swigomit */ 
AREXPORT double ArUtil::readDoubleValue(const char *buf, bool *ok)
{
  // TODO i18n This needs to be investigated.  LC_NUMERIC was causing a
  // crash in the client code.  LC_ALL may cause display problems when
  // the locale is Japanese (not sure).
  std::setlocale(LC_ALL, "C");
  char *endPtr = NULL;
  double ret = strtod(buf, &endPtr);
  if (endPtr[0] == '\0' && endPtr != buf) {
    if (ok != NULL) {
      *ok = true;
    }
    return ret;
  }
  else {
    if (ok != NULL) {
      *ok = false;
    }
    return 0;
  }
 
}

AREXPORT void ArUtil::functorPrintf(ArFunctor1<const char *> *functor,
				    const char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  //vsprintf(buf, str, ptr);
  vsnprintf(buf, sizeof(buf) - 1, str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}


AREXPORT void ArUtil::functorPrintf(ArFunctor1<const char *> *functor,
				    char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  //vsprintf(buf, str, ptr);
  vsnprintf(buf, sizeof(buf) - 1, str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}


AREXPORT void ArUtil::writeToFile(const char *str, FILE *file)
{
  fputs(str, file);
}


/** 
   This function reads a string from a file.
   The file can contain spaces or tabs, but a '\\r'
   or '\\n' will be treated as the end of the string, and the string
   cannot have more characters than the value given by strLen.  This is mostly for internal use
   with Linux to determine the Aria directory from a file in /etc, but
   will work with Linux or Windows. 

   @param fileName name of the file in which to look
   @param str the string to copy the file contents into
   @param strLen the maximum allocated length of str
**/
AREXPORT bool ArUtil::getStringFromFile(const char *fileName, 
					char *str, size_t strLen)
{
  FILE *strFile;
  unsigned int i;
  
  str[0] = '\0';
  
  if ((strFile = ArUtil::fopen(fileName, "r")) != NULL)
  {
    fgets(str, strLen, strFile);
    for (i = 0; i < strLen; i++)
    {
      if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
      {
	str[i] = '\0';
	fclose(strFile);
	break;
      }
    }
  }
  else
  {
    str[0] = '\0';
    return false;
  }
  return true;
}

/**
 * Look up the given value under the given key, within the given registry root
 * key.

   @param root the root key to use, one of the REGKEY enum values

   @param key the name of the key to find

   @param value the value name in which to find the string

   @param str where to put the string found, or if it could not be
   found, an empty (length() == 0) string

   @param len the length of the allocated memory in str

   @return true if the string was found, false if it was not found or if there was a problem such as the string not being long enough 
 **/

AREXPORT bool ArUtil::getStringFromRegistry(REGKEY root,
						   const char *key,
						   const char *value,
						   char *str,
						   int len)
{
#ifndef WIN32
  return false;
#else // WIN32

  HKEY hkey;
  int err;
  unsigned long numKeys;
  unsigned long longestKey;
  unsigned long numValues;
  unsigned long longestValue;
  unsigned long longestDataLength;
  char *valueName;
  unsigned long valueLength;
  unsigned long type;
  char *data;
  unsigned long dataLength;
  HKEY rootKey;


  switch (root)
  {
  case REGKEY_CLASSES_ROOT:
    rootKey = HKEY_CLASSES_ROOT;
    break;
  case REGKEY_CURRENT_CONFIG:
    rootKey = HKEY_CURRENT_CONFIG;
    break;
  case REGKEY_CURRENT_USER:
    rootKey = HKEY_CURRENT_USER;
    break;
  case REGKEY_LOCAL_MACHINE:
    rootKey = HKEY_LOCAL_MACHINE;
    break;
  case REGKEY_USERS:
    rootKey=HKEY_USERS;
    break;
  default:
    ArLog::log(ArLog::Terse, 
	       "ArUtil::getStringFromRegistry: Bad root key given.");
    return false;
  }


  if ((err = RegOpenKeyExA(rootKey, key, 0, KEY_READ, &hkey)) == ERROR_SUCCESS) // was RegOpenKeyEx()
  {
    //printf("Got a key\n");
    if (RegQueryInfoKeyA(hkey, NULL, NULL, NULL, &numKeys, &longestKey, NULL, 
			&numValues, &longestValue, &longestDataLength, NULL, NULL) == ERROR_SUCCESS)
    // was RegQueryInfoKey()
    {
	/*
      printf("Have %d keys longest is %d, have %d values longest name is %d, longest data is %d\n",
	     numKeys, longestKey, numValues, longestValue, longestDataLength);
	*/	 
      data = new char[longestDataLength+2];
      valueName = new char[longestValue+2];
      for (unsigned long i = 0; i < numValues; ++i)
      {
	dataLength = longestDataLength+1;
	valueLength = longestValue+1;
  if ((err = RegEnumValueA(hkey, i, valueName, &valueLength, NULL,
				&type, (unsigned char *)data, &dataLength)) == ERROR_SUCCESS)
        // Was RegEnumValue()
	{
		//printf("Enumed value %d, name is %s, value is %s\n", i, valueName, data);
	  if (strcmp(value, valueName) == 0)
	  {
	    if (len < dataLength)
	    {
	      ArLog::log(ArLog::Terse,"ArUtil::getStringFromRegistry: str passed in not long enough for data.");
	      delete[] data;
	      delete[] valueName;
	      return false;
	    }
	    strncpy(str, data, len);
	    delete[] data;
	    delete[] valueName;
	    return true;
	  }
	}
	/*
	else
		printf("Couldn't enum value %d cause %d\n",i,  err);
		*/
	    }
      delete[] data;
      delete[] valueName;
    }
	/*
    else
      printf("QueryInfoKey failed\n");
	  */
  }
  /*
  else
    printf("No key %d\n", err);
  */
  return false;
#endif
}
  
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK) && !defined(__MACH__)
bool ArTime::ourMonotonicClock = true;
#else
bool ArTime::ourMonotonicClock = false;
#endif 

AREXPORT void ArTime::setToNow(void)
{
// if we have the best way of finding time use that
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK) && !defined(__MACH__)
  if (ourMonotonicClock)
  {
    struct timespec timeNow;
    if (clock_gettime(CLOCK_MONOTONIC, &timeNow) == 0)
    {
      // start a million seconds into the future so we have some room
      // to go backwards
      mySec = timeNow.tv_sec + 1000000;
      myMSec = timeNow.tv_nsec / 1000000;
      return;
    }
    else
    {
      ourMonotonicClock = false;
      ArLog::logNoLock(ArLog::Terse, "ArTime::setToNow: invalid return from clock_gettime.");
    }
  }
#endif
// if our good way didn't work use the old ways
#ifndef WIN32
  struct timeval timeNow;
  
  if (gettimeofday(&timeNow, NULL) == 0)
  {
    // start a million seconds into the future so we have some room
    // to go backwards
    mySec = timeNow.tv_sec + 1000000;
    myMSec = timeNow.tv_usec / 1000;
  }
  else
    ArLog::logNoLock(ArLog::Terse, "ArTime::setToNow: invalid return from gettimeofday.");
// thats probably not available in windows, so this is the one we've been using
#else
      /* this should be the better way, but it doesn't really work...
	 this would be seconds from 1970, but it is based on the
	 hardware timer or something and so winds up not being updated
	 all the time and winds up being some number of ms < 20 ms off
      struct _timeb startTime;
      _ftime(&startTime);
      mySec = startTime.time;
      myMSec = startTime.millitm;*/
      // so we're going with just their normal function, msec since boot
  long timeNow;
  timeNow = timeGetTime();
  // start a million seconds into the future so we have some room
  // to go backwards 
  mySec = timeNow / 1000 + 1000000;
  myMSec = timeNow % 1000;
// but if the good way isn't available use the old way...
#endif
      
}

AREXPORT ArRunningAverage::ArRunningAverage(size_t numToAverage)
{
  myNumToAverage = numToAverage;
  myTotal = 0;
  myNum = 0;
  myUseRootMeanSquare = false;
}

AREXPORT ArRunningAverage::~ArRunningAverage()
{

}

AREXPORT double ArRunningAverage::getAverage(void) const
{
  if (myNum == 0)
    return 0.0;

  if (myUseRootMeanSquare)
    return sqrt(myTotal / myNum);
  else
    return myTotal / myNum;
}

AREXPORT void ArRunningAverage::add(double val)
{
  if (myUseRootMeanSquare)
    myTotal += (val * val);
  else
    myTotal += val;
  myNum++;
  myVals.push_front(val);
  if (myVals.size() > myNumToAverage || myNum > myNumToAverage)
  {
    if (myUseRootMeanSquare)
      myTotal -= (myVals.back() * myVals.back());
    else
      myTotal -= myVals.back();
    myNum--;
    myVals.pop_back();
  }
}

AREXPORT void ArRunningAverage::clear(void)
{
  while (myVals.size() > 0)
    myVals.pop_back();
  myNum = 0;
  myTotal = 0;
}

AREXPORT size_t ArRunningAverage::getNumToAverage(void) const
{
  return myNumToAverage;
}

AREXPORT void ArRunningAverage::setNumToAverage(size_t numToAverage)
{
  myNumToAverage = numToAverage;
  while (myVals.size() > myNumToAverage)
  {
    if (myUseRootMeanSquare)
      myTotal -= (myVals.back() * myVals.back());
    else
      myTotal -= myVals.back();
    myNum--;
    myVals.pop_back();
  }
}

AREXPORT size_t ArRunningAverage::getCurrentNumAveraged(void)
{
  return myNum;
}

AREXPORT void ArRunningAverage::setUseRootMeanSquare(bool useRootMeanSquare)
{
  if (myUseRootMeanSquare != useRootMeanSquare)
  {
    myTotal = 0;
    std::list<double>::iterator it;
    for (it = myVals.begin(); it != myVals.end(); it++)
    {
      if (useRootMeanSquare)
	myTotal += ((*it) * (*it));
      else
	myTotal += (*it);
    }
  }

  myUseRootMeanSquare = useRootMeanSquare;
}

AREXPORT bool ArRunningAverage::getUseRootMeanSquare(void)
{
  return myUseRootMeanSquare;
}

AREXPORT ArRootMeanSquareCalculator::ArRootMeanSquareCalculator()
{
  clear();
  myName = "ArRootMeanSquareCalculator";
}

AREXPORT ArRootMeanSquareCalculator::~ArRootMeanSquareCalculator()
{

}

AREXPORT double ArRootMeanSquareCalculator::getRootMeanSquare (void) const
{
  if (myNum == 0)
    return 0;
  else
    return sqrt((double) myTotal / (double)myNum);
}

AREXPORT void ArRootMeanSquareCalculator::add(int val)
{
  myTotal += val * val;
  myNum++;
  if (myTotal < 0)
  {
    ArLog::log(ArLog::Normal, "%s: total wrapped, resetting", myName.c_str());
    clear();
    // this isn't a clean fix, but won't let it infinitely loop on a bad value
    //add(val);
  }
}

AREXPORT void ArRootMeanSquareCalculator::clear(void)
{
  myTotal = 0;
  myNum = 0;
}

AREXPORT size_t ArRootMeanSquareCalculator::getCurrentNumAveraged(void)
{
  return myNum;
}

AREXPORT void ArRootMeanSquareCalculator::setName(const char *name)
{
  if (name != NULL)
    myName = name;
  else
    myName = "ArRootMeanSquareCalculator";
}

AREXPORT const char *ArRootMeanSquareCalculator::getName(void)
{
  return myName.c_str();
}

#ifndef WIN32

AREXPORT ArDaemonizer::ArDaemonizer(int *argc, char **argv, 
				    bool closeStdErrAndStdOut) :
  myParser(argc, argv),
  myLogOptionsCB(this, &ArDaemonizer::logOptions)
{
  myIsDaemonized = false;
  myCloseStdErrAndStdOut = closeStdErrAndStdOut;
  Aria::addLogOptionsCB(&myLogOptionsCB);
}

AREXPORT ArDaemonizer::~ArDaemonizer()
{

}

AREXPORT bool ArDaemonizer::daemonize(void)
{
  if (myParser.checkArgument("-daemonize") ||
      myParser.checkArgument("-d"))
  {
    return forceDaemonize();
  }
  else
    return true;

}

/**
   This returns true if daemonizing worked, returns false if it
   didn't... the parent process exits here if forking worked.
 **/
AREXPORT bool ArDaemonizer::forceDaemonize(void)
{
    switch (fork())
    {
    case 0: // child process just return
      myIsDaemonized = true;
      if (myCloseStdErrAndStdOut)
      {
	fclose(stdout);
	fclose(stderr);
      }
      return true;
    case -1: // error.... fail
      printf("Can't fork");
      ArLog::log(ArLog::Terse, "ArDaemonizer: Can't fork");
      return false;
    default: // parent process
      printf("Daemon started\n");
      exit(0);
    }
}

AREXPORT void ArDaemonizer::logOptions(void) const
{
  ArLog::log(ArLog::Terse, "Options for Daemonizing:");
  ArLog::log(ArLog::Terse, "-daemonize");
  ArLog::log(ArLog::Terse, "-d");
  ArLog::log(ArLog::Terse, "");
}

#endif // WIN32


std::map<ArPriority::Priority, std::string> ArPriority::ourPriorityNames;
std::map<std::string, ArPriority::Priority, ArStrCaseCmpOp> ArPriority::ourNameToPriorityMap;

std::string ArPriority::ourUnknownPriorityName;
bool ArPriority::ourStringsInited = false;

AREXPORT const char *ArPriority::getPriorityName(Priority priority) 
{

  if (!ourStringsInited)
  {
    ourPriorityNames[IMPORTANT]     = "Basic";
    ourPriorityNames[NORMAL]        = "Intermediate";
    ourPriorityNames[TRIVIAL]       = "Advanced";
    ourPriorityNames[DETAILED]      = "Advanced";

    ourPriorityNames[EXPERT]        = "Expert";
    ourPriorityNames[FACTORY]       = "Factory";
    ourPriorityNames[CALIBRATION]   = "Calibration";
    
    for (std::map<ArPriority::Priority, std::string>::iterator iter = ourPriorityNames.begin();
         iter != ourPriorityNames.end();
         iter++) {
      ourNameToPriorityMap[iter->second] = iter->first;
    }

    ourUnknownPriorityName  = "Unknown";
    ourStringsInited        = true;
  }

  std::map<ArPriority::Priority, std::string>::iterator iter = 
                                                  ourPriorityNames.find(priority);
  if (iter != ourPriorityNames.end()) {
    return iter->second.c_str();
  }
  else {
    return ourUnknownPriorityName.c_str();
  }
}

AREXPORT ArPriority::Priority ArPriority::getPriorityFromName(const char *text, 
                                                              bool *ok)
{
  // This is merely called to initialize the map
  if (!ourStringsInited) {
     getPriorityName(IMPORTANT);
  }
   
  // Assume failure (until successful) 
  if (ok != NULL) {
    *ok = false;
  }

  if (ArUtil::isStrEmpty(text)) {
    ArLog::log(ArLog::Normal,
               "ArPriority::getPriorityFromName() error finding priority for empty text");
    return LAST_PRIORITY;
  }

  std::map<std::string, ArPriority::Priority, ArStrCaseCmpOp>::iterator iter = 
                                                  ourNameToPriorityMap.find(text);
  if (iter != ourNameToPriorityMap.end()) {
    if (ok != NULL) {
      *ok = true;
    }
    return iter->second;
  }
  
  ArLog::log(ArLog::Normal,
             "ArPriority::getPriorityFromName() error finding priority for %s",
             text);

  return LAST_PRIORITY;
  
} // end method getPriorityFromName


AREXPORT void ArUtil::putCurrentYearInString(char* s, size_t len)
{
  struct tm t;
  ArUtil::localtime(&t);
  snprintf(s, len, "%4d", 1900 + t.tm_year);
  s[len-1] = '\0';
}

AREXPORT void ArUtil::putCurrentMonthInString(char* s, size_t len)
{

  struct tm t;
  ArUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_mon + 1);
  s[len-1] = '\0';
}
AREXPORT void ArUtil::putCurrentDayInString(char* s, size_t len)
{
  struct tm t;
  ArUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_mday);
  s[len-1] = '\0';
}
AREXPORT void ArUtil::putCurrentHourInString(char* s, size_t len)
{
  struct tm t;
  ArUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_hour);
  s[len-1] = '\0';
}
AREXPORT void ArUtil::putCurrentMinuteInString(char* s, size_t len)
{
  struct tm t; 
  ArUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_min);
  s[len-1] = '\0';
}
AREXPORT void ArUtil::putCurrentSecondInString(char* s, size_t len)
{
  struct tm t;
  ArUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_sec);
  s[len-1] = '\0';
}



AREXPORT time_t ArUtil::parseTime(const char *str, bool *ok, bool toToday)
{

  struct tm tmOut;
  if (toToday)
  {
    struct tm now;
    if (!localtime(&now))
    {
      *ok = false;
      return 0;
    }
    memcpy(&tmOut, &now, sizeof(now));
  }
  else
  {
    memset(&tmOut, 0, sizeof(tmOut));
    // The day-of-the-month starts at 1 (not 0)...
    tmOut.tm_mday = 1;
    // Setting the year to 70 because if it is left at 0 or 1, then
    // the call to mktime() returns an apparently bogus value.  Think
    // that 70 makes sense since times are generally measured from
    // 1/1/1970 (but still, it's all a little strange).
    tmOut.tm_year = 70;
    tmOut.tm_isdst = -1; // Negative value means unknown
  }

  bool isValid = true;
  int hrs = -1;
  int min = -1;
  int sec = 0;

  ArArgumentBuilder separator(512, ':');
  separator.add(str);
  
  // if there's the wrong number of args, or any of the args aren't
  // integers then it's invalid and we won't parse it
  if ((separator.getArgc() != 2 && separator.getArgc() != 3) || 
      !separator.isArgInt(0) || !separator.isArgInt(1) ||
      (separator.getArgc() == 3 && !separator.isArgInt(2)))
  {
    //printf("Invalid... %d\n", separator.getArgc());
    //separator.log();
    isValid = false;
  }
  else
  {
    hrs = separator.getArgInt(0);
    min = separator.getArgInt(1);
    if (separator.getArgc() == 3)
      sec = separator.getArgInt(2);
    //printf("Was %02d:%02d:%02d", hrs, min, sec);
  }

  /*
  char *tempBuf = new char[strlen(str) + 1];
  strncpy(tempBuf, str, sizeof(tempBuf));
  
  // Attempted to use strptime, but it doesn't seem to be universally
  // available.
  char *pch = strtok(tempBuf, ":");
  if (pch != NULL) {
    hrs = atoi(pch);
  }
  
  pch = strtok(NULL, ":");
  if (pch != NULL) {
    min = atoi(pch);
  }
  */

  // make sure the actual numbers are valid
  if (!((hrs  >= 0) && (hrs < 24) && (min >= 0) && (min < 60) && 
	(sec >= 0) && (sec < 60)))
    isValid = false;
  
  if (isValid) 
  {
    tmOut.tm_hour = hrs;
    tmOut.tm_min  = min;
    tmOut.tm_sec  = sec;
  }
  
  time_t newTime = mktime(&tmOut);
  
  if (ok != NULL) 
  {
    *ok = (isValid && (newTime != -1));
  }
  
  //delete [] tempBuf;
  
  return newTime;

} // end method parseTime



AREXPORT bool ArUtil::localtime(const time_t *timep, struct tm *result) 
{
#ifdef WIN32
  ourLocaltimeMutex.lock();
  struct tm *r = ::localtime(timep);
  if(r == NULL) { 
    ourLocaltimeMutex.unlock();
    return false;
  }
  *result = *r; // copy the 'struct tm' object before unlocking.
  ourLocaltimeMutex.unlock();
  return true;
#else
  return (::localtime_r(timep, result) != NULL);
#endif
}

/** Call ArUtil::localtime() with the current time obtained by calling
* time(NULL).
*  @return false on error (e.g. invalid input), otherwise true.
*/
AREXPORT bool ArUtil::localtime(struct tm *result) 
{ 
  time_t now = time(NULL);
  return ArUtil::localtime(&now, result); 
}


#ifndef WIN32
/**
   @param baseDir the base directory to work from
   @param fileName the fileName to squash the case from
   
   @param result where to put the result
   @param resultLen length of the result

   @return true if it could find the file, the result is in result,
   false if it couldn't find the file
**/
AREXPORT bool ArUtil::matchCase(const char *baseDir, 
					   const char *fileName,
					   char *result,
					   size_t resultLen)
{

  /***
  ArLog::log(ArLog::Normal, 
             "ArUtil::matchCase() baseDir = \"%s\" fileName = \"%s\"",
             baseDir,
             fileName);
  ***/

  DIR *dir;
  struct dirent *ent;

  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif

  result[0] = '\0';

  std::list<std::string> split = splitFileName(fileName);
  std::list<std::string>::iterator it = split.begin();
  std::string finding = (*it);

  /*
  for (it = split.begin(); it != split.end(); it++)
  {
    printf("@@@@@@@@ %s\n", (*it).c_str());
  }
  */
  
  // how this works is we start at the base dir then read through
  // until we find what the next name we need, if entry is a directory
  // and we're not at the end of our string list then we change into
  // that dir and the while loop keeps going, if the entry isn't a
  // directory and matchs and its the last in our string list we've
  // found what we want
  if ((dir = opendir(baseDir)) == NULL)
  {
    ArLog::log(ArLog::Normal, 
	       "ArUtil: No such directory '%s' for base", 
	       baseDir);
    return false;
  }

  if (finding == ".")
  {
    it++;
    if (it != split.end())
    {
      finding = (*it);
    }
    else
    {
      ArLog::log(ArLog::Normal, 
		             "ArUtil: No file or directory given (base = %s file = %s)", 
		             baseDir,
                 fileName);
      closedir(dir);

      // KMC NEED TO DETERMINE WHICH IS CORRECT.
      // The following change appears to be necessary for maps, but is still
      // undergoing testing....
      //   Just return the given ".". (This is necessary to find maps in the local 
      //   directory under some circumstances.)
      //   snprintf(result, resultLen, finding.c_str());
      //   return true;  

      return false;

      
    }
  }

  while ((ent = readdir(dir)) != NULL)
  {
    // ignore some of these
    if (ent->d_name[0] == '.')
    {
      //printf("Ignoring %s\n", ent->d_name[0]);
      continue;
    }
    //printf("NAME %s finding %s\n", ent->d_name, finding.c_str());
    
    // we've found what we were looking for
    if (ArUtil::strcasecmp(ent->d_name, finding) == 0)
    {
      size_t lenOfResult;
      lenOfResult = strlen(result);

      // make sure we can put the filename in
      if (strlen(ent->d_name) > resultLen - lenOfResult - 2)
      {
	ArLog::log(ArLog::Normal, 
		   "ArUtil::matchCase: result not long enough");
	closedir(dir);
	return false;
      }
      //printf("Before %s", result);
      if (lenOfResult != 0)
      {
	result[lenOfResult] = separator;
	result[lenOfResult+1] = '\0';
      }
      // put the filename in
      strcpy(&result[strlen(result)], ent->d_name);
      //printf("after %s\n", result);
      // see if we're at the end
      it++;
      if (it != split.end())
      {
	//printf("Um.........\n");
	finding = (*it);
	std::string wholeDir;
	wholeDir = baseDir;
	wholeDir += result;
	closedir(dir);
	//printf("'%s' '%s' '%s'\n", baseDir, result, wholeDir.c_str());
	if ((dir = opendir(wholeDir.c_str())) == NULL)
	{
	  ArLog::log(ArLog::Normal, 
		     "ArUtil::matchCase: Error going into %s", 
		     result);
	  return false;
	}
      }
      else
      {
	//printf("\n########## Got it %s\n", result);
	closedir(dir);
	return true;
      }
    }
  }
  ArLog::log(ArLog::Normal, 
	     "ArUtil::matchCase: %s doesn't exist in %s", fileName, 
	     baseDir);
  //printf("!!!!!!!! %s", finding.c_str());
  closedir(dir);
  return false;
} 

#endif // !WIN32


AREXPORT bool ArUtil::getDirectory(const char *fileName, 
					     char *result, size_t resultLen)
{
  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif
  
  if (fileName == NULL || fileName[0] == '\0' || resultLen == 0)
  {
    ArLog::log(ArLog::Normal, "ArUtil: getDirectory, bad setup");
    return false;
  }
  
  // just play in the result buffer
  strncpy(result, fileName, resultLen - 1);
  // make sure its nulled
  result[resultLen - 1] = '\0';
  char *toPos;
  ArUtil::fixSlashes(result, resultLen);
  // see where the last directory is
  toPos = strrchr(result, separator);
  // if there's no divider it must just be a file name
  if (toPos == NULL)
  {
    result[0] = '\0';
    return true;
  }
  // otherwise just toss a null into the last separator and we're done
  else
  {    
    *toPos = '\0';
    return true;
  }
}

AREXPORT bool ArUtil::getFileName(const char *fileName, 
					 char *result, size_t resultLen)
{
  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif
  
  if (fileName == NULL || fileName[0] == '\0' || resultLen == 0)
  {
    ArLog::log(ArLog::Normal, "ArUtil: getFileName, bad setup");
    return false;
  }

  char *str;
  size_t fileNameLen = strlen(fileName);
  str = new char[fileNameLen + 1];
  //printf("0 %s\n", fileName);
  // just play in the result buffer
  strncpy(str, fileName, fileNameLen);
  // make sure its nulled
  str[fileNameLen] = '\0';
  //printf("1 %s\n", str);

  char *toPos;
  ArUtil::fixSlashes(str, fileNameLen + 1);
  //printf("2 %s\n", str);
  // see where the last directory is
  toPos = strrchr(str, separator);
  // if there's no divider it must just be a file name
  if (toPos == NULL)
  {
    // copy the filename in and make sure it has a null
    strncpy(result, str, resultLen - 1);
    result[resultLen - 1] = '\0';
    //printf("3 %s\n", result);
    delete[] str;
    return true;
  }
  // otherwise take the section from that separator to the end
  else
  {
    strncpy(result, &str[toPos - str + 1], resultLen - 2);
    result[resultLen - 1] = '\0';
    //printf("4 %s\n", result);
    delete[] str;
    return true;
  }
}

int ArUtil::filestat(const std::string &fileName, struct stat *buffer)
{
#if defined(WIN32) && !defined(MINGW)
  // On Windows, stat() does not work if the filename contains any
  // extended/unicode characters.  Convert to the wide char version.
  // On Windows, it is necessary to convert the given UTF-8
  // strings to UTF-16, and then call the special "wide" version
  // of fopen.  This allows Japanese characters, etc.

  // Determine the number of characters in the output buffer.
  // Pass -1 as the src size to convert everything up to and
  // including the nul terminator.
  int wideFileNameLen = MultiByteToWideChar(
                        CP_UTF8, 0, fileName.c_str(), -1, NULL, 0);

  if (wideFileNameLen > 0) {

    // Allocate buffers for the UTF16 strings.
    wchar_t *wideFileName = new wchar_t[wideFileNameLen + 1];

    // Convert the UTF8 strings to UTF16.
    int s = MultiByteToWideChar(
      CP_UTF8, 0, fileName.c_str(), -1, &wideFileName[0], wideFileNameLen);

    if (wideFileNameLen == s) {

      struct _stat64i32 wideBuffer;
      // Open the file.
      int result = ::_wstat(wideFileName, &wideBuffer);

      if (result == 0) {
        buffer->st_dev = wideBuffer.st_dev;
        buffer->st_ino = wideBuffer.st_ino;
        buffer->st_mode = wideBuffer.st_mode;
        buffer->st_nlink = wideBuffer.st_nlink;
        buffer->st_uid = wideBuffer.st_uid;
        buffer->st_gid = wideBuffer.st_gid;
        buffer->st_rdev = wideBuffer.st_rdev;
        buffer->st_size = wideBuffer.st_size;

        // Possible loss of data
        buffer->st_atime = wideBuffer.st_atime;
        buffer->st_mtime = wideBuffer.st_mtime;
        buffer->st_ctime = wideBuffer.st_ctime;

      }
    }

    delete [] wideFileName;
  }

#else // WIN32

  return ::stat(fileName.c_str(), buffer);

#endif // WIN32

   return -1; // error
}

#ifndef WIN32
/**
   This function assumes the slashes are all heading the right way already.
**/
std::list<std::string> ArUtil::splitFileName(const char *fileName)
{
  std::list<std::string> split;
  if (fileName == NULL)
    return split;

  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif

  size_t len;
  size_t i;
  size_t last;
  bool justSepped;
  char entry[2048];
  for (i = 0, justSepped = false, last = 0, len = strlen(fileName); 
       ; 
       i++)
  {
    
    if ((fileName[i] == separator && !justSepped) 
	|| fileName[i] == '\0' || i >= len)
    {
      if (i - last > 2047)
      {
	ArLog::log(ArLog::Normal, "ArUtil::splitFileName: some directory or file too long");
      }
      if (!justSepped)
      {
	strncpy(entry, &fileName[last], i - last);
	entry[i-last] = '\0';
	split.push_back(entry);

	justSepped = true;
      }
      if (fileName[i] == '\0' || i >= len)
	return split;
    }
    else if (fileName[i] == separator && justSepped)
    {
      justSepped = true;
      last = i;
    }
    else if (fileName[i] != separator && justSepped)
    {
      justSepped = false;
      last = i;
    }
  }
  ArLog::log(ArLog::Normal, "ArUtil::splitFileName: file str ('%s') happened weird", fileName);
  return split;
}



#endif // !WIN32


AREXPORT bool ArUtil::changeFileTimestamp(const char *fileName, 
                                          time_t timestamp) 
{
  if (ArUtil::isStrEmpty(fileName)) {
    ArLog::log(ArLog::Normal,
               "Cannot change date on file with empty name");
    return false;
  }
#ifdef WIN32

  FILETIME fileTime;

  HANDLE hFile = CreateFileA(fileName,
                            GENERIC_READ | GENERIC_WRITE,
                             0,NULL,
                             OPEN_EXISTING,
                             0,NULL);
   // note was CreateFile()

  if (hFile == NULL) {
    return false;
  }


  // The following is extracted from the MSDN article "Converting a time_t Value
  // to a File Time".
  LONGLONG temp = Int32x32To64(timestamp, 10000000) + 116444736000000000;
  fileTime.dwLowDateTime = (DWORD) temp;
  fileTime.dwHighDateTime = temp >> 32;

  SetFileTime(hFile, 
              &fileTime, 
              (LPFILETIME) NULL,  // don't change last access time (?)
              &fileTime);

  CloseHandle(hFile);

#else // unix
        
  char timeBuf[500];
  strftime(timeBuf, sizeof(timeBuf), "%c", ::localtime(&timestamp));
  ArLog::log(ArLog::Normal,
             "Changing file %s modified time to %s",
             fileName,
             timeBuf);


  // time_t newTime = mktime(&timestamp);
  struct utimbuf fileTime;
  fileTime.actime  = timestamp;
  fileTime.modtime = timestamp;
  utime(fileName, &fileTime);

#endif // else unix

  return true;

} // end method changeFileTimestamp



AREXPORT std::string ArUtil::insertSuffixBeforeExtension(const char *filename,
							 const char *suffix)
{
  std::string ret = filename;
  size_t startExtension = ret.rfind(".");
  if (startExtension == std::string::npos)
  {
    ret += suffix;
    return ret;
  } 
  ret.insert(startExtension, suffix);
  return ret;
}


AREXPORT void ArUtil::setFileCloseOnExec(int fd, bool closeOnExec)
{
#ifndef WIN32
  if (fd <= 0)
    return;

  int flags;

  if ((flags = fcntl(fd, F_GETFD)) < 0)
  {
    ArLog::log(ArLog::Normal, "ArUtil::setFileCloseOnExec: Cannot use F_GETFD in fnctl on fd %d", fd);
    return;
  }

  if (closeOnExec)
    flags |= FD_CLOEXEC;
  else
    flags &= ~FD_CLOEXEC;

  if (fcntl(fd, F_SETFD, flags) < 0)
  {
    ArLog::log(ArLog::Normal, "ArUtil::setFileCloseOnExec: Cannot use F_GETFD in fnctl on fd %d", fd);
    return;
  }
#endif
}

AREXPORT void ArUtil::setFileCloseOnExec(FILE *file, bool closeOnExec)
{
  if (file != NULL)
    setFileCloseOnExec(fileno(file));
}

AREXPORT FILE *ArUtil::fopen(const char *path, const char *mode, 
			     bool closeOnExec)
{

  FILE *file = NULL;

#ifndef _WIN32
  file = ::fopen(path, mode);
#else

  // On Windows, it is necessary to convert the given UTF-8
  // strings to UTF-16, and then call the special "wide" version
  // of fopen.  This allows Japanese characters, etc.

  // Determine the number of characters in the output buffer.
  // Pass -1 as the src size to convert everything up to and
  // including the nul terminator.
  int widePathLen = MultiByteToWideChar(
                      CP_UTF8, 0, path, -1, NULL, 0);
  int wideModeLen = MultiByteToWideChar(
                      CP_UTF8, 0, mode, -1, NULL, 0);

  if (widePathLen > 0) {

    // Allocate buffers for the UTF16 strings.
    wchar_t *widePath = new wchar_t[widePathLen + 1];
    wchar_t *wideMode = new wchar_t[wideModeLen + 1];

    // Convert the UTF8 strings to UTF16.
    int s = MultiByteToWideChar(
      CP_UTF8, 0, path, -1, &widePath[0], widePathLen);
    int t = MultiByteToWideChar(
      CP_UTF8, 0, mode, -1, &wideMode[0], wideModeLen);

    if (widePathLen == s) {

      // Open the file.
      file = ::_wfopen(widePath, wideMode);
    }
    else { // unexpected number of characters converted

      ArLog::log(ArLog::Normal,
                 "ArUtil::fopen() unexpected number of characters in conversion %s",
                 path);

    } // end else unexpected number of characters converted

    delete [] widePath;
    delete [] wideMode;
  }
  else { // wideLen <= 0

    ArLog::log(ArLog::Normal,
               "ArUtil::fopen() error determining wide char length for %s",
               path);

  } // end if wideLen <= 0

#endif

  setFileCloseOnExec(file, closeOnExec);
  return file;
}

AREXPORT int ArUtil::open(const char *pathname, int flags, 
			  bool closeOnExec)
{
  int fd;
  fd = ::open(pathname, flags);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

AREXPORT int ArUtil::open(const char *pathname, int flags, mode_t mode, 
			  bool closeOnExec)
{
  int fd;
  fd = ::open(pathname, flags, mode);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

AREXPORT int ArUtil::close(int fd)
{
	return ::close(fd);
}

AREXPORT int ArUtil::creat(const char *pathname, mode_t mode, 
			   bool closeOnExec)
{
  int fd;
  fd = ::creat(pathname, mode);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

AREXPORT FILE *ArUtil::popen(const char *command, const char *type, 
			     bool closeOnExec)
{
  FILE *file;
#ifndef WIN32
  file = ::popen(command, type);
#else
  file = _popen(command, type);
#endif
  setFileCloseOnExec(file, closeOnExec);
  return file;
}


AREXPORT bool ArUtil::floatIsNormal(double f)
{
#ifdef WIN32
	  return (!::_isnan(f) && ::_finite(f));
#else
	  return isnormal(f);
#endif
}

AREXPORT int ArUtil::atoi(const char *str, bool *ok, bool forceHex) 
{
  bool isSuccess = false;
  int ret = 0;

  // if the argument isn't bogus
  if (str != NULL) {
  
    int base = 10;
    if (forceHex)
      base = 16;
    // see if it has the hex prefix and strip it
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
      str = &str[2];
      base = 16;
    }
    char *endPtr = NULL;
    ret = strtol(str, &endPtr, base);
 
    if (endPtr[0] == '\0' && endPtr != str) {
      isSuccess = true;
    }
  } // end if valid arg

  if (ok != NULL) {
    *ok = isSuccess;
  }
  
  if (isSuccess) 
    return ret;
  else 
    return 0;

} // end method atoi


AREXPORT long ArMath::randomInRange(long m, long n)
{
    // simple method
    return m + (random() % (n - m));
    //return m + random() / (ourRandMax / (n - m + 1) + 1);
    // alternate method is to use drand48, multiply and round (does Windows have
    // drand48?), or keep trying numbers until we get one in range.
}

AREXPORT double ArMath::epsilon() { return ourEpsilon; }
AREXPORT long ArMath::getRandMax() { return ourRandMax; }


AREXPORT ArThreadedCallbackList::ArThreadedCallbackList(
	int mSecsBetweenCallbacks, const char *name)
{
  myMSecsBetweenCallbacks = mSecsBetweenCallbacks;

  std::string threadName = "ArThreadedCallbackList";
  threadName += "::";
  threadName += name;
  setThreadName(threadName.c_str());
  runAsync();
}


AREXPORT ArThreadedCallbackList::~ArThreadedCallbackList()
{

}

AREXPORT void *ArThreadedCallbackList::runThread(void *arg)
{
  threadStarted();
  
  while (getRunning())
  {
    myCBList.invoke();
    
    ArUtil::sleep(myMSecsBetweenCallbacks);
  }

  threadFinished();
  return NULL;
}





ArGlobalRetFunctor2<ArLaser *, int, const char *> 
ArLaserCreatorHelper::ourLMS2xxCB(&ArLaserCreatorHelper::createLMS2xx);

ArGlobalRetFunctor2<ArLaser *, int, const char *> 
ArLaserCreatorHelper::ourUrgCB(&ArLaserCreatorHelper::createUrg);

ArGlobalRetFunctor2<ArLaser *, int, const char *> 
ArLaserCreatorHelper::ourLMS1XXCB(&ArLaserCreatorHelper::createLMS1XX);

ArGlobalRetFunctor2<ArLaser *, int, const char *> 
ArLaserCreatorHelper::ourS3SeriesCB(&ArLaserCreatorHelper::createS3Series);

ArGlobalRetFunctor2<ArLaser *, int, const char *>
ArLaserCreatorHelper::ourUrg_2_0CB(&ArLaserCreatorHelper::createUrg_2_0);

ArGlobalRetFunctor2<ArLaser *, int, const char *>
ArLaserCreatorHelper::ourLMS5XXCB(&ArLaserCreatorHelper::createLMS5XX);

ArGlobalRetFunctor2<ArLaser *, int, const char *>
ArLaserCreatorHelper::ourTiM3XXCB(&ArLaserCreatorHelper::createTiM3XX);

ArGlobalRetFunctor2<ArLaser *, int, const char *>
ArLaserCreatorHelper::ourSZSeriesCB(&ArLaserCreatorHelper::createSZSeries);

ArLaser *createAnyLMS1xx(int laserNumber, const char *logPrefix, const char *name, ArLMS1XX::LaserModel model)
{
	return new ArLMS1XX(laserNumber, name, model);
}

ArGlobalRetFunctor4<ArLaser*, int, const char*, const char *, ArLMS1XX::LaserModel>
TiM551CB(&createAnyLMS1xx, -1, "", "tim551", ArLMS1XX::TiM551);

ArGlobalRetFunctor4<ArLaser*, int, const char*, const char *, ArLMS1XX::LaserModel>
TiM561CB(&createAnyLMS1xx, -1, "", "tim561", ArLMS1XX::TiM561); 

ArGlobalRetFunctor4<ArLaser*, int, const char*, const char *, ArLMS1XX::LaserModel>
TiM571CB(&createAnyLMS1xx, -1, "", "tim571", ArLMS1XX::TiM571);

ArRetFunctor2<ArLaser *, int, const char *> *
ArLaserCreatorHelper::getCreateTiM551CB(void) {
  return &TiM551CB;
}

ArRetFunctor2<ArLaser *, int, const char *> *
ArLaserCreatorHelper::getCreateTiM561CB(void) {
  return &TiM561CB;
}

ArRetFunctor2<ArLaser *, int, const char *> *
ArLaserCreatorHelper::getCreateTiM571CB(void) {
  return &TiM571CB;
}


ArGlobalRetFunctor2<ArBatteryMTX *, int, const char *>
ArBatteryMTXCreatorHelper::ourBatteryMTXCB(&ArBatteryMTXCreatorHelper::createBatteryMTX);

ArGlobalRetFunctor2<ArLCDMTX *, int, const char *>
ArLCDMTXCreatorHelper::ourLCDMTXCB(&ArLCDMTXCreatorHelper::createLCDMTX);

ArGlobalRetFunctor2<ArSonarMTX *, int, const char *>
ArSonarMTXCreatorHelper::ourSonarMTXCB(&ArSonarMTXCreatorHelper::createSonarMTX);


ArLaser *ArLaserCreatorHelper::createLMS2xx(int laserNumber, 
					    const char *logPrefix)
{
  return new ArLMS2xx(laserNumber);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateLMS2xxCB(void)
{
  return &ourLMS2xxCB;
}

ArLaser *ArLaserCreatorHelper::createUrg(int laserNumber, const char *logPrefix)
{
  return new ArUrg(laserNumber);
}


ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateUrgCB(void)
{
  return &ourUrgCB;
}

ArLaser *ArLaserCreatorHelper::createLMS1XX(int laserNumber, const char *logPrefix)
{
	return new ArLMS1XX(laserNumber, "lms1xx", ArLMS1XX::LMS1XX);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateLMS1XXCB(void)
{
  return &ourLMS1XXCB;
}

ArLaser *ArLaserCreatorHelper::createS3Series(int laserNumber, 
					    const char *logPrefix)
{
  return new ArS3Series(laserNumber);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateS3SeriesCB(void)
{
  return &ourS3SeriesCB;
}


ArLaser *ArLaserCreatorHelper::createUrg_2_0(int laserNumber, 
					     const char *logPrefix)
{
  return new ArUrg_2_0(laserNumber);
}


ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateUrg_2_0CB(void)
{
  return &ourUrg_2_0CB;
}

ArLaser *ArLaserCreatorHelper::createLMS5XX(int laserNumber,
		const char *logPrefix)
{

	// PS 8/22/11 - added "lms5xx" and flag specifying laser is an lms5xx
	return new ArLMS1XX(laserNumber, "lms5XX", ArLMS1XX::LMS5XX);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateLMS5XXCB(void)
{
  return &ourLMS5XXCB;
}

ArLaser *ArLaserCreatorHelper::createTiM3XX(int laserNumber,
		const char *logPrefix)
{

	// PS 8/22/11 - added "lms5xx" and flag specifying laser is an lms5xx
	return new ArLMS1XX(laserNumber, "tim3XX", ArLMS1XX::TiM3XX);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateTiM3XXCB(void)
{
  return &ourTiM3XXCB;
}


ArLaser *ArLaserCreatorHelper::createSZSeries(int laserNumber,
					    const char *logPrefix)
{
  return new ArSZSeries(laserNumber);
}

ArRetFunctor2<ArLaser *, int, const char *> *ArLaserCreatorHelper::getCreateSZSeriesCB(void)
{
  return &ourSZSeriesCB;
}

ArBatteryMTX *ArBatteryMTXCreatorHelper::createBatteryMTX(int batteryNumber,
					    const char *logPrefix)
{
  return new ArBatteryMTX(batteryNumber);
}

ArRetFunctor2<ArBatteryMTX *, int, const char *> *ArBatteryMTXCreatorHelper::getCreateBatteryMTXCB(void)
{
  return &ourBatteryMTXCB;
}

ArLCDMTX *ArLCDMTXCreatorHelper::createLCDMTX(int lcdNumber,
					    const char *logPrefix)
{
  return new ArLCDMTX(lcdNumber);
}

ArRetFunctor2<ArLCDMTX *, int, const char *> *ArLCDMTXCreatorHelper::getCreateLCDMTXCB(void)
{
  return &ourLCDMTXCB;
}

ArSonarMTX *ArSonarMTXCreatorHelper::createSonarMTX(int sonarNumber,
					    const char *logPrefix)
{
  return new ArSonarMTX(sonarNumber);
}

ArRetFunctor2<ArSonarMTX *, int, const char *> *ArSonarMTXCreatorHelper::getCreateSonarMTXCB(void)
{
  return &ourSonarMTXCB;
}


ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> 
ArDeviceConnectionCreatorHelper::ourSerialCB(
	&ArDeviceConnectionCreatorHelper::createSerialConnection);
ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> 
ArDeviceConnectionCreatorHelper::ourTcpCB(
	&ArDeviceConnectionCreatorHelper::createTcpConnection);
ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *>
ArDeviceConnectionCreatorHelper::ourSerial422CB(
	&ArDeviceConnectionCreatorHelper::createSerial422Connection);
ArLog::LogLevel ArDeviceConnectionCreatorHelper::ourSuccessLogLevel = ArLog::Verbose;

ArDeviceConnection *ArDeviceConnectionCreatorHelper::createSerialConnection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
	ArDeviceConnection *devConn;

	devConn = internalCreateSerialConnection(port, defaultInfo, logPrefix, false);

	return devConn;
}

ArDeviceConnection *ArDeviceConnectionCreatorHelper::createSerial422Connection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
	ArDeviceConnection *devConn;

	devConn = internalCreateSerialConnection(port, defaultInfo, logPrefix, true);

	return devConn;
}


ArDeviceConnection *ArDeviceConnectionCreatorHelper::internalCreateSerialConnection(
	const char *port, const char *defaultInfo, const char *logPrefix, bool is422)
{
  ArSerialConnection *serConn = new ArSerialConnection(is422);
  
  std::string serPort;
  if (strcasecmp(port, "COM1") == 0)
    serPort = ArUtil::COM1;
  else if (strcasecmp(port, "COM2") == 0)
    serPort = ArUtil::COM2;
  else if (strcasecmp(port, "COM3") == 0)
    serPort = ArUtil::COM3;
  else if (strcasecmp(port, "COM4") == 0)
    serPort = ArUtil::COM4;
  else if (strcasecmp(port, "COM5") == 0)
    serPort = ArUtil::COM5;
  else if (strcasecmp(port, "COM6") == 0)
    serPort = ArUtil::COM6;
  else if (strcasecmp(port, "COM7") == 0)
    serPort = ArUtil::COM7;
  else if (strcasecmp(port, "COM8") == 0)
    serPort = ArUtil::COM8;
  else if (strcasecmp(port, "COM9") == 0)
    serPort = ArUtil::COM9;
  else if (strcasecmp(port, "COM10") == 0)
    serPort = ArUtil::COM10;
  else if (strcasecmp(port, "COM11") == 0)
    serPort = ArUtil::COM11;
  else if (strcasecmp(port, "COM12") == 0)
    serPort = ArUtil::COM12;
  else if (strcasecmp(port, "COM13") == 0)
    serPort = ArUtil::COM13;
  else if (strcasecmp(port, "COM14") == 0)
    serPort = ArUtil::COM14;
  else if (strcasecmp(port, "COM15") == 0)
    serPort = ArUtil::COM15;
  else if (strcasecmp(port, "COM16") == 0)
    serPort = ArUtil::COM16;
  else if (port != NULL)
    serPort = port;
  
  ArLog::log(ourSuccessLogLevel, "%sSet serial port to open %s", 
	     logPrefix, serPort.c_str());
  serConn->setPort(serPort.c_str());
  return serConn;
  /*  
      This code is commented out because it created problems with demo
      (or any other program that used ArLaserConnector::connectLasers
      with addAllLasersToRobot as true)

  int ret;
  
  if ((ret = serConn->open(serPort.c_str())) == 0)
  {
    ArLog::log(ourSuccessLogLevel, "%sOpened serial port %s", 
	       logPrefix, serPort.c_str());
    return serConn;
  }
  else
  {
    ArLog::log(ArLog::Normal, "%sCould not open serial port %s (from %s), because %s", 
	       logPrefix, serPort.c_str(), port,
	       serConn->getOpenMessage(ret));
    delete serConn;
    return NULL;
  }
  */
}


ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *
ArDeviceConnectionCreatorHelper::getCreateSerialCB(void)
{
  return &ourSerialCB;
}

ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *
ArDeviceConnectionCreatorHelper::getCreateSerial422CB(void)
{
  return &ourSerial422CB;
}

ArDeviceConnection *ArDeviceConnectionCreatorHelper::createTcpConnection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
  ArTcpConnection *tcpConn = new ArTcpConnection;
  //int ret;

  tcpConn->setPort(port, atoi(defaultInfo));
  ArLog::log(ourSuccessLogLevel, 
	     "%sSet tcp connection to open %s (and port %d)", 
	     logPrefix, port, atoi(defaultInfo));
  return tcpConn;

  /*
      This code is commented out because it created problems with demo
      (or any other program that used ArLaserConnector::connectLasers
      with addAllLasersToRobot as true)
  
  if ((ret = tcpConn->open(port, atoi(defaultInfo))) == 0)
  {
    ArLog::log(ourSuccessLogLevel, 
	       "%sOpened tcp connection from %s (and port %d)", 
	       logPrefix, port, atoi(defaultInfo));
    return tcpConn;
  }
  else
  {
    ArLog::log(ArLog::Normal, "%sCould not open a tcp connection to host '%s' with default port %d (from '%s'), because %s", 
	       logPrefix, port, atoi(defaultInfo), defaultInfo,
	       tcpConn->getOpenMessage(ret));
    delete tcpConn;
    return NULL;
  }
  */
}

ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *
ArDeviceConnectionCreatorHelper::getCreateTcpCB(void)
{
  return &ourTcpCB;
}

void ArDeviceConnectionCreatorHelper::setSuccessLogLevel(
	ArLog::LogLevel successLogLevel)
{
  ourSuccessLogLevel = successLogLevel;
}

ArLog::LogLevel ArDeviceConnectionCreatorHelper::setSuccessLogLevel(void)
{
  return ourSuccessLogLevel;
}

AREXPORT std::list<ArPose> ArPoseUtil::findCornersFromRobotBounds(
	double radius, double widthLeft, double widthRight, 
	double lengthFront, double lengthRear, bool fastButUnsafe)
{

  std::list<ArPose> ret;

  if (fastButUnsafe)
  {
    ArPose frontLeft;   
    if (lengthFront >= radius && widthLeft >= radius)
      frontLeft.setPose(lengthFront,
			widthLeft);
    else if (lengthFront >= radius)
      frontLeft.setPose(lengthFront,
			0);
    else
      frontLeft.setPose(lengthFront,
			sqrt(radius * radius - lengthFront * lengthFront));
    
    ArPose leftFront;
    if (widthLeft >= radius && lengthFront >= radius)
      leftFront.setPose(lengthFront, 
			widthLeft);
    else if (widthLeft >= radius)
      leftFront.setPose(0, 
			widthLeft);
    else
      leftFront.setPose(sqrt(radius * radius - widthLeft * widthLeft),
			widthLeft);

    ArPose leftRear;
    if (widthLeft >= radius && lengthRear >= radius)
      leftRear.setPose(-lengthRear, 
		       widthLeft);
    else if (widthLeft >= radius)
      leftRear.setPose(0, 
		       widthLeft);
    else
      leftRear.setPose(-sqrt(radius * radius - widthLeft * widthLeft),
		       widthLeft);

    ArPose rearLeft;
    if (lengthRear >= radius && widthLeft >= radius)
      rearLeft.setPose(-lengthRear, 
		       widthLeft);
    else if (lengthRear >= radius)
      rearLeft.setPose(-lengthRear, 
		       0);
    else
      rearLeft.setPose(-lengthRear,
		       sqrt(radius * radius - lengthRear * lengthRear ));


    ArPose rearRight;
    if (lengthRear >= radius && widthRight >= radius)
      rearRight.setPose(-lengthRear, 
			-widthRight);
    else if (lengthRear >= radius)
      rearRight.setPose(-lengthRear, 
		       0);
    else
      rearRight.setPose(-lengthRear,
			-sqrt(radius * radius - lengthRear * lengthRear));


    ArPose rightRear;
    if (widthRight >= radius && lengthRear >= radius)      
      rightRear.setPose(-lengthRear, 
			-widthRight);
    else if (widthRight >= radius)
      rightRear.setPose(0, 
			-widthRight);
    else
      rightRear.setPose(-sqrt(radius * radius - widthRight * widthRight),
			-widthRight);

    ArPose rightFront;
    if (widthRight >= radius && lengthFront >= radius)
      rightFront.setPose(lengthFront, 
			 -widthRight);
    else if (widthRight >= radius)
      rightFront.setPose(0, 
			-widthRight);
    else
      rightFront.setPose(sqrt(radius * radius - widthRight * widthRight),
			 -widthRight);

    ArPose frontRight;
    if (lengthFront >= radius && widthRight >= radius)
      frontRight.setPose(lengthFront,
			 -widthRight);
    else if (lengthFront >= radius)
      frontRight.setPose(lengthFront,
			0);
    else
      frontRight.setPose(lengthFront,
			 -sqrt(radius * radius - lengthFront * lengthFront));

    if (frontRight.squaredFindDistanceTo(frontLeft) > 1)
      ret.push_back(frontLeft);

    if (frontLeft.squaredFindDistanceTo(leftFront) > 1)
      ret.push_back(leftFront);
    if (leftFront.squaredFindDistanceTo(leftRear) > 1)
      ret.push_back(leftRear);

    if (leftRear.squaredFindDistanceTo(rearLeft) > 1) 
      ret.push_back(rearLeft);
    if (rearLeft.squaredFindDistanceTo(rearRight) > 1) 
      ret.push_back(rearRight);

    if (rearRight.squaredFindDistanceTo(rightRear) > 1)
      ret.push_back(rightRear);
    if (rightRear.squaredFindDistanceTo(rightFront) > 1)
      ret.push_back(rightFront);

    if (rightFront.squaredFindDistanceTo(frontRight) > 1)
      ret.push_back(frontRight);
    return ret;
  }

  return ret;

  /// MPL this code worked, but didn't give us good corners when the width/length got nuts
#if 0
  if (fastButUnsafe)
  {

    ArPose frontLeft;
    if (lengthFront >= radius)
      frontLeft.setPose(lengthFront,
			0);
    else
      frontLeft.setPose(lengthFront,
			sqrt(radius * radius - lengthFront * lengthFront));
    
    ArPose leftFront;
    if (widthLeft >= radius)
      leftFront.setPose(0, 
			widthLeft);
    else
      leftFront.setPose(sqrt(radius * radius - widthLeft * widthLeft),
			widthLeft);

    ArPose leftRear(-leftFront.getX(), 
		    leftFront.getY());
    /*
    ArPose leftRear;
    if (widthLefth >= radius)
      leftRear.setPose(0, widthLeft);
    else
      leftRear.setPose(-sqrt(radius * radius - widthLeft * widthLeft),
		       widthLeft);
    */

    ArPose rearLeft;
    if (lengthRear >= radius)
      rearLeft.setPose(-lengthRear, 
		       0);
    else
      rearLeft.setPose(-lengthRear,
		       sqrt(radius * radius - lengthRear * lengthRear ));

    ArPose rearRight(rearLeft.getX(), 
		     -rearLeft.getY());
    /*
    ArPose rearRight;
    if (lengthRear >= radius)
      rightRear.setPose(lengthRear, 
		       0);
    else
      rightRear.setPose(lengthRear,
			-sqrt(radius * radius - lengthRear * lengthRear));
    */

    ArPose rightRear;
    if (widthRight >= radius)
      rightRear.setPose(0, 
			-widthRight);
    else
      rightRear.setPose(-sqrt(radius * radius - widthRight * widthRight),
			-widthRight);

    ArPose rightFront(-rightRear.getX(),
		      rightRear.getY());
    /*
    ArPose rightFront;
    if (widthRight >= radius)
      rightFront.setPose(0, 
			-widthRight);
    else
      rightFront.setPose(sqrt(radius * radius - widthRight * widthRight),
			 -widthRight);
    */

    ArPose frontRight(frontLeft.getX(),
		      -frontLeft.getY());
    /*
    ArPose frontRight;
    if (lengthFront >= radius)
      rightFront.setPose(lengthFront,
			0);
    else
      rightFront.setPose(lengthFront,
			 -sqrt(radius * radius - lengthFront * lengthFront));
    */

    std::list<ArPose> ret;
    ret.push_back(frontLeft);

    ret.push_back(leftFront);
    ret.push_back(leftRear);

    ret.push_back(rearLeft);
    ret.push_back(rearRight);

    ret.push_back(rightRear);
    ret.push_back(rightFront);

    ret.push_back(frontRight);
    return ret;
  }
#endif	    
}


AREXPORT std::list<ArPose> ArPoseUtil::breakUpDistanceEvenly(
	ArPose start, ArPose end, int resolution)
{
  std::list<ArPose> ret;

  ret.push_back(start);

  double dist = start.findDistanceTo(end);
  double angle = start.findAngleTo(end);
  double cos = ArMath::cos(angle);
  double sin = ArMath::sin(angle);

  if (dist > resolution)
  {
    // we're using integer truncation here
    int steps = dist / resolution + 1;
    double increment = dist / steps;

    double atX = start.getX();
    double atY = start.getY();
    
    // now walk the length of the line and see if we should put the points in
    for (int ii = 1; ii <= steps; ii++)
    {
      atX += increment * cos;
      atY += increment * sin;
      ret.push_back(ArPose(atX, atY));
    }
  }

  ret.push_back(end);
  return ret;
}

AREXPORT ArTimeChecker::ArTimeChecker(const char *name, int defaultMSecs)
{
  if (name != NULL)
    myName = name;
  else
    myName = "Unknown";
  myMSecs = defaultMSecs;
  myLogEverything = false;
}

AREXPORT ArTimeChecker::~ArTimeChecker()
{

}

AREXPORT void ArTimeChecker::start(void)
{
  if (myLogEverything)
    ArLog::log(ArLog::Normal, "%s: Start", myName.c_str());
  myStarted.setToNow();
  myLastCheck.setToNow();
}

AREXPORT void ArTimeChecker::check(const char *subName)
{
  long long took = myLastCheck.mSecSinceLL();

  if (myLogEverything) 
    ArLog::log(ArLog::Normal, "%s: %s\ttook %lld msecs in thread %s",
	       myName.c_str(), subName, took, 
	       ArThread::self()->getThreadName());

  if (took > (long long) myMSecs && subName != NULL)
    ArLog::log(ArLog::Normal, "%s::%s took too long (%lld msecs) in thread %s",
	       myName.c_str(), subName, took, 
	       ArThread::self()->getThreadName());

  myLastCheck.setToNow();
}


AREXPORT void ArTimeChecker::finish(void)
{
  long long took = myStarted.mSecSinceLL();
  
  if (myLogEverything) 
    ArLog::log(ArLog::Normal, "%s: Finish\ttook %lld msecs in thread %s",
	       myName.c_str(), took, 
	       ArThread::self()->getThreadName());

  if (took > (long long) myMSecs)
    ArLog::log(ArLog::Normal, "%s took too long (%lld msecs) in thread %s",
	       myName.c_str(), took, ArThread::self()->getThreadName());
}


AREXPORT unsigned long ArUtil::availableDiskSpaceKB(const char *path, bool *ok)
{
  if(!path) {
    if(ok) *ok = false;
    return ULONG_MAX;
  }
#ifdef WIN32
  ULARGE_INTEGER bytesAvail;
  BOOL b = GetDiskFreeSpaceEx(path, &bytesAvail, NULL, NULL);
  if(b != 0) {
    if(ok) *ok = true;
    return (unsigned long)(bytesAvail.QuadPart)/1024;
  }
#else
  struct statvfs s;
  int r = statvfs(path, &s);
  if(r == 0) {
    if(ok) *ok = true;
    return s.f_bavail * s.f_frsize / 1024;
  }
#endif
  ArLog::logErrorFromOS(ArLog::Verbose, "ArUtil::availableDiskSpace: Error requesting filesystem status");
  if(ok) *ok = false;
  return ULONG_MAX;
}

