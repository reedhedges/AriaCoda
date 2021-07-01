
#include <cstdint>
#include <cassert>
#include <stddef.h>
#include <stdio.h>
#include <climits>


int main()
{

  printf("CHAR_BIT=%u\nsizeof(char)=%lu sizeof(int)=%lu sizeof(short)=%lu sizeof(long)=%lu sizeof(long long)=%lu\nCHAR_MAX=%d SCHAR_MAX=%d UCHAR_MAX=%u INT_MAX=%d UINT_MAX=%u SHRT_MAX=%d USHRT_MAX=%u  LONG_MAX=%ld ULONG_MAX=%lu\n",
      CHAR_BIT, sizeof(char), sizeof(int), sizeof(short), sizeof(long),
      sizeof(long long), CHAR_MAX, SCHAR_MAX, UCHAR_MAX, INT_MAX, UINT_MAX, SHRT_MAX,
      USHRT_MAX, LONG_MAX, ULONG_MAX);
  return 0;
}
