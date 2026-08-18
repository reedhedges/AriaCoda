#include "Aria/ArRobotPacket.h"
#include <cstdio>
#include <cstring>

static int fails = 0;

static void fail(const char *msg)
{
  std::fprintf(stderr, "FAIL: %s\n", msg);
  ++fails;
}

// Build a trivial valid-looking buffer: sync, length, id, no data, checksum slots.
// Robot packet: headerLen=4, footerLen=2.
static void fillHeader(ArRobotPacket &pkt, unsigned char lenByte, unsigned char id = 0)
{
  char *buf = pkt.getBuf();
  std::memset(buf, 0, pkt.getMaxLength());
  buf[0] = static_cast<char>(0xfa);
  buf[1] = static_cast<char>(0xfb);
  buf[2] = static_cast<char>(lenByte);
  buf[3] = static_cast<char>(id);
}

int main()
{
  // --- verifyCheckSum: length too short must return false (no OOB) ---
  {
    ArRobotPacket pkt;
    fillHeader(pkt, 0);
    pkt.setLength(0);
    if (pkt.verifyCheckSum())
      fail("verifyCheckSum true with myLength==0");
  }
  {
    ArRobotPacket pkt;
    fillHeader(pkt, 1);
    pkt.setLength(1);
    if (pkt.verifyCheckSum())
      fail("verifyCheckSum true with myLength==1");
  }
  {
    ArRobotPacket pkt;
    fillHeader(pkt, 2);
    pkt.setLength(2); // still < header+footer
    if (pkt.verifyCheckSum())
      fail("verifyCheckSum true with myLength==2 (too short for header+footer)");
  }

  // --- calcCheckSum: length byte 0 or 1 must not walk off buffer ---
  // After fix: should return quickly (e.g. 0) without reading past max length.
  {
    ArRobotPacket pkt;
    fillHeader(pkt, 0);
    pkt.setLength(4); // header only; length byte still 0
    UNUSED const int16_t c = pkt.calcCheckSum();
    // If we got here without crashing, short length was handled.
    // Optionally assert a defined value once the fix chooses one:
    // if (c != 0) fail("calcCheckSum with lenByte 0 should return 0");
  }
  {
    ArRobotPacket pkt;
    fillHeader(pkt, 1);
    pkt.setLength(4);
    UNUSED const int16_t c = pkt.calcCheckSum();
  }

  // --- Positive smoke: finalize / verify a tiny packet ---
  {
    ArRobotPacket pkt;
    pkt.empty();
    // Use normal packet construction path if you prefer:
    // write header via finalizePacket after appending data.
    // Minimal check: empty packet with insufficient length rejects.
    pkt.setLength(pkt.getHeaderLength()); // no footer yet
    if (pkt.verifyCheckSum())
      fail("verifyCheckSum true on header-only packet");
  }

  if (fails) {
    std::fprintf(stderr, "%d test(s) failed\n", fails);
    return 1;
  }
  std::puts("robotPacketTests: ok");
  return 0;
}
