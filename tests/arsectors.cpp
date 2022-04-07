#include "Aria/ariaUtil.h"
#include <iostream>

int main()
{
  puts("8 sectors, all updated...");
  {
    // default number of sectors (8)
    ArSectors<> sectors;
    std::cout << "newly created: " << sectors << "\n";
    assert(!sectors.didAll());
    int i = 0;
    for(double a = 22.5; a <= 360; a += 45)
    {
      std::cout << a << "; ";
      sectors.update(a);
      ++i;
    }
    std::cout << "\n" << sectors << "\n";
    assert(i == 8);
    assert(sectors.didAll());
  }

  puts("----\n8 sectors, only 7 updated...");

  // miss last sector
  {
    ArSectors<> sectors;
    int i = 0;
    for(double a = 22.4; a <= 315; a += 45)
    {
      std::cout << a << "; ";
      sectors.update(a);
      ++i;
    }
    std::cout <<  "\n" << sectors << "\n";
    assert(i == 7);
    assert( ! sectors.didAll());
  }

  puts("----\n16 sectors, all updated...");
 
  // sixteen sectors
  {
    ArSectors<16> sectors;
    int i = 0;
    for(double a = 11.74; a <= 360; a += 22.5)
    {
      sectors.update(a);
      ++i;
    }
    std::cout <<  "\n" << sectors << "\n";
    assert(i == 16);
    assert(sectors.didAll());
  } 

  puts("----\n16 sectors, only some sectors updated...");

  // miss every other sector
  {
    ArSectors<16> sectors;
    int i = 0;
    for(double a = 22.4; a <= 360; a += 45)
    {
      sectors.update(a);
      ++i;
    }
    std::cout <<  "\n" << sectors << "\n";
    assert(i == 8);
    assert( ! sectors.didAll());
  }

  puts("----\n8 sectors, update on sector boundaries...");
 
  // hit sector boundaries
  {
    ArSectors<> sectors;
    int i = 0;
    for(double a = 0; a < 360; a += 45)
    {
      std::cout << a << "; ";
      sectors.update(a);
      ++i;
    }
    std::cout <<  "\n" << sectors << "\n";
    assert(i == 8);
    assert(sectors.didAll());
  } 

  puts("all tests passed");

}
