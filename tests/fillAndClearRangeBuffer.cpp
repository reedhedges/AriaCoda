
#include "Aria/ariaInternal.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArRangeBuffer.h"

// Bit of a benchmark to test ArRangeBuffer data management performance.
// this simulates a range sensor that provides lots of data 
// which will be stored in a buffer of 1000 items (which is a bit larger than normal, but should still result in the buffer filling
// at items needing to be re-used).  After every 5000 items are added, every other item in the buffer is invalidated.

// Before changing ArRangeBuffer to store list of ArPoseWithTime objects with
// simple list pointer changes to add/reuse/reserve (via splice())  it stored a
// list of pointers to allocated ArPoseWithTime objects, which were
// allocated/deallocated to add/delete and pointers were copied to reuse and
// reserve items.  I timed this test with the old method to be about 4 seconds. 
// Test uses a range buffer size of 1000, storing 500,000 random points 
// (x in range (0,30000), y in range (0,30000)) omitting near distance of 10,000mm 
// (so it should have to search the whole list every time), and clearing out 
// the buffer every 5000 points added.  Every 10,000 iterations, we clear any items older than 1 ms.
// I timed this with the old method to be 2.6 seconds each test, with the new
// method from range_buffer_container_changes branch to be slightly faster at
// about 2.3 seconds each test.

#include <limits>

unsigned int test()
{
  ArRangeBuffer rangebuffer(1000);
  ArTime totalTime;
  ArTime clearTime;
  static_assert(500'000 <= std::numeric_limits<unsigned long>::max(), "");
  puts("Testing 500,000 readings in rangebuffer (with limit 100)...");
  for (unsigned long i = 0; i < 500'000; ++i)
  {
    rangebuffer.addReadingConditional( ArMath::randomInRange(0, 30000), ArMath::randomInRange(0, 30000), 100*100 );
    //printf("%6u  %10lu     \r", i, rangebuffer.getBuffer().size());

    // do periodic clearing of buffer
    if(i % 5000 == 0)
    {
      //rangebuffer.clear();
      //printf("%6u  clear     \r", i);
      //fflush(stdout);

      rangebuffer.beginInvalidationSweep();
      for (auto i = rangebuffer.getBegin(); i != rangebuffer.getEnd(); )
      {
        rangebuffer.invalidateReading(i);
        if( (++i) == rangebuffer.getEnd())
          break;
        ++i;
      }
      rangebuffer.endInvalidationSweep();
    }

    if(i % 10'000 == 0)
    {
      rangebuffer.clearOlderThan(1);
    }
  }
  //rangebuffer.logData(ArLog::Verbose, "", "none", "test");
  printf("Time=%ld ms\n", totalTime.mSecSince());
  return totalTime.mSecSince();
}

int main()
{
  unsigned int sum = 0;
  const int n = 10;
  for (int i = 0; i < n; ++i)
  {
    sum += test();
    ArUtil::sleep(500);
  }
  printf("\nAverage=%u ms\n", sum / n);
  return 0;
}
