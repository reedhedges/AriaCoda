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
#ifndef ARARGUMENTBUILDER_H
#define ARARGUMENTBUILDER_H

#include "ariaTypedefs.h"

/// This class is to build arguments for things that require argc and argv
/// @ingroup ImportantClasses
class ArArgumentBuilder
{
public:
  /// Constructor
  AREXPORT ArArgumentBuilder(size_t argvLen = 512, 
                             char extraSpaceChar = '\0',
			                       bool ignoreNormalSpaces = false,
                             bool isPreCompressQuotes = false);
  /// Copy Constructor
  AREXPORT ArArgumentBuilder(const ArArgumentBuilder &builder);

  AREXPORT ArArgumentBuilder &operator=(const ArArgumentBuilder &builder);

  /// Destructor
  AREXPORT virtual ~ArArgumentBuilder();
#ifndef SWIG
  /** @brief Adds the given string, with varargs, separates if there are spaces
   *  @swignote Not available
   */
  AREXPORT void add(const char *str, ...);
#endif
  /// Adds the given string, without varargs (wrapper for java)
  AREXPORT void addPlain(const char *str, int position = -1);
  /// Adds the given string, without varargs and without touching the str
  AREXPORT void addPlainAsIs(const char *str, int position = -1);
  /// Adds the given string thats divided
  AREXPORT void addStrings(char **argv, int argc, int position = -1);
  /// Adds the given string thats divided
  AREXPORT void addStrings(int argc, char **argv, int position = -1);
  /// Adds the given string thats divided (but doesn't touch the strings)
  AREXPORT void addStringsAsIs(int argc, char **argv, int position = -1);
  /// Gets the original string of the input
  AREXPORT const char *getFullString(void) const;
  /// Sets the full string (this is so you can have a more raw full string)
  AREXPORT void setFullString(const char *str);
  /// Gets the extra string of the input, used differently by different things
  AREXPORT const char *getExtraString(void) const;
  /// Sets the extra string of the input, used differently by different things
  AREXPORT void setExtraString(const char *str);
  /// Prints out the arguments
  AREXPORT void log(void) const;
  /// Gets the argc
  AREXPORT size_t getArgc(void) const;
  /// Gets the argv
  AREXPORT char** getArgv(void) const;
  /// Gets a specific argument as a string
  AREXPORT const char* getArg(size_t whichArg) const;

  /// Sees if an argument is a bool
  AREXPORT bool isArgBool(size_t whichArg) const;

  /// Gets the value of an argument as a boolean 
  /**
   * Valid boolean values are "true" and "false", and "1" and "0".
   * There are two ways to to verify that the specified argument is a bool.
   * Either call isArgBool() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient since the 
   * argument string is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return bool the retrieved argument value; valid only if ok or isArgBool
   * is true
  **/
  AREXPORT bool getArgBool(size_t whichArg,
                           bool *ok = NULL) const;

  /// Sees if an argument is an int
  /**
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param forceHex if true this makes it find the int in base 16 instead of 10
   */
  AREXPORT bool isArgInt(size_t whichArg, bool forceHex = false) const;

  /// Gets the value of an argument as an integer
  /**
   * There are two ways to to verify that the specified argument is an integer.
   * Either call isArgInt() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @param forceHex if true this makes it find the int in base 16 instead of 10
   * @return int the retrieved argument value; valid only if ok or isArgInt 
   * is true
  **/
  AREXPORT int getArgInt(size_t whichArg,
                         bool *ok = NULL, bool forceHex = false) const;

  /// Sees if an argument is a long long int
  AREXPORT bool isArgLongLongInt(size_t whichArg) const;

  /// Gets the value of an argument as a long long integer
  /**
   * There are two ways to to verify that the specified argument is an integer.
   * Either call isArgInt() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return int the retrieved argument value; valid only if ok or isArgInt 
   * is true
  **/
  AREXPORT int getArgLongLongInt(size_t whichArg,
				 bool *ok = NULL) const;

  /// Sees if an argument is a double
  AREXPORT bool isArgDouble(size_t whichArg) const;

  /// Gets the value of an argument as a double
  /**
   * There are two ways to to verify that the specified argument is a double.
   * Either call isArgDouble() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return double the retrieved argument value; valid only if ok or 
   * isArgDouble is true
  **/
  AREXPORT double getArgDouble(size_t whichArg,
                               bool *ok = NULL) const;

  /// Delete a particular arg, you MUST finish adding before you can remove
  AREXPORT void removeArg(size_t which,
									        bool isRebuildFullString = false);
  /// Combines quoted arguments into one
  AREXPORT void compressQuoted(bool stripQuotationMarks = false);

  /// Turn on this flag to reduce the number of verbose log messages.
  AREXPORT void setQuiet(bool isQuiet);

protected:
  AREXPORT void internalAdd(const char *str, int position = -1);
  AREXPORT void internalAddAsIs(const char *str, int position = -1);
	AREXPORT void rebuildFullString();

  /// Characters that may be used to separate arguments; bitwise flags so QUOTE can be combined with spaces
  enum ArgSeparatorType {
    SPACE              = 1,               // Normal space character
    SPECIAL            = SPACE << 1,      // The special "extra" space character, if any
    ANY_SPACE          = SPACE | SPECIAL, // Either normal space or special extra space
    QUOTE              = SPECIAL << 1,    // Double-quote, must be used in combination with spaces 
 };

  /// Determines whether the current buffer position marks the start of an argument
  /**
   * This method should only be called when an argument is not currently being 
   * parsed (i.e. the previous character was a space).  
   * @param buf the char * buffer that is being parsed; must be non-NULL
   * @param len the maximum number of characters in the buffer
   * @param index the int buffer position of the character to be tested
   * @param endArgFlagsOut a pointer to an output int that will indicate which separators
   * will mark the end of the argument.  If quotes are being pre-processed, and the
   * current argument starts with space-quote, then the argument must end with a quote-space.
  **/
  bool isStartArg(const char *buf, 
                  int len, 
                  int index,
                  int *endArgFlagsOut);

  /// Determines whether the current buffer position marks the end of an argument
  /**
   * This method should only be called when an argument is currently being 
   * parsed (i.e. isStartArg returned true).  
   * @param buf the char * buffer that is being parsed; must be non-NULL
   * @param len the maximum number of characters in the buffer
   * @param indexInOut the input/output int buffer position of the character to be tested; 
   * if the argument ends with a quote-space, then the index will be incremented to mark
   * the space; otherwise, it will remain unchanged
   * @param endArgFlags an int that indicates which separators mark the end of the 
   * argument.  If quotes are being pre-processed, and the current argument started
   * with space-quote, then the argument must end with a quote-space.
  **/
  bool isEndArg(const char *buf, 
                int len, 
                int &indexInOut,
                int endArgFlags);

  /// Determines whether the specified character is an acceptable space (either normal or extra)
  bool isSpace(char c);

  size_t getArgvLen(void) const { return myArgvLen; }
  // how many arguments we had originally (so we can delete 'em)
  size_t myOrigArgc;
  // how many arguments we have
  size_t myArgc;
  // argument list
  char **myArgv;
  // argv length
  size_t myArgvLen;
  // the extra string (utility thing)
  std::string myExtraString;
  // the full string
  std::string myFullString;
  // whether this is our first add or not
  bool myFirstAdd;
  // a character to alternately treat as a space
  char myExtraSpace;
  // if we should ignore normal spaces
  bool myIgnoreNormalSpaces;
  /// Whether to treat double-quotes as arg delimiters (preserving spaces within)
  bool myIsPreCompressQuotes;

  bool myIsQuiet;
};

// ----------------------------------------------------------------------------

/// Comparator that returns true if arg1's full string is less than arg2's.
struct ArArgumentBuilderCompareOp
{
public:

  /// Compares arg1's full string to arg2's.
  AREXPORT bool operator() (ArArgumentBuilder* arg1, ArArgumentBuilder* arg2) const;

}; // end struct ArArgumentBuilderCompareOp



#endif // ARARGUMENTBUILDER_H
