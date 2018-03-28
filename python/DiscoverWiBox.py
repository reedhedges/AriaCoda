"""
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
"""

# module to search network and find Lantronix devices

import socket
import sys
import time

HOST = ''
PORT = 30718
 

# Return list of found addresses, or
def search(searchtime=4, verbose=False):
 
  try:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    s.bind((HOST,PORT))
    s.settimeout(searchtime/4)
  except socket.error as e:
    if verbose: print('DiscoverWiBox: Failed to create and configure socket: ', e)
    return
   
  msg = chr(0)+chr(0)+chr(0)+chr(0xf6)

  try:
    if verbose: print('DiscoverWiBox: sending broadcast query')
    s.sendto(msg, ('<broadcast>', PORT))
  except socket.error as e:
    if verbose: print('DiscoverWiBox: Error Code : ' + str(e[0]) + ' Message ' + e[1])
    return

  t0 = time.time() 
  t1 = t0
  addresses = []
  data = []
    
  while (t1-t0 < searchtime):
    try:
      d, address = s.recvfrom(128)
      packet = ''.join('%02x' % ord(c) for c in d)
      if packet[6:8] == 'f7':
        addresses.append(address[0])
        data.append(packet[48:60])
    except Exception as e:
      s.close()
      # ignore errors reading
    
    time.sleep(1)
    t1 = time.time()

  if verbose: print('%d devices found ' % len(addresses))
  
  # TODO what to do with extra data? is it a name?

  # Test:
  #addresses.append('1.2.3.4')
  #addresses.append('1.2.3.5')

  return addresses

def _find_getch():
    try:
        import termios
    except ImportError:
        # Non-POSIX. Return msvcrt's (Windows') getch.
        import msvcrt
        return msvcrt.getch

    # POSIX system. Create and return a getch that manipulates the tty.
    import sys, tty
    def _getch():
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

    return _getch

getch = _find_getch()

# Search for wiboxes, present interactive menu to user to choose one, return 
# chosen address or None if cancelled or no boxes found. There is a limit of up
# to 8 boxes. 'localhost' is always available as the first option.
def choose(logprefix = None, verbose=False):
  if logprefix == None:
    logprefix = ''
  else:
    logprefix += ': '
  print('%sSearching Network for WiBox Interfaces...' % (logprefix))
  boxes = search(3, verbose)
  if boxes == None or len(boxes) == 0:
    print('%s: No WiBoxes found.' % (logprefix))
    return None
  boxes.insert(0, 'localhost')
  del boxes[9:]
  for i, n in enumerate(boxes):
    print('\t%d: %s ' % (i+1, n))
  print 'Enter Number to Choose Interface: ', 
  i = int(getch())
  print('')
  return boxes[i-1]


if __name__ == '__main__':
  #l = search(5, True)
  #for i, n in enumerate(l):
  #  print '%s ' % (n)
  a = choose()
  print('You chose ', a)
  
  
  #raw_input('Press the Enter Key to Exit\n')


