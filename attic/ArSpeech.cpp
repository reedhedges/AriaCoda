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
#include "ArSpeech.h"
#include "ArConfig.h"
#include "ariaInternal.h"


AREXPORT ArSpeechSynth::ArSpeechSynth() : 
  mySpeakCB(this, &ArSpeechSynth::speak), 
  myInitCB(this, &ArSpeechSynth::init), 
  myInterruptCB(this, &ArSpeechSynth::interrupt),
  myAudioPlaybackCB(0),
  myProcessConfigCB(this, &ArSpeechSynth::processConfig)
{
  myProcessConfigCB.setName("ArSpeechSynth");
}


AREXPORT bool ArSpeechSynth::init()
{
  return true;
}

AREXPORT void ArSpeechSynth::addToConfig(ArConfig *config)
{
  addVoiceConfigParam(config);
  config->addProcessFileCB(&myProcessConfigCB, 100);
}

AREXPORT ArSpeechSynth::~ArSpeechSynth()
{
}

AREXPORT ArRetFunctorC<bool, ArSpeechSynth>* ArSpeechSynth::getInitCallback(void) 
{
  return &myInitCB;
}

AREXPORT ArRetFunctor2C<bool, ArSpeechSynth, const char*, const char*>* ArSpeechSynth::getSpeakCallback(void) 
{
  return &mySpeakCB;
}


AREXPORT ArFunctorC<ArSpeechSynth>*  ArSpeechSynth::getInterruptCallback() 
{
  return &myInterruptCB;
}

AREXPORT void ArSpeechSynth::setAudioCallback(ArRetFunctor2<bool, ArTypes::Byte2*, int>* cb)
{
  myAudioPlaybackCB = cb;
}


AREXPORT bool ArSpeechSynth::speak(const char* text, const char* voiceParams) {
  return speak(text, voiceParams, NULL, 0);
}
  


bool ArSpeechSynth::processConfig()
{
  setVoice(myConfigVoice);
  return true;
}

void ArSpeechSynth::addVoiceConfigParam(ArConfig *config)
{
  const char *current = getCurrentVoiceName();
  if(current)
  {
    strncpy(myConfigVoice, current, sizeof(myConfigVoice));
  }
  else
  {
    myConfigVoice[0] = 0;
  }
  std::string displayHint;
  std::list<std::string> voices = getVoiceNames();
  for(std::list<std::string>::const_iterator i = voices.begin(); i != voices.end(); i++)
  {
    if(i == voices.begin())
      displayHint = "Choices:";
    else
      displayHint += ";;";
    displayHint += *i;
  }
  config->addParam(
    ArConfigArg("Voice", myConfigVoice, "Name of voice to use for speech synthesis", sizeof(myConfigVoice)), 
    "Speech Synthesis",
    ArPriority::NORMAL,
    displayHint.c_str()
  );
}


