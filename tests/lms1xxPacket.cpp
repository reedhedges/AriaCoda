

/* 
  Test ArLMS1XX packet parsing

  ArLMS1XXPacket's "unsigned" accessors parse "Ascii" ("CoLa A") data as hexadecimal text strings using strtol(). 
  They are assumed to be separated by whitespace or '\003', but non-digits are also skipped  by strtol().
  


  ArLMS1XXPacket used to contain some other implementation in the "signed" accessors, but these seemed to have bugs are were not used, so have been removed.
  
  Note that ArLMS1XXPacket always starts the buffer read index (myReadLength) at 1 (in constructor, resetRead(), etc.), so test data must start at offset of at least 1.

  For documentation on LMS1xx protocol, see SICK document:
    "Telegram Listing, LiDAR sensors LMS1xx, LMS5xx, TiM2xx, TiM5xx, TiM7xx, LMS1000, MRS1000, MRS6000, NAV310, LD-OEM15xx, LD-LRS36xx, LMS4000"
  
  * "Ascii" mode sequences start with \002 and end with \003, each number is a pair of hexadecimal text (ascii) digits separated by spaces
*/


#include "Aria/ArLMS1XX.h"
#include <cassert>



// todo test negative numbers? 
// todo test large numbers
// todo test malformed packets

int main()
{
  ArLMS1XXPacket packet;
  
  // first buffer is "hex" mode so we need to use "unsigned" accessors
  // hex numbers int8_t: 0, 1, 2, int16_t: 3, 4, 5, int32_t: 10, 11, 12, 13, 14, 15
  // size 24 bytes
  const char *test1 = "x0 1 2 3 4 5 A B C D E F\003";

  packet.setBuf(const_cast<char*>(test1), 24);  // packet does not actually modify buffer
  packet.setLength(24);
  assert(packet.isValid());

  assert(packet.isValid());
  assert(packet.bufToUByte() == 0);
  auto x0 = packet.bufToUByte();
  //printf("First value in packet payload (sholud be 1) == %d\n", x0);
  assert(packet.isValid());
  assert(x0 == 1);
  assert(packet.bufToUByte() == 2);
  assert(packet.isValid());

  assert(packet.bufToUByte2() == 3);
  assert(packet.isValid());
  uint16_t x1 = packet.bufToUByte2();
  assert(x1 == 4);
  assert(packet.isValid());
  assert(packet.bufToUByte2() == 5);
  assert(packet.isValid());

  assert(packet.bufToUByte4() == 10);
  assert(packet.isValid());
  uint32_t x2 = packet.bufToUByte4();
  assert(x2 == 11);
  assert(packet.isValid());
  assert(packet.bufToUByte4() == 12);
  assert(packet.isValid());
  assert(packet.bufToUByte4() == 13);
  assert(packet.isValid());
  assert(packet.bufToUByte4() == 14);
  assert(packet.isValid());
  assert(packet.bufToUByte4() == 15);


  puts("ok test sucessful");
  return 0;
}
