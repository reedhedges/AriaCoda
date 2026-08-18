#include "Aria/ariaUtil.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>

static int fails = 0;

static void fail(const char *msg)
{
  std::fprintf(stderr, "FAIL: %s\n", msg);
  ++fails;
}

int main()
{
  // --- getFileName: resultLen==1 must not underflow / must stay safe ---
  {
    char out[1];
    out[0] = 'X';
    // Path with separator exercises the basename branch (resultLen - 2 bug).
    const bool ok = ArUtil::getFileName("/tmp/foo", out, 1);
    if (!ok) {
      // Acceptable: reject tiny buffer
    } else if (out[0] != '\0') {
      fail("getFileName(resultLen=1) did not leave an empty/truncated C string");
    }
  }
  {
    char out[8];
    if (!ArUtil::getFileName("/tmp/foo", out, sizeof(out)))
      fail("getFileName failed on normal buffer");
    else if (std::strcmp(out, "foo") != 0) {
      std::fprintf(stderr, "FAIL: getFileName expected \"foo\", got \"%s\"\n", out);
      ++fails;
    }
  }
  {
    char out[8];
    if (!ArUtil::getFileName("bare", out, sizeof(out)))
      fail("getFileName failed on basename-only path");
    else if (std::strcmp(out, "bare") != 0)
      fail("getFileName basename-only mismatch");
  }

  // --- ArRootMeanSquareCalculator: large |val| must not corrupt via int overflow ---
  {
    ArRootMeanSquareCalculator rms;
    const int v = 100000; // 1e5^2 = 1e10, overflows 32-bit int product
    rms.add(v);
    const double got = rms.getRootMeanSquare();
    const double expect = static_cast<double>(v); // sqrt(v*v) == |v|
    if (std::fabs(got - expect) > 1e-6) {
      std::fprintf(stderr, "FAIL: RMS expected %g got %g\n", expect, got);
      ++fails;
    }
    if (rms.getCurrentNumAveraged() != 1)
      fail("RMS cleared or lost sample after large add");
  }
  {
    ArRootMeanSquareCalculator rms;
    rms.add(INT_MAX / 2);
    if (rms.getCurrentNumAveraged() != 1)
      fail("RMS lost sample after INT_MAX/2 add");
  }

  if (fails) {
    std::fprintf(stderr, "%d test(s) failed\n", fails);
    return 1;
  }
  std::puts("arutilTests: ok");
  return 0;
}
