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

#ifndef ARIAUTIL_H
#define ARIAUTIL_H

#define _GNU_SOURCE 1
#include <string>
// #define _XOPEN_SOURCE 500
#include <list>
#include <map>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <float.h>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
#include <sys/timeb.h>
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <strings.h>
#endif // ifndef win32

#include <time.h>
#include "ariaTypedefs.h"
#include "ArLog.h"
#include "ArFunctor.h"
#include "ArArgumentParser.h"
//#include "ariaInternal.h"
#include "ariaOSDef.h"
#include "ArASyncTask.h"

class ArLaser;

class ArBatteryMTX;
class ArLCDMTX;
class ArSonarMTX;
class ArDeviceConnection;

#ifndef M_PI
#define M_PI 3.1415927
#endif // of M_PI, windows has a function call instead of a define

/// Contains various utility functions, including cross-platform wrappers around common system functions.
/** @ingroup UtilityClasses
    @ingroup ImportantClasses
*/
class ArUtil
{
public:
  /// Values for the bits from 0 to 16
  enum BITS { 
    BIT0 = 0x1, ///< value of BIT0
    BIT1 = 0x2, ///< value of BIT1
    BIT2 = 0x4, ///< value of BIT2
    BIT3 = 0x8, ///< value of BIT3
    BIT4 = 0x10, ///< value of BIT4
    BIT5 = 0x20, ///< value of BIT5
    BIT6 = 0x40, ///< value of BIT6
    BIT7 = 0x80, ///< value of BIT7
    BIT8 = 0x100, ///< value of BIT8
    BIT9 = 0x200, ///< value of BIT9
    BIT10 = 0x400, ///< value of BIT10
    BIT11 = 0x800, ///< value of BIT11
    BIT12 = 0x1000, ///< value of BIT12
    BIT13 = 0x2000, ///< value of BIT13
    BIT14 = 0x4000, ///< value of BIT14
    BIT15 = 0x8000, ///< value of BIT15
  };

#ifdef WIN32
  typedef int mode_t;
#endif

  // These are functions we can use as the dummies
  //   for functor callbacks when we want it to
  //   always return true/false. 
  AREXPORT static bool ReturnTrue() { return true; }
  AREXPORT static bool ReturnFalse() { return false; }



  /// Sleep for the given number of milliseconds
  /// @ingroup easy
  AREXPORT static void sleep(unsigned int ms);
  
  /// Get the time in milliseconds
  AREXPORT static unsigned int getTime(void);

  /** Deletes (deallocates) memory pointed to by pointers in an iteratable
      container (but does not remove the pointer items from the container).

      Use this when you have a container of pointers (e.g. 
      std::list<Class*>) and have allocated instances
      with new and added them to the container.

      Assumes that T is an iterator that supports the operator*, operator!=
      and operator++. The iterators are assumed to refer to container 
      members which are pointers to objects to be deleted using the 'delete' operator.

      Only the objects pointed to by the pointers in the container are
      deallocated, the container will still contain the (now invalid) pointers.  You
      must either replace them with new valid pointers, remove/erase them from the container,
      or clear the container if you will continue to use the container.

      Example:
      @code
        std::vector<MyBigClass*> pointers;
        pointers.push_back(new MyBigClass());
        pointers.push_back(new MyBigClass());
        pointers.push_back(new MyBigClass());

        ...later... 

        ArUtil::deleteSet(pointers.begin(), pointers.end()); 
        // MyBigClass instances have now been deleted using delete keyword, so old pointers are now invalid 
        pointers.erase(pointers.begin(), pointers.end());
      @endcode
  */
  template<class T> static void deleteSet(T begin, T end)
    {
      for (; begin != end; ++begin)
      {
	delete (*begin);
      }
    }


  /** Deletes (deallocates) memory pointed to by pointers in a std::map or other iteratable
      container of pairs (but does not remove the pointer items from the container).

      Use this when you have a container of std::pair values in which the second
item in the pair is a pointer (e.g. 
      std::map<int, Class*>) and have allocated instances
      with new and added them to the container.

      Assumes that T is an iterator that supports the operator*, operator!=
      and operator++. The iterators are assumed to refer to container 
      members which are std::pair objects in which the second member of the pair
is a pointer to object to be deleted using the 'delete' operator.

      Only the objects pointed to by the pointers in the pairs are
      deallocated, the pairs will still contain the (now invalid) pointers.  You
      must either replace them with new valid pointers, remove/erase them from the container,
      or clear the container if you will continue to use the container.
  */
  template<class T> static void deleteSetPairs(T begin, T end)
    {
      for (; begin != end; ++begin)
      {
	delete (*begin).second;
      }
    }

  /// Returns the minimum of the two values
  /// @ingroup easy
  static int findMin(int first, int second) 
    { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static int findMax(int first, int second) 
    { if (first > second) return first; else return second; }

  /// Returns the minimum of the two values
  /// @ingroup easy
  static unsigned int findMinU(unsigned int first, unsigned int second) 
    { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static unsigned int findMaxU(unsigned int first, unsigned int second) 
    { if (first > second) return first; else return second; }

  /// Returns the minimum of the two values
  /// @ingroup easy
  static double findMin(double first, double second) 
    { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static double findMax(double first, double second) 
    { if (first > second) return first; else return second; }

  /// OS-independent way of finding the size of a file.
  AREXPORT static long sizeFile(const char *fileName);

  /// OS-independent way of finding the size of a file.
  AREXPORT static long sizeFile(std::string fileName);

  /// OS-independent way of checking to see if a file exists and is readable.
  AREXPORT static bool findFile(const char *fileName);

  // OS-independent way of stripping the directory from the fileName.
  // commented out with std::string changes since this didn't seem worth fixing right now
  //AREXPORT static bool stripDir(std::string fileIn, std::string &fileOut);

  // OS-independent way of stripping the fileName from the directory.
  // commented out with std::string changes since this didn't seem worth fixing right now
  //AREXPORT static bool stripFile(std::string fileIn, std::string &fileOut);

  /// Appends a slash to a path if there is not one there already
  AREXPORT static void appendSlash(char *path, size_t pathLength);
  
  /// Appends a slash to the given string path if necessary.
  AREXPORT static void appendSlash(std::string &path);

  /// Fix the slash orientation in file path string for windows or linux
  AREXPORT static void fixSlashes(char *path, size_t pathLength);
  
  /// Fixes the slash orientation in the given file path string for the current platform
  AREXPORT static void fixSlashes(std::string &path); 

  /// Fix the slash orientation in file path string to be all forward
  AREXPORT static void fixSlashesForward(char *path, size_t pathLength);

  /// Fix the slash orientation in file path string to be all backward
  AREXPORT static void fixSlashesBackward(char *path, size_t pathLength);

  /// Returns the slash (i.e. separator) character for the current platform
  AREXPORT static char getSlash();

  /// Adds two directories, taking care of all slash issues
  AREXPORT static void addDirectories(char *dest, size_t destLength,
				      const char *baseDir, 
				      const char *insideDir);




  /// Finds out if two strings are equal
  AREXPORT static int strcmp(const std::string &str, const std::string &str2);

  /// Finds out if two strings are equal
  AREXPORT static int strcmp(const std::string &str, const char *str2);

  /// Finds out if two strings are equal
  AREXPORT static int strcmp(const char *str, const std::string &str2);

  /// Finds out if two strings are equal
  AREXPORT static int strcmp(const char *str, const char *str2);

  /// Finds out if two strings are equal (ignoring case)
  AREXPORT static int strcasecmp(const std::string &str, const std::string &str2);

  /// Finds out if two strings are equal (ignoring case)
  AREXPORT static int strcasecmp(const std::string &str, const char *str2);

  /// Finds out if two strings are equal (ignoring case)
  AREXPORT static int strcasecmp(const char *str, const std::string &str2);

  /// Finds out if two strings are equal (ignoring case)
  AREXPORT static int strcasecmp(const char *str, const char *str2);

  /// Finds out if a string has a suffix 
  AREXPORT static bool strSuffixCmp(const char *str, const char *suffix);

  /// Finds out if a string has a suffix 
  AREXPORT static bool strSuffixCaseCmp(const char *str, const char *suffix);
  

  /// Compares two strings (ignoring case and surrounding quotes)
  /**
   * This helper method is primarily used to ignore surrounding quotes 
   * when comparing ArArgumentBuilder args.
   * @return int set to 0 if the two strings are equivalent, a negative 
   * number if str1 is "less than" str2, and a postive number if it is
   * "greater than".
  **/
  AREXPORT static int strcasequotecmp(const std::string &str1, 
                                      const std::string &str2);


  /// Puts a \ before spaces in src, puts it into dest
  AREXPORT static void escapeSpaces(char *dest, const char *src, 
				    size_t maxLen);

  /// Strips out the quotes in the src buffer into the dest buffer
  AREXPORT static bool stripQuotes(char *dest, const char *src,size_t destLen);

  /// Strips the quotes from the given string.
  AREXPORT static bool stripQuotes(std::string *strToStrip);


  /// Fixes the bad characters in the given string.
  AREXPORT static bool fixBadCharacters(std::string *strToFix, 
					bool removeSpaces, bool fixOtherWhiteSpace = true);


  /// Lowers a string from src into dest, make sure there's enough space
  AREXPORT static void lower(char *dest, const char *src, 
			     size_t maxLen);
  /// Returns true if this string is only alphanumeric (i.e. it contains only leters and numbers), false if it contains any non alphanumeric characters (punctuation, whitespace, control characters, etc.)
  AREXPORT static bool isOnlyAlphaNumeric(const char *str);

  /// Returns true if this string is only numeric (i.e. it contains only numeric
  //digits), or it's null, or false if it contains any non nonnumeric characters (alphabetic, punctuation, whitespace, control characters, etc.)
  AREXPORT static bool isOnlyNumeric(const char *str);

  /// Returns true if the given string is null or of zero length, false otherwise
  AREXPORT static bool isStrEmpty(const char *str);

  /// Determines whether the given text is contained in the given list of strings.
  AREXPORT static bool isStrInList(const char *str,
                                   const std::list<std::string> &list,
                                   bool isIgnoreCase = false);

  /// Returns the floating point number from the string representation of that number in @a nptr, or HUGE_VAL for "inf" or -HUGE_VAL for "-inf".
  AREXPORT static double atof(const char *nptr);

#ifndef SWIG
  /// Prints the given format string and arguments in a NON-localized fashion.
  /** @swigomit */
  AREXPORT static bool printValue(char *buf, size_t bufLen, const char *formatString, ...);
  
  /// Reads a double value from the given input string in a NON-localized fashion.
  /** @swigomit */ 
  AREXPORT static double readDoubleValue(const char *buf, bool *ok);


#endif


  /// Converts an integer value into a string for true or false
  AREXPORT static const char *convertBool(int val);

#ifndef SWIG
  /** Invoke a functor with a string generated via sprintf format conversion
      @param functor The functor to invoke with the formatted string
      @param formatstr The format string into which additional argument values are inserted using vsnprintf() 
      @param ... Additional arguments are values to interpolate into @a formatstr to generate the final string passed as the argument in the functor invocation.
      @swigomit
  */
  AREXPORT static void functorPrintf(ArFunctor1<const char *> *functor,
				     const char *formatstr, ...);
  /// @deprecated format string should be a const char*
  AREXPORT static void functorPrintf(ArFunctor1<const char *> *functor,
				     char *formatstr, ...);
#endif

  /// Function for doing a fprintf to a file (here to make a functor for)
  AREXPORT static void writeToFile(const char *str, FILE *file);

  /// Gets a string contained in an arbitrary file
  AREXPORT static bool getStringFromFile(const char *fileName, 
					 char *str, size_t strLen);
  /** 
  These are for passing into getStringFromRegistry
  **/
  enum REGKEY {
    REGKEY_CLASSES_ROOT, ///< use HKEY_CLASSES_ROOT
    REGKEY_CURRENT_CONFIG, ///< use HKEY_CURRENT_CONFIG
    REGKEY_CURRENT_USER, ///< use HKEY_CURRENT_USER
    REGKEY_LOCAL_MACHINE, ///< use HKEY_LOCAL_MACHINE
    REGKEY_USERS ///< use HKEY_USERS
  };

  /// Returns a string from the Windows registry
  AREXPORT static bool getStringFromRegistry(REGKEY root,
					     const char *key,
					     const char *value,
					     char *str,
					     int len);

  /// Returns a string from the Windows registry, searching each of the following registry root paths in order: REGKEY_CURRENT_USER, REGKEY_LOCAL_MACHINE
 static bool findFirstStringInRegistry(const char* key, const char* value, char* str, int len) {
	if(!getStringFromRegistry(REGKEY_CURRENT_USER, key, value, str, len))
		return getStringFromRegistry(REGKEY_LOCAL_MACHINE, key, value, str, len);
	return true;
  }

  AREXPORT static const char *COM1; ///< First serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM2; ///< Second serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM3; ///< Third serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM4; ///< Fourth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM5; ///< Fifth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM6; ///< Sixth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM7; ///< Seventh serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM8; ///< Eighth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM9; ///< Ninth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM10; ///< Tenth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM11; ///< Eleventh serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM12; ///< Twelth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM13; ///< Thirteenth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM14; ///< Fourteenth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM15; ///< Fifteenth serial port device name (value depends on compilation platform)
  AREXPORT static const char *COM16; ///< Sixteenth serial port device name (value depends on compilation platform)

  AREXPORT static const char *TRUESTRING; ///< "true"
  AREXPORT static const char *FALSESTRING; ///< "false"

  /** Put the current year (GMT) in s (e.g. "2005"). 
   *  @param s String buffer (allocated) to write year into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentYearInString(char* s, size_t len);
  /** Put the current month (GMT) in s (e.g. "09" if September). 
   *  @param s String buffer (allocated) to write month into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentMonthInString(char* s, size_t len);
  /** Put the current day (GMT) of the month in s (e.g. "20"). 
   *  @param s String buffer (allocated) to write day into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentDayInString(char* s, size_t len);
  /** Put the current hour (GMT) in s (e.g. "13" for 1 o'clock PM). 
   *  @param s String buffer (allocated) to write hour into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentHourInString(char* s, size_t len);
  /** Put the current minute (GMT) in s (e.g. "05"). 
   *  @param s String buffer (allocated) to write minutes into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentMinuteInString(char* s, size_t len);
  /** Put the current second (GMT) in s (e.g. "59"). 
   *  @param s String buffer (allocated) to write seconds into
   *  @param len Size of @a s
   */
  AREXPORT static void putCurrentSecondInString(char* s, size_t len);
  
  
  /// Parses the given time string (h:mm) and returns the corresponding time.
  /**
   * @param str the char * string to be parsed; in the 24-hour format h:mm
   * @param ok an output bool * set to true if the time is successfully parsed;
   * false, otherwise
   * @param toToday true to find the time on the current day, false to find the time on 1/1/70
   * @return time_t if toToday is true then its the parsed time on the current day, if toToday is false then its the parsed time on 1/1/70
   * 1/1/70
  **/
  AREXPORT static time_t parseTime(const char *str, bool *ok = NULL, bool toToday = true);


  /** Interface to native platform localtime() function.
   *  On Linux, this is equivalent to a call to localtime_r(@a timep, @a result) (which is threadsafe, including the returned pointer, since it uses a different time struct for each thread)
   *  On Windows, this is equivalent to a call to localtime(@a timep, @a result). In addition, a static mutex is used to make it threadsafe.
   *
   *  @param timep Pointer to current time (Unix time_t; seconds since epoch) 
   *  @param result The result of calling platform localtime function is copied into this struct, so it must have been allocated.
   *  @return false on error (e.g. invalid input), otherwise true.
   *
   *  Example:
   *  @code
   *  struct tm t;
   *  ArUtil::localtime(time(NULL), &t);
   *  ArLog::log("Current month is %d.\n", t.tm_mon);
   *  @endcode
   */
  AREXPORT static bool localtime(const time_t *timep, struct tm *result);

   
  /** Call ArUtil::localtime(const time_t*, struct tm *) with the current time obtained by calling
   * time(NULL).
   *  @return false on error (e.g. invalid input), otherwise true.
   */
  AREXPORT static bool localtime(struct tm *result);

  // these aren't needed in windows since it ignores case anyhow
#ifndef WIN32
  /// this matches the case out of what file we want
  AREXPORT static bool matchCase(const char *baseDir, const char *fileName, 
			   char * result, size_t resultLen);
#endif 
  /// Pulls the directory out of a file name
  AREXPORT static bool getDirectory(const char *fileName, 
				     char * result, size_t resultLen);
  /// Pulls the filename out of the file name
  AREXPORT static bool getFileName(const char *fileName, 
				     char * result, size_t resultLen);
  
#ifndef SWIG
  /// Helper method that calls the appropriate version of the file stat() utility.
  AREXPORT static int filestat(const std::string &fileName,
                               struct stat *buffer);
#endif

  /// Sets the timestamp on the specified file
  AREXPORT static bool changeFileTimestamp(const char *fileName, 
                                           time_t timestamp);

  /// Takes a file name, adds a suffix before the extension (if any)
  AREXPORT static std::string insertSuffixBeforeExtension(const char *filename,
							  const char *suffix);


#ifndef SWIG
  /// Opens a file, defaulting it so that the file will close on exec
  AREXPORT static FILE *fopen(const char *path, const char *mode, 
			      bool closeOnExec = true);
  /// Opens a file, defaulting it so that the file will close on exec
  AREXPORT static int open(const char *pathname, int flags, 
			   bool closeOnExec = true);
  /// Opens a file, defaulting it so that the file will close on exec
  AREXPORT static int open(const char *pathname, int flags, mode_t mode, 
			   bool closeOnExec = true);
  AREXPORT static int close(int fd);
  /// Opens a file, defaulting it so that the file will close on exec
  AREXPORT static int creat(const char *pathname, mode_t mode,
			    bool closeOnExec = true);
  /// Opens a pipe, defaulting it so that the file will close on exec
  AREXPORT static FILE *popen(const char *command, const char *type, 
			      bool closeOnExec = true);
#endif

  /// Sets if the file descriptor will be closed on exec or not
  AREXPORT static void setFileCloseOnExec(int fd, bool closeOnExec = true);
  /// Sets if the file descriptor will be closed on exec or not
  AREXPORT static void setFileCloseOnExec(FILE *file, bool closeOnExec = true);

  /** Return true if the value of @a f is not NaN and is not infinite (+/- INF) */
  AREXPORT static bool floatIsNormal(double f);

  /** Convert seconds to milliseconds */
  static double secToMSec(const double sec) { return sec * 1000.0; }
  
  /** Convert milliseconds to seconds */
  static double mSecToSec(const double msec) { return msec / 1000.0; }

  /** Convert meters to US feet */
  static double metersToFeet(const double m) { return m * 3.2808399; }

  /** Convert US feet  to meters */
  static double feetToMeters(const double f) { return f / 3.2808399; }

  AREXPORT static int atoi(const char *str, bool *ok = NULL, 
			   bool forceHex = false);

  /** @return Amount of free space on filesystem/disk containing the file given by
   * @a path, in kilobytes, or ULONG_MAX on error.  If @a ok is not NULL it is
   * also set to false on error or true on no error. */
  AREXPORT static unsigned long availableDiskSpaceKB(const char *path, bool *ok = NULL);

  /** @return Amount of free space on filesystem/disk containing the file given by
   * @a path, in megabytes, or ULONG_MAX on error.  If @a ok is not NULL it is
   * also set to false on error or true on no error. */
  static unsigned long availableDiskSpaceMB(const char *path, bool *ok = NULL) {
    const unsigned long kb = availableDiskSpaceKB(path, ok);
    if(kb == ULONG_MAX) return ULONG_MAX;
    return kb/1024;
  }
  
protected:
//#ifndef WIN32
  /// this splits up a file name (it isn't exported since it'd crash with dlls)
  static std::list<std::string> splitFileName(const char *fileName);
//#endif

private:

  /// The character used as a file separator on the current platform (i.e. Linux or Windows)
  static const char SEPARATOR_CHAR;
  /// The character used as a file separator on the current platform, in a string format
  static const char *SEPARATOR_STRING;
  /// The character used as a file separator on the other platforms (i.e. slash in opposite direction)
  static const char OTHER_SEPARATOR_CHAR;

#ifdef WIN32
  // Used on Windows to make ArUtil::localtime() function threadsafe
  static ArMutex ourLocaltimeMutex;
#endif
};

/** Common math operations
    @ingroup UtilityClasses
    @ingroup easy
*/
class ArMath
{
private:
  /* see ArMath::epsilon() */
  static const double ourEpsilon; 

  // see getRandMax())
  static const long ourRandMax;

public:
   
  /** @return a very small number which can be used for comparisons of floating 
   * point values, etc. */
  AREXPORT static double epsilon();


  /// This adds two angles together and fixes the result to [-180, 180] 
  /**
     @param ang1 first angle
     @param ang2 second angle, added to first
     @return sum of the angles, in range [-180,180]
     @see subAngle
     @see fixAngle 
     @ingroup easy
  */
  static double addAngle(double ang1, double ang2) 
    { return fixAngle(ang1 + ang2); }

  /// This subtracts one angle from another and fixes the result to [-180,180]
  /**
     @param ang1 first angle
     @param ang2 second angle, subtracted from first angle
     @return resulting angle, in range [-180,180]
     @see addAngle
     @see fixAngle
     @ingroup easy
  */
  static double subAngle(double ang1, double ang2) 
    { return fixAngle(ang1 - ang2); }

  /// Takes an angle and returns the angle in range (-180,180]
  /**
     @param angle the angle to fix
     @return the angle in range (-180,180]
     @see addAngle
     @see subAngle
     @ingroup easy
  */
  static double fixAngle(double angle) 
    {
      if (angle >= 360)
	angle = angle - 360.0 * (double)((int)angle / 360);
      if (angle < -360)
	angle = angle + 360.0 * (double)((int)angle / -360);
      if (angle <= -180)
	angle = + 180.0 + (angle + 180.0);
      if (angle > 180)
	angle = - 180.0 + (angle - 180.0);
      return angle;
    } 
  
  /// Converts an angle in degrees to an angle in radians
  /**
     @param deg the angle in degrees
     @return the angle in radians
     @see radToDeg
     @ingroup easy
  */     
  static double degToRad(double deg) { return deg * M_PI / 180.0; }

  /// Converts an angle in radians to an angle in degrees
  /**
     @param rad the angle in radians
     @return the angle in degrees
     @see degToRad
     @ingroup easy
  */
  static double radToDeg(double rad) { return rad * 180.0 / M_PI; }

  /// Finds the cos, from angles in degrees
  /**
     @param angle angle to find the cos of, in degrees
     @return the cos of the angle
     @see sin
     @ingroup easy
  */
  static double cos(double angle) { return ::cos(ArMath::degToRad(angle)); }

  /// Finds the sin, from angles in degrees
  /**
     @param angle angle to find the sin of, in degrees
     @return the sin of the angle
     @see cos
     @ingroup easy
  */
  static double sin(double angle) { return ::sin(ArMath::degToRad(angle)); }

  /// Finds the tan, from angles in degrees
  /**
     @param angle angle to find the tan of, in degrees
     @return the tan of the angle
     @ingroup easy
  */
  static double tan(double angle) { return ::tan(ArMath::degToRad(angle)); }

  /// Finds the arctan of the given y/x pair
  /**
     @param y the y distance
     @param x the x distance
     @return the angle y and x form
     @ingroup easy
  */
  static double atan2(double y, double x) 
    { return ArMath::radToDeg(::atan2(y, x)); }

  /// Finds if one angle is between two other angles
  /// @ingroup easy
  static bool angleBetween(double angle, double startAngle, double endAngle)
    {
      angle = fixAngle(angle);
      startAngle = fixAngle(startAngle);
      endAngle = fixAngle(endAngle);
      if ((startAngle < endAngle && angle > startAngle && angle < endAngle) ||
	  (startAngle > endAngle && (angle > startAngle || angle < endAngle)))
	return true;
      else
	return false;
    }

  /// Finds the absolute value of a double
  /**
     @param val the number to find the absolute value of
     @return the absolute value of the number
  */
  static double fabs(double val) 
    {
      if (val < 0.0)
	return -val;
      else
	return val;
    }

  /// Finds the closest integer to double given
  /**
     @param val the double to find the nearest integer to
     @return the integer the value is nearest to (also caps it within 
     int bounds)
  */
  static int roundInt(double val) 
    { 
      val += .49;
      if (val > INT_MAX)
	return (int) INT_MAX;
      else if (val < INT_MIN)
	return (int) INT_MIN;
      else
	return((int) floor(val)); 
    }
    
  /// Finds the closest short to double given
  /**
     @param val the double to find the nearest short to
     @return the integer the value is nearest to (also caps it within 
     short bounds)
  */
  static short roundShort(double val) 
    { 
      val += .49;
      if (val > 32767)
	return (short) 32767;
      else if (val < -32768)
	return (short) -32768;
      else
	return((short) floor(val)); 
    }
    

  /// Rotates a point around 0 by degrees given
  static void pointRotate(double *x, double *y, double th)
    {
      double cs, sn, xt, yt;
      cs = cos(th);
      sn = sin(th);
      xt = *x;  
      yt = *y;
      *x = cs*xt + sn*yt;
      *y = cs*yt - sn*xt;
    }
  
  /** Returns a random number between 0 and RAND_MAX on Windows, 2^31 on Linux
   * (see ArUtil::getRandMax()). On Windows, rand() is used, on Linux, lrand48(). */
  static long random(void)
    {
#ifdef WIN32
      return(rand());
#else
      return(lrand48());
#endif
    }
  
  /// Maximum of value returned by random()
  AREXPORT static long getRandMax();

  /** Returns a random number between @a m and @a n. On Windows, rand() is used,
   * on Linux lrand48(). 
   * @ingroup easy
  */
  AREXPORT static long randomInRange(long m, long n);

  /// Finds the distance between two coordinates
  /**
     @param x1 the first coords x position
     @param y1 the first coords y position
     @param x2 the second coords x position
     @param y2 the second coords y position
     @return the distance between (x1, y1) and (x2, y2)
   * @ingroup easy
  **/
  static double distanceBetween(double x1, double y1, double x2, double y2)
    { return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));  }

  /// Finds the squared distance between two coordinates
  /**
     use this only where speed really matters
     @param x1 the first coords x position
     @param y1 the first coords y position
     @param x2 the second coords x position
     @param y2 the second coords y position
     @return the distance between (x1, y1) and (x2, y2)
  **/
  static double squaredDistanceBetween(double x1, double y1, double x2, double y2)
    { return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);  }

  /** Base-2 logarithm */
  static double log2(double x)
  {
    return log10(x) / 0.3010303;  // 0.301... is log10(2.0).
  }

  /// Platform-independent call to determine whether the given double is not-a-number.
  static bool isNan(double d) {
#ifdef WIN32
    return _isnan(d);
#else 
    return isnan(d);
#endif
  }

  static bool isNan(float f) {
#ifdef WIN32
	  return _isnan(f);
#else
	  return isnan(f);
#endif
  }

  static bool isFinite(float f) {
#ifdef WIN32
	  return _finite(f);
#else
          return isfinite(f);
#endif
  }

  static bool isFinite(double d) {
#ifdef WIN32
	  return _finite(d);
#else
	  return isfinite(d);
#endif
  }

  static bool compareFloats(double f1, double f2, double epsilon)
  {
    return (fabs(f2-f1) <= epsilon);
  }

  static bool compareFloats(double f1, double f2)
  {
    return compareFloats(f1, f2, epsilon());
  }


}; // end class ArMath

/// Represents an x, y position with an orientation
/** 
    This class represents a robot position with heading.  The heading is 
    automatically adjusted to be in the range -180 to 180.  It also defaults
    to 0, and so does not need to be used. (This avoids having 2 types of 
    positions.)  Everything in the class is inline so it should be fast.

  @ingroup UtilityClasses
  @ingroup easy
  @sa ArPoseWithTime
*/
class ArPose
{
public:


  /// Constructor, with optional initial values
  /** 
      Sets the pose to the given values.  The constructor can be called with no 
      parameters, with just x and y, or with x, y, and th.  The given heading (th)
      is automatically adjusted to be in the range -180 to 180.

      @param x the double to set the x position to, default of 0
      @param y the double to set the y position to, default of 0
      @param th the double value for the pose's heading (or th), default of 0
  */
  ArPose(double x = 0, double y = 0, double th = 0) :
    myX(x),
    myY(y),
    myTh(ArMath::fixAngle(th))
  {}
    
  /// Copy Constructor
  ArPose(const ArPose &pose) : 
    myX(pose.myX), myY(pose.myY), myTh(pose.myTh) {}

  /// Destructor
  virtual ~ArPose() {}
  /// Sets the position to the given values
  /** 
      Sets the position with the given three values, but the theta does not
      need to be given as it defaults to 0.
      @param x the position to set the x position to
      @param y the position to set the y position to
      @param th the position to set the th position to, default of 0
  */
  virtual void setPose(double x, double y, double th = 0) 
    { setX(x); setY(y); setTh(th); }
  /// Sets the position equal to the given position
  /** @param position the position value this instance should be set to */
  virtual void setPose(ArPose position)
    {
      setX(position.getX());
      setY(position.getY());
      setTh(position.getTh());
    }
  /// Sets the x position
  void setX(double x) { myX = x; }
  /// Sets the y position
  void setY(double y) { myY = y; }
  /// Sets the heading
  void setTh(double th) { myTh = ArMath::fixAngle(th); }
  /// Sets the heading, using radians
  void setThRad(double th) { myTh = ArMath::fixAngle(ArMath::radToDeg(th)); }
  /// Gets the x position
  double getX(void) const { return myX; }
  /// Gets the y position
  double getY(void) const { return myY; }
  /// Gets the heading
  double getTh(void) const { return myTh; }
  /// Gets the heading, in radians
  double getThRad(void) const { return ArMath::degToRad(myTh); }
  /// Gets the whole position in one function call
  /**
     Gets the whole position at once, by giving it 2 or 3 pointers to 
     doubles.  If you give the function a null pointer for a value it won't
     try to use the null pointer, so you can pass in a NULL if you don't 
     care about that value.  Also note that th defaults to NULL so you can 
     use this with just x and y.
     @param x a pointer to a double to set the x position to
     @param y a pointer to a double to set the y position to
     @param th a pointer to a double to set the heading to, defaults to NULL
   */
  void getPose(double *x, double *y, double *th = NULL) const
    { 
      if (x != NULL) 
	      *x = myX;
      if (y != NULL) 
	      *y = myY; 
      if (th != NULL) 
	      *th = myTh; 
    }
  /// Finds the distance from this position to the given position
  /**
     @param position the position to find the distance to
     @return the distance to the position from this instance
  */
  virtual double findDistanceTo(ArPose position) const
    {
      return ArMath::distanceBetween(getX(), getY(), 
				     position.getX(), 
				     position.getY());
    }

  /// Finds the square distance from this position to the given position
  /**
     This is only here for speed, if you aren't doing this thousands
     of times a second don't use this one use findDistanceTo

     @param position the position to find the distance to
     @return the distance to the position from this instance 
  **/
  virtual double squaredFindDistanceTo(ArPose position) const
    {
      return ArMath::squaredDistanceBetween(getX(), getY(), 
					    position.getX(), 
					    position.getY());
    }
  /// Finds the angle between this position and the given position
  /** 
      @param position the position to find the angle to
      @return the angle to the given position from this instance, in degrees
  */
  virtual double findAngleTo(ArPose position) const
    {
      return ArMath::radToDeg(atan2(position.getY() - getY(),
				                            position.getX() - getX()));
    }
  /// Logs the coordinates using ArLog
  virtual void log(void) const
    { ArLog::log(ArLog::Terse, "%.0f %.0f %.1f", myX, myY, myTh); }

  /// Add the other pose's X, Y and theta to this pose's X, Y, and theta (sum in theta will be normalized to (-180,180)), and return the result
  virtual ArPose operator+(const ArPose& other) const
  {
    return ArPose( myX + other.getX(), 
                   myY + other.getY(), 
                   ArMath::fixAngle(myTh + other.getTh()) );
  }

  /// Substract the other pose's X, Y, and theta from this pose's X, Y, and theta (difference in theta will be normalized to (-180,180)), and return the result

  virtual ArPose operator-(const ArPose& other) const
  {
    return ArPose( myX - other.getX(), 
                   myY - other.getY(), 
                   ArMath::fixAngle(myTh - other.getTh()) );
  }
  
  /** Adds the given pose to this one.
   *  @swigomit
   */
	ArPose & operator+= ( const ArPose & other)
  {
    myX += other.myX;
    myY += other.myY;
    myTh = ArMath::fixAngle(myTh + other.myTh);
    return *this;
  }

	/** Subtracts the given pose from this one.
     *  @swigomit
     */
	ArPose & operator-= ( const ArPose & other)
  {
    myX -= other.myX;
    myY -= other.myY;
    myTh = ArMath::fixAngle(myTh - other.myTh);
    return *this;
  }

  /// Equality operator (for sets)
  virtual bool operator==(const ArPose& other) const
  {
    return ((fabs(myX - other.myX) < ArMath::epsilon()) &&
            (fabs(myY - other.myY) < ArMath::epsilon()) &&
            (fabs(myTh - other.myTh) < ArMath::epsilon()));
  }

  virtual bool operator!=(const ArPose& other) const
  {
    return ((fabs(myX - other.myX) > ArMath::epsilon()) ||
            (fabs(myY - other.myY) > ArMath::epsilon()) ||
            (fabs(myTh - other.myTh) > ArMath::epsilon()));
  }

  /// Less than operator (for sets)
  virtual bool operator<(const ArPose& other) const
  {

    if (fabs(myX - other.myX) > ArMath::epsilon()) {
      return myX < other.myX;
    }
    else if (fabs(myY - other.myY) > ArMath::epsilon()) {
      return myY < other.myY;  
    }
    else if (fabs(myTh - other.myTh) > ArMath::epsilon()) {
      return myTh < other.myTh;
    }
    // Otherwise... x, y, and th are equal
    return false;
    
  } // end operator <

  /// Finds the distance between two poses (static function, uses no
  /// data from any instance and shouldn't be able to be called on an
  /// instance)
  /**
     @param pose1 the first coords
     @param pose2 the second coords
     @return the distance between the poses
  **/
  static double distanceBetween(ArPose pose1, ArPose pose2)
    { return ArMath::distanceBetween(pose1.getX(), pose1.getY(),
				     pose2.getX(), pose2.getY()); }

  /// Return true if the X value of p1 is less than the X value of p2
  static bool compareX(const ArPose& p1, const ArPose &p2)
    { return (p1.getX() < p2.getX()); } 
  /// Return true if the Y value of p1 is less than the X value of p2
  static bool compareY(const ArPose& p1, const ArPose &p2)
    { return (p1.getY() < p2.getY()); } 

  bool isInsidePolygon(const std::vector<ArPose>& vertices) const
  {
    bool inside = false;
    const size_t n = vertices.size();
    size_t i = 0;
    size_t j = n-1;
    for(; i < n; j = i++)
    {
      const double x1 = vertices[i].getX();
      const double x2 = vertices[j].getX();
      const double y1 = vertices[i].getY();
      const double y2 = vertices[j].getY();
      if((((y1 < getY()) && (getY() < y2)) || ((y2 <= getY()) && (getY() < y1))) && (getX() <= (x2 - x1) * (getY() - y1) / (y2 - y1) + x1))
        inside = !inside;
    }
    return inside;
  }

protected:

  double myX;
  double myY;
  double myTh;
};


/// Like ArPose, but with objects of this type, Theta component must be ignored.
/// Can be used to distinguish between pose with theta, and position only, by
/// object type, but objects this type can still be used directly in place of normal ArPose.
class ArPos2D : public ArPose
{
public:
  ArPos2D(double x, double y) : ArPose(x, y, 0) {}
private:
  // make some methods from ArPose private so they can't be used
  double getTh() const { return 0.0; }
  double getThRad() const { return 0.0; }
  void setTh(double) {}
  void setThRat(double) {}
};


/// A class for time readings and measuring durations
/** 
    This class is for timing durations or time between events.
    The time values it stores are relative to an abritrary starting time; it
    does not correspond to "real world" or "wall clock" time in any way,
    so DON'T use this for keeping track of what time it is, 
    just for timestamps and relative timing (e.g. "this loop needs to sleep another 100 ms").

    The recommended methods to use are setToNow() to reset the time,
    mSecSince() to obtain the number of milliseconds elapsed since it was
    last reset (or secSince() if you don't need millisecond precision), and
    mSecSince(ArTime) or secSince(ArTime) to find the difference between 
    two ArTime objects.

    On systems where it is supported this will use a monotonic clock,
    this is an ever increasing system that is not dependent on what
    the time of day is set to.  Normally for linux gettimeofday is
    used, but if the time is changed forwards or backwards then bad
    things can happen.  Windows uses a time since bootup, which
    functions the same as the monotonic clock anyways.  You can use
    ArTime::usingMonotonicClock() to see if this is being used.  Note
    that an ArTime will have had to have been set to for this to be a
    good value... Aria::init does this however, so that should not be
    an issue.  It looks like the monotonic clocks won't work on linux
    kernels before 2.6.

  @ingroup UtilityClasses
*/

class ArTime
{
public:
  /// Constructor. Time is initialized to the current time.
  /// @ingroup easy
  ArTime() { setToNow(); }

  /// Copy constructor
  //
  ArTime(const ArTime &other) :
    mySec(other.mySec),
    myMSec(other.myMSec)
  {}

  /// Assignment operator 
  ArTime &operator=(const ArTime &other) 
  {
    if (this != &other) {
      mySec = other.mySec;
      myMSec = other.myMSec;
    }
    return *this;
  }

  //
  /// Destructor
  ~ArTime() {}
  
  /// Gets the number of milliseconds since the given timestamp to this one
  /// @ingroup easy
  long mSecSince(ArTime since) const 
    {
      long long ret = mSecSinceLL(since);
      if (ret > INT_MAX)
	return INT_MAX;
      if (ret < -INT_MAX)
	return -INT_MAX;
      return ret;
      /*  The old way that had problems with wrapping
      long long timeSince, timeThis;

      timeSince = since.getSec() * 1000 + since.getMSec();
      timeThis = mySec * 1000 + myMSec;
      return timeSince - timeThis;
      */
    }
  /// Gets the number of milliseconds since the given timestamp to this one
  long long mSecSinceLL(ArTime since) const 
    {
      long long timeSince, timeThis;

      timeSince = since.getSecLL() * 1000 + since.getMSecLL();
      timeThis = mySec * 1000 + myMSec;
      return timeSince - timeThis;
    }
  /// Gets the number of seconds since the given timestamp to this one
  /// @ingroup easy
  long secSince(ArTime since) const
    {
      return mSecSince(since)/1000;
    }
  /// Gets the number of seconds since the given timestamp to this one
  long long secSinceLL(ArTime since) const
    {
      return mSecSinceLL(since)/1000;
    }
  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  /// @ingroup easy
  long mSecTo(void) const
    {
      ArTime now;
      now.setToNow();
      return -mSecSince(now);
    }
  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  long long mSecToLL(void) const
    {
      ArTime now;
      now.setToNow();
      return -mSecSinceLL(now);
    }
  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  /// @ingroup easy
  long secTo(void) const
    {
      return mSecTo()/1000;
    }
  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  long long secToLL(void) const
    {
      return mSecToLL()/1000;
    }
  /// Finds the number of milliseconds from this timestamp to now
  long mSecSince(void) const
    {
      ArTime now;
      now.setToNow();
      return mSecSince(now);
    }
  /// Finds the number of milliseconds from this timestamp to now
  long long mSecSinceLL(void) const
    {
      ArTime now;
      now.setToNow();
      return mSecSinceLL(now);
    }
  /// Finds the number of seconds from when this timestamp was set to now
  long secSince(void) const
    {
      return mSecSince()/1000;
    }
  /// Finds the number of seconds from when this timestamp was set to now
  long long secSinceLL(void) const
    {
      return mSecSinceLL()/1000;
    }
  /// returns whether the given time is before this one or not
  /// @ingroup easy
  bool isBefore(ArTime testTime) const
    {
      if (mSecSince(testTime) < 0)
	return true;
      else
	return false;
    }
  /// returns whether the given time is equal to this time or not
  bool isAt(ArTime testTime) const
    {
      if (mSecSince(testTime) == 0)
	return true;
      else
	return false;
    }
  /// returns whether the given time is after this one or not
  /// @ingroup easy
  bool isAfter(ArTime testTime) const
    {
      if (mSecSince(testTime) > 0)
	return true;
      else
	return false;
    }
  /// Resets the time
  /// @ingroup easy
  AREXPORT void setToNow(void);
  /// Add some milliseconds (can be negative) to this time
  bool addMSec(long ms)
    {
      //unsigned long timeThis;
      long long timeThis;
      timeThis = mySec * 1000 + myMSec;
      //if (ms < 0 && (unsigned)abs(ms) > timeThis)
      if (ms < 0 && -ms > timeThis)
      {
        ArLog::log(ArLog::Terse, "ArTime::addMSec: tried to subtract too many milliseconds, would result in a negative time.");
        mySec = 0;
        myMSec = 0;
        return false;
      }
      else 
      {
        timeThis += ms;
        mySec = timeThis / 1000;
	myMSec = timeThis % 1000;
      }
      return true;
    } // end method addMSec

  /// Add some milliseconds (can be negative) to this time
  bool addMSecLL(long long ms)
    {
      //unsigned long timeThis;
      long long timeThis;
      timeThis = mySec * 1000 + myMSec;
      //if (ms < 0 && (unsigned)abs(ms) > timeThis)
      if (ms < 0 && -ms > timeThis)
      {
        ArLog::log(ArLog::Terse, "ArTime::addMSec: tried to subtract too many milliseconds, would result in a negative time.");
        mySec = 0;
        myMSec = 0;
        return false;
      }
      else 
      {
        timeThis += ms;
        mySec = timeThis / 1000;
	myMSec = timeThis % 1000;
      }
      return true;
    } // end method addMSec
  
  
  /// Sets the seconds value (since the arbitrary starting time)
  void setSec(unsigned long sec) { mySec = sec; }
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSec(unsigned long msec) { myMSec = msec; }
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long getSec(void) const { return mySec; }
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long getMSec(void) const { return myMSec; }

  /// Sets the seconds value (since the arbitrary starting time)
  void setSecLL(unsigned long long sec) { mySec = sec; }
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSecLL(unsigned long long msec) { myMSec = msec; }
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long long getSecLL(void) const { return mySec; }
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long long getMSecLL(void) const { return myMSec; }
  /// Logs the time
  /// @ingroup easy
  void log(const char *prefix = NULL) const
    { ArLog::log(ArLog::Terse, 
                 "%sTime: %lld.%lld", 
                 ((prefix != NULL) ? prefix : ""),
                 mySec, 
		 myMSec); }
  /// Gets if we're using a monotonic (ever increasing) clock
  static bool usingMonotonicClock()
    {
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
      return ourMonotonicClock;
#endif
#ifdef WIN32
      return true;
#endif
      return false;
    }
  
  /// Equality operator (for sets)
  bool operator==(const ArTime& other) const
  {
    return isAt(other);
  }

  bool operator!=(const ArTime& other) const
  {
    return (!isAt(other));
  }
 
  // Less than operator for sets
  /// @ingroup easy
  bool operator<(const ArTime& other) const
  {
    return isBefore(other);
  } // end operator <

  /// @ingroup easy
  bool operator>(const ArTime& other) const
  {
    return isAfter(other);
  }

protected:
  unsigned long long mySec;
  unsigned long long myMSec;
//#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  static bool ourMonotonicClock; // will be statically initialized to true in ariaUtil.cpp if at compile we have _POSIX_TIMERS and _POSIX_MONOTONIC_CLOCK, otherwise to false.
//#endif 

}; // end class ArTime




/// A subclass of ArPose that also stores a timestamp (ArTime) 
/**
  @ingroup UtilityClasses
 */
class ArPoseWithTime : public ArPose
{
public:
  ArPoseWithTime(double x = 0, double y = 0, double th = 0,
	 ArTime thisTime = ArTime()) : ArPose(x, y, th)
    { myTime = thisTime; }
  /// Copy Constructor
  ArPoseWithTime(const ArPose &pose) : ArPose(pose) {}
  virtual ~ArPoseWithTime() {}
  void setTime(ArTime newTime) { myTime = newTime; }
  void setTimeToNow(void) { myTime.setToNow(); }
  ArTime getTime(void) const { return myTime; }
protected:
  ArTime myTime;
};

/// A class for keeping track of if a complete revolution has been attained
/**
   This class can be used to keep track of if a complete revolution has been
   done, it is used by doing doing a clearQuadrants when you want to stat
   the revolution.  Then at each point doing an updateQuadrant with the current
   heading of the robot.  When didAllQuadrants returns true, then all the 
   quadrants have been done.
  @ingroup UtilityClasses
*/
class ArSectors
{
public:
  /// Constructor
  ArSectors(int numSectors = 8) 
    { 
      mySectorSize = 360/numSectors;
      mySectors = new int[numSectors]; 
      myNumSectors = numSectors; 
      clear();
    }
  /// Destructor
  virtual ~ArSectors() { delete mySectors; }
  /// Clears all quadrants
  void clear(void) 
    {
      int i;
      for (i = 0; i < myNumSectors; i++)
	mySectors[i] = false;
    }
  /// Updates the appropriate quadrant for the given angle
  void update(double angle)
    {
      int angleInt;
      angleInt = ArMath::roundInt(ArMath::fixAngle(angle) + 180);
      mySectors[angleInt / mySectorSize] = true;
    }
  /// Returns true if the all of the quadrants have been gone through
  bool didAll(void) const
    {
      int i;
      for (i = 0; i < myNumSectors; i++)
	if (mySectors[i] == false)
	  return false;
      return true;
    }
protected:
  int *mySectors;
  int myNumSectors;
  int mySectorSize;
};




/// Represents geometry of a line in two-dimensional space.
/**
   Note this the theoretical line, i.e. it goes infinitely. 
   For a line segment with endpoints, use ArLineSegment.
   @sa ArLineSegment
  @ingroup UtilityClasses
**/
class ArLine
{
public:
  ///// Empty constructor
  ArLine() {}
  /// Constructor with parameters
  ArLine(double a, double b, double c) { newParameters(a, b, c); }
  /// Constructor with endpoints
  ArLine(double x1, double y1, double x2, double y2) 
  { newParametersFromEndpoints(x1, y1, x2, y2); }
  /// Destructor
  virtual ~ArLine() {}
  /// Sets the line parameters (make it not a segment)
  void newParameters(double a, double b, double c) 
    { myA = a; myB = b; myC = c; }
  /// Sets the line parameters from endpoints, but makes it not a segment
  void newParametersFromEndpoints(double x1, double y1, double x2, double y2)
    { myA = y1 - y2; myB = x2 - x1; myC = (x2*y1) - (y2*x1);}
  /// Gets the A line parameter
  double getA(void) const { return myA; }
  /// Gets the B line parameter
  double getB(void) const { return myB; }
  /// Gets the C line parameter
  double getC(void) const { return myC; }
  /// finds the intersection of this line with another line
  /** 
      @param line the line to check if it intersects with this line
      @param pose if the lines intersect, the pose is set to the location
      @return true if they intersect, false if they do not 
  **/
  bool intersects(const ArLine *line, ArPose *pose) const
    {
      double x, y;
      double n;
      n = (line->getB() * getA()) - (line->getA() * getB());
      n*=-1;
      // if this is 0 the lines are parallel
      if (fabs(n) < .0000000000001)
      {
	return false;
      }
      // they weren't parallel so see where the intersection is
      x = ((line->getC() * getB()) - (line->getB() * getC())) / n;
      y = ((getC() * line->getA()) - (getA() * line->getC())) / n;
      pose->setPose(x, y);
      return true;
    }
  /// Makes the given line perpendicular to this one through the given pose
  void makeLinePerp(const ArPose *pose, ArLine *line) const
    {
      line->newParameters(-getB(), getA(),
			  (getA() * pose->getY()) - (getB() * pose->getX()));
    }
   /// Calculate the distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect line it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpDist(const ArPose &pose) const
    {
      ArPose perpPose;
      ArLine perpLine;
      makeLinePerp(&pose, &perpLine);
      if (!intersects(&perpLine, &perpPose))
	return -1;
      return (perpPose.findDistanceTo(pose));
    }
   /// Calculate the squared distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect line it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpSquaredDist(const ArPose &pose) const
    {
      ArPose perpPose;
      ArLine perpLine;
      makeLinePerp(&pose, &perpLine);
      if (!intersects(&perpLine, &perpPose))
	return -1;
      return (perpPose.squaredFindDistanceTo(pose));
    }
  /// Determine the intersection point between this line segment, and a perpendicular line passing through the given pose (i.e. projects the given pose onto this line segment.)
  /**
   * If there is no intersection, false is returned.
     @param pose The X and Y components of this pose object indicate the point to project onto this line segment.
     @param perpPoint The X and Y components of this pose object are set to indicate the intersection point
     @return true if an intersection was found and perpPoint was modified, false otherwise.
     @swigomit
  **/
  bool getPerpPoint(const ArPose &pose, ArPose *perpPoint) const
    {
      ArLine perpLine;
      makeLinePerp(&pose, &perpLine);
      return intersects(&perpLine, perpPoint);
    }

  /// Equality operator
  virtual bool operator==(const ArLine &other) const
  {

    return ((fabs(myA - other.myA) <= ArMath::epsilon()) &&
            (fabs(myB - other.myB) <= ArMath::epsilon()) &&
            (fabs(myC - other.myC) <= ArMath::epsilon()));
  }
  /// Inequality operator
  virtual bool operator!=(const ArLine &other) const
  {
    return ((fabs(myA - other.myA) > ArMath::epsilon()) ||
            (fabs(myB - other.myB) > ArMath::epsilon()) ||
            (fabs(myC - other.myC) > ArMath::epsilon()));

  }

protected:
  double myA, myB, myC;
};

/// Represents a line segment in two-dimensional space.
/** The segment is defined by the coordinates of each endpoint. 
  @ingroup UtilityClasses
*/
class ArLineSegment
{
public:
#ifndef SWIG
  /** @swigomit */
  ArLineSegment() {}
  /** @brief Constructor with endpoints
   *  @swigomit
   */
  ArLineSegment(double x1, double y1, double x2, double y2)
    { 	newEndPoints(x1, y1, x2, y2); }
#endif // SWIG
  /// Constructor with endpoints as ArPose objects. Only X and Y components of the poses will be used.
  ArLineSegment(ArPose pose1, ArPose pose2)
    { 	newEndPoints(pose1.getX(), pose1.getY(), pose2.getX(), pose2.getY()); }
  virtual ~ArLineSegment() {}
  /// Set new end points for this line segment
  void newEndPoints(double x1, double y1, double x2, double y2)
    {
      myX1 = x1; myY1 = y1; myX2 = x2; myY2 = y2; 
      myLine.newParametersFromEndpoints(myX1, myY1, myX2, myY2);
    }
  /// Set new end points for this line segment
  void newEndPoints(const ArPose& pt1, const ArPose& pt2)
    {
      newEndPoints(pt1.getX(), pt1.getY(), pt2.getX(), pt2.getY());
    }
  /// Get the first endpoint (X1, Y1)
  ArPose getEndPoint1(void) const { return ArPose(myX1, myY1); }
  /// Get the second endpoint of (X2, Y2)
  ArPose getEndPoint2(void) const { return ArPose(myX2, myY2); }
  /// Determine where a line intersects this line segment
  /**
      @param line Line to check for intersection against this line segment.
      @param pose if the lines intersect, the X and Y components of this pose are set to the point of intersection.
      @return true if they intersect, false if they do not 
   **/
  bool intersects(const ArLine *line, ArPose *pose) const
    {
      ArPose intersection;
      // see if it intersects, then make sure its in the coords of this line
      if (myLine.intersects(line, &intersection) &&
	  linePointIsInSegment(&intersection))
      {
	pose->setPose(intersection);
	return true;
      }
      else
	return false;
    }

  /** @copydoc intersects(const ArLine *line, ArPose *pose) const */
  bool intersects(ArLineSegment *line, ArPose *pose) const
    {
      ArPose intersection;
      // see if it intersects, then make sure its in the coords of this line
      if (myLine.intersects(line->getLine(), &intersection) &&
	  linePointIsInSegment(&intersection) &&
	  line->linePointIsInSegment(&intersection))
      {
	pose->setPose(intersection);
	return true;
      }
      else
	return false;
    }
#ifndef SWIG
  /** Determine the intersection point between this line segment, and a perpendicular line passing through the given pose (i.e. projects the given pose onto this line segment.)
     If there is no intersection, false is returned.
     @param pose The X and Y components of this pose object indicate the point to project onto this line segment.
     @param perpPoint The X and Y components of this pose object are set to indicate the intersection point
     @return true if an intersection was found and perpPoint was modified, false otherwise.
     @swigomit
  **/
  bool getPerpPoint(const ArPose &pose, ArPose *perpPoint) const
    {
      ArLine perpLine;
      myLine.makeLinePerp(&pose, &perpLine);
      return intersects(&perpLine, perpPoint);
    }
#endif
  /** @copydoc getPerpPoint(const ArPose&, ArPose*) const
   *  (This version simply allows you to pass the first pose as a pointer, in
   *  time-critical situations where a full copy of the object would impact
   *  performance.)
  */
  bool getPerpPoint(const ArPose *pose, ArPose *perpPoint) const
    {
      ArLine perpLine;
      myLine.makeLinePerp(pose, &perpLine);
      return intersects(&perpLine, perpPoint);
    }
   /// Calculate the distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect segment it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpDist(const ArPose &pose) const
    {
      ArPose perpPose;
      ArLine perpLine;
      myLine.makeLinePerp(&pose, &perpLine);
      if (!intersects(&perpLine, &perpPose))
	return -1;
      return (perpPose.findDistanceTo(pose));
    }
   /// Calculate the squared distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect segment it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpSquaredDist(const ArPose &pose) const
    {
      ArPose perpPose;
      ArLine perpLine;
      myLine.makeLinePerp(&pose, &perpLine);
      if (!intersects(&perpLine, &perpPose))
	      return -1;
      return (perpPose.squaredFindDistanceTo(pose));
    }

   /// Gets the distance from this line segment to a point.
  /**
   * If the point can be projected onto this line segment (i.e. a
   * perpendicular line can be drawn through the point), then
   * return that distance. Otherwise, return the distance to the closest
   * endpoint.
     @param pose the pointer of the pose to find the distance to
  **/
  double getDistToLine(const ArPose &pose) const
    {
      ArPose perpPose;
      ArLine perpLine;
      myLine.makeLinePerp(&pose, &perpLine);
      if (!intersects(&perpLine, &perpPose))
      {
	      return ArUtil::findMin(
		                    ArMath::roundInt(getEndPoint1().findDistanceTo(pose)),
		                    ArMath::roundInt(getEndPoint2().findDistanceTo(pose)));
      }
      return (perpPose.findDistanceTo(pose));
    }
  
  /// Determines the length of the line segment
  double getLengthOf() const
  {
    return ArMath::distanceBetween(myX1, myY1, myX2, myY2);
  }

  /// Determines the mid point of the line segment
  ArPose getMidPoint() const
  {
    return ArPose(((myX1 + myX2) / 2.0),
                  ((myY1 + myY2) / 2.0));
  }


  /// Gets the x coordinate of the first endpoint
  double getX1(void) const { return myX1; }
  /// Gets the y coordinate of the first endpoint
  double getY1(void) const { return myY1; } 
  /// Gets the x coordinate of the second endpoint
  double getX2(void) const { return myX2; }
  /// Gets the y coordinate of the second endpoint
  double getY2(void) const { return myY2; }
  /// Gets the A line parameter (see ArLine)
  double getA(void) const { return myLine.getA(); }
  /// Gets the B line parameter (see ArLine)
  double getB(void) const { return myLine.getB(); }
  /// Gets the C line parameter (see ArLine)
  double getC(void) const { return myLine.getC(); }

  /// Internal function for seeing if a point on our line is within our segment
  bool linePointIsInSegment(ArPose *pose) const
    {
      bool isVertical = (ArMath::fabs(myX1 - myX2) < ArMath::epsilon());
      bool isHorizontal = (ArMath::fabs(myY1 - myY2) < ArMath::epsilon());

      if (!isVertical || !isHorizontal) {

        return (((isVertical) || 
	               (pose->getX() >= myX1 && pose->getX() <= myX2) || 
	               (pose->getX() <= myX1 && pose->getX() >= myX2)) &&
	              ((isHorizontal) || 
	               (pose->getY() >= myY1 && pose->getY() <= myY2) || 
	               (pose->getY() <= myY1 && pose->getY() >= myY2)));
      }
      else { // single point segment

        return ((ArMath::fabs(myX1 - pose->getX()) < ArMath::epsilon()) &&
                (ArMath::fabs(myY1 - pose->getY()) < ArMath::epsilon()));

      } // end else single point segment
    }

  const ArLine *getLine(void) const { return &myLine; }

  /// Equality operator (for sets)
  virtual bool operator==(const ArLineSegment& other) const
  {

    return ((fabs(myX1 - other.myX1) < ArMath::epsilon()) &&
            (fabs(myY1 - other.myY1) < ArMath::epsilon()) &&
            (fabs(myX2 - other.myX2) < ArMath::epsilon()) &&
            (fabs(myY2 - other.myY2) < ArMath::epsilon()));
  }

  virtual bool operator!=(const ArLineSegment& other) const
  {
    return ((fabs(myX1 - other.myX1) > ArMath::epsilon()) ||
            (fabs(myY1 - other.myY1) > ArMath::epsilon()) ||
            (fabs(myX2 - other.myX2) > ArMath::epsilon()) ||
            (fabs(myY2 - other.myY2) > ArMath::epsilon()));

  }

  /// Less than operator (for sets)
  virtual bool operator<(const ArLineSegment& other) const
  {

    if (fabs(myX1 - other.myX1) > ArMath::epsilon()) {
      return myX1 < other.myX1;
    }
    else if (fabs(myY1 - other.myY1) > ArMath::epsilon()) {
      return myY1 < other.myY1;  
    }
    if (fabs(myX2 - other.myX2) > ArMath::epsilon()) {
      return myX2 < other.myX2;
    }
    else if (fabs(myY2 - other.myY2) > ArMath::epsilon()) {
      return myY2 < other.myY2;
    }
    // Otherwise... all coords are equal
    return false;
  }

protected:
  double myX1, myY1, myX2, myY2;
  ArLine myLine;
};

/**
   @brief Use for computing a running average of a number of elements
   @ingroup UtilityClasses
*/
class ArRunningAverage
{
public:
  /// Constructor, give it the number of elements to store to compute the average
  AREXPORT ArRunningAverage(size_t numToAverage);
  /// Destructor
  AREXPORT ~ArRunningAverage();
  /// Gets the average
  AREXPORT double getAverage(void) const;
  /// Adds a value to the average. An old value is discarded if the number of elements to average has been reached.
  AREXPORT void add(double val);
  /// Clears the average
  AREXPORT void clear(void);
  /// Gets the number of elements
  AREXPORT size_t getNumToAverage(void) const;
  /// Sets the number of elements
  AREXPORT void setNumToAverage(size_t numToAverage);
  /// Sets if this is using a the root mean square average or just the normal average
  AREXPORT void setUseRootMeanSquare(bool useRootMeanSquare);
  /// Gets if this is using a the root mean square average or just the normal average
  AREXPORT bool getUseRootMeanSquare(void);
  /// Gets the number of values currently averaged so far
  AREXPORT size_t getCurrentNumAveraged(void);
protected:
  size_t myNumToAverage;
  double myTotal;
  size_t myNum;
  bool myUseRootMeanSquare;
  std::list<double> myVals;
};

/// This is a class for computing a root mean square average of a number of elements
/// @ingroup UtilityClasses
class ArRootMeanSquareCalculator
{
public:
  /// Constructor
  AREXPORT ArRootMeanSquareCalculator();
  /// Destructor
  AREXPORT ~ArRootMeanSquareCalculator();
  /// Gets the average
  AREXPORT double getRootMeanSquare (void) const;
  /// Adds a number
  AREXPORT void add(int val);
  /// Clears the average
  AREXPORT void clear(void);
  /// Sets the name
  AREXPORT void setName(const char *name);
  /// Gets the name
  AREXPORT const char *getName(void);  
  /// Gets the num averaged
  AREXPORT size_t getCurrentNumAveraged(void);
protected:
  long long myTotal;
  size_t myNum;
  std::string myName;
};


//class ArStrCaseCmpOp :  public std::binary_function <const std::string&, const std::string&, bool> 
/// strcasecmp for sets
/// @ingroup UtilityClasses
struct ArStrCaseCmpOp 
{
public:
  bool operator() (const std::string &s1, const std::string &s2) const
  {
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
  }
};

/// ArPose less than comparison for sets
/// @ingroup UtilityClasses
struct ArPoseCmpOp
{
public:
  bool operator() (const ArPose &pose1, const ArPose &pose2) const
  {
    return (pose1 < pose2);

    //return (pose1.getX() < pose2.getX() || pose1.getY() < pose2.getY() ||
	  //        pose1.getTh() < pose2.getTh());
  }
};

/// ArLineSegment less than comparison for sets
/// @ingroup UtilityClasses
struct ArLineSegmentCmpOp
{
public:
  bool operator() (const ArLineSegment &line1, 
		               const ArLineSegment &line2) const
  {
    return (line1 < line2);

    //return (line1.getX1() < line2.getX1() || line1.getY1() < line2.getY1() ||
	  //  line1.getX2() < line2.getX2() || line1.getY2() < line2.getY2());
  }
};


#if !defined(WIN32) && !defined(SWIG)
/** @brief Switch to running the program as a background daemon (i.e. fork) (Only available in Linux)
  @swigomit
  @notwindows
  @ingroup UtilityClasses
  @ingroup OptionalClasses
 */
class ArDaemonizer
{
public:
  /// Constructor that sets up for daemonizing if arg checking
  AREXPORT ArDaemonizer(int *argc, char **argv, bool closeStdErrAndStdOut);
  /// Destructor
  AREXPORT ~ArDaemonizer();
  /// Daemonizes if asked too by arguments
  AREXPORT bool daemonize(void);
  /// Daemonizes always
  AREXPORT bool forceDaemonize(void);
  /// Logs the options
  AREXPORT void logOptions(void) const;
  /// Returns if we're daemonized or not
  bool isDaemonized(void) { return myIsDaemonized; }
protected:
  ArArgumentParser myParser;
  bool myIsDaemonized;
  bool myCloseStdErrAndStdOut;
  ArConstFunctorC<ArDaemonizer> myLogOptionsCB;
};
#endif // !win32 && !swig



/// Contains enumeration of four user-oriented priority levels (used primarily by ArConfig)
class ArPriority
{
public:
  enum Priority 
  {
    INVALID_PRIORITY = -1, ///< An invalid value.

    IMPORTANT, ///< Basic things that should be modified to suit 
    BASIC = IMPORTANT,  ///< Basic things that should be modified to suit 
    FIRST_PRIORITY = IMPORTANT,

    NORMAL,    ///< Intermediate things that users may want to modify
    INTERMEDIATE = NORMAL, ///< Intermediate things that users may want to modify

    DETAILED, ///< Advanced items that probably shouldn't be modified
    TRIVIAL = DETAILED, ///< Advanced items (alias for historic reasons)
    ADVANCED = DETAILED, ///< Advanced items that probably shouldn't be modified

    EXPERT,  ///< Items that should be modified only by expert users or developers
    FACTORY, ///< Items that should be modified at the factory, often apply to a robot model

    CALIBRATION, ///< Items that apply to a particular hardware instance

    LAST_PRIORITY = CALIBRATION ///< Last value in the enumeration
  };

  enum {
    PRIORITY_COUNT = LAST_PRIORITY + 1 ///< Number of priority values
  };

  /// Returns the displayable text string for the given priority
  AREXPORT static const char * getPriorityName(Priority priority);
   
  /// Returns the priority value that corresponds to the given displayable text string
  /**
   * @param text the char * to be converted to a priority value
   * @param ok an optional bool * set to true if the text was successfully 
   * converted; false if the text was not recognized as a priority
  **/
  AREXPORT static Priority getPriorityFromName(const char *text,
                                               bool *ok = NULL);

protected:

  /// Whether the map of priorities to display text has been initialized
  static bool ourStringsInited;
  /// Map of priorities to displayable text
  static std::map<Priority, std::string> ourPriorityNames;
  /// Map of displayable text to priorities
  static std::map<std::string, ArPriority::Priority, ArStrCaseCmpOp> ourNameToPriorityMap;

  /// Display text used when a priority's displayable text has not been defined
  static std::string ourUnknownPriorityName;
};

/// holds information about ArStringInfo component strings (it's a helper class for other things)
/**
   This class holds information for about different strings that are available 
 **/
class ArStringInfoHolder
{
public:
  /// Constructor
  ArStringInfoHolder(const char *name, ArTypes::UByte2 maxLength, 
		     ArFunctor2<char *, ArTypes::UByte2> *functor)
    { myName = name; myMaxLength = maxLength; myFunctor = functor; }
  /// Destructor
  virtual ~ArStringInfoHolder() {}
  /// Gets the name of this piece of info
  const char *getName(void) { return myName.c_str(); }
  /// Gets the maximum length of this piece of info
  ArTypes::UByte2 getMaxLength(void) { return myMaxLength; }
  /// Gets the function that will fill in this piece of info
  ArFunctor2<char *, ArTypes::UByte2> *getFunctor(void) { return myFunctor; }
protected:
  std::string myName;
  ArTypes::UByte2 myMaxLength;
  ArFunctor2<char *, ArTypes::UByte2> *myFunctor;
};

/// This class just holds some helper functions for the ArStringInfoHolder 
class ArStringInfoHolderFunctions
{
public:
  static void intWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			 ArRetFunctor<int> *functor, const char *format, int navalue)
  { 
    const int value = functor->invokeR();
    if(value >= navalue)
      strncpy(buffer, "N/A", bufferLen-1);
    else
      snprintf(buffer, bufferLen - 1, format, value);
    buffer[bufferLen-1] = '\0'; 
  }
  static void doubleWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			    ArRetFunctor<double> *functor, const char *format)
    { snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; }
  static void boolWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			  ArRetFunctor<bool> *functor, const char *format)
    { snprintf(buffer, bufferLen - 1, format, 
	       ArUtil::convertBool(functor->invokeR())); 
    buffer[bufferLen-1] = '\0'; }
  static void stringWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			    ArRetFunctor<const char *> *functor, 
			    const char *format)
  { snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
  buffer[bufferLen-1] = '\0'; }

  static void cppStringWrapper(char *buffer, ArTypes::UByte2 bufferLen, 
          ArRetFunctor<std::string> *functor)
  { 
    snprintf(buffer, bufferLen - 1, "%s", functor->invokeR().c_str());
    buffer[bufferLen-1] = '\0'; 
  }

  static void unsignedLongWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			 ArRetFunctor<unsigned long> *functor, const char *format, unsigned long navalue)
  { 
    const unsigned long value = functor->invokeR();
    if(value >= navalue)
      strncpy(buffer, "N/A", bufferLen-1);
    else
      snprintf(buffer, bufferLen - 1, format, value);
    buffer[bufferLen-1] = '\0'; 
  }
  static void longWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			 ArRetFunctor<long> *functor, const char *format, long navalue)
  { 
    const long value = functor->invokeR();
    if(value >= navalue)
      strncpy(buffer, "N/A", bufferLen-1);
    else
      snprintf(buffer, bufferLen - 1, format, value);
    buffer[bufferLen-1] = '\0'; 
  }

  static void arTimeWrapper(char *buffer, ArTypes::UByte2 bufferLen,
        ArRetFunctor<ArTime> *functor, const char *format)
  {
    ArTime t = functor->invokeR();
    snprintf(buffer, bufferLen-1, format, t.getSecLL(), t.getMSecLL());
    buffer[bufferLen-1] = '\0'; 
  }

  static void floatWrapper(char * buffer, ArTypes::UByte2 bufferLen, 
			    ArRetFunctor<float> *functor, const char *format)
    { snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; }
};

/** A class to hold a list of callbacks to call
    GenericFunctor must be a pointer to an ArFunctor or subclass.
    @see ArCallbackList

    e.g. declare like this:
    @code
      ArGenericCallbackList< ArFunctorC<MyClass> * > callbackList;
    @endcode
    then invoke it like this:
    @code
      callbackList.invoke();
    @endcode
    To pass an argument to the callbacks, use ArCallbackList1 instead.
    @ingroup UtilityClasses
**/

template<class GenericFunctor> 
class ArGenericCallbackList
{
public:
  /// Constructor
  ArGenericCallbackList(const char *name = "", 
				 ArLog::LogLevel logLevel = ArLog::Verbose,
				 bool singleShot = false)
    {
      myName = name;
      mySingleShot = singleShot;
      setLogLevel(logLevel);
      std::string mutexName;
      mutexName = "ArGenericCallbackList::";
      mutexName += name;
      mutexName += "::myDataMutex";
      myDataMutex.setLogName(mutexName.c_str());
      myLogging = true;
    }
  /// Destructor
  virtual ~ArGenericCallbackList()
    {
    }
  /// Adds a callback
  void addCallback(GenericFunctor functor, int position = 50)
    {
      myDataMutex.lock();
      myList.insert(
	      std::pair<int, GenericFunctor>(-position, 
					     functor));
      myDataMutex.unlock();
    }
  /// Removes a callback
  void remCallback(GenericFunctor functor)
    {
      myDataMutex.lock();
      typename std::multimap<int, GenericFunctor>::iterator it;
      
      for (it = myList.begin(); it != myList.end(); it++)
      {
	if ((*it).second == functor)
	{
	  myList.erase(it);
	  myDataMutex.unlock();
	  remCallback(functor);
	  return;
	}
      }
      myDataMutex.unlock();
    }
  /// Sets the name
  void setName(const char *name)
    {
      myDataMutex.lock();
      myName = name;
      myDataMutex.unlock();
    }
#ifndef SWIG
  /// Sets the name with formatting
  /** @swigomit use setName() */
  void setNameVar(const char *name, ...)
    {
      char arg[2048];
      va_list ptr;
      va_start(ptr, name);
      vsnprintf(arg, sizeof(arg), name, ptr);
      arg[sizeof(arg) - 1] = '\0';
      va_end(ptr);
      return setName(arg);
    }
#endif
  /// Sets the log level
  void setLogLevel(ArLog::LogLevel logLevel)
    {
      myDataMutex.lock();
      myLogLevel = logLevel;
      myDataMutex.unlock();
    }
  /// Sets if its single shot
  void setSingleShot(bool singleShot)
    {
      myDataMutex.lock();
      mySingleShot = singleShot;
      myDataMutex.unlock();
    }
  /// Enable or disable logging when invoking the list. Logging is enabled by default at the log level given in the constructor.
  void setLogging(bool on) {
    myLogging = on;
  }
protected:
  ArMutex myDataMutex;
  ArLog::LogLevel myLogLevel;
  std::string myName;
  std::multimap<int, GenericFunctor> myList;
  bool mySingleShot;
  bool myLogging;
};

/** Stores a list of ArFunctor objects together.
    invoke() will invoke each of the ArFunctor objects.
    The functors added to the list must be pointers to the same ArFunctor subclass.
    Use ArCallbackList for ArFunctor objects with no arguments.
    Use ArCallbackList1 for ArFunctor1 objects with 1 argument.
    Use ArCallbackList2 for ArFunctor2 objects with 2 arguments.
    Use ArCallbackList3 for ArFunctor3 objects with 3 arguments.
    Use ArCallbackListp for ArFunctor4 objects with 4 arguments.

    Example: Declare a callback list for callbacks accepting one argument like this:
    @code
      ArCallbackList1<int> callbackList;
    @endcode
    then add a functor like this:
    @code
      ArFunctor1C<MyClass, int> func;
      ...
      callbackList.addCallback(&func);
    @endcode
    then invoke it like this:
    @code
      callbackList.invoke(23);
    @endcode

    A "singleShot" flag may be set, in which case the ArFunctor objects are removed
    from the list after being invoked.
    This list is threadsafe: the list contains a mutex (ArMutex), which is locked when 
    modifying the list and when invoking the ArFunctor objects.  Set singleShot
    in the constructor or call setSingleShot().

    Call setLogging() to enable more detailed logging including names of
    ArFunctor objects being invoked (if the ArFunctor objects have been given
    names.) Call setLogLevel() to select ArLog log level.

    @ingroup UtilityClasses
**/
class ArCallbackList : public ArGenericCallbackList<ArFunctor *>
{
public:
  /// Constructor
  ArCallbackList(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    ArGenericCallbackList<ArFunctor *>(name, logLevel, singleShot)
    {
    }
  /// Destructor
  virtual ~ArCallbackList()
    {
    }
  /// Calls the callback list
  void invoke(void)
    {
      myDataMutex.lock();
      
      std::multimap<int, ArFunctor *>::iterator it;
      ArFunctor *functor;
      
      if(myLogging)
        ArLog::log( myLogLevel,  "%s: Starting calls to %d functors", myName.c_str(), myList.size());
      
      for (it = myList.begin(); 
	   it != myList.end(); 
	   it++)
      {
	functor = (*it).second;
	if (functor == NULL) 
	  continue;
	
	if(myLogging)
	{
	  if (functor->getName() != NULL && functor->getName()[0] != '\0')
	    ArLog::log(myLogLevel, "%s: Calling functor '%s' at %d",
		       myName.c_str(), functor->getName(), -(*it).first);
	  else
	    ArLog::log(myLogLevel, "%s: Calling unnamed functor at %d", 
		       myName.c_str(), -(*it).first);
	}
	functor->invoke();
      }
      
      if(myLogging)
	ArLog::log(myLogLevel, "%s: Ended calls", myName.c_str());
      
      if (mySingleShot)
      {
	if(myLogging)
	  ArLog::log(myLogLevel, "%s: Clearing callbacks", myName.c_str());
	myList.clear();
      }
      myDataMutex.unlock();
    }
protected:
};

/** @copydoc ArCallbackList */
template<class P1>
class ArCallbackList1 : public ArGenericCallbackList<ArFunctor1<P1> *>
{
public:
  /// Constructor
  ArCallbackList1(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    ArGenericCallbackList<ArFunctor1<P1> *>(name, logLevel, singleShot)
    {
    }
  /// Destructor
  virtual ~ArCallbackList1()
    {
    }
  /// Calls the callback list
  void invoke(P1 p1)
    {
      ArGenericCallbackList<ArFunctor1<P1> *>::myDataMutex.lock();
      
      typename std::multimap<int, ArFunctor1<P1> *>::iterator it;
      ArFunctor1<P1> *functor;
      
      if(ArGenericCallbackList<ArFunctor1<P1> *>::myLogging)
	ArLog::log(
		ArGenericCallbackList<ArFunctor1<P1> *>::myLogLevel, 
		"%s: Starting calls to %d functors", 
		ArGenericCallbackList<ArFunctor1<P1> *>::myName.c_str(),
                ArGenericCallbackList<ArFunctor1<P1> *>::myList.size());
      
      for (it = ArGenericCallbackList<ArFunctor1<P1> *>::myList.begin(); 
	   it != ArGenericCallbackList<ArFunctor1<P1> *>::myList.end(); 
	   it++)
      {
	functor = (*it).second;
	if (functor == NULL) 
	  continue;
	
	if(ArGenericCallbackList<ArFunctor1<P1> *>::myLogging)
	{
	  if (functor->getName() != NULL && functor->getName()[0] != '\0')
	    ArLog::log(ArGenericCallbackList<ArFunctor1<P1> *>::myLogLevel,
		       "%s: Calling functor '%s' at %d",
		       ArGenericCallbackList<ArFunctor1<P1> *>::myName.c_str(), 
		       functor->getName(), -(*it).first);
	  else
	    ArLog::log(ArGenericCallbackList<ArFunctor1<P1> *>::myLogLevel, 
		       "%s: Calling unnamed functor at %d", 
		       ArGenericCallbackList<ArFunctor1<P1> *>::myName.c_str(), 
		       -(*it).first);
	}
	functor->invoke(p1);
      }
      
      if(ArGenericCallbackList<ArFunctor1<P1> *>::myLogging)
	ArLog::log(ArGenericCallbackList<ArFunctor1<P1> *>::myLogLevel, "%s: Ended calls", ArGenericCallbackList<ArFunctor1<P1> *>::myName.c_str());
      
      if (ArGenericCallbackList<ArFunctor1<P1> *>::mySingleShot)
      {
	if(ArGenericCallbackList<ArFunctor1<P1> *>::myLogging)
	  ArLog::log(ArGenericCallbackList<ArFunctor1<P1> *>::myLogLevel, 
		     "%s: Clearing callbacks", 
		     ArGenericCallbackList<ArFunctor1<P1> *>::myName.c_str());
	ArGenericCallbackList<ArFunctor1<P1> *>::myList.clear();
      }
      ArGenericCallbackList<ArFunctor1<P1> *>::myDataMutex.unlock();
    }
protected:
};

/** @copydoc ArCallbackList */
template<class P1, class P2>
class ArCallbackList2 : public ArGenericCallbackList<ArFunctor2<P1, P2> *>
{
public:
  typedef ArFunctor2<P1, P2> FunctorType;
  typedef ArGenericCallbackList<FunctorType*> Super;

  ArCallbackList2(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    Super(name, logLevel, singleShot)
  {
  }

  void invoke(P1 p1, P2 p2)
  {
    Super::myDataMutex.lock();
      
    typename std::multimap<int, FunctorType*>::iterator it;
    FunctorType *functor;
      
    if(Super::myLogging)
      ArLog::log(Super::myLogLevel, "%s: Starting calls to %d functors", Super::myName.c_str(), Super::myList.size());
      
    for (it = Super::myList.begin(); it != Super::myList.end(); ++it)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
      
      if(Super::myLogging)
      {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          ArLog::log(Super::myLogLevel, "%s: Calling functor '%s' at %d", Super::myName.c_str(), functor->getName(), -(*it).first);
        else
          ArLog::log(Super::myLogLevel, "%s: Calling unnamed functor at %d", Super::myName.c_str(), -(*it).first);
      }
      functor->invoke(p1, p2);
    }
          
    if(Super::myLogging)
      ArLog::log(Super::myLogLevel, "%s: Ended calls", Super::myName.c_str());
          
    if (Super::mySingleShot)
    {
      if(Super::myLogging)
        ArLog::log(Super::myLogLevel, "%s: Clearing callbacks", Super::myName.c_str()); 
      Super::myList.clear();
    }
    Super::myDataMutex.unlock();
  }
};


/** @copydoc ArCallbackList */
template<class P1, class P2, class P3>
class ArCallbackList3 : public ArGenericCallbackList<ArFunctor3<P1, P2, P3> *>
{
public:
  typedef ArFunctor3<P1, P2, P3> FunctorType;
  typedef ArGenericCallbackList<FunctorType*> Super;

  ArCallbackList3(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    Super(name, logLevel, singleShot)
  {
  }

  void invoke(P1 p1, P2 p2, P3 p3)
  {
    Super::myDataMutex.lock();
      
    typename std::multimap<int, FunctorType*>::iterator it;
    FunctorType *functor;
      
    if(Super::myLogging)
      ArLog::log(Super::myLogLevel, "%s: Starting calls to %d functors", Super::myName.c_str(), Super::myList.size());
      
    for (it = Super::myList.begin(); it != Super::myList.end(); ++it)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
      
      if(Super::myLogging)
      {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          ArLog::log(Super::myLogLevel, "%s: Calling functor '%s' at %d", Super::myName.c_str(), functor->getName(), -(*it).first);
        else
          ArLog::log(Super::myLogLevel, "%s: Calling unnamed functor at %d", Super::myName.c_str(), -(*it).first);
      }
      functor->invoke(p1, p2, p3);
    }
          
    if(Super::myLogging)
      ArLog::log(Super::myLogLevel, "%s: Ended calls", Super::myName.c_str());
          
    if (Super::mySingleShot)
    {
      if(Super::myLogging)
        ArLog::log(Super::myLogLevel, "%s: Clearing callbacks", Super::myName.c_str()); 
      Super::myList.clear();
    }
    Super::myDataMutex.unlock();
  }
};


/** @copydoc ArCallbackList */
template<class P1, class P2, class P3, class P4>
class ArCallbackList4 : public ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4>*>
{
public:
  ArCallbackList4(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>(name, logLevel, singleShot)
    {
    }
  virtual ~ArCallbackList4()
    {
    }
  void invoke(P1 p1, P2 p2, P3 p3, P4 p4)
    {
      // references to members of parent class for clarity below
      ArMutex &mutex = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myDataMutex;
      ArLog::LogLevel &loglevel = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myLogLevel;
      const char *name = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myName.c_str();
	    std::multimap< int, ArFunctor4<P1, P2, P3, P4>* > &list = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myList; 
      bool &singleshot = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::mySingleShot;
      bool &logging = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myLogging;
      
      mutex.lock();
      
      typename std::multimap<int, ArFunctor4<P1, P2, P3, P4> *>::iterator it;
      ArFunctor4<P1, P2, P3, P4> *functor;
      
      if(logging)
        ArLog::log( loglevel,  "%s: Starting calls to %d functors", name, list.size());
      
      for (it = list.begin();  it != list.end(); ++it)
      {
        functor = (*it).second;
        if (functor == NULL) 
          continue;
	
        if(logging)
        {
          if (functor->getName() != NULL && functor->getName()[0] != '\0')
            ArLog::log(loglevel, "%s: Calling functor '%s' (0x%x) at %d", name, functor->getName(), functor, -(*it).first);
          else
            ArLog::log(loglevel, "%s: Calling unnamed functor (0x%x) at %d", name, functor, -(*it).first);
        }
        functor->invoke(p1, p2, p3, p4);
      }
      
      if(logging)
        ArLog::log(loglevel, "%s: Ended calls", name);
      
      if (singleshot)
      {
        if(logging)
          ArLog::log(loglevel, "%s: Clearing callbacks", name);
        list.clear();
      }

      mutex.unlock();
    }
};


/// A simple thread that just calls a list of callbacks at an interval
class ArThreadedCallbackList : public ArASyncTask
{
public:
  /// Constructor
  AREXPORT ArThreadedCallbackList(int mSecsBetweenCallbacks, 
				    const char *name);
  /// Destructor
  AREXPORT ~ArThreadedCallbackList();

  /// Adds a functor that will be called 
  AREXPORT void addCallback(ArFunctor *functor, int position = 50)
    { myCBList.addCallback(functor, position); }

  /// Adds a functor that will be called 
  AREXPORT void remCallback(ArFunctor *functor)
    { myCBList.remCallback(functor); }
  
  // thread 
  AREXPORT virtual void *runThread(void *arg);  
protected:
  int myMSecsBetweenCallbacks;
  ArCallbackList myCBList;
};

#ifndef SWIG
/// @internal
class ArLaserCreatorHelper
{
public:
  /// Creates an ArLMS2xx
  static ArLaser *createLMS2xx(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArLMS2xx
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateLMS2xxCB(void);
  /// Creates an ArUrg
  static ArLaser *createUrg(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArUrg
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateUrgCB(void);
  /// Creates an ArLMS1XX
  static ArLaser *createLMS1XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArLMS1XX
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateLMS1XXCB(void);

  /// Creates an ArUrg using SCIP 2.0
  static ArLaser *createUrg_2_0(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArUrg
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateUrg_2_0CB(void);
  /// Creates an ArS3Series
  static ArLaser *createS3Series(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArS3Series
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateS3SeriesCB(void);
  /// Creates an ArLMS5XX
  static ArLaser *createLMS5XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArLMS5XX
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateLMS5XXCB(void);
  /// Creates an ArTiM3XX
  static ArLaser *createTiM3XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArTiM3XX
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateTiM3XXCB(void);
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateTiM551CB(void);
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateTiM561CB(void);
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateTiM571CB(void);
  /// Creates an ArSZSeries
  static ArLaser *createSZSeries(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an ArSZSeries
  static ArRetFunctor2<ArLaser *, int, const char *> *getCreateSZSeriesCB(void);

protected:
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourLMS2xxCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourUrgCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourLMS1XXCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourUrg_2_0CB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourS3SeriesCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourLMS5XXCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourTiM3XXCB;
  static ArGlobalRetFunctor2<ArLaser *, int, const char *> ourSZSeriesCB;
};

/// @internal
class ArBatteryMTXCreatorHelper
{
public:
  /// Creates an ArBatteryMTX
  static ArBatteryMTX *createBatteryMTX(int batteryNumber, const char *logPrefix);
  /// Gets functor for creating an ArBatteryMTX
  static ArRetFunctor2<ArBatteryMTX *, int, const char *> *getCreateBatteryMTXCB(void);

protected:
  static ArGlobalRetFunctor2<ArBatteryMTX *, int, const char *> ourBatteryMTXCB;
};

/// @internal
class ArLCDMTXCreatorHelper
{
public:
  /// Creates an ArLCDMTX
  static ArLCDMTX *createLCDMTX(int lcdNumber, const char *logPrefix);
  /// Gets functor for creating an ArLCDMTX
  static ArRetFunctor2<ArLCDMTX *, int, const char *> *getCreateLCDMTXCB(void);

protected:
  static ArGlobalRetFunctor2<ArLCDMTX *, int, const char *> ourLCDMTXCB;
};

/// @internal
class ArSonarMTXCreatorHelper
{
public:
  /// Creates an ArSonarMTX
  static ArSonarMTX *createSonarMTX(int sonarNumber, const char *logPrefix);
  /// Gets functor for creating an ArSonarMTX
  static ArRetFunctor2<ArSonarMTX *, int, const char *> *getCreateSonarMTXCB(void);

protected:
  static ArGlobalRetFunctor2<ArSonarMTX *, int, const char *> ourSonarMTXCB;
};

#endif // SWIG

#ifndef SWIG
/// @internal
class ArDeviceConnectionCreatorHelper
{
public:
  /// Creates an ArSerialConnection
  static ArDeviceConnection *createSerialConnection(
	  const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an ArSerialConnection
  static ArRetFunctor3<ArDeviceConnection *, const char *, const char *, 
		       const char *> *getCreateSerialCB(void);

  /// Creates an ArTcpConnection
  static ArDeviceConnection *createTcpConnection(
	  const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an ArTcpConnection
  static ArRetFunctor3<ArDeviceConnection *, const char *, const char *, 
		       const char *> *getCreateTcpCB(void);

  /// Creates an ArSerialConnection for RS422
  static ArDeviceConnection *createSerial422Connection(
	  const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an ArSerialConnection
  static ArRetFunctor3<ArDeviceConnection *, const char *, const char *,
		       const char *> *getCreateSerial422CB(void);

  /// Sets the success log level
  static void setSuccessLogLevel(ArLog::LogLevel successLogLevel);
  /// Sets the success log level
  static ArLog::LogLevel setSuccessLogLevel(void);
protected:
  /// Internal Create ArSerialConnection
  static ArDeviceConnection *internalCreateSerialConnection(
	  const char *port, const char *defaultInfo, const char *logPrefix, bool is422);
  static ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *, 
			     const char *> ourSerialCB;
  static ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *, 
			     const char *> ourTcpCB;
  static ArGlobalRetFunctor3<ArDeviceConnection *, const char *, const char *,
			     const char *> ourSerial422CB;
  static ArLog::LogLevel ourSuccessLogLevel;
};
#endif // SWIG

/// Class for finding robot bounds from the basic measurements
class ArPoseUtil
{
public:
  AREXPORT static std::list<ArPose> findCornersFromRobotBounds(
	  double radius, double widthLeft, double widthRight, 
	  double lengthFront, double lengthRear, bool fastButUnsafe);
  AREXPORT static std::list<ArPose> breakUpDistanceEvenly(ArPose start, ArPose end, 
						 int resolution);
};

/// class for checking if something took too long and logging it
class ArTimeChecker
{
public:
  /// Constructor
  AREXPORT ArTimeChecker(const char *name = "Unknown", int defaultMSecs = 100);
  /// Destructor
  AREXPORT virtual ~ArTimeChecker();
  /// Sets the name
  void setName(const char *name) { myName = name; }
  /// Sets the default mSecs
  void setDefaultMSecs(int defaultMSecs) { myMSecs = defaultMSecs; }
  /// starts the check
  AREXPORT void start(void);
  /// checks, optionally with a subname (only one subname logged per cycle)
  AREXPORT void check(const char *subName);
  /// Finishes the check
  AREXPORT void finish(void);
  /// Gets the last time a check happened (a start counts as a check too)
  ArTime getLastCheckTime() { return myLastCheck; }
protected:
  std::string myName;
  int myMSecs;
  ArTime myStarted;
  ArTime myLastCheck;
  bool myLogEverything;
};


/// ArRetFunctor subclass which returns a formatted std::string using value obtained by calling an accessor (via supplied ArRetFunctor)
/// The value is formatted using sprintf, using the format string provided. The resulting string must be less than 1024 bytes.
template <typename T>
class ArStringFormatRetFunctor : public virtual ArRetFunctor<std::string>
{
  ArRetFunctor<T> *myAccessor;
  const char *myFormat;
  char myBuf[1024];
public:
  ArStringFormatRetFunctor() {}
  ArStringFormatRetFunctor(ArRetFunctor<T> *func, const char *format) :
    myAccessor(func),
    myFormat(format)
  {}
  virtual std::string invokeR() {
    snprintf(myBuf, 1024, myFormat, myAccessor->invokeR());
    return myBuf;
  }
  virtual void invoke() {
    myAccessor->invoke();
  }
  void setAccessor(ArRetFunctor<T> *f) { myAccessor = f; }
};

/// Functor which, when invoked, formats a string representation of a value
/// obtained by calling a target accessor (via supplied ArRetFunctor), and stores
/// it in a buffer.
/// The value is formatted using sprintf, using the format string provided. 
/// The type used when specifying the buffer size may be specified as a template
/// parameter, or the default size_t used.
template <typename T, typename SizeType = size_t>
class ArStringFormatBufFunctor : public virtual ArFunctor2<char*, SizeType>
{
  ArRetFunctor<T> *myAccessor;
  const char *myFormat;
public:
  ArStringFormatBufFunctor() {}
  ArStringFormatBufFunctor(ArRetFunctor<T> *func, const char *format) :
    myAccessor(func),
    myFormat(format)
  {}
  virtual void invoke(char *buf, SizeType maxlen) {
    snprintf(buf, maxlen, myFormat, myAccessor->invokeR());
  }
  virtual void invoke() {
    // can't do anything
  }
  virtual void invoke(char *buf) {
    // can't do anything
  }
};

/** Specialization of ArStringFormatBufFunctor for ArPose. 
    The format string should include three floating point values: X, Y and Theta.
    (e.g. "(%.2f,%.2f,%.2f)")
    @see ArStringFormatBufFunctor
    @see ArPose
 */
template <typename SizeType>
class ArStringFormatBufFunctor <ArPose, SizeType> : public virtual ArFunctor2<char *, SizeType>
{
  ArRetFunctor<ArPose> *myAccessor;
  const char *myFormat;
public:
  ArStringFormatBufFunctor() {}
  ArStringFormatBufFunctor(ArRetFunctor<ArPose> *func, const char *format) :
    myAccessor(func),
    myFormat(format)
  {}
  virtual void invoke(char *buf, SizeType maxlen) {
    ArPose p = myAccessor->invokeR();
    snprintf(buf, maxlen, myFormat, p.getX(), p.getY(), p.getTh());
  }
  virtual void invoke() {
    // can't do anything
  }
  virtual void invoke(char *buf) {
    // can't do anything
  }
};

/** Specialization of ArStringFormatBufFunctor for ArTime. 
    The format string should match two unsigned long value, seconds and additional milliseconds.
    (e.g. "%lu.%lu")
    @see ArStringFormatBufFunctor
    @see ArTime
 */
template <typename SizeType>
class ArStringFormatBufFunctor <ArTime, SizeType> : public virtual ArFunctor2<char *, SizeType>
{
  ArRetFunctor<ArTime> *myAccessor;
  const char *myFormat;
public:
  ArStringFormatBufFunctor() {}
  ArStringFormatBufFunctor(ArRetFunctor<ArTime> *func, const char *format) :
    myAccessor(func),
    myFormat(format)
  {}
  virtual void invoke(char *buf, SizeType maxlen) {
    ArTime t = myAccessor->invokeR();
    snprintf(buf, maxlen, myFormat, t.getSec(), t.getMSec());
  }
  virtual void invoke() {
    // can't do anything
  }
  virtual void invoke(char *buf) {
    // can't do anything
  }
};
  

#endif // ARIAUTIL_H


