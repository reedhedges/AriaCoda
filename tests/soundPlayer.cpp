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


#include "ArSoundPlayer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) 
{
  puts("reading PCM16 sound \"testsound.sw.raw\"...");
  int fd = open("testsound.sw.raw", O_RDONLY);
  char data[60000];
  char* p = data;
  int len = 0;
  for(len = 0; ( (len <= 60000) && (read(fd, p++, 1) > 0) ); len++);
  close(fd);
  printf("playing PCM16 sound: %d bytes with %d samples of 2 bytes each...\n", len, len/2);
  ArSoundPlayer::playSoundPCM16(data, len/2);
  puts("done playing test sound.\n");

  puts("playing test sound \"../examples/sound-r2a.wav\"...");
  ArSoundPlayer::playWavFile("../examples/sound-r2a.wav", NULL);
  puts("done playing test sound.\n");



  puts("attempting to play nonexistant sound \"nonexistant\". should fail (error output from \"play\" program)...");
  ArSoundPlayer::playWavFile("nonexistant", NULL);
  puts("done playing bogus test sound.\n");

  puts("attempting to play test sound \"../examples/sound-r2a.wav\" but with the bogus play program \"badplay\" (set via PLAY_WAV environment variable)... should fail with descriptive error message");
  setenv("PLAY_WAV", "badplay", 1);
  ArSoundPlayer::playWavFile("../examples/sound-r2a.wav", NULL);
  setenv("PLAY_WAV", "play", 1);
  puts("done playing test sound with bogus player.\n");

  puts("allocating a lot of memory, then trying to play sound.... (If ArSoundPlayer is using fork() to run the sound player, then you can get fork() to fail with ENOMEM by reducing available system memory and swap, and increasing the number of mallocs and their size. if it is using vfork() then this won't affect the creation of the child process.)");
  unsigned long total = 0;
  const unsigned long allocsize = 20000;
  const int alloctimes = 29999;
  char *arrays[alloctimes];
  for(int i = 0; i < alloctimes; ++i)
  {
    arrays[i] = (char*) malloc(allocsize);
    if(arrays[i] == NULL)
      perror("Error allocating memory!");
    total += allocsize;
    if(i % 1000 == 0)
      printf("...allocated %d bytes (%f MB) in %d blocks...\n", total, ((double)total)/(1024.0*1024.0), i+1);
  }
  ArSoundPlayer::playWavFile("../examples/sound-r2a.wav", NULL);
  puts("ok, played test sound. deallocating memory...");
  for(int i = 0; i < alloctimes; ++i)
  {
    free(arrays[i]);
  }
  puts("done deallocating memory.");
  


  puts("Playing WAV file in a loop (forever times)...");
  while(true)
    ArSoundPlayer::playWavFile("../examples/sound-r2a.wav", NULL);
  puts("done playing in a loop.\n");

  return 0;
}


