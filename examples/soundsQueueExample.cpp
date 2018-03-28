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

/** @example soundsQueueExample.cpp  This program demonstrates the sound output queue ArSoundsQueue
 *
 * Shows how to operate the sounds queue and add WAV files to play.
 *
 * You may specify up to 10 file names on the command line.
 * This example only demonstrates using the sounds queue for
 * WAV file playback. For demonstration of using the sounds
 * queue for speech synthesis, see the examples provided with
 * either the ArSpeechSynth_Cepstral or ArSpeechSynth_Festival 
 * libraries.
 *
 * Usage:
 *    soundQueue <i>&lt;wav file names&gt;</i>
 */

#include "Aria.h"
#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArSoundsQueue.h"
#include "ArSoundPlayer.h"
#include <iostream>
#include <vector>

using namespace std;

void queueNowEmpty() {
  printf("The sound queue is now empty.\n");
}

void queueNowNonempty() {
  printf("The sound queue is now non-empty.\n");
}

bool no() {
  // just a false tautology
  return false;
}

int main(int argc, char** argv) {
  Aria::init();
  //ArLog::init(ArLog::StdErr, ArLog::Verbose);

  // Create the sound queue.
  ArSoundsQueue soundQueue;

  // Set WAV file callbacks 
  soundQueue.setPlayWavFileCallback(ArSoundPlayer::getPlayWavFileCallback());
  soundQueue.setInterruptWavFileCallback(ArSoundPlayer::getStopPlayingCallback());

  // Notifications when the queue goes empty or non-empty.
  soundQueue.addQueueEmptyCallback(new ArGlobalFunctor(&queueNowEmpty));
  soundQueue.addQueueNonemptyCallback(new ArGlobalFunctor(&queueNowNonempty));

  // Run the sound queue in a new thread
  soundQueue.runAsync();

  // Get WAV file names from command line
  if(argc < 2) 
  {
    cerr << "Usage: " << argv[0] << " <up to ten WAV sound file names...>\n";
    Aria::exit(-1);
  }
  std::vector<const char*> filenames;
  for(int i = 1; i < min(argc, 11); i++) 
  {
    filenames.push_back(argv[i]);
  }

  // This functor can be used to cancel all sound playback until removed
  ArGlobalRetFunctor<bool> dontPlayItem(&no);
  
  while(Aria::getRunning())
  {
    cout << "Queue is " << 
      string(soundQueue.isPaused()?"paused":(soundQueue.isPlaying()?"playing":"ready")) 
      << ", with " << soundQueue.getCurrentQueueSize() << " pending sounds." << endl
      << "Enter a command followed by the enter key:\n"
      << "\tp\trequest pause state (cumulative)\n"
      << "\tr\trequest resume state (cumulative)\n" 
      << "\ti\tinterrupt current sound\n"
      << "\tc\tclear the queue\n"
      << "\tC\tclear priority < 4 from the queue.\n"
      << "\tn\tAdd " << filenames[0] << " to the queue, but with a condition callback to prevent it playing.\n"
      << "\tv\tAdjust volume -50%\n"
      << "\tV\tAdjust volume +50%\n"
      << "\to\tAdjust volume -100%\n"
      << "\tO\tAdjust volume +100%\n"
      << "\tl\tAdjust volume -200%\n"
      << "\tL\tAdjust volume +200%\n"
      << "\t-\tSet volume adjustment to normal level\n"
      ;
    for(size_t i = 0; i < filenames.size(); i++)
      cout << "\t" << i << "\tadd " << filenames[i] << " to the queue\n";
    cout << "\tq\tquit\n\n";

    int c = getchar();
    if(c == '\n')
      continue;
    switch(c)
    {
      case 'p': soundQueue.pause(); break;
      case 'r': soundQueue.resume(); break;
      case 'i': soundQueue.interrupt(); break;
      case 'q': soundQueue.stop(); ArUtil::sleep(100); Aria::exit(0);
      case 'c': soundQueue.clearQueue(); break;
      case 'C': soundQueue.removePendingItems(4); break;
      case 'n': 
      {
        cout << "Adding \"" << filenames[0] << "\" but with a condition callback that will prevent it from playing...\n";
        ArSoundsQueue::Item item = soundQueue.createDefaultFileItem(filenames[0]);
        item.playbackConditionCallbacks.push_back(&dontPlayItem);
        soundQueue.addItem(item);
        break;
      }
      case 'v': ArSoundPlayer::setVolumePercent(-50.0); break;
      case 'V': ArSoundPlayer::setVolumePercent(50.0); break;
      case 'o': ArSoundPlayer::setVolumePercent(-100.0); break;
      case 'O': ArSoundPlayer::setVolumePercent(100.0); break;
      case 'l': ArSoundPlayer::setVolumePercent(-200.0); break;
      case 'L': ArSoundPlayer::setVolumePercent(200.0); break;
      case '-': ArSoundPlayer::setVolumePercent(0.0); break; 
      default:
        if(filenames.size() > 0 && c >= '0' && c <= '9')
        {
          size_t i = c - '0';
          if(i < filenames.size()) 
          {
            cout << "Adding \"" << filenames[i] << "\" to the queue...\n"; 
            soundQueue.play(filenames[i]);
          } 
        }
    }
  }
  cout << "ended.\n";
  Aria::exit(0);
  return 0;
}


