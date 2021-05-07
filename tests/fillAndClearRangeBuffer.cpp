
#include "Aria/ariaInternal.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArRangeBuffer.h"

// use this to test ArRangeBuffer data management performance.
// this simulates a range sensor that provides lots of data 
// which will be stored in a buffer of 1000 items (which is a bit larger than normal, but should still result in the buffer filling
// at items needing to be re-used).  After every 5000 items are added, every other item in the buffer is invalidated.

// Before changing ArRangeBuffer to store list of ArPoseWithTime objects with
// simple list pointer changes to add/reuse/reserve (via splice())  it stored a
// list of pointers to allocated ArPoseWithTime objects, which were
// allocated/deallocated to add/delete and pointers were copied to reuse and
// reserve items.  I timed this test with the old method to be about 4 seconds, 
// with a range buffer size of 1000, storing 200,000 random points 
// (x in range (0,30000), y in range (0,30000)) omitting near distance of 10,000mm (so it shuold have to search the whole list every time), and clearing out 
// the buffer every 5000 points added.
// I timed this with the new method about 900 ms.

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
  }
  //rangebuffer.logData(ArLog::Verbose, "", "none", "test");
  printf("\nTime=%ld ms\n", t.mSecSince());
}
