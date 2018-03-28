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
/*! \file ArMD5Calculator.h
 *  \brief Contains the ArMD5Calculator class.
 *  \date 06/27/08
 *  \author K. Cunningham
**/
#ifndef ARMD5CALCULATOR_H
#define ARMD5CALCULATOR_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"

#include "md5.h"

/// Calculates the MD5 checksum when reading or writing a text file using ArFunctors.
/**
 * ArMD5Calculator is a small helper class used to calculate MD5 checksums
 * on text files.  It contains a static method that will calculate the 
 * checksum of a given file, independently performing all I/O.  
 * 
 * The calculator also contains a functor that can be used to calculate
 * a checksum interactively.  For example, it can accumulate the checksum 
 * while a file is being written using a writeToFunctor method.  If the 
 * optional second functor is specified, then it will be invoked after the 
 * checksum functor. 
 *
 * @ingroup UtilityClasses
**/
class ArMD5Calculator {

public:

	enum {
		DIGEST_LENGTH = 16, ///< Number of bytes in the checksum buffer
    DISPLAY_LENGTH = (DIGEST_LENGTH * 2) + 1 ///< Number of characters needed to display the checksum
	};
   
  // ---------------------------------------------------------------------------
  // Static Methods
  // ---------------------------------------------------------------------------
 
  /// Converts the given checksum buffer to a displayable text string
  /**
   * @param digestBuf a pointer to the byte array that contains the checksum
   * @param digestLength the length of the disgestBuf; should be DIGEST_LENGTH
   * @param displayBuf a pointer to the output text buffer that will contain the 
   * displayable text string
   * @param displayLength the length of the displayBuf; should be at least 
   * DISPLAY_LENGTH
  **/
	AREXPORT static void toDisplay(const unsigned char *digestBuf,
													       size_t digestLength,
													       char *displayBuf,
													       size_t displayLength);

  /// Calculates the checksum for the specified file.
  /**
   * @param fileName the name of the file of which to calculate the checksum
   * @param md5DigestBuffer a pointer to the output buffer in which to store 
   * the calculated checksum
   * @param md5DigestBufferLen the length of the md5DigestBuffer; should be
   * DIGEST_LENGTH
   * @return bool true if the file was successfully opened and the checksum
   * calculated; false, otherwise
  **/
  AREXPORT static bool calculateChecksum(const char *fileName,
                                         unsigned char *md5DigestBuffer,
                                         size_t md5DigestBufferLen);

  // ---------------------------------------------------------------------------
  // Instance Methods
  // ---------------------------------------------------------------------------

  /// Creates a new calculator, with an optional functor.
  /**
   * @param secondFunctor the optional functor to be invoked on the current
   * text string after its checksum has been calculated
  **/
	AREXPORT ArMD5Calculator(ArFunctor1<const char*> *secondFunctor = NULL); 

  /// Destructor
	AREXPORT ~ArMD5Calculator();


  /// Resets the calculator so that a new checksum can be calculated
	AREXPORT void reset();

  /// Calculates the checksum for the given text line, and accumulates the results.
	AREXPORT void append(const char *str);

  /// Returns a pointer to the internal buffer that accumulates the checksum results.
	AREXPORT unsigned char *getDigest();


  /// Returns the internal functor used to calculate the checksum
  /**
   * If the calculator is being used interactively with writeToFunctor, then 
   * this is the functor to use.  It calls append on each text line which
   * accumulates the checksum and calls the second functor if necessary.
  **/
	AREXPORT ArFunctor1<const char *> *getFunctor();

  /// Returns the optional secondary functor to be called on each text line.
	AREXPORT ArFunctor1<const char *> *getSecondFunctor();

  /// Sets the optional secondary functor to be called on each text line.
	AREXPORT void setSecondFunctor(ArFunctor1<const char *> *secondFunctor);

private:

  /// Functor that accumulates the checksum
  ArFunctor1C<ArMD5Calculator, const char*> myFunctor;
  /// Optional secondary functor to be invoked on each text line
  ArFunctor1<const char*> *mySecondFunctor;
  /// State of the md5 library
  md5_state_t myState;
  /// Buffer in which to store the md5 results
  md5_byte_t myDigest[DIGEST_LENGTH];

  /// Whether the calculator is finished, i.e. the md5_finish method has been called.
	bool myIsFinished;

}; // end class ArMD5Calculator

#endif // ARMD5CALCULATOR_H

