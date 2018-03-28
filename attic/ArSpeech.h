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


#ifndef ARSPEECH_H
#define ARSPEECH_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ArConfig.h"
#include <list>
#include <string>


/** @brief Abstract interface to speech synthesis.  
 *  
 *  @deprecated
 *
 *  This class defines the abstract interface for speech synthesizers 
 *  used with Aria.
 *  Implementations are provided in the separate ArSpeechSynth_Cepstral and
 *  ArSpeechSynth_Festival libraries.
 *  This class provides a common-denominator
 *  interface. Implementations (especially ArCepstral) may support more
 *  features, or behave differently; refer to their documentation for more
 *  information.
 *
 *  This class registers one parameter with the global ArConfig object in the
 *  "Speech Synthesis" section:  The parameter is named "voice" and sets the voice
 *  used for speech synthesis, if multiple voices is supported and the 
 *  new voice can be loaded. 
 */

class ArSpeechSynth
{
public:

  /** Don't forget to call this from derived classes. */
  AREXPORT ArSpeechSynth();  

  AREXPORT virtual ~ArSpeechSynth();


  /** Perform  synthesizer initialization, if necessary. You must call
   * this method. 
   *
   * (Subclass implementations should call this method *after* initializing their 
   * speech engine.)
   */
  AREXPORT virtual bool init(); 

  /** Use the given ArConfig object to read parameters such as voice, speaking
   * rate, volume.
   */
  AREXPORT virtual void addToConfig(ArConfig *config);

  /** Speaks the given text. 
   * @param str The text to speak.
   * @param voiceParams Voice selection criteria expression
   *    (implementation-specific)
   * @param audioOutputCB If not NULL, send synthesized audio data to this
   *    callback (may be called several times). Otherwise, play using default
   *    AudioCallback, or directly out the speakers
   * @param sampleRate if given, temporarily use this sample rate for this
   *    speech, then restore. If 0, use current sample rate.
   */
  AREXPORT virtual bool speak(const char *str, const char* voiceParams, ArRetFunctor2<bool, ArTypes::Byte2*, int>* audioOutputCB, unsigned short sampleRate = 0) = 0;

  /** Speaks the given text. 
   * @param str The text to speak.
   * @param voiceParams Voice selection criteria expression
   *    (implementation-specific)
   */
  AREXPORT virtual bool speak(const char *str, const char* voiceParams = NULL);

  /** Speaks the given string, using current voice and output settings,
   *  taking varargs and a format string (like printf)
   */
  AREXPORT virtual  bool speakf(const char* fmt, ...) = 0; 

  /** If any speech is currently ongoing, interrupt it. 
   */
  AREXPORT virtual void interrupt() = 0; 

  /** @return a functor for init() to use with ArSoundsQueue */
  AREXPORT ArRetFunctorC<bool, ArSpeechSynth>* getInitCallback();

  /** @return a functor for speak() to use with ArSoundsQueue */
  AREXPORT ArRetFunctor2C<bool, ArSpeechSynth, const char*, const char*>* getSpeakCallback(void) ;


  /** @return a functor for interrupt() */
  AREXPORT ArFunctorC<ArSpeechSynth>* getInterruptCallback();


  /** Instead of playing synthesized audio using the synthesizer's internal
   *  audio playback, call the given callback when a chunk of audio has
   *  been synthesized. Audio is passed to the callback in the first parameter
   *  as signed 16-bit samples (PCM16).  The sample rate is 16kHz but may be
   *  changed with setAudioSampleRate(). The second parameter is the number
   *  of samples.  The return value from the callback is ignored.
   */
  AREXPORT void setAudioCallback(ArRetFunctor2<bool, ArTypes::Byte2*, int>* cb);


  /** Change audio sample rate (Hz). Normal rate is 16000 Hz.
   *  Suggested values are 8000, 16000, or 44400
   */
  AREXPORT virtual void setAudioSampleRate(int rate) = 0;

  AREXPORT virtual int getAudioSampleRate() = 0;

  /** Lock, if neccesary */
  AREXPORT virtual void lock() { }
  /** Unlock, if neccesary */
  AREXPORT virtual void unlock() { }

  /** Set the current voice by name.
   *  Replaces fully any previous required voice criteria.
   * @sa getVoiceNames
   */
  AREXPORT virtual bool setVoice(const char* name) = 0;

  /** Get name of current voice, if set with setVoice (else returns NULL) */
  AREXPORT virtual const char* getCurrentVoiceName() = 0;

  /** Return a list of available voice names, if possible. */
  AREXPORT virtual std::list<std::string> getVoiceNames() = 0;

protected:
  ArRetFunctor2C<bool, ArSpeechSynth, const char*, const char*> mySpeakCB;
  ArRetFunctorC<bool, ArSpeechSynth> myInitCB;
  ArFunctorC<ArSpeechSynth> myInterruptCB;
  ArRetFunctor2<bool, ArTypes::Byte2*, int> *myAudioPlaybackCB; ///< If set, send audio to this callback instead of playing it directly
private:
  ArRetFunctorC<bool, ArSpeechSynth> myProcessConfigCB;
  char myConfigVoice[32];
  bool processConfig();
  void addVoiceConfigParam(ArConfig *config);

};



#endif
