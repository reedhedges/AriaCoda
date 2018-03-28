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
#ifndef ARPACKETUTIL_H
#define ARPACKETUTIL_H


#include <vector>
#include <list>
#include <stdexcept>

#include "ariaUtil.h"
#include "ArBasePacket.h"

/** 
  Contains utility functions for adding and reading fields to and from packets (e.g. ArBasePacket, ArNetPacket, etc.) 
  @since ARIA 2.9.3 
*/
class ArPacketUtil
{
public:
    
  /// Choose and pack field into a packet  according to the type specified for T
  AREXPORT template <typename T> static void addField(ArBasePacket &p, const T& value);

  /// Version of addField() that adds length, then each item in a std::vector
  template <typename VT> static void addVectorFields(ArBasePacket &p, const std::vector<VT>& vec)
  {
    p.uByte4ToBuf(vec.size());
    for(typename std::vector<VT>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
      addField<VT>(p, *i);
    }
    // todo deal with vector too long for one packet
  }

  /// Version of addField() that adds length, then each item in a std::list
  template <typename LT> static void addListFields(ArBasePacket &p, const std::list<LT>& l)
  {
    p.uByte4ToBuf(l.size());
    for(typename std::list<LT>::const_iterator i = l.begin(); i != l.end(); ++i)
    {
      addField<LT>(p, *i);
    }
    // todo deal with list too long for one packet
  }

  /// Unpack a field from a packet according to the type specified for T
  AREXPORT template <typename T> static T getNextField(ArBasePacket& p) throw(std::out_of_range);

  /// @copydoc getNextField()
  AREXPORT template <typename VT> static std::vector<VT> getNextVectorField(ArBasePacket &p);

  /// @copydoc getNextField()
  AREXPORT template <typename LT> static std::list<LT> getNextListField(ArBasePacket &p);

  //AREXPORT template <typename T> static void getNextField(std::string *ptr);
  //AREXPORT template <typename T> static void getNextField(T *ptr);

  /// @copydoc getNextField()
  AREXPORT template <typename VT> static void getNextField(ArBasePacket &p, std::vector<VT> *ptr);

  /// @copydoc getNextField()
  AREXPORT template <typename LT> static void getNextField(ArBasePacket& p, std::list<LT> *ptr);

};


#endif // ARPACKETUTIL_H
