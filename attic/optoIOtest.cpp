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
/** This program will step through various test of the analog and digital IO
 * on the Versalogic VSBC-6/8 and EBX-12 boards.
 **/

#include <Aria.h>


int main(void)
{
  int i, j;
  bool EBX12 = true;

  ArVersalogicIO amrioIO;
  if (!amrioIO.isEnabled())
  {
    printf("Device disabled! Exiting....\n");
    return 0;
  }
  //  double an0, an1, an2, an3, an4, an5, an6, an7;
  bool retval;
  printf("\nsetting digital banks to outputs\n");
  {
    retval = amrioIO.setDigitalBankDirection(0, ArVersalogicIO::DIGITAL_OUTPUT);
    printf("Bank 0:\t%d\n", retval);
    retval = amrioIO.setDigitalBankDirection(1, ArVersalogicIO::DIGITAL_OUTPUT);
    printf("Bank 1:\t%d\n", retval);
    if (EBX12)
    {
      retval = amrioIO.setDigitalBankDirection(2, ArVersalogicIO::DIGITAL_OUTPUT);
      printf("Bank 2:\t%d\n", retval);
      retval = amrioIO.setDigitalBankDirection(3, ArVersalogicIO::DIGITAL_OUTPUT);
      printf("Bank 3:\t%d\n", retval);
    }
  }

  unsigned char bits[33];
  for (i=0;i<32;i++)
    bits[i] = 0x30;
  bits[32] = 0x0;

  printf("\nwriting data to banks\n");
  for (i=0;i<4;i++)
  {
    printf("\r%s", bits);
    fflush(stdout);
    for (j=0;j<=7;j++)
    {
      bits[j] = 0x31;
      if (!(retval = amrioIO.setDigitalBankOutputs(0, (unsigned char) 1 << j)))
	printf("\nretval %d on bit %d\n", retval, j);
      else
	printf("\r%32s", bits);
      bits[j] = 0x30;
      fflush(stdout);
      ArUtil::sleep(100);
//      bits[j] = 0x30;
    }
    // clear out the last bit
    if (!(retval = amrioIO.setDigitalBankOutputs(0, (unsigned char) 0)))
      printf("\nretval %d on bit 7\n", retval);    
    for (j=8;j<=15;j++)
    {
      bits[j] = 0x31;
      if (!(retval = amrioIO.setDigitalBankOutputs(1, (unsigned char) 1 << (j-8))))
	printf("\nretval %d on bit %d\n", retval, j);
      else
	printf("\r%s", bits);
      bits[j] = 0x30;
      fflush(stdout);
      ArUtil::sleep(100);
//	bits[j] = 0x30;
    }
    // clear out the last bit
    if (!(retval = amrioIO.setDigitalBankOutputs(1, (unsigned char) 0)))
      printf("\nretval %d on bit 15\n", retval);    
    if (EBX12)
    {
      for (j=16;j<=23;j++)
      {
	bits[j] = 0x31;
	if (!(retval = amrioIO.setDigitalBankOutputs(2, (unsigned char) 1 << (j-16))))
	  printf("\nretval %d on bit %d\n", retval, j);
	else
	  printf("\r%s", bits);
	bits[j] = 0x30;
	fflush(stdout);
	ArUtil::sleep(100);
//	bits[j] = 0x30;
      }
      // clear out the last bit
      if (!(retval = amrioIO.setDigitalBankOutputs(2, (unsigned char) 0)))
	printf("\nretval %d on bit 23\n", retval);    
      for (j=24;j<=31;j++)
      {
	bits[j] = 0x31;
	if (!(retval = amrioIO.setDigitalBankOutputs(3, (unsigned char) 1 << (j-24))))
	  printf("\nretval %d on bit %d\n", retval, j);
	else
	  printf("\r%s", bits);
	bits[j] = 0x30;
	fflush(stdout);
	ArUtil::sleep(100);
//	bits[j] = 0x30;
      }
      // clear out the last bit
      if (!(retval = amrioIO.setDigitalBankOutputs(3, (unsigned char) 0)))
	printf("\nretval %d on bit 31\n", retval);    
    }
  }
  for (i=0;i<32;i++)
    bits[i] = 0x31;
  printf("\n\nsetting all digital outputs high\n");
  if (!(retval = amrioIO.setDigitalBankOutputs(0, 0xFF)))
    printf("\nretval %d on bank 0\n", retval);
  if (!(retval = amrioIO.setDigitalBankOutputs(1, 0xFF)))
    printf("\nretval %d on bank 1\n", retval);
  if (EBX12)
  {
    if (!(retval = amrioIO.setDigitalBankOutputs(2, 0xFF)))
      printf("\nretval %d on bank 2\n", retval);
    if (!(retval = amrioIO.setDigitalBankOutputs(3, 0xFF)))
      printf("\nretval %d on bank 3\n", retval);
  }
  printf("%s\n", bits);
  fflush(stdout);
  ArUtil::sleep(2000);
  for (i=0;i<32;i++)
    bits[i] = 0x30;
  printf("\nsetting all digital outputs low\n");
  if (!(retval = amrioIO.setDigitalBankOutputs(0, 0)))
  printf("\nretval %d on bank 0\n", retval);
  if (!(retval = amrioIO.setDigitalBankOutputs(1, 0)))
  printf("\nretval %d on bank 1\n", retval);
  if (EBX12)
  {
    if (!(retval = amrioIO.setDigitalBankOutputs(2, 0)))
    printf("\nretval %d on bank 2\n", retval);
    if (!(retval = amrioIO.setDigitalBankOutputs(3, 0)))
    printf("\nretval %d on bank 3\n", retval);
  }
  printf("%s\n", bits);
  fflush(stdout);
  ArUtil::sleep(2000);

  printf("\n\nsetting digital banks to inputs\n");
  {
    retval = amrioIO.setDigitalBankDirection(0, ArVersalogicIO::DIGITAL_INPUT);
    printf("Bank 0:\t%d\n", retval);
    retval = amrioIO.setDigitalBankDirection(1, ArVersalogicIO::DIGITAL_INPUT);
    printf("Bank 1:\t%d\n", retval);
    if (EBX12)
    {
      retval = amrioIO.setDigitalBankDirection(2, ArVersalogicIO::DIGITAL_INPUT);
      printf("Bank 2:\t%d\n", retval);
      retval = amrioIO.setDigitalBankDirection(3, ArVersalogicIO::DIGITAL_INPUT);
      printf("Bank 3:\t%d\n", retval);
    }
  }

  for (i=0;i<32;i++)
    bits[i] = 0x30;
  printf("\ngetting digital input values\n");
  unsigned char bank;
  if (!(retval = amrioIO.getDigitalBankInputs(0, &bank)))
    printf("\nretval %d on bank 0\n", retval);
  for (i=0;i<8;i++)
    if (bank & ( 1 << i ))
      bits[i] = 0x31;
  if (!(retval = amrioIO.getDigitalBankInputs(1, &bank)))
    printf("\nretval %d on bank 1\n", retval);
  for (i=0;i<8;i++)
    if (bank & ( 1 << i ))
      bits[i+8] = 0x31;
  if (EBX12)
  {
    if (!(retval = amrioIO.getDigitalBankInputs(2, &bank)))
      printf("\nretval %d on bank 2\n", retval);
    for (i=0;i<8;i++)
      if (bank & ( 1 << i ))
	bits[i+16] = 0x31;
    if (!(retval = amrioIO.getDigitalBankInputs(3, &bank)))
      printf("\nretval %d on bank 3\n", retval);
    for (i=0;i<8;i++)
      if (bank & ( 1 << i ))
	bits[i+24] = 0x31;
  }
  printf("Digital input values:\n");
  printf("%s\n", bits);

  printf("\n");
  /*
  unsigned char bank0, bank1, bank2, bank3;
  printf("Digital Low High  An0   An1   An2   An3   An4   An5   An6   An7\n");
  an0 = 0;
  an1 = 0;
  an2 = 0;
  an3 = 0;
  an4 = 0;
  an5 = 0;
  an6 = 0;
  an7 = 0;
  while(true)
  {
    if (!amrioIO.getDigitalBankInputs(0, &bank0))
      printf("getDigitalBank 0 failed\n");
    if (!amrioIO.getDigitalBankInputs(1, &bank1))
      printf("getDigitalBank 1 failed\n");
    if (EBX12)
    {
      if (!amrioIO.getDigitalBankInputs(2, &bank2))
	printf("getDigitalBank 2 failed\n");
      if (!amrioIO.getDigitalBankInputs(3, &bank3))
	printf("getDigitalBank 3 failed\n");
    }

    retval = amrioIO.getAnalogValue(0, &an0);
    if (!retval)
      printf("an0 failed\n");
    retval = amrioIO.getAnalogValue(1, &an1);
    if (!retval)
      printf("an1 failed\n");
    retval = amrioIO.getAnalogValue(2, &an2);
    if (!retval)
      printf("an2 failed\n");
    retval = amrioIO.getAnalogValue(3, &an3);
    if (!retval)
      printf("an3 failed\n");
    retval = amrioIO.getAnalogValue(4, &an4);
    if (!retval)
      printf("an4 failed\n");
    retval = amrioIO.getAnalogValue(5, &an5);
    if (!retval)
      printf("an5 failed\n");
    retval = amrioIO.getAnalogValue(6, &an6);
    if (!retval)
      printf("an6 failed\n");
    retval = amrioIO.getAnalogValue(7, &an7);
    if (!retval)
      printf("an7 failed\n");
    printf("\r\t%.2x %.2x", bank0, bank1);
    if (EBX12)
      printf(" %.2x %.2x", bank2, bank3);
    printf("    %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f", an0, an1, an2, an3, an4, an5, an6, an7);
    fflush(stdout);
    ArUtil::sleep(1000);
  }
  */
  printf("\n");

}
