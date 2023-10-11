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


*/
#ifndef ARTYPEDEFS_H
#define ARTYPEDEFS_H


#define ARIACODA 3
#define ARIA_3 3

#include <time.h>
#include <string>
#include <map>
#include <list>
#include <cstdint>

#ifdef _WIN32


#ifndef SWIG
#if !defined(ARIA_STATIC) && !defined(AREXPORT) && !defined(MINGW)
#define AREXPORT _declspec(dllimport)
#elif !defined(AREXPORT) // ARIA_STATIC
#define AREXPORT
#endif // ARIA_STATIC
#else
#define AREXPORT
#endif

#define NOMINMAX // Windows defines min and max functions which for some reason end up preventing std::min or std::max from working.  
#include <winsock2.h>
#include <windows.h>

#endif //_WIN32



#ifdef ARIABUILD
// Building ARIA itself
#define PUBLICDEPRECATED(m)
#else
// Not building ARIA itself, external use of ARIA
#define PUBLICDEPRECATED(m) [[deprecated(m)]]
#endif

#ifndef _WIN32

////
//// Linux
////
#define AREXPORT

#endif // linux



// These are C++17 attributes that we use anyway. They weren't added until G++ version 7, so use these alternatives:
#if defined(__GNUC__) && (__GNUC__ < 7)

#define NODISCARD

#if __has_attribute(__unused__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED  
#endif

#if __has_attribute(__fallthrough__)
#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH  
#endif

#else

// On G++ version 7 and later, on on all other platforms including Windows, use standard C++ attributes:

#define NODISCARD [[nodiscard]]
#define UNUSED [[maybe_unused]]
#define FALLTHROUGH [[fallthrough]]

#endif



typedef std::map<int, std::string> ArStrMap;

/// has enum for position in list
class ArListPos
{
public:
  typedef enum {
      FIRST = 1, ///< place item first in the list
      LAST = 2 ///< place item last in the list
  } Pos;
};

/// Contains platform independent sized variable types
/// @deprecated Use standard fixed size integer types instead: int8_t or signed char, uint8_t or unsigned char, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
class
//[[deprecated("Use standard fixed size integer types instead")]]  
ArTypes
{
public:
  /// A single signed byte
  [[deprecated("Use standard type std::int8_t instead")]] 
  typedef std::int8_t Byte;
  /// Two signed bytes
  [[deprecated("Use standard type std::int16_t instead")]] 
  typedef std::int16_t Byte2;
  /// Four signed bytes
  [[deprecated("Use standard type std::int32_t instead")]] 
  typedef std::int32_t Byte4;
  /// Eight signed bytes
  [[deprecated("Use standard type std::int64_t instead")]] 
  typedef std::int64_t Byte8;

  /// A single unsigned byte
  [[deprecated("Use standard type std::uint8_t instead")]] 
  typedef std::uint8_t UByte;
  /// Two unsigned bytes
  [[deprecated("Use standard type std::uint16_t instead")]] 
  typedef std::uint16_t UByte2;
  /// Four unsigned bytes
  [[deprecated("Use standard type std::uint32_t instead")]] 
  typedef std::uint32_t UByte4;
  /// Eight unsigned bytes
  [[deprecated("Use standard type std::uint64_t instead")]] 
  typedef std::uint64_t UByte8;

  /*
  /// A single signed byte
  typedef char Byte;
  /// Two signed bytes
  typedef short Byte2;
  /// Four signed bytes
  typedef int Byte4;
  /// Eight signed bytes
  typedef long long Byte8;

  /// A single unsigned byte
  typedef unsigned char UByte;
  /// Two unsigned bytes
  typedef unsigned short UByte2;
  /// Four unsigned bytes
  typedef unsigned int UByte4;
  /// Eight unsigned bytes
  typedef unsigned long long UByte8;
  */
};


#endif
