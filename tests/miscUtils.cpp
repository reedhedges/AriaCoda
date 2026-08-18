

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"
#include <cassert>
#include <cstdio>

int main()
{
  puts("Misc tests of ArUtil and type definitions...");

  static_assert(sizeof(ArTypes::Byte) == 1);
  static_assert(sizeof(ArTypes::Byte2) == 2);
  static_assert(sizeof(ArTypes::Byte4) == 4);
  static_assert(sizeof(ArTypes::Byte8) == 8);
  static_assert(sizeof(ArTypes::UByte) == 1);
  static_assert(sizeof(ArTypes::UByte2) == 2);
  static_assert(sizeof(ArTypes::UByte4) == 4);
  static_assert(sizeof(ArTypes::UByte8) == 8);

  static_assert(sizeof(ArTypes::Byte) == sizeof(std::int8_t));
  static_assert(sizeof(ArTypes::Byte2) == sizeof(std::int16_t));
  static_assert(sizeof(ArTypes::Byte4) == sizeof(std::int32_t));
  static_assert(sizeof(ArTypes::Byte8) == sizeof(std::int64_t));
  static_assert(sizeof(ArTypes::UByte) == sizeof(std::uint8_t));
  static_assert(sizeof(ArTypes::UByte2) == sizeof(std::uint16_t));
  static_assert(sizeof(ArTypes::UByte4) == sizeof(std::uint32_t));
  static_assert(sizeof(ArTypes::UByte8) == sizeof(std::uint64_t));

  // TODO check the actual types too, (signedness etc)


  uint16_t bits = 1;
  assert(bits == ArUtil::BIT0);
  assert(bits != ArUtil::BIT1);
  bits = bits << 1; assert(bits == ArUtil::BIT1);
  bits = bits << 1; assert(bits == ArUtil::BIT2);
  bits = bits << 1; assert(bits == ArUtil::BIT3);
  bits = bits << 1; assert(bits == ArUtil::BIT4);
  bits = bits << 1; assert(bits == ArUtil::BIT5);
  bits = bits << 1; assert(bits == ArUtil::BIT6);
  bits = bits << 1; assert(bits == ArUtil::BIT7);
  bits = bits << 1; assert(bits == ArUtil::BIT8);
  bits = bits << 1; assert(bits == ArUtil::BIT9);
  bits = bits << 1; assert(bits == ArUtil::BIT10);
  bits = bits << 1; assert(bits == ArUtil::BIT11);
  bits = bits << 1; assert(bits == ArUtil::BIT12);
  bits = bits << 1; assert(bits == ArUtil::BIT13);
  bits = bits << 1; assert(bits == ArUtil::BIT14);
  bits = bits << 1; assert(bits == ArUtil::BIT15);

  

  puts("OK.");
  return 0;
}
