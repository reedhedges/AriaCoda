
#include <cstdint>
#include <cassert>
#include <stddef.h>
#include <stdio.h>
#include <climits>


int main()
{

  printf("CHAR_BIT=%u\nsizeof(char)=%lu sizeof(int)=%lu sizeof(short)=%lu sizeof(long)=%lu sizeof(long long)=%lu\n"
      "CHAR_MAX=%d SCHAR_MAX=%d UCHAR_MAX=%u SHRT_MAX=%d USHRT_MAX=%u INT_MAX=%d UINT_MAX=%u LONG_MAX=%ld ULONG_MAX=%lu\n",
      "CHAR_MAX=%d SCHAR_MAX=%d UCHAR_MAX=%u SHRT_MAX=%d USHRT_MAX=%u INT_MAX=%d UINT_MAX=%u LONG_MAX=%ld ULONG_MAX=%lu\n",
      CHAR_BIT, 
      sizeof(char), sizeof(int), sizeof(short), sizeof(long), sizeof(long long), 
      CHAR_MAX, SCHAR_MAX, UCHAR_MAX, SHRT_MAX, USHRT_MAX, INT_MAX, UINT_MAX, LONG_MAX, ULONG_MAX);
      INT8_MAX, INT8_MAX, UINT8_MAX, INT16_MAX, UINT16_MAX, INT32_MAX, UINT32_MAX, INT64_MAX, UINT64_MAX
  return 0;
}
