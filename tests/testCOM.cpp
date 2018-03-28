/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
//#define DEBUG

#include "Aria.h"
#include "string.h"

#define BUF_LEN 50
#define LOOP 100000
#define MAGIC 0x5A
#define MAX_BR 10
#define START_BR 8

#define NOMAGIC

void sleep(int br);

int main(int argc, char** argv)
{
  unsigned int real_loop, real_len;
  int actual_start;
  unsigned int c1, c2;
  int ret;
  unsigned long drops;
  unsigned long fail, n, eachtry1, eachtry2, eachfail1, eachfail2, tx;
  char bufWrite[BUF_LEN];
  char bufRead[BUF_LEN];
  int foo, pctcomplete;
  int i;
  float pct;
  for (i = 0; i < BUF_LEN; i++)
#ifdef NOMAGIC
    bufWrite[i] = i+1;
#else
    bufWrite[i] = MAGIC;
#endif

  srand(time(NULL));
  
  int bauds[10];
  float pcts1[10], pcts2[10];
//not all standard baud rates are supported, "ArSerialConnection::rateToBaud" supports these:
  bauds[0] = 300;
  bauds[1] = 1200;
  bauds[2] = 1800;
  bauds[3] = 2400;
  bauds[4] = 4800;
  bauds[5] = 9600;
  bauds[6] = 19200;
  bauds[7] = 38400;
  bauds[8] = 57600;
  bauds[9] = 115200;

  eachfail1 = 0;
  eachfail2 = 0;
  eachtry1 = 0;
  eachtry2 = 0;

  for(i=0;i<10;i++)
    {
    pcts1[i] = 0.0;
    pcts2[i] = 0.0;
    }

  fail = 0;
  n = 0;
  c1 = 5;
  c2 = 5;
  real_loop = LOOP;

  int bytes1 = 0;
  int bytes2 = 0;
  //int numToWrite = 1;

#ifdef DEBUG
  for (i=1;i<argc;i++)
    {
    printf("%s  %d\n", argv[i], ((unsigned int) *argv[i]));
    }
  if(argc>3)
    printf("\n\n%d\n\n",(48 + MAX_BR) - ((unsigned int) *argv[3]));
#endif

  c1 = (unsigned int) *argv[1];
  c2 = (unsigned int) *argv[2];

  if ((c1 == c2) || ('1'>c1) || ('1'>c2) || ('4'<c1) || ('4'<c2))
    {
    printf("\n\nInvalid input    %d  %d\n\n",c1,c2);
    exit(0);
    }

////////////optional command line arguments
  if (argc < 4)
    {
    actual_start = START_BR;
    }
  else
    {
    if (strlen(argv[3]) == 1)
      {//add in ascii offset of (text) zero
      actual_start = (48 + MAX_BR) - ((unsigned int) *argv[3]);
      }
    else
      {
      actual_start = 0;//hard coded, but it'll work....  i hate doing this....
      }
    }

  if (argc < 5)
    {
    real_loop = LOOP;
    }
  else
    {//account for ascii offset of (text) zero
    real_loop = 0;
    for (unsigned char k=0;k<strlen(argv[4]);k++)
      {
#ifdef DEBUG
      printf("DEBUG loop calculation: %d\n",real_loop);
#endif
      real_loop += (((unsigned int) *argv[4]+k) - 48) * 10^(strlen(argv[4]) - (k+1));
      }
    }
////////////re-check optional command line arguments
#ifdef DEBUG
printf("\nreal_loop: %d, start_br: %d\nraw 3: %s, raw 4: %s\n",real_loop,actual_start,argv[3],argv[4]);
printf("strlen [3]: %d\n",strlen(argv[3]));
exit(0);
#endif


  ArSerialConnection ser1;
  ArSerialConnection ser2;

  ArUtil::sleep(50);

  switch(c1)
    {
    case '1':
      ser1.setPort(ArUtil::COM1);
      break;
    case '2':
      ser1.setPort(ArUtil::COM2);
      break;
    case '3':
      ser1.setPort(ArUtil::COM3);
      break;
    default:
      ser1.setPort(ArUtil::COM4);
      break;
    }
  ArUtil::sleep(50);
  switch(c2)
    {
    case '1':
      ser2.setPort(ArUtil::COM1);
      break;
    case '2':
      ser2.setPort(ArUtil::COM2);
      break;
    case '3':
      ser2.setPort(ArUtil::COM3);
      break;
    default:
      ser2.setPort(ArUtil::COM4);
      break;
    }
  ArUtil::sleep(50);
  if (!ser1.openSimple())
    {
      printf("Exiting since open failed\n");
      exit(0);
    }
  ArUtil::sleep(50);
  if (!ser2.openSimple())
    {
      printf("Exiting since open failed\n");
      exit(0);
    }
  ArUtil::sleep(250);

  if ((0>real_loop) || (LOOP<real_loop))
    {
    real_loop = LOOP;
    }
  

  if ((0>actual_start) || (MAX_BR<actual_start))
    {
    actual_start = START_BR;
    }

for(unsigned int br=actual_start;br<MAX_BR;br++)
  {
  printf("\nReconfig COM%s to %d...",argv[1],bauds[br]);
  fflush(stdout);
  ser1.setBaud(bauds[br]);
  ArUtil::sleep(50);
  printf(" success!  ");
  printf("Reconfig COM%s to %d...",argv[2],bauds[br]);
  fflush(stdout);
  ser2.setBaud(bauds[br]);
  ArUtil::sleep(250);
  printf(" success!\n");
  fflush(stdout);

  eachfail1 = 0;
  eachfail2 = 0;
  eachtry1 = 0;
  eachtry2 = 0;

  for(unsigned int j=0;j<real_loop;j++)
    {
    drops = 0;
    #ifdef NOMAGIC
      srand (time(NULL));
      for (i = 0; i < BUF_LEN; i++)
        bufWrite[i] = ((unsigned char) (rand() % 100))+ i;
    #endif
    sleep(bauds[br]);
    if (ser1.write(bufWrite, BUF_LEN) < 0)
      {
      printf("Failed write  ");
      eachfail1 += BUF_LEN;
      }
    sleep(bauds[br]);
    if ((ret = ser2.read(bufRead, BUF_LEN)) < 0)
      {
      printf("Failed read  ");
      eachfail1 += BUF_LEN;
      }
    else
      {
      if (ret > 0) 
        {
        for (i = 0; i < ret; i++)
          {
#ifdef NOMAGIC
          if ((bufRead[i] != bufWrite[i]) && (bufRead[i] != bufWrite[BUF_LEN - ret + i]))
#else
          if (bufRead[i] != MAGIC)
#endif
            {
//            printf("Failed2 0x%x %d/%d  ", bufRead[i], i, ret);
            eachfail1 ++;
            }
          }
        }
      }
    bytes2 += ret;
    eachtry1 += ret;

//    ArUtil::sleep(1000);
       
    sleep(bauds[br]);
    if (ser2.write(bufWrite, BUF_LEN) < 0)
      {
      printf("Failed2 write  ");
      eachfail2 += BUF_LEN;
      }
    sleep(bauds[br]);
    if ((ret = ser1.read(bufRead, BUF_LEN)) < 0)
      {
      printf("Failed2 read  ");
      eachfail2 += BUF_LEN;
      }
    else
      {
      if (ret > 0)
	{
        for (i = 0; i < ret; i++)
          {
#ifdef NOMAGIC
          if ((bufRead[i] != bufWrite[i]) && (bufRead[i] != bufWrite[BUF_LEN - ret + i]))
//          if (bufRead[i] != bufWrite[i])
#else
          if (bufRead[i] != MAGIC)
#endif
            {
//            printf("Failed2 0x%x %d/%d  ", bufRead[i], i, ret);
            eachfail2 ++;
            }
          }
        }
      }
    bytes1 += ret;
    eachtry2 += ret;

    printf("\r");
    printf("{");
    pctcomplete = (j*50)/real_loop;
    for(int k=0;k<50;k++)
      {
      if (pctcomplete>k)
        {
        printf("*");
        }
      else
        {
        if(pctcomplete==k)
          {
          foo = j % 4;
          switch(foo)
            {
            case 0:
              printf("\\");
              break;
            case 1:
              printf("|");
              break;
            case 2:
              printf("/");
              break;
            default:
              printf("-");
              break;
            }
          }
        else
          printf(" ");
        }
      }
    printf("}");
    drops += ((2*(j+1)*BUF_LEN)-(eachtry1 + eachtry2));
    printf("%7d,%7d,%4d,%ld,%d", bytes1, bytes2, j, drops, bufRead[5]);
    fflush(stdout);
    }

  tx = (BUF_LEN*real_loop);

  if(eachtry1<tx)
    {
    printf("\nCOM%s->COM%s dropped %ld bytes.",argv[1],argv[2],(tx - eachtry1));
    eachfail1 +=(tx - eachtry1);
    eachtry1 = tx;
    }
  if(eachtry2<tx)
    {
    printf("\nCOM%s->COM%s dropped %ld bytes.",argv[2],argv[1],(tx - eachtry2));
    eachfail2 +=(tx - eachtry2);
    eachtry2 = tx;
    }

  pcts1[br] = 100.0 * (1.0 - ((float)eachfail1 / (float)eachtry1));
  pcts2[br] = 100.0 * (1.0 - ((float)eachfail2 / (float)eachtry2));
  printf("\n%f%%  ", pcts1[br]);
  printf("Port1->2 %ld failures out of %ld attempts. (%d baud)\n", eachfail1, eachtry1, bauds[br]);
  printf("%f%%  ", pcts2[br]);
  printf("Port2->1 %ld failures out of %ld attempts. (%d baud)", eachfail2, eachtry2, bauds[br]);
  fail += eachfail1;
  fail += eachfail2;
  n += eachtry1;
  n += eachtry2;
  }

tx = (BUF_LEN*2*real_loop*(MAX_BR-actual_start));

if(n<tx)
  {
  printf("\n\nDropped bytes:\n%ld RX\n%ld TX",n,tx);
  fail+=(tx - n);
  n = tx;
  }

printf("\n\n                 COM%s->COM%s:          COM%s->COM%s:",argv[1],argv[2],argv[2],argv[1]);
for(int k=0;k<10;k++)
  {
  if (0.0 == pcts1[k])
    {
    printf("\n%6d baud:     untested             untested",bauds[k]);
    }
  else
    {
    if (100.0==pcts1[k])
      {
      printf("\n%6d baud:     %05f%%          %f%%",bauds[k],pcts1[k],pcts2[k]);
      }
    else
      {
      printf("\n%6d baud:     %05f%%           %f%%",bauds[k],pcts1[k],pcts2[k]);
      }
    }
  }
/*
printf("\n\nPort COM%s->COM%s:",argv[2],argv[1]);
for(int k=0;k<10;k++)
  {
  if (0.0 == pcts2[k])
    {
    printf("\n%6d baud: untested",bauds[k]);
    }
  else
    {
    printf("\n%6d baud: %f%%",bauds[k],pcts2[k]);
    }
  }
*/

pct = 100.0 * (1.0 - ((float)fail / (float)n));
printf("\n\n%f%%  Overall\n", pct);

if (99.8>pct)
  {
  printf("\n*****************************************\n");
  printf(" %ld errors -- test failed.",fail);
  printf("\n*****************************************\n");
  printf("\n  FFFFF  AAA  IIIII L");
  printf("\n  F     A   A   I   L");
  printf("\n  FFFF  AAAAA   I   L");
  printf("\n  F     A   A   I   L");
  printf("\n  F     A   A IIIII LLLLL\n\n");
  }
else
  {
  if(100.0==pct)
    {
    printf("\n*****************************************\n");
    printf("Test completed successfully.");
    printf("\n*****************************************\n");
    printf("\n  PPPP   AAA   SSSS  SSSS");
    printf("\n  P   P A   A S     S    ");
    printf("\n  PPPP  AAAAA  SSS   SSS");
    printf("\n  P     A   A     S     S");
    printf("\n  P     A   A SSSS  SSSS \n\n");
    }
  else
    {
    printf("\nMarginal success.....  please re-test.\n\n");
    }
  }

ser1.close();
ser2.close();
}


void sleep(int br)
{
ArUtil::sleep((200000 / (long)br) + 2);
//ArUtil::sleep(2);
return;
}
