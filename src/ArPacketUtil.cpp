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

#include "ArPacketUtil.h"

AREXPORT template<> void ArPacketUtil::addField<int>(ArBasePacket& p, const int& value)
{
  p.byte4ToBuf((ArTypes::Byte4) value);
}


AREXPORT template<> void ArPacketUtil::addField<unsigned int>(ArBasePacket& p, const unsigned int& value)
{
  p.uByte4ToBuf((ArTypes::UByte4) value);
}

AREXPORT template<> void ArPacketUtil::addField<short>(ArBasePacket& p, const short& value)
{
  p.byte2ToBuf((ArTypes::Byte2) value);
}

AREXPORT template<> void ArPacketUtil::addField<unsigned short>(ArBasePacket& p, const unsigned short& value)
{
  p.uByte2ToBuf((ArTypes::UByte2) value);
}

AREXPORT template<> void ArPacketUtil::addField<std::string>(ArBasePacket& p, const std::string& value)
{
  p.strToBuf(value.c_str());
}

AREXPORT template<> void ArPacketUtil::addField<float>(ArBasePacket& p, const float& value)
{
  p.byte4ToBuf(value * 10e4);
}

AREXPORT template<> void ArPacketUtil::addField<double>(ArBasePacket& p, const double& value)
{
  p.byte4ToBuf(value * 10e4);
}

AREXPORT template<> void ArPacketUtil::addField<bool>(ArBasePacket& p, const bool& value)
{
  p.uByteToBuf(value?1:0);
}

AREXPORT template<> void ArPacketUtil::addField<char>(ArBasePacket& p, const char& value)
{
  p.byteToBuf((ArTypes::Byte)value);
}

AREXPORT template<> void ArPacketUtil::addField<unsigned char>(ArBasePacket& p, const unsigned char& value)
{
  p.uByteToBuf((ArTypes::UByte)value);
}

/// X, Y and Theta are truncated to integer values only
AREXPORT template<> void ArPacketUtil::addField<ArPose>(ArBasePacket& pkt, const ArPose& pose)
{
  pkt.byte4ToBuf((ArTypes::Byte4)pose.getX());
  pkt.byte4ToBuf((ArTypes::Byte4)pose.getY());
  pkt.byte4ToBuf((ArTypes::Byte4)pose.getTh());
}

/// X, Y are truncated to integer values 
AREXPORT template<> void ArPacketUtil::addField<ArLineSegment>(ArBasePacket& p, const ArLineSegment& l)
{
  p.byte4ToBuf((ArTypes::Byte4)l.getX1());
  p.byte4ToBuf((ArTypes::Byte4)l.getY1());
  p.byte4ToBuf((ArTypes::Byte4)l.getX2());
  p.byte4ToBuf((ArTypes::Byte4)l.getY2());
}

/// X, Y are truncated to integer values
AREXPORT template<> void ArPacketUtil::addField<ArPos2D>(ArBasePacket& pkt, const ArPos2D& pos)
{
  pkt.byte4ToBuf((ArTypes::Byte4)pos.getX());
  pkt.byte4ToBuf((ArTypes::Byte4)pos.getY());
}

/** Append @a value to the packet data according to its type (specified with T template
 *  parameter).
 * 
 * T (and type of value) may be one of:
 * <ul>
 *   <li>int - value is added as a 32-bit signed value (see ArPacketUtil::byte4ToBuf())
 *   <li>unsigned int - value is added as a 32-bit unsigned value (see
 *   ArPacketUtil::uByte4ToBuf())
 *   <li>short - value is added as a 16-bit signed value using ArPacketUtil::byte2ToBuf()
 *   <li>unsigned short - value is added as a 16-bit unsigned value using
 *   ArPacketUtil::uByte2ToBuf()
 *   <li>std::string - value is added as a null-terminated string using
 *   ArPacketUtil::strToBuf()
 *   <li>float - value is multiplied by 10^4 and added using ArPacketUtil::byte4ToBuf().
 *   Therefore, note that only 5 places of decimal precision is preserved, and the
 *   maximum value is xxx. Use your own conversion/packing approach if you might
 *   need more precision or a higher maximum value.
 *   <li>double - value is multiplied by 10^4 and added using ArPacketUtil::byte4ToBuf().
 *   Therefore, note that only 5 places of precision is preserved, and the
 *   maximum value is xxx. Use your own conversion/packing approach if you might
 *   need more precision or a higher maximum value.
 *   <li>bool - an unsigned char is added with the value 1 for true, 0 for
 *   false.
 *   <li>char - added with ArPacketUtil::byteToBuf()
 *   <li>unsigned char - added with ArPacketUtil::uByteToBuf().
 *   <li>std::vector of any of the above - the length of the vector is added as
 *   a 32-bit unsigned value (with ArPacketUtil::uByte4ToBuf()) followed by each element
 *   (according to type of vector). Note the maximum length of the vector is 
 *   therefore UINT32_MAX or the maximum that can fit in the type of packet you are
 *   constructing. 
 *   <li>std::list of any of the above - the length of the vector is added as
 *   a 32-bit unsigned value (with ArPacketUtil::uByte4ToBuf()) followed by each element
 *   (according to type of vector). Note that the maximum length of the list is
 *   therefore UINT32_MAX or the maximum that can fit in the type of packet you are
 *   constructing. 
 *   <li>An ArPose, ArLineSegment or ArPos2D object - the coordinates are stored
 *   using ArPacketUtil::byte4ToBuf(): note they are truncated to integers 
 * </ul>
 *
 * @todo also specialize for int32_t, uint32_t, int16_t, uint16_t, etc.
 */

AREXPORT template<typename T> void ArPacketUtil::addField(ArBasePacket& p, const T& value)
{
  // this is the base template type, called for T not specialized above.
  // cause an error with abort so programmer
  // can debug with stack trace or in debugger:
  // (alternative could be just to memcpy from value into packet buffer)
  ArLog::log(ArLog::Terse, "INTERNAL ERROR: ArPacketUtil::addField<T> called with unhandled type for T! Type must be one of: int, unsigned int, short, unsigned short, std::string, float, double, bool, char, unsigned char, or vector or list of any of the above.");
  abort();
}


#define PACKET_OUT_OF_RANGE_ERR "No more data fields of this type in the packet."

AREXPORT template<> int ArPacketUtil::getNextField<int>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (int) p.bufToByte4();
}


AREXPORT template<> unsigned int ArPacketUtil::getNextField<unsigned int>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::UByte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (unsigned int) p.bufToUByte4();
}

AREXPORT template<> short ArPacketUtil::getNextField<short>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::Byte2))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (short) p.bufToByte2();
}

AREXPORT template<> unsigned short ArPacketUtil::getNextField<unsigned short>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::UByte2))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (unsigned short) p.bufToUByte2();
}

AREXPORT template<> std::string ArPacketUtil::getNextField<std::string>(ArBasePacket& p) throw (std::out_of_range)
{
  return p.bufToString();
}

/*
AREXPORT template<> void ArPacketUtil::getNextField<std::string>(ArBasePacket& p, std::string *ptr)
{
  p.bufToString(ptr);
}
*/

AREXPORT template<> float ArPacketUtil::getNextField<float>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (float)p.bufToByte4() / 10e4;
}

AREXPORT template<> double ArPacketUtil::getNextField<double>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return (double)p.bufToByte4() / 10e4;
}

AREXPORT template<> bool ArPacketUtil::getNextField<bool>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::UByte))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return(p.bufToUByte() != 0);
}

AREXPORT template<> char ArPacketUtil::getNextField<char>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::Byte))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return p.bufToByte();
}

AREXPORT template<> unsigned char ArPacketUtil::getNextField<unsigned char>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(sizeof(ArTypes::UByte))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return p.bufToUByte();
}

/// X, Y and Theta are truncated to integer values only. If only two 32-bit
/// integer values are
/// in the packet, return an ArPose with X and Y set but 0 for Theta.
AREXPORT template<> ArPose ArPacketUtil::getNextField<ArPose>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(2 * sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return ArPose(p.bufToByte4(), p.bufToByte4(), p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0);
/*
  return ArPose(
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0
  );
*/
}

/// X, Y are truncated to integer values 
AREXPORT template<> ArLineSegment ArPacketUtil::getNextField<ArLineSegment>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(4 * sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return ArLineSegment(p.bufToByte4(), p.bufToByte4(), p.bufToByte4(), p.bufToByte4());
/*
  return ArLineSegment(
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0
  );
*/
}

/// X, Y are truncated to integer values
AREXPORT template<> ArPos2D ArPacketUtil::getNextField<ArPos2D>(ArBasePacket& p) throw(std::out_of_range)
{
  if(!p.bufferContainsBytes(2 * sizeof(ArTypes::Byte4))) throw std::out_of_range(PACKET_OUT_OF_RANGE_ERR);
  return ArPos2D(p.bufToByte4(), p.bufToByte4());
/*
  return ArPos2D(
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0, 
    p.bufferContainsBytes(sizeof(ArTypes::Byte4)) ? p.bufToByte4() : 0
  );
*/
}

AREXPORT template<typename VT> std::vector<VT> ArPacketUtil::getNextVectorField(ArBasePacket& p)
{
  std::vector<VT> vec;
  size_t len = p.bufToUByte4();
  vec.reserve(len);
  for(int i = 0; i < len; ++i)
  {
    vec.push_back(getNextField<VT>(p));
  }
  // todo deal with truncated packet, we need to specialize bufferContainsBytes for ecah
  // type. just return vec so far 
  return vec;
}

AREXPORT template<typename LT> std::list<LT> ArPacketUtil::getNextListField(ArBasePacket& p)
{
  std::list<LT> l;
  size_t len = p.bufToUByte4();
  l.reserve(len);
  for(int i = 0; i < len; ++i)
  {
    l.push_back(getNextField<LT>(p));
  }
  // todo deal with truncated packet, we need to specialize bufferContainsBytes for ecah
  // type. just return list so far
  return l;
}

AREXPORT template<typename VT> void ArPacketUtil::getNextField(ArBasePacket& p, std::vector<VT> *vec)
{
  // todo resize and reuse existing vector space
  vec->clear();
  size_t len = p.bufToUByte4();
  vec->reserve(len);
  for(int i = 0; i < len; ++i)
  {
    vec->push_back(getNextField<VT>(p));
  }
  // todo deal with truncated packet
}

AREXPORT template<typename LT> void ArPacketUtil::getNextField(ArBasePacket& p, std::list<LT> *l)
{
  // todo resize and reuse existing list space??
  l->clear();
  size_t len = p.bufToUByte4();
  l->reserve(len);
  for(int i = 0; i < len; ++i)
  {
    l->push_back(getNextField<LT>(p));
  }
  // todo deal with Truncated packet
}


/** Unpack a value to the packet according to type (specified with T template
 *  parameter).
 * 
 * T (and type of value) may be one of:
 * <ul>
 *   <li>int - value is assumed to be a 32-bit signed value (see ArPacketUtil::bufToByte4())
 *   <li>unsigned int - value is a 32-bit unsigned value (see ArPacketUtil::bufToUByte4())
 *   <li>short - value a 16-bit signed value using ArPacketUtil::bufToByte2()
 *   <li>unsigned short - value is a 16-bit unsigned value using ArPacketUtil::bufToByte4()
 *   <li>std::string - value is a null-terminated string using ArPacketUtil::bufToString()
 *   <li>float - value is divided by 10^4 after extracted using ArPacketUtil::bufToByte4().
 *   Therefore, note that only 5 places of decimal precision is preserved, and the
 *   maximum value is xxx. Use your own conversion/unpacking approach if you might
 *   need more precision or a higher maximum value.
 *   <li>double - value is divided by 10^4 after extracted using ArPacketUtil::bufToByte4().
 *   Therefore, note that only 5 places of precision is preserved, and the
 *   maximum value is xxx. Use your own conversion/unpacking approach if you might
 *   need more precision or a higher maximum value.
 *   <li>bool - an unsigned char is extracted with the value 1 indicating true, 0 for
 *   false.
 *   <li>char - char extracted with ArPacketUtil::bufToByte()
 *   <li>unsigned char - extracted with ArPacketUtil::bufToUByte().
 *   <li>std::vector of any of the above - the length of the vector is unpacked as
 *   a 32-bit unsigned value (with ArPacketUtil::bufToUByte4()) followed by each element
 *   (according to type of vector). Note the maximum length of the vector is 
 *   therefore UINT32_MAX or the maximum that can fit in the type of packet you are
 *   constructing. 
 *   <li>std::list of any of the above - the length of the vector is unpacked as
 *   a 32-bit unsigned value (with ArPacketUtil::bufToUByte4()) followed by each element
 *   (according to type of vector). Note that the maximum length of the list is
 *   therefore UINT32_MAX or the maximum that can fit in the type of packet you are
 *   constructing. 
 *   <li>An ArPose, ArLineSegment or ArPos2D object - each coordinate is
 *   extracted using ArPacketUtil::bufToByte4(): note they are truncated to integers 
 * </ul>
 *
 * @todo specialize for int32_t, uint32_t, int16_t, uint16_t, etc.
 */
AREXPORT template<typename T> T ArPacketUtil::getNextField(ArBasePacket& p) throw (std::out_of_range)
{
  // this is the base template type, called for T not specialized above.
  // cause an error with abort so programmer
  // can debug with stack trace or in debugger:
  // (alternative would just be to memcpy from the packet to return value)
  ArLog::log(ArLog::Terse, "INTERNAL ERROR: ArPacketUtil::getNextField<T>(ArBasePacket& p) called with unhandled type for T! Type must be one of: int, unsigned int, short, unsigned short, std::string, float, double, bool, char, unsigned char, or vector or list of any of the above.");
  abort();
}
