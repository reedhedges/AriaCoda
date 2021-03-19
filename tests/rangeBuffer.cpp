
#include "Aria/ariaInternal.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArRangeBuffer.h"

// use this to test ArRangeBuffer data management performance.
// this simulates a range sensor that provides lots of data 
// which will be stored in an unusually large buffer.

// Before changing ArRangeBuffer to store ArPoseWithTime directly (it stored allocated ArPoseWithTime objects), 
// Reed tested this to be about 4 seconds, with a range buffer size of 1000, storing 200,000 random points 
// (x=(0,30000), y=(0,30000)) omitting near distance of 100mm, and clearing out the buffer every 5000 points added.

int main()
{
  ArRangeBuffer rangebuffer(1000);
  ArTime t;
  for (unsigned int i = 0; i < 200'000; ++i)
  {
    rangebuffer.addReadingConditional( ArMath::randomInRange(0, 30000), ArMath::randomInRange(0, 30000), 100*100 );
    //printf("%6u  %10lu     \r", i, rangebuffer.getBuffer().size());

    // do periodic clearing of buffer
    if(i % 5000 == 0)
    {
      rangebuffer.clear();
      //printf("%6u  clear     \r", i);
      //fflush(stdout);
    }
  }
  printf("\nTime=%ld ms\n", t.mSecSince());

}
