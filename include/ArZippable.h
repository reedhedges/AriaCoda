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

#ifndef ARZIPPABLE_H
#define ARZIPPABLE_H

#include "ariaTypedefs.h"


/// Interface to access and modify a specified zip file.
/**
 * ArZippable is an abstract base class that defines the methods to add
 * and extract files from a zip file.
 * 
 * An ArZippable is expected to be used in a rather "atomic" manner.  It can
 * be opened in one of two modes (ZIP_MODE or UNZIP_MODE), and the available 
 * operations are subsequently restricted based on the mode:  
 * <ul>
 * <li>ZIP_MODE: addAll() and addFile()</li>
 * <li>UNZIP_MODE: getMemberInfoList(), extractAll(), and extractFile()</li>
 * </ul>
 * After the files are added to or extracted from the zip file, it should be
 * close()'ed.
 * 
**/
class ArZippable
{
public:

  // TODO Mutex for multithreaded access?

  // TODO Add more values and allow this to be configured?
  enum {
    CASE_INSENSITIVE = 2
  };

  /// Constructor
	AREXPORT ArZippable() 
  {}
	
  /// Destructor
	AREXPORT virtual ~ArZippable()
  {}

  /// Returns the complete file path name of the zip file
  AREXPORT virtual const char *getFilePathName() const  = 0;

  /// Modes in which the zip file may be opened
  enum ModeType {
    ZIP_MODE,     //< Mode for zipping, i.e. adding files to the zip file
    UNZIP_MODE    //< Mode for unzipping, i.e. extracting files from the zip file
  };


  /// Opens the zip file in the specified mode
  /**
   * @param mode the ModeType in which the file should be opened
   * @return bool true if the zip file was successfully opened in the specified
   * mode; false, otherwise.
  **/
  AREXPORT virtual bool open(ModeType mode) = 0;

  /// Closes the zip file
  AREXPORT virtual void close() = 0;

  /// Determines whether the zip file exists (according to the file path name)
  /**
   * @return bool true if the file exists and can be read; false, otherwise
  **/
  AREXPORT virtual bool exists() const = 0;


  /// Determines whether the zip file contains the specified member name
  /**
   * @param memberName the char * name of the member to be found
   * @return bool true if the zip file contains the specified member; false
   * otherwise
  **/
  AREXPORT virtual bool contains(const char *memberName) = 0;

  
  // TODO: Perhaps add a callback to be invoked when isOverwrite is false and
  // an overwrite condition is detected

  /// Extracts all of the files from the zip file.
  /**
   * The zip file must have been opened in UNZIP_MODE.
   *
   * @param destDir the char * name of the directory into which the extracted
   * files are to be placed
   * @param isOverwrite a bool set to true if an extracted file should overwrite
   * an existing file of the same name in the destination directory; false if 
   * it should be skipped
   * @param password an optional char * password to be applied to the extraction
   * @return bool set to true if all files were successfully extracted (or 
   * correctly skipped); false if an error occurred
  **/
  AREXPORT virtual bool extractAll(const char *destDir,
                                   bool isOverwrite,
                                   const char *password = NULL) = 0;

  /// Extracts the specified file from the zip file.
  /**
   * The zip file must have been opened in UNZIP_MODE.
   *
   * @param memberName the char * name of the file that should be extracted
   * @param destDir the char * name of the directory into which the extracted
   * files are to be placed
   * @param isOverwrite a bool set to true if an extracted file should overwrite
   * an existing file of the same name in the destination directory; false if 
   * it should be skipped
   * @param destFileName an optional char * name of the destination file to be
   * written; if NULL, then memberName is used
   * @param password an optional char * password to be applied to the extraction
   * @return bool set to true if the file was successfully extracted (or 
   * correctly skipped); false if an error occurred
  **/
  AREXPORT virtual bool extractFile(const char *memberName, 
                                    const char *destDir,
                                    bool isOverwrite,
                                    const char *destFileName = NULL,
                                    const char *password = NULL) = 0;


  /// Adds all of the files in the specified directory to the zip file.
  /**
   * The zip file must have been opened in ZIP_MODE.
   *
   * @param sourceDir the char * name of the directory from which the files
   * are to be added
   * @param isIncludeSubDirs a bool set to true if all files in all subdirectories
   * should also be added to the zip file; false otherwise
   * @param isOverwrite a bool set to true if a file to be added should overwrite
   * an existing zip file member of the same name; false if it should be skipped
   * @param password an optional char * password to be applied to the addition
   * @param flush an optional int indication of how the zip file should be flushed
   * when writing (for error recovery); implementation dependent
   * @return bool set to true if all files were successfully added (or 
   * correctly skipped); false if an error occurred
  **/
  AREXPORT virtual bool addAll(const char *sourceDir,
                               bool isIncludeSubDirs,
                               bool isOverwrite,
                               const char *password = NULL,
                               int flush = 0) = 0;

  /// Adds the specified file to the zip file.
  /**
   * The zip file must have been opened in ZIP_MODE.
   *
   * @param memberName the char * name of the member to be added in the zip file
   * @param isOverwrite a bool set to true if a file to be added should overwrite
   * an existing zip file member of the same name; false if it should be skipped
   * @param sourceFileName the char * name of the file to be added
   * @param password an optional char * password to be applied to the addition
   * @param flush an optional int indication of how the zip file should be flushed
   * when writing (for error recovery); implementation dependent
   * @return bool set to true if all files were successfully added (or 
   * correctly skipped); false if an error occurred
  **/ 
  AREXPORT virtual bool addFile(const char *memberName,
                                bool isOverwrite,
                                const char *sourceFileName,
                                const char *password,
                                int flush = 0) = 0;


private:

  /// Disabled copy constructor
  ArZippable( const ArZippable & );
	/// Disabled assignment operator.
  ArZippable &operator=( const ArZippable & );

protected:


}; // end class ArZippable


#endif // ARZIPPABLE_H

