#include "Aria/ArBasePacket.h"
#include <cstdio>
#include <cstdint>

static int fails = 0;

static void fail(const char *msg)
{
  std::fprintf(stderr, "FAIL: %s\n", msg);
  ++fails;
}

int main()
{
  // --- isNextGood: length < footer must not underflow / must reject ---
  {
    ArBasePacket pkt(/*bufSize*/ 64, /*header*/ 4, nullptr, /*footer*/ 10);
    pkt.setLength(5); // 5 < footer 10
    pkt.setReadLength(4);
    if (pkt.bufferContainsBytes(1))
      fail("isNextGood/bufferContainsBytes true when myLength < myFooterLength");
  }

  // --- getDataReadLength: read still in header must not wrap ---
  {
    ArBasePacket pkt(64, 4, nullptr, 0);
    pkt.setLength(4);
    pkt.setReadLength(2); // < headerLength
    const uint16_t drl = pkt.getDataReadLength();
    if (drl != 0)
      fail("getDataReadLength should be 0 when readLength < headerLength");
  }

  // --- bufToByte4 / bufToUByte4: MSB set (would UB with signed << 24) ---
  {
    ArBasePacket pkt(64, 0, nullptr, 0);
    pkt.uByte4ToBuf(0x80000000u);
    pkt.resetRead();
    const uint32_t u = pkt.bufToUByte4();
    if (u != 0x80000000u)
      fail("bufToUByte4 mis-decoded value with high bit set");

    pkt.empty();
    pkt.byte4ToBuf(static_cast<int32_t>(0x80000000u));
    pkt.resetRead();
    const int32_t s = pkt.bufToByte4();
    if (s != static_cast<int32_t>(0x80000000u))
      fail("bufToByte4 mis-decoded value with high bit set");
  }

  // Round-trip edge values
  {
    ArBasePacket pkt(64, 0, nullptr, 0);
    const uint32_t vals[] = { 0u, 1u, 0x7fffffffu, 0xffffffffu };
    for (uint32_t v : vals) {
      pkt.empty();
      pkt.uByte4ToBuf(v);
      pkt.resetRead();
      if (pkt.bufToUByte4() != v) {
        std::fprintf(stderr, "FAIL: bufToUByte4 round-trip 0x%08x\n", v);
        ++fails;
      }
    }
  }

  if (fails) {
    std::fprintf(stderr, "%d test(s) failed\n", fails);
    return 1;
  }
  std::puts("basePacketTests: ok");
  return 0;
}
