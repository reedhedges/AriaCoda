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
#ifndef AREXITERRORSOURCE_H
#define AREXITERRORSOURCE_H 

/// Small interface for obtaining exit-on-error information
/**
 * ArExitErrorSource may be implemented by classes that wish to 
 * provide information if and when they cause an erroneous 
 * application exit.  The use of this interface is entirely at 
 * the discretion of the application.  Aria does not invoke its
 * methods.
**/
class ArExitErrorSource 
{
public:
 
  /// Constructor
  ArExitErrorSource() {}

  /// Destructor
  virtual ~ArExitErrorSource() {}

  /// Returns a textual description of the error source.
  /**
   * @param buf a char array in which the method puts the output 
   * error description 
   * @param bufLen the int number of char's in the array
   * @return bool true if the description was successfully written;
   * false if an error occurred
  **/
  virtual bool getExitErrorDesc(char *buf, int bufLen) = 0;

  /// Returns a textual description of the error source intended for a user (it will be prefixed by something stating the action taking place)
  /**
   * @param buf a char array in which the method puts the output user 
   * error description 
   * @param bufLen the int number of char's in the array
   * @return bool true if the description was successfully written;
   * false if an error occurred
  **/
  virtual bool getExitErrorUserDesc(char *buf, int bufLen) = 0;

  /// Returns the error code used for the exit call
  /**
   * Ideally, the returned error code should be unique across all
   * error sources.  (Past implementations have the method spec and
   * body on a single line so that it's easily searchable...  Current
   * implementations have that OR this string on a line so that it'll
   * show up searchable easily still).
  **/
  virtual int getExitErrorCode() const = 0;

}; // end class ArExitErrorSource

#endif // AREXITERRORSOURCE_H
