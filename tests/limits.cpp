
#include <cstdint>
#include <cassert>
#include <stddef.h>
#include <stdio.h>
#include <climits>


// just print out sizeof and *_MAX values.

int main()
{

  printf("CHAR_BIT=%d\nsizeof(char)=%zu\nsizeof(int)=%zu\nsizeof(short)=%zu\nsizeof(long)=%zu\nsizeof(long long)=%zu\n"
         "CHAR_MAX=%d\nSCHAR_MAX=%d    \nUCHAR_MAX=%u   \nSHRT_MAX=%d      \nUSHRT_MAX=%u    \nINT_MAX=%d  \nUINT_MAX=%u  \nLONG_MAX=%ld \nULONG_MAX=%lu\n"
         "INT8_MAX=%d                  \nUINT8_MAX=%u    \nINT16_MAX=%d    \nUINT16_MAX=%u   \nINT32_MAX=%d\nUINT32_MAX=%u\nINT64_MAX=%ld\nUINT64_MAX=%lu\n",

          CHAR_BIT,   (size_t)sizeof(char), sizeof(int),  sizeof(short),    sizeof(long),      sizeof(long long), 
          CHAR_MAX,   SCHAR_MAX,         UCHAR_MAX,       SHRT_MAX,         USHRT_MAX,         INT_MAX,      UINT_MAX,      LONG_MAX,     ULONG_MAX,
          INT8_MAX,                      UINT8_MAX,       INT16_MAX,        UINT16_MAX,        INT32_MAX,    UINT32_MAX,    INT64_MAX,    UINT64_MAX
  );
  return 0;
}
