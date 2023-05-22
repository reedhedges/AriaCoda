
#include "Aria/ariaUtil.h"
#include "Aria/ArRingBuffer.h"
#include <array>

int main()
{
  ArRingBuffer<int, 10, std::array<int, 10>> buf;
  printf("Empty: ");
  buf.print();
  assert(buf.empty());
  for(int i = 1; i <= 10; ++i)
  {
    buf.push(i);
    printf("After pushing %2d: ", i);
    buf.print();
    assert(*(buf.front()) == 1);
    assert(buf.size() == (size_t)i); 
  }

  assert(buf.size() == buf.capacity());

  buf.push(11);
  printf("After pushing 11: ");
  buf.print();
  assert(*(buf.front()) == 2);

  buf.push(12);
  printf("After pushing 12: ");
  buf.print();
  assert(*(buf.front()) == 3);

  buf.reset();
  printf("After reset: ");
  buf.print();
  assert(buf.empty());
  buf.push(23);
  printf("After pushing 23: ");
  buf.print();
  assert(*(buf.front()) == 23);
  return 0;
}

