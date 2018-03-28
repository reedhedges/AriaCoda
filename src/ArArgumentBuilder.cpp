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
#include "ariaOSDef.h"
#include "ArArgumentBuilder.h"
#include "ArLog.h"
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

char * cppstrdup(const char *str)
{
  char *ret;
  ret = new char[strlen(str) + 1];
  strcpy(ret, str);
  return ret;
}
/**
 * @param argvLen the largest number of arguments to parse
 * @param extraSpaceChar if not NULL, then this character will also be 
 * used to break up arguments (in addition to whitespace)
 * @param ignoreNormalSpaces a bool set to true if only the extraSpaceChar
 * should be used to separate arguments
 * @param isPreCompressQuotes a bool set to true if strings enclosed in 
 * double-quotes should be parsed as a single argument (such strings
 * must be surrounded by spaces).  This is roughly equivalent to calling 
 * ArArgumentBuilder::compressQuoted(false) on the resulting builder, but 
 * is more efficient and handles embedded spaces better.  The default value
 * is false and preserves the original behavior where each argument is a 
 * space-separated alphanumeric string.
**/
AREXPORT ArArgumentBuilder::ArArgumentBuilder(size_t argvLen, 
					                                    char extraSpaceChar,
					                                    bool ignoreNormalSpaces,
                                              bool isPreCompressQuotes)
{

  myArgc = 0;
  myOrigArgc = 0;
  myArgvLen = argvLen;
  myArgv = new char *[myArgvLen];
  myFirstAdd = true;
  myExtraSpace = extraSpaceChar;
  myIgnoreNormalSpaces = ignoreNormalSpaces;
  myIsPreCompressQuotes = isPreCompressQuotes;
  myIsQuiet = false;
}

AREXPORT ArArgumentBuilder::ArArgumentBuilder(const ArArgumentBuilder & builder)
{
  size_t i;
  myFullString = builder.myFullString;
  myExtraString = builder.myExtraString;
  myArgc = builder.getArgc();
  myArgvLen = builder.getArgvLen();
  myOrigArgc = myArgc;
  myArgv = new char *[myArgvLen];
  for (i = 0; i < myArgc; i++)
    myArgv[i] = cppstrdup(builder.getArg(i));
  //myArgv[i] = strdup(builder.getArg(i));
  myIsQuiet = builder.myIsQuiet;
  myExtraSpace = builder.myExtraSpace;
  myIgnoreNormalSpaces = builder.myIgnoreNormalSpaces;
  myIsPreCompressQuotes = builder.myIsPreCompressQuotes;
}

AREXPORT ArArgumentBuilder &ArArgumentBuilder::operator=(const ArArgumentBuilder & builder)
{
  if (this != &builder) {

    size_t i = 0;

    // Delete old stuff...  
    if (myOrigArgc > 0)
    {
      for (i = 0; i < myOrigArgc; ++i)
        delete[] myArgv[i];
    }
    delete[] myArgv;

    // Then copy new stuff...
    myFullString = builder.myFullString;
    myExtraString = builder.myExtraString;
    myArgc = builder.getArgc();
    myArgvLen = builder.getArgvLen();

    myOrigArgc = myArgc;
    myArgv = new char *[myArgvLen];
    for (i = 0; i < myArgc; i++) {
      myArgv[i] = cppstrdup(builder.getArg(i));
    }
    myIsQuiet = builder.myIsQuiet;
    myExtraSpace = builder.myExtraSpace;
    myIgnoreNormalSpaces = builder.myIgnoreNormalSpaces;
    myIsPreCompressQuotes = builder.myIsPreCompressQuotes;
  }
  return *this;
}

AREXPORT ArArgumentBuilder::~ArArgumentBuilder()
{
  size_t i;
  if (myOrigArgc > 0)
  {
    for (i = 0; i < myOrigArgc; ++i)
      delete[] myArgv[i];
  }
  delete[] myArgv;
}


AREXPORT void ArArgumentBuilder::removeArg(size_t which,
																					 bool isRebuildFullString)
{
  size_t i;
  char *temp;

	if (which < 0) {
		ArLog::log(ArLog::Terse, "ArArgumentBuilder::removeArg: cannot remove arg at negative index (%i)",
							 which);
		return;
	}
  if (which > myArgc - 1)
  {
    ArLog::log(ArLog::Terse, "ArArgumentBuilder::removeArg: %d is greater than the number of arguments which is %d", which, myArgc);
    return;
  }


  temp = myArgv[which];
  //delete[] myArgv[which]; 
  for (i = which; i < myArgc - 1; i++)
    myArgv[i] = myArgv[i+1];
  // delete the one off the end
  myArgc -= 1;
  // Just stuffing the deleted argument value at the end of the array (not sure why)
	myArgv[i] = temp;

	// Note: It seems that compressQuoted calls removeArg and depends on it not 
	// changing the full string.  Therefore, the parameter was added so that the
	// desired behavior could be specified by the caller.
  if (isRebuildFullString) {	
	  rebuildFullString();
	}

}

AREXPORT void ArArgumentBuilder::add(const char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, sizeof(buf), str, ptr);
  internalAdd(buf, -1);
  va_end(ptr);
}


bool ArArgumentBuilder::isSpace(char c)
{
  if ((!myIgnoreNormalSpaces) && (isspace(c))) {
    return true;
  }
  else if ((myExtraSpace != '\0') && (c == myExtraSpace)) {
    return true;
  }
  return false;

} // end method isSpace


bool ArArgumentBuilder::isStartArg(const char *buf, 
                                   int len, 
                                   int index,
                                   int *endArgFlagsOut)
{
  if (index < len) {

    if (!isSpace(buf[index])) {

      if ((myIsPreCompressQuotes) && (buf[index] == '\"')) {
        if (endArgFlagsOut != NULL) {
          *endArgFlagsOut = ANY_SPACE | QUOTE;
        }
        
      }
      else { // not precompressed quote
        if (endArgFlagsOut != NULL) {
          // This is set to ANY_SPACE to preserve the original behavior -- i.e. space
          // character "matches" with myExtraSpace (if set)
          *endArgFlagsOut = ANY_SPACE; 
        }
      } // end else not precompressed quote
    
      return true;

    } // end if not space
  } // end if not end of buf
 
  return false;
 
} // end method isStartArg


bool ArArgumentBuilder::isEndArg(const char *buf, 
                                 int len, 
                                 int &index,
                                 int endArgFlags)
{
  if (index >= len) {
    return true;
  }
  
  if ((endArgFlags & QUOTE) != 0) {

    if (buf[index] == '\"') {
      if ((index + 1 >= len) || (buf[index + 1] == '\0')) {
        // Quote is at EOL
        index++;
        return true;
      }
      else if (isSpace(buf[index + 1])) {

        // Space follows quote
        index++;
        return true;
      }
    } // end if quote found

  } // end if need to find quote
  else { // just looking for a space
    if (isSpace(buf[index])) {
      return true;
    }
  } // end else just looking for a space

  return false;

} // end method isEndArg


/**
   Internal function that adds a string starting at some given space
   
   @param str the string to add

   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
 **/
AREXPORT void ArArgumentBuilder::internalAdd(const char *str, int position)
{
  char buf[10000];
  int i = 0;
  int j = 0;
  size_t k = 0;
  int len = 0;
  bool addAtEnd = true;
  //size_t startingArgc = getArgc();

  bool isArgInProgress = false;
  int curArgStartIndex = -1;


  if (position < 0 || (size_t)position > myArgc)
    addAtEnd = true;
  else
    addAtEnd = false;

  strncpy(buf, str, sizeof(buf));
  len = strlen(buf);

  // can do whatever you want with the buf now
  // first we advance to non-space
  for (i = 0; i < len; ++i)
  {
    if (!isSpace(buf[i])) {
       //(!myIgnoreNormalSpaces && !isspace(buf[i])) || 
       // (myExtraSpace != '\0' && buf[i] != myExtraSpace))
      break;
    } // end if non-space found
  } // end for each char in buffer

  // see if we're done
  if (i == len)
  {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, "All white space add for argument builder.");
    }
    return;
  }

  int endArgFlags = ANY_SPACE;

  // walk through the line until we get to the end of the buffer...
  // we keep track of if we're looking for white space or non-white...
  // if we're looking for white space when we find it we have finished
  // one argument, so we toss that into argv, reset pointers and moveon
  for (curArgStartIndex = i; ; ++i)
  {

    // Remove the slash of escaped spaces.  This is primarily done to handle command 
    // line arguments (especially on Linux). If quotes are "pre-compressed", then
    // the backslash is preserved.  Consecutive backslashes are also preserved 
    // (i.e. "\\ " is not modified).
    if (!myIsPreCompressQuotes &&
        (buf[i] == '\\') && (i + 1 < len) && (buf[i + 1] == ' ') &&
        ((i == 0) || (buf[i - 1] != '\\')))
    {
      for (j = i; j < len && j != '\0'; j++)
      {
        buf[j] = buf[j + 1];
      }
      --len;
    } // end if escaped space
  
    // If we're not in the middle of an argument, then determine whether the 
    // current buffer position marks the start of one.
    else if ((!isArgInProgress) && 
             (i < len) &&
             (buf[i] != '\0') &&
             (isStartArg(buf, len, i, &endArgFlags))) 
    {
      curArgStartIndex = i;
      isArgInProgress = true;
    }
    // If we are in the middle of an argument, then determine whether the current
    // buffer position marks the end of it.  (Note that i may be incremented by
    // isEndArg when quotes are pre-compressed.)
    else if (isArgInProgress && 
             ((i == len) || 
              (buf[i] == '\0') ||
              (isEndArg(buf, len, i, endArgFlags)))) 

    {
      // see if we have room in our argvLen
      if (myArgc + 1 >= myArgvLen)
      {
        ArLog::log(ArLog::Terse, "ArArgumentBuilder::Add: could not add argument since argc (%u) has grown beyond the argv given in the constructor (%u)", myArgc, myArgvLen);
      }
      else // room in arg array
      {
        // if we're adding at the end just put it there, also put it
        // at the end if its too far out
        if (addAtEnd)
        {
          myArgv[myArgc] = new char[i - curArgStartIndex + 1];
          strncpy(myArgv[myArgc], &buf[curArgStartIndex], i - curArgStartIndex);
          myArgv[myArgc][i - curArgStartIndex] = '\0';
          // add to our full string
          // if its not our first add a space (or whatever our space char is)
          if (!myFirstAdd && myExtraSpace == '\0')
            myFullString += " ";
          else if (!myFirstAdd)
            myFullString += myExtraSpace;

          myFullString += myArgv[myArgc];
          myFirstAdd = false;

          myArgc++;
          myOrigArgc = myArgc;
        }
        // otherwise stick it where we wanted it if we can or just 
        else // insert arg at specified position
        {
          // first move things down
          for (k = myArgc + 1; k > (size_t)position; k--)
          {
            myArgv[k] = myArgv[k - 1];
          }
          myArgc++;
          myOrigArgc = myArgc;

          myArgv[position] = new char[i - curArgStartIndex + 1];
          strncpy(myArgv[position], &buf[curArgStartIndex], i - curArgStartIndex);
          myArgv[position][i - curArgStartIndex] = '\0';
          position++;

          rebuildFullString();
          myFirstAdd = false;

        } // end else insert arg at specified position

      } // end else room in arg array

      isArgInProgress = false;
      endArgFlags = ANY_SPACE;

    } // end else if found end of argument

    // if we're at the end or its a null, we're at the end of the buf
    if (i == len || buf[i] == '\0')
      break;

  } // end for each char in buffer (after non-whitespace found)

} // end method internalAdd


/**
   @param str the string to add

   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
**/
AREXPORT void ArArgumentBuilder::addPlain(const char *str, int position)
{
  internalAdd(str, position);
}

/**
   @param argc how many arguments to add
   @param argv the strings to add
   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
**/
AREXPORT void ArArgumentBuilder::addStrings(char **argv, int argc,
					    int position)
{
  addStrings(argc, argv, position);
}

/**
   @param argc how many arguments to add
   @param argv the strings to add
   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
**/
AREXPORT void ArArgumentBuilder::addStrings(int argc, char **argv, 
					    int position)
{
  int i;
  if(position < 0)
  {
    // Don't try to use incremental positions, since the sequence would be out
    // of order, just keep using the same special "at-end" meaning of negative position
    // value
    for (i = 0; i < argc; i++)
      internalAdd(argv[i], position);
  }
  else
  {
    for (i = 0; i < argc; i++)
      internalAdd(argv[i], position + i);
  }
}

/**
   @param argc how many arguments to add
   @param argv the strings to add
   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
**/
AREXPORT void ArArgumentBuilder::addStringsAsIs(int argc, char **argv, 
						int position)
{
  int i;
  if(position < 0)
  {
    // Don't try to use incremental positions, since the sequence would be out
    // of order, just keep using the same special "at-end" meaning of negative position
    // value
    for (i = 0; i < argc; i++)
      internalAddAsIs(argv[i], position);
  }
  else
  {
    for (i = 0; i < argc; i++)
      internalAddAsIs(argv[i], position + i);
  }
}

/**
   @param str the string to add

   @param position the position to add the string at, a position less
   than 0 means to add at the end, if this number is greater than how
   many positions exist then it will also be added at the end
**/
AREXPORT void ArArgumentBuilder::addPlainAsIs(const char *str, int position)
{
  internalAddAsIs(str, position);
}

AREXPORT void ArArgumentBuilder::internalAddAsIs(const char *str, int position)
{ 
  size_t k = 0;

  bool addAtEnd;
  if (position < 0 || (size_t)position > myArgc)
    addAtEnd = true;
  else
    addAtEnd = false;


  if (addAtEnd)
  {
    myArgv[myArgc] = new char[strlen(str) + 1];
    strcpy(myArgv[myArgc], str);
    myArgv[myArgc][strlen(str)] = '\0';
    
    // add to our full string
    // if its not our first add a space (or whatever our space char is)
    if (!myFirstAdd && myExtraSpace == '\0')
      myFullString += " ";
    else if (!myFirstAdd)
      myFullString += myExtraSpace;
    
    myFullString += myArgv[myArgc];
    myFirstAdd = false;

    myArgc++;
    myOrigArgc = myArgc;
  }
  else
  {
    // first move things down
    for (k = myArgc + 1; k > (size_t)position; k--)
    {
      myArgv[k] = myArgv[k - 1];
    }
    myArgc++;
    myOrigArgc = myArgc;
    
    myArgv[position] = new char[strlen(str) + 1];
    strcpy(myArgv[position], str);
    myArgv[position][strlen(str)] = '\0';
    
    rebuildFullString();
    myFirstAdd = false;
  }


}

AREXPORT size_t ArArgumentBuilder::getArgc(void) const
{
  return myArgc;
}

AREXPORT char** ArArgumentBuilder::getArgv(void) const
{
  return myArgv;
}

AREXPORT const char *ArArgumentBuilder::getFullString(void) const
{
  return myFullString.c_str();
}

AREXPORT const char *ArArgumentBuilder::getExtraString(void) const
{
  return myExtraString.c_str();
}

AREXPORT void ArArgumentBuilder::setExtraString(const char *str)
{
  myExtraString = str;
}

AREXPORT void ArArgumentBuilder::setFullString(const char *str)
{
  myFullString = str;
}

AREXPORT const char* ArArgumentBuilder::getArg(size_t whichArg) const
{
  if ((whichArg >= 0) && (whichArg < myArgc)) {
    return myArgv[whichArg];
  }
  else {
    return NULL;
  }
}

AREXPORT void ArArgumentBuilder::log(void) const
{
  size_t i;
  ArLog::log(ArLog::Terse, "Num arguments: %d", myArgc);
  for (i = 0; i < myArgc; ++i)
    ArLog::log(ArLog::Terse, "Arg %d: %s", i, myArgv[i]);
}

AREXPORT bool ArArgumentBuilder::isArgBool(size_t whichArg) const
{
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;

  if (strcasecmp(getArg(whichArg), "true") == 0 || 
      strcasecmp(getArg(whichArg), "1") == 0 || 
      strcasecmp(getArg(whichArg), "false") == 0 || 
      strcasecmp(getArg(whichArg), "0") == 0)
    return true;
  else
    return false;
}

AREXPORT bool ArArgumentBuilder::getArgBool(size_t whichArg, 
                                            bool *ok) const
{
  bool isSuccess = false;
  bool ret = false;
 
  const char *str = getArg(whichArg);

  // If the arg was successfully obtained...
  if (str != NULL) {

    if (strcasecmp(str, "true") == 0 || 
        strcasecmp(str, "1") == 0) {
      isSuccess = true;
      ret = true;
    }
    else if (strcasecmp(str, "false") == 0 || 
             strcasecmp(str, "0") == 0) {
      isSuccess = true;
      ret = false;
    }
  } // end if valid arg

  if (ok != NULL) {
    *ok = isSuccess;
  }
  if (isSuccess) {
    return ret;
  }
  else {
    return 0;
  }
} // end method getArgBool


AREXPORT bool ArArgumentBuilder::isArgInt(size_t whichArg, bool forceHex) const
{
  const char *str;
  char *endPtr;
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;

  int base = 10;
  str = getArg(whichArg);

  if (forceHex)
    base = 16;
  // see if it has the hex prefix and strip it
  if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
  {
    str = &str[2];
    base = 16;
  }

  strtol(str, &endPtr, base);
  if (endPtr[0] == '\0' && endPtr != str)
    return true;
  else
    return false;
}

AREXPORT int ArArgumentBuilder::getArgInt(size_t whichArg,
                                          bool *ok, bool forceHex) const
{
  bool isSuccess = false;
  int  ret = 0;
    
  const char *str = getArg(whichArg);

  // If the specified arg was successfully obtained...
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

} // end method getArgInt

AREXPORT bool ArArgumentBuilder::isArgLongLongInt(size_t whichArg) const
{
  const char *str;
  char *endPtr;
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;

  int base = 10;
  str = getArg(whichArg);
  // see if its a hex number
  if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
  {
    str = &str[2];
    base = 16;
  }

#ifndef _MSC_VER
    strtoll(str, &endPtr, base);
#else
    _strtoi64(str, &endPtr, base);
#endif

  if (endPtr[0] == '\0' && endPtr != str)
    return true;
  else
    return false;
}

AREXPORT int ArArgumentBuilder::getArgLongLongInt(size_t whichArg,
						  bool *ok) const
{
  bool isSuccess = false;
  long long ret = 0;
    
  const char *str = getArg(whichArg);

  // If the specified arg was successfully obtained...
  if (str != NULL) {
  
    int base = 10;
    // see if its a hex number
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
      str = &str[2];
      base = 16;
    }

    char *endPtr = NULL;
#ifndef _MSC_VER
    ret = strtoll(str, &endPtr, base);
#else
    ret = _strtoi64(str, &endPtr, base);
#endif
 
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

} // end method getArgInt

AREXPORT bool ArArgumentBuilder::isArgDouble(size_t whichArg) const
{
  const char *str;
  char *endPtr;
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;

  str = getArg(whichArg);
  if (strcmp(str, "-INF") == 0)
  {
	return true;
  }
  else if (strcmp(str, "INF") == 0)
  {
	return true;
  }
  else
  {
    strtod(str, &endPtr);
    if (endPtr[0] == '\0' && endPtr != str)
      return true;
    else
      return false;
  }

}

AREXPORT double ArArgumentBuilder::getArgDouble(size_t whichArg,
                                                bool *ok) const
{
  bool isSuccess = false;
  double ret = 0;
    
  const char *str = getArg(whichArg);

  // If the specified arg was successfully obtained...
  if (str != NULL) {

    if (strcmp(str, "-INF") == 0)
    {
      isSuccess = true;
      ret = -HUGE_VAL;
    }
    else if (strcmp(str, "INF") == 0)
    {
      isSuccess = true;
      ret = HUGE_VAL;
    }
    else { // check regular values

      char *endPtr = NULL;
      ret = strtod(str, &endPtr);
      if (endPtr[0] == '\0' && endPtr != str) {
        isSuccess = true;
      }
    } // end else check regular values
  } // end if valid arg
  
  if (ok != NULL) {
    *ok = isSuccess;
  }
  
  if (isSuccess) 
    return ret;
  else 
    return 0;

} // end method getArgDouble


AREXPORT void ArArgumentBuilder::compressQuoted(bool stripQuotationMarks)
{
  size_t argLen;
  size_t i;
  std::string myNewArg;

  for (i = 0; i < myArgc; i++)
  {
    argLen = strlen(myArgv[i]);
    if (stripQuotationMarks && argLen >= 2 && 
	myArgv[i][0] == '"' && myArgv[i][argLen - 1] == '"')
    {
      myNewArg = &myArgv[i][1];
      myNewArg[myNewArg.size() - 1] = '\0';
      delete[] myArgv[i];
      // but replacing ourself with the new arg
      myArgv[i] = cppstrdup(myNewArg.c_str());
      //myArgv[i] = strdup(myNewArg.c_str());
      continue;
    }
    // if this arg begins with a quote but doesn't end with one
    if (argLen >= 2 && myArgv[i][0] == '"' && myArgv[i][argLen - 1] != '"')
    {
      // start the new value for this arg, if stripping quotations
      // then start after the quote
      if (stripQuotationMarks)
	myNewArg = &myArgv[i][1];
      else
	myNewArg = myArgv[i];

      bool isEndQuoteFound = false;

      // now while the end char of the next args isn't the end of our
      // start quote we toss things into this arg
      while ((i + 1 < myArgc) && !isEndQuoteFound) {
          
        int nextArgLen = strlen(myArgv[i+1]);

        // Check whether the next arg contains the ending quote...
        if ((nextArgLen > 0) &&
	    (myArgv[i+1][nextArgLen - 1] == '"')) 
	{
	      isEndQuoteFound = true;
        }
    
        // Concatenate the next arg to this one...
        myNewArg += " ";
        myNewArg += myArgv[i+1];
	// if we are striping quotes off then replace the quote
	if (stripQuotationMarks && myNewArg.size() > 0 && isEndQuoteFound)
	  myNewArg[myNewArg.size() - 1] = '\0';
        // removing those next args
        removeArg(i+1);
        // and ourself
        delete[] myArgv[i];

        // but replacing ourself with the new arg
        myArgv[i] = cppstrdup(myNewArg.c_str());
	//myArgv[i] = strdup(myNewArg.c_str());
      }
    }
  }
}

AREXPORT void ArArgumentBuilder::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
}

AREXPORT void ArArgumentBuilder::rebuildFullString()
{
	myFullString = "";
	for (size_t k = 0; k < myArgc; k++)
	{
	  myFullString += myArgv[k];
    // Don't tack an extra space on at the end.
    if (k < myArgc - 1) {
	    myFullString += " ";
    }
	}

} // end method rebuildFullString

// ----------------------------------------------------------------------------

AREXPORT bool ArArgumentBuilderCompareOp::operator()(ArArgumentBuilder* arg1, 
                                            ArArgumentBuilder* arg2) const
{
  if (arg1 == NULL) {
    return true;
  }
  else if (arg2 == NULL) {
    return false;
  }
  std::string arg1String = arg1->getFullString();
  std::string arg2String = arg2->getFullString();

  return (arg1String.compare(arg2String) < 0);
  
}
