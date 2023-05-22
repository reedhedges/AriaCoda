
#include <cstdint>
#include <cassert>
#include <stddef.h>
#include <stdio.h>
#include <climits>

#include "Aria/ariaUtil.h"
#include "Aria/ArSensorReading.h"

// just print out sizeof and *_MAX values. And some hypothetical smaller structs.

template<typename T>
struct Pose {
  T x, y, th;
  constexpr static size_t sizeof_contents = (sizeof(T) * 3);
};

template<typename PoseT, typename NumT>
struct SensorReading {
  PoseT r, lr, rt, ept, sp;
  float cos, sin;
  NumT dist, ang;
  unsigned int range, counter;
  int extra;
  ArTime tm;
  bool ignore, adj;
  constexpr static size_t sizeof_contents = ( (sizeof(PoseT) * 5) + (sizeof(float) * 2) + (sizeof(NumT) * 2) + (sizeof(unsigned int) * 2) + sizeof(int) + sizeof(ArTime) + (sizeof(bool) * 2) );
};

int main()
{

  printf("CHAR_BIT=%d\nsizeof(char)=%zu\nsizeof(int)=%zu\nsizeof(short)=%zu\nsizeof(long)=%zu\nsizeof(long long)=%zu\nsizeof(float)=%zu\nsizeof(double)=%zu\n"
         "CHAR_MAX=%d\nSCHAR_MAX=%d    \nUCHAR_MAX=%u   \nSHRT_MAX=%d      \nUSHRT_MAX=%u    \nINT_MAX=%d  \nUINT_MAX=%u  \nLONG_MAX=%ld \nULONG_MAX=%lu\n"
         "INT8_MAX=%d                  \nUINT8_MAX=%u    \nINT16_MAX=%d    \nUINT16_MAX=%u   \nINT32_MAX=%d\nUINT32_MAX=%u\nINT64_MAX=%ld\nUINT64_MAX=%lu\n"
         "FLT_MAX=%f\nDBL_MAX=%f\n\n",

          CHAR_BIT,   (size_t)sizeof(char), sizeof(int),  sizeof(short),    sizeof(long),      sizeof(long long),     sizeof(float),     sizeof(double),
          CHAR_MAX,   SCHAR_MAX,         UCHAR_MAX,       SHRT_MAX,         USHRT_MAX,         INT_MAX,      UINT_MAX,      LONG_MAX,     ULONG_MAX,
          INT8_MAX,                      UINT8_MAX,       INT16_MAX,        UINT16_MAX,        INT32_MAX,    UINT32_MAX,    INT64_MAX,    UINT64_MAX,
          FLT_MAX, DBL_MAX
  );

  printf("sizeof(ArPose)=%zu\nsizeof(ArPoseWithTime)=%zu\nsizeof(ArSensorReading)=%zu\n\n",
    sizeof(ArPose), sizeof(ArPoseWithTime), sizeof(ArSensorReading));



  printf("sizeof(Pose<float>)=%zu [contents=%zu]\n"
"sizeof(SensorReading<Pose<float>, float>)=%zu [contents=%zu]\n"
"sizeof(Pose<long>)=%zu [contents=%zu]\n"
"sizeof(SensorReading<Pose<long>, long>)=%zu [contents=%zu]\n"
"sizeof(Pose<int>)=%zu [contents=%zu]\n"
"sizeof(SensorReading<Pose<int>,int>)=%zu [contents=%zu]\n\n",
    sizeof(Pose<float>), Pose<float>::sizeof_contents, sizeof(SensorReading<Pose<float>, float>), SensorReading<Pose<float>, float>::sizeof_contents,
    sizeof(Pose<long>), Pose<long>::sizeof_contents, sizeof(SensorReading<Pose<long>, long>), SensorReading<Pose<long>, long>::sizeof_contents,
    sizeof(Pose<int>), Pose<int>::sizeof_contents, sizeof(SensorReading<Pose<int>, int>), SensorReading<Pose<int>, int>::sizeof_contents );
    


  return 0;
}
