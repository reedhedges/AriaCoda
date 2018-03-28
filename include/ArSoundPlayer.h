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

#ifndef _ARSOUNDPLAYER_H_
#define _ARSOUNDPLAYER_H_


#include "ArFunctor.h"



/** 
 * @brief This class provides a cross-platform interface for playing short sound samples.
 * (Currently implemented for Windows and Linux).
 * @sa For I/O and network transfer of encoded audio, see the ArNetAudio library.
 * @sa ArSoundsQueue
 *
 * @note Uses an external program to play WAV files on Linux. If an environment
 * variable named PLAY_WAV is set, that program is used, otherwise, 'play' from
 * the 'sox' toolset is used.  PLAY_WAV must contain one word (the command; no arguments)
 * A call to playWavFile() will return immediately after
 * 'play' has finished, even though Linux may still be playing back the sound data. In general,
 * this kind of thing is a problem, especially with speech recognition immediately after playing
 * a sound. Ideally, we should be able to truly block until the sound has finished playback.
 * Alas, it is not an ideal world. Another potential pitfall due to the use of
 * an external program invocation: the program you call must not attempt to
 * issue any output.  'play' from the 'sox' toolset automatically supresses
 * normal output if it isn't called from an interactive terminal, but it may
 * still issue some error messages, which will cause it to hang indefinately.
 *
 * The volume (level) of audio output from a robot is determined by two things:
 * the computer sound device mixer, and also the amplifier which drives the
 * speakers.  The computer's mixer can be adjusted through the operating system:
 * on Linux, you can use the 'aumix' program to adjust the Master and PCM
 * levels.  On Windows, use the Windows mixer program.  If on Linux, ArSoundPlayer also
 * prodives the setVolume() method, which adjusts the volume of the sound before
 * it is played.
 *  
    @ingroup UtilityClasses
 */
class ArSoundPlayer
{
 public:
  /** Play a WAV (Windows RIFF) file 
    * @note Uses an external program to play WAV files on Linux. If an environment
    * variable named PLAY_WAV is set, that program is used, otherwise, 'play' from
    * the 'sox' toolset is used. See detailed note in the overview for this
    * cass.
    * @param filename Name of the file to play
    * @param params ignored
    */
  AREXPORT static bool playWavFile(const char* filename, const char* params);

  AREXPORT static bool playWavFile(const char* filename) { return playWavFile(filename, NULL); }

  /** Play a file in some native file format for the compilation platform. */
  AREXPORT static bool playNativeFile(const char* filename, const char* params);

  /** Cancel (interrupt) any current sound or file playback. */
  AREXPORT static void stopPlaying();

  /** Return the static functor for playWavFile */
  AREXPORT static ArRetFunctor2<bool, const char*, const char*> *getPlayWavFileCallback();

  /** Return the static functor for stopPlaying(). */
  AREXPORT static ArFunctor* getStopPlayingCallback();

  /** Play raw uncompressed PCM16 sound data. The format of this data is 
   *  numSamples samples of two bytes each. Each byte pair is a signed little endian
   *  integer.
   *  The sound will be played back at 16kHz, monaurally.
   *  @return false on error, true on success.
   */
  AREXPORT static bool playSoundPCM16(char* data, int numSamples);

  /** Set a volume adjustment applied to all sounds right before playing.
     (So this adjusts the volume in addition to, not instead of, the
      computer audio mixer). 
      Any value less than or equal to 0 is no volume i.e. muted or no output.
      @linuxonly
  */
  AREXPORT static void setVolume(double v);

  /**
      Set volume as a "percent" of normal, where 100% is normal or natural
      volume, 50% is increased by 50%, -50% is decreased by 50%, etc. (-100.0% is 
      no volume, or mute.)
      @linuxonly
  */
  AREXPORT static void setVolumePercent(double pct);
 
protected:
  static int ourPlayChildPID; ///< Only used on Linux.
  static ArGlobalRetFunctor2<bool, const char*, const char*> ourPlayWavFileCB;
  static ArGlobalFunctor ourStopPlayingCB;
  static double ourVolume;
};
    
  
#endif // _ARSOUNDPLAYER_H_
