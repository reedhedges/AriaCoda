
#include <cstdint>
#include <cassert>
#include <stddef.h>
#include <stdio.h>
#include <climits>

// get one byte from unsigned char buffer as int8_t. Can also be converted to signed char
// or unsigned char.
int8_t getInt8FromCharBuf(unsigned char *buf, size_t offset)
{
  return (int8_t)buf[offset];
}

// get one byte from unsigned char buf as uint8_t. Can also be converted to unsigned char8_t,
// unsigned char, or unsigned char.
uint8_t getUInt8FromCharBuf(unsigned char *buf, size_t offset)
{
  return (uint8_t)buf[offset];
}

// get two bytes from unsigned char buffer as LSB int16_t. Can also be converted to short.
int16_t getInt16FromCharBuf(unsigned char *buf, size_t offset)
{
  return (int16_t) ((signed char *)buf)[offset] << 8 | (int16_t)((signed char*)buf)[offset+1];
}

// get two bytes from unsigned char buffer as LSB uint16_t. Can also be converted to short.
uint16_t getUInt16FromCharBuf(unsigned char *buf, size_t offset)
{
  return (uint16_t)buf[offset] << 8 | (uint16_t)buf[offset+1];
}

// get four bytes from unsigned char buffer as LSB int32_t. Can also be converted to long, 
// or to int on 32-bit platforms.
int32_t getInt32FromCharBuf(unsigned char *buf, size_t offset)
{
  return (int32_t)((signed char*)buf)[offset]   << 24 | 
         (int32_t)((signed char*)buf)[offset+1] << 16 |
         (int32_t)((signed char*)buf)[offset+2] << 8  | 
         (int32_t)((signed char*)buf)[offset+3];
}

// get four bytes from unsigned char buffer as LSB uint32_t. Can also be converted to
// unsigned long, and to unsigned int on 32-bit platforms.
uint32_t getUInt32FromCharBuf(unsigned char *buf, size_t offset)
{
  return (uint32_t)buf[offset]   << 24 | 
         (uint32_t)buf[offset+1] << 16 |
         (uint32_t)buf[offset+2] << 8  | 
         (uint32_t)buf[offset+3];
}

// get 8 bytes from unsigned char buffer as LSB int64_t. Can also be converted to unsigned long long.
int64_t getInt64FromCharBuf(unsigned char *buf, size_t offset)
{
  return (int64_t)((signed char*)buf)[offset]   << 56 | 
         (int64_t)((signed char*)buf)[offset+1] << 48 |
         (int64_t)((signed char*)buf)[offset+2] << 40 | 
         (int64_t)((signed char*)buf)[offset+3] << 32 | 
         (int64_t)((signed char*)buf)[offset+4] << 24 | 
         (int64_t)((signed char*)buf)[offset+5] << 16 | 
         (int64_t)((signed char*)buf)[offset+6] << 8  | 
         (int64_t)((signed char*)buf)[offset+7];
}

// get 8 bytes from unsigned char buffer as LSB uint64_t. Can also be converted to long long.
uint64_t getUInt64FromCharBuf(unsigned char *buf, size_t offset)
{
  return (uint64_t)buf[offset]   << 56 | 
         (uint64_t)buf[offset+1] << 48 |
         (uint64_t)buf[offset+2] << 40 | 
         (uint64_t)buf[offset+3] << 32 | 
         (uint64_t)buf[offset+4] << 24 | 
         (uint64_t)buf[offset+5] << 16 | 
         (uint64_t)buf[offset+6] << 8  | 
         (uint64_t)buf[offset+7];
}

int main()
{

  printf("CHAR_BIT=%u sizeof(char)=%lu sizeof(int)=%lu sizeof(short)=%lu sizeof(long)=%lu sizeof(long long)=%lu CHAR_MAX=%d SCHAR_MAX=%d UCHAR_MAX=%u INT_MAX=%d UINT_MAX=%u\n",
CHAR_BIT, sizeof(char), sizeof(int), sizeof(short), sizeof(long), sizeof(long long), CHAR_MAX, SCHAR_MAX, UCHAR_MAX, INT_MAX, UINT_MAX);

  unsigned char buf[32];

  // TODO test with larger values (i.e. values that could be negative in
  // unsigned)

  buf[0] = -2; // signed char -2

  buf[1] = 200; // unsigned char 280

  buf[2] = 0; // signed int16_t 100 high bits
  buf[3] = 100; // signed int16_t 100 low bits

  buf[4] = 0; // signed int16_t -100 high bits
  buf[5] = -100; // signed int16_t -100 low bits

  buf[6] = 0; // unsigned uint16_t 23 high bits
  buf[7] = 23; // unsigned uint16_t 23 low bits

  buf[8] = 0; // signed int32_t -100 highest byte
  buf[9] = 0; // signed int32_t -100 next byte
  buf[10] = 0; // signed int32_t -100 next byte
  buf[11] = -100; // signed int32_t -100 low byte

  buf[12] = 0; // unsigned uint32_t 23 high byte
  buf[13] = 0; // unsigned uint32_t 23 next byte
  buf[14] = 0; // unsigned uint32_t 23 next byte
  buf[15] = 23; // unsigned uint32_t 23 low byte

  buf[16] = 0; // signed int64_t -100 highest byte
  buf[17] = 0; // signed int64_t -100 next byte
  buf[18] = 0; // signed int64_t -100 next byte
  buf[19] = 0; // signed int64_t -100 next byte
  buf[20] = 0; // signed int64_t -100 next byte
  buf[21] = 0; // signed int64_t -100 next byte
  buf[22] = 0; // signed int64_t -100 next byte
  buf[23] = -100; // signed int64_t -100 low byte

  buf[24] = 0; // unsigned uint64_t 23 high byte
  buf[25] = 0; // unsigned uint64_t 23 next byte
  buf[26] = 0; // unsigned uint64_t 23 next byte
  buf[27] = 0; // unsigned uint64_t 23 next byte
  buf[28] = 0; // unsigned uint64_t 23 next byte
  buf[29] = 0; // unsigned uint64_t 23 next byte
  buf[30] = 0; // unsigned uint64_t 23 next byte
  buf[31] = 23; // unsigned uint64_t 23 low byte

  int8_t i8 = getInt8FromCharBuf(buf, 0);
  printf("int8 got %d expected -2\n", i8);
  assert (i8 == -2);

  uint8_t u8 = getUInt8FromCharBuf(buf, 1); 
  printf("uint8 got %u expected 200\n", u8);
  assert (u8 == 200);

  uint16_t u16 = getUInt16FromCharBuf(buf, 6);
  printf("uint16 got %u expected 23\n", u16);
  assert(u16 == 23);

  int16_t i16a = getInt16FromCharBuf(buf, 2);
  printf("int16 got %d expected 100\n", i16a);
  assert(i16a == 100);

  int16_t i16 = getInt16FromCharBuf(buf, 4);
  printf("int16 got %d expected -100\n", i16);
  assert(i16 == -100);

  short s = getInt16FromCharBuf(buf, 4);
  printf("short got %d expected -100\n", s);
  assert(s == -100);

  unsigned short us = getUInt16FromCharBuf(buf, 6);
  printf("unsigned short got %u expected 23\n", us);
  assert(us == 23);

  int32_t i32 = getInt32FromCharBuf(buf, 8);
  printf("int32 got %d expected -100\n", i32);
  assert(i32 == -100);

  uint32_t u32 = getUInt32FromCharBuf(buf, 12);
  printf("uint32 got %u expected 23\n", u32);
  assert(u32 == 23);

  int64_t i64 = getInt64FromCharBuf(buf, 16); 
  printf("int64 got %ld expected -100\n", i64);
  assert (i64 == -100);

  uint64_t u64 = getUInt64FromCharBuf(buf, 24); 
  printf("uint64 got %lu expected 23\n", u64);
  assert (u64 == 23);

  return 0;
}

