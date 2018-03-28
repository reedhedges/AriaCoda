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

#include "ArExport.h"
#include "ArSoundPlayer.h"
#include "ArLog.h"
#include "ariaUtil.h"
#include <string.h>
#include <errno.h>

int ArSoundPlayer::ourPlayChildPID = -1;
ArGlobalRetFunctor2<bool, const char*, const char*> ArSoundPlayer::ourPlayWavFileCB(&ArSoundPlayer::playWavFile);
ArGlobalFunctor ArSoundPlayer::ourStopPlayingCB(&ArSoundPlayer::stopPlaying);
double ArSoundPlayer::ourVolume = 1.0;

AREXPORT ArRetFunctor2<bool, const char*, const char*>* ArSoundPlayer::getPlayWavFileCallback() 
{
  return &ourPlayWavFileCB;
}


AREXPORT ArFunctor* ArSoundPlayer::getStopPlayingCallback()
{
  return &ourStopPlayingCB;
}


#ifdef WIN32

      /* Windows: */

#include <assert.h>

AREXPORT bool ArSoundPlayer::playWavFile(const char* filename, const char* params) 
{
  return (PlaySound(filename, NULL, SND_FILENAME) == TRUE);
}

AREXPORT bool ArSoundPlayer::playNativeFile(const char* filename, const char* params)
{
  /* WAV is the Windows native format */
  return playWavFile(filename, 0);
}

AREXPORT void ArSoundPlayer::stopPlaying()
{
  PlaySound(NULL, NULL, NULL);
}


AREXPORT bool ArSoundPlayer::playSoundPCM16(char* data, int numSamples)
{
  ArLog::log(ArLog::Terse, "INTERNAL ERROR: ArSoundPlayer::playSoundPCM16() is not implemented for Windows yet! Bug reed@activmedia.com about it!");
  assert(false);

  return false;
}

#else

      /* Linux: */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef linux
#include <sys/soundcard.h>
//#include <linux/soundcard.h>
#endif


bool ArSoundPlayer::playNativeFile(const char* filename, const char* params)
{
  int snd_fd = ArUtil::open("/dev/dsp", O_WRONLY); // | O_NONBLOCK);
  if(snd_fd < 0) {
    return false;
  }
  int file_fd = ArUtil::open(filename, O_RDONLY);
  if(file_fd < 0)
  {
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSoundPlayer::playNativeFile: open failed");
    return false;
  }
  int len;
  const int buflen = 512;
  char buf[buflen];
  while((len = read(file_fd, buf, buflen)) > 0)
  {
    if (write(snd_fd, buf, len) != len) {
      ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSoundPlayer::playNativeFile: write failed");
    }
  }
  close(file_fd);
  close(snd_fd);
  return true;
}

bool ArSoundPlayer::playWavFile(const char* filename, const char* params)
{
  ArArgumentBuilder builder;
  //builder.addPlain("sleep .35; play");
  builder.addPlain("play");
  builder.add("-v %.2f", ourVolume);
  builder.addPlain(filename);
  builder.addPlain(params);
  ArLog::log(ArLog::Normal, "ArSoundPlayer: Playing file \"%s\" with \"%s\"", 
	     filename, builder.getFullString());
  
  int ret;
  if ((ret = system(builder.getFullString())) != -1)
  {
    ArLog::log(ArLog::Normal, "ArSoundPlayer: Played file \"%s\" with \"%s\" (got %d)", 
	       filename, builder.getFullString(), ret);
    return true;
  }
  else
  {
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSoundPlayer::playWaveFile: system call failed");
    return false;
  }


  /*
    This was an old mechanism for doing a fork then exec in order to
    mimic a system call, so that it could have the child PID for
    killing... however in our embedded linux killing the play command
    doesn't also kill the sox command, so the kill doesn't work at
    all... and the fork() would also reserve a lot of memory for the new process
    ...
   so it was replaced with the above system call

  const char* prog = NULL;
  prog = getenv("PLAY_WAV");
  if(prog == NULL)
    prog = "play";
  char volstr[4];
  if(strcmp(prog, "play") == 0)
  {
    snprintf(volstr, 4, "%f", ourVolume);
  }  

  ArLog::log(ArLog::Normal, "ArSoundPlayer: Playing file \"%s\" using playback program \"%s\" with argument: -v %s", filename, prog, volstr);
  ourPlayChildPID = fork();

  //ourPlayChildPID = vfork(); // XXX rh experimental, avoids the memory copy cost of fork()
  // NOTE: after vfork() you can ONLY safely use an exec function or _exit() in the
  // child process. Any other call, if it modifies memory still shared by the
  // parent, will result in problems.
  if(ourPlayChildPID == -1) 
  {
    ArLog::log(ArLog::Terse, "ArSoundPlayer: error forking! (%d: %s)", errno, 
      (errno == EAGAIN) ? "EAGAIN reached process limit, or insufficient memory to copy page tables" : 
        ( (errno == ENOMEM) ? "ENOMEM out of kernel memory" : "unknown error" ) );

    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSoundPlayer::playWaveFile: fork failed");
    return false;
  }
  if(ourPlayChildPID == 0)
  {
    // child process: execute sox
    int r = -1;
    r = execlp(prog, prog, "-v", volstr, filename, (char*)0);
    if(r < 0)
    {
      int err = errno;
      const char *errstr = strerror(err);
      printf("ArSoundPlayer (child process): Error executing Wav file playback program \"%s %s\" (%d: %s)\n", prog, filename, err, errstr);
      //_exit(-1);   // need to use _exit with vfork
      exit(-1);
    }
  } 
  // parent process: wait for child to finish
  ArLog::log(ArLog::Verbose, "ArSoundPlayer: created child process %d to play wav file \"%s\".", 
      ourPlayChildPID, filename);
  int status;
  waitpid(ourPlayChildPID, &status, 0);
  if(WEXITSTATUS(status) != 0) {
    ArLog::log(ArLog::Terse, "ArSoundPlayer: Error: Wav file playback program \"%s\" with file \"%s\" exited with error code %d.", prog, filename, WEXITSTATUS(status));
    ourPlayChildPID = -1;
    return false;
  }
  ArLog::log(ArLog::Verbose, "ArSoundPlayer: child process %d finished.", ourPlayChildPID);
  ourPlayChildPID = -1;
  return true;
  */
}



void ArSoundPlayer::stopPlaying()
{

  ArLog::log(ArLog::Normal, 
	     "ArSoundPlayer::stopPlaying: killing play and sox");

  // so if the system call below is "killall -9 play; killall -9 sox"
  // then on linux 2.4 kernels a sound played immediately afterwards
  // will simply not play and return (cause that's what play does if
  // it can't play)
  int ret;

  if ((ret = system("killall play; killall -9 sox")) != -1)
  {
    ArLog::log(ArLog::Normal, 
	       "ArSoundPlayer::stopPlaying: killed play and sox (got %d)", 
	       ret);
    return;
  }
  else
  {
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSoundPlayer::stopPlaying: system call failed");
    return;
  }


  /* This was for old mechanism in playWavFile but since it doesn't
   * work, it was replaced with the above

  // Kill a child processes (created by playWavFile) if it exists.
  if(ourPlayChildPID > 0)
  {
    ArLog::log(ArLog::Verbose, "ArSoundPlayer: Sending SIGTERM to child process %d.", ourPlayChildPID);
    kill(ourPlayChildPID, SIGTERM);
  }
  */
}


bool ArSoundPlayer::playSoundPCM16(char* data, int numSamples)
{
#ifdef linux
  //ArLog::log(ArLog::Normal, "ArSoundPlayer::playSoundPCM16[linux]: opening sound device.");
  int fd = ArUtil::open("/dev/dsp", O_WRONLY); // | O_NONBLOCK);
  if(fd < 0)
    return false;
  int arg = AFMT_S16_LE;
  if(ioctl(fd, SNDCTL_DSP_SETFMT, &arg) != 0)
  {
    close(fd);
    return false;
  }
  arg = 0;
  if(ioctl(fd, SNDCTL_DSP_STEREO, &arg) != 0)
  {
    close(fd);
    return false;
  }
  arg = 16000;
  if(ioctl(fd, SNDCTL_DSP_SPEED, &arg) != 0)
  {
    close(fd);
    return false;
  }
  //ArLog::log(ArLog::Normal, "ArSoundPlayer::playSoundPCM16[linux]: writing %d bytes to sound device.", 2*numSamples);
  int r;
  if((r = write(fd, data, 2*numSamples) < 0))
  {
    close(fd);
    return false;
  }
  close(fd);
  ArLog::log(ArLog::Verbose, "ArSoundPlayer::playSoundPCM16[linux]: finished playing sound. (wrote %d bytes of 16-bit monaural signed sound data to /dev/dsp)", r);
  return true;
#else
  ArLog::log(ArLog::Terse, "ArSoundPlayer: Internal Error: playSoundPCM16 not implemented for non-Linux unixlike systems");
  return false;
#endif
}



#endif  // ifdef WIN32

AREXPORT void ArSoundPlayer::setVolumePercent(double pct)
{
  setVolume(1.0 + (pct / 100.0));
}

AREXPORT void ArSoundPlayer::setVolume(double v)
{
  if(v < 0) ourVolume = 0;
  else ourVolume = v;
}

