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
/* 
 * Contains utility classes for the ArMap class.
 *
 * \author K. Cunningham
 * 
 * This file contains miscellaneous helper classes for the ArMap class.  
 * These are primarily related to tracking changes to the map and include
 * the following:
 *
 *  - ArMapId : The unique identifier for an Aria map.
 *
 *  - ArMapFileLine : The data regarding a text line in a map file; this  
 *    includes the line number and text.  
 *
 *  - ArMapFileLineGroup : A semantic parent / child relationship between 
 *    text lines in a map file. 
 *
 *  - ArMapFileLineSet : A list of map file line groups to be compared.  
 *    Contains helper methods to determines lines that have been added or 
 *    deleted.
 *
 *  - ArMapChangeDetails : A collection of ArMapFileLineSets that describes 
 *    all of the changes that were made to an Aria map.
 *
 *  - ArMapFileLineSetWriter : An output functor that is used to populate an
 *    ArMapFileLineSet.
 *
 *  - ArMapChangedHelper : A collection of callbacks and methods to invoke 
 *    them after the Aria map has been changed.
 */

#ifndef ARMAPUTILS_H
#define ARMAPUTILS_H

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ariaTypedefs.h"
#include "ariaUtil.h"


class ArArgumentBuilder;
class ArBasePacket;

// ============================================================================
// ArMapId
// ============================================================================

/// Enapsulates the data used to uniquely identify an Aria map
/**
 * ArMapId is a small helper class that contains all of the information 
 * needed to uniquely identify an Aria map.  In addition to the standard
 * file attributes (such as filename, size, and timestamps), it 
 * contains the name of the originating source robot or server.  It 
 * also contains the checksum of the file contents.  This checksum is
 * currently calculated by the ArMD5Calculator class.  All of the 
 * data should allow one to determine whether two map files are the same 
 * with a reasonable degree of certainty.
 * <p>
 * The ArMapId class is not thread-safe.
 *
 * @see ArMD5Calculator
**/
class ArMapId {

public:
  // --------------------------------------------------------------------------
  // Static Methods
  // --------------------------------------------------------------------------

  /// Given a local file name, creates the map ID
  /**
   * This method calculates the checksum of the specified file
   * @param fileName the const char * name of the file for which to create the 
   * map ID
   * @param mapIdOut a pointer to the map ID to be filled in with the results
   * @return bool true if the file was found and the map ID created; false, 
   * otherwise.
  **/
  AREXPORT static bool create(const char *fileName,
                              ArMapId *mapIdOut);

  /// Inserts the given map ID into a network packet
  /**
   * The format of the map ID in the network packet is as follows:
   * <pre>
   *   string:  source robot or server name
   *   string:  map file name
   *   uByte4:  number of bytes in the checksum
   *   data  :  checksum, included only if the number of bytes in the checksum 
   *            is greater than 0
   *   uByte4:  file size
   *   byte4 :  file last modified time
   * </pre>
   * @param mapId the ArMapId to be inserted into the packet
   * @param packetOut the ArBasePacket * to be modified
  **/
  AREXPORT static bool toPacket(const ArMapId &mapId,
                                ArBasePacket *packetOut);
                                
  /// Extracts a map ID from the given network packet
  /**
   * @param packetIn the ArBasePacket from which to read the map ID
   * @param mapIdOut a pointer to the ArMapId to be filled in with the results
   * @return bool true if the map ID was successfully read from the given
   * packet, false otherwise.
   * @see toPacket for a description of the packet format
  **/
  AREXPORT static bool fromPacket(ArBasePacket *packetIn,
                                  ArMapId *mapIdOut);

  // --------------------------------------------------------------------------
  // Instance Methods
  // --------------------------------------------------------------------------
  
  /// Default contructor creates a null map ID.
	AREXPORT ArMapId();
	
	/// Creates a map ID with the given attributes.
	/**
	 * @param sourceName the const char * name of the robot or server 
	 * from which the map originated
	 * @param fileName the const char * name of the map file
	 * @param checksum the unsigned char * buffer that contains the 
	 * file checksum
	 * @param checksumLength the size_t length of the checksum buffer
	 * @param size the long int number of bytes in the map file
	 * @param timestamp the time_t last modified time of the map file
	 * @see ArMD5Calculator
	**/
	AREXPORT ArMapId(const char *sourceName,
                   const char *fileName,
						       const unsigned char *checksum,
                   size_t checksumLength,
						       long int size,
						       const time_t timestamp);
 
  /// Copy constructor
	AREXPORT ArMapId(const ArMapId &other); 
	
	/// Assignment operator
	AREXPORT ArMapId &operator=(const ArMapId &other);
	
	/// Destructor
	AREXPORT ~ArMapId();


  // --------------------------------------------------------------------------
  // Getters
  // --------------------------------------------------------------------------
  
  /// Returns whether this map ID is null 
  AREXPORT bool isNull() const;

  /// Returns the string name of the originating robot or server
	AREXPORT const char *getSourceName() const;
	
	/// Returns the string name of the file 
	/**
	 * TODO: Does this include path name??
	**/
	AREXPORT const char *getFileName() const;
	
	/// Returns a pointer to the buffer that contains the checksum
	AREXPORT const unsigned char *getChecksum() const;
	/// Returns the length of the checksum
  AREXPORT size_t getChecksumLength() const;
	
	/// Returns the checksum in a human readable string format
  AREXPORT const char *getDisplayChecksum() const;
	
	/// Returns the number of bytes in the map file
  AREXPORT long int getSize() const;
  /// Returns the last modified time of the file
	AREXPORT time_t getTimestamp() const;

 
  // --------------------------------------------------------------------------
  // Setters
  // --------------------------------------------------------------------------
  
  /// Clears this map ID, i.e. sets it to null
  AREXPORT void clear();
   
  /// Sets the name of the source robot or server that originated the map file
	AREXPORT void setSourceName(const char *sourceName);
	/// Sets the map file name
	AREXPORT void setFileName(const char *fileName);
	/// Sets the checksum of the map file
	AREXPORT void setChecksum(const unsigned char *checksum,
													  size_t checksumLen);
  /// Sets the number of bytes in the map file
	AREXPORT void setSize(long int size);
	/// Sets the last modified time of the map file
	AREXPORT void setTimestamp(const time_t &timestamp);

  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------
 
  /// TODO Think that this is the same as operator==
  AREXPORT bool isSameFile(const ArMapId &other) const;
  
  /// Returns whether the source and file names are identical
  AREXPORT bool isVersionOfSameFile(const ArMapId &other) const;
  
  /// Returns true if the timestamp is valid; false if it's a special 'not-set' indicator 
  AREXPORT bool isValidTimestamp() const;
  
  /// Returns whether the two map IDs are equivalent
  /**
   * Note that if either map ID specifies a NULL timestamp, then the timestamp
   * will not be used for comparison purposes.
  **/
	AREXPORT friend bool operator==(const ArMapId & id1, const ArMapId & id2);
	
  /// Returns whether the two map IDs are not equal
  /**
   * Note that if either map ID specifies a NULL timestamp, then the timestamp
   * will not be used for comparison purposes.
  **/
	AREXPORT friend bool operator!=(const ArMapId & id1, const ArMapId & id2);

  /// Writes the map ID to the output log file, with the specified prefix /header.
  AREXPORT void log(const char *prefix) const;

protected:

  /// Name of the source robot or server from which the map file originated
	std::string mySourceName;
	/// Name of the map file
	std::string myFileName;
	/// Buffer that contains the checksum of the map file
  unsigned char *myChecksum;
  /// Length of the buffer that contains the checksum of the map file
  size_t myChecksumLength;
 
  /// Buffer that contains the checksum in human readable format
  mutable char *myDisplayChecksum;
  /// Length of the displayable checksum buffer
  mutable size_t myDisplayChecksumLength;
  
  /// Number of bytes in the map file
  long int mySize;
  /// Last modified time of the map file
  time_t myTimestamp;

}; // end class ArMapId


#ifndef SWIG


// ============================================================================
// ArMapFileLine, etc
// ============================================================================

/// Encapsulates the data regarding a text line in a map file.
/**
 * ArMapFileLine is the building block for map change comparisons.  It is 
 * simply the line number and associated text.
 * @internal
 * @swigomit
**/
class ArMapFileLine {

public:

  /// Default constructor
  ArMapFileLine() :
    myLineNum(0),
    myLineText()
  {}

  /// Constructor which sets the line number and text 
  ArMapFileLine(int lineNum,
                const char *lineText) :
    myLineNum(lineNum),
    myLineText((lineText != NULL) ? lineText : "")
  {}

  /// Copy constructor
  ArMapFileLine(const ArMapFileLine &other) :
    myLineNum(other.myLineNum),
    myLineText(other.myLineText)
  {}

  /// Assignment operator
  ArMapFileLine &operator=(const ArMapFileLine &other) 
  {
    if (this != &other) {
      myLineNum  = other.myLineNum;
      myLineText = other.myLineText;
    }
    return *this;
  }

  /// Destructor
  ~ArMapFileLine() 
  {}

  /// Returns the line number of the map file line  
  int getLineNum() const {
    return myLineNum;
  }
  /// Returns the text of the map file line
  const char *getLineText() const {
    return myLineText.c_str();
  }

	friend inline bool operator< ( const ArMapFileLine & line1, const ArMapFileLine & line2 );
  friend struct ArMapFileLineCompare;

protected:

  int myLineNum;          ///< Line number
  std::string myLineText; ///< Line text

}; // end class ArMapFileLine


/// Determines whether the first ArMapFileLine is less than the second
/**
 * Line1 is less than Line2 if its line number is less than that of Line2.
 * If the two line nubmers are equal, then Line1 is less than Line2 if its
 * text is less.
 * @internal
 * @swigomit
**/
inline bool operator<(const ArMapFileLine & line1, const ArMapFileLine & line2 )
{
    bool b = (line1.myLineNum < line2.myLineNum);
    if (!b) {
      if (line1.myLineNum == line2.myLineNum) {
        b = (line1.myLineText < line2.myLineText);
      }
    }
    return b;
}

/// Comparator used to sort ArMapFileLine objects.
/**
 * @internal
 * @swigomit
 */
struct ArMapFileLineCompare : public std::binary_function<const ArMapFileLine &, const ArMapFileLine &, bool> 
{
  /// Returns true if line1 is less than line2; false, otherwise.
  bool operator()(const ArMapFileLine &line1,
                  const ArMapFileLine &line2) 
  { 
    bool b = (line1.myLineNum < line2.myLineNum);
    if (!b) {
      if (line1.myLineNum == line2.myLineNum) {
        b = (line1.myLineText < line2.myLineText);
      }
    }
    return b;
  }
}; // end struct MapFileLineCompare

// ============================================================================
// ArMapFileLineGroup
// ============================================================================

/// A group of map file lines which have a logical parent / child relationship.
/**
 * Some of the text lines in a map file may have a parent / child relationship.
 * For example, the text line that starts a route is the parent of the following
 * lines that describe the contents of the route.  This class is used to 
 * express this relationship.  Note that there is currently no concept of 
 * a grandparent relationship in the map file lines.
 * @internal
 * @swigomit
**/
class ArMapFileLineGroup {

public:

  /// Contructs a new group with the specified parent.
  ArMapFileLineGroup(const ArMapFileLine &parentLine) :
    myParentLine(parentLine),
    myChildLines()
  {}

  /// Destructor
  ~ArMapFileLineGroup() {}

  /// Returns a pointer to the parent map file line.
  ArMapFileLine *getParentLine() {
    return &myParentLine;
  }
  /// Returns a pointer to the list of child map file lines.
  std::vector<ArMapFileLine> *getChildLines() {
    return &myChildLines;
  }

	friend inline bool operator<(const ArMapFileLineGroup & line1, 
                               const ArMapFileLineGroup & line2);
  friend struct ArMapFileLineGroupCompare;

  /// Writes the group to the Aria log.
  void log();

public: // users may access class members directly

  ArMapFileLine myParentLine; ///< The map file line that is the parent of the group
  std::vector<ArMapFileLine> myChildLines; ///< A list of child map file lines

}; // end class ArFileLineGroup

/// Determines whether group1's parent text is less than group2's parent text.
/** @internal  
 *  @swigomit
 */
inline bool operator<(const ArMapFileLineGroup & group1, 
                      const ArMapFileLineGroup & group2)
{
  bool b = (strcmp(group1.myParentLine.getLineText(), 
                   group2.myParentLine.getLineText()) < 0);
  return b;
}


/// Comparator used to sort groups in order of ascending parent text.
/** @internal  
 *  @swigomit
 */
struct ArMapFileLineGroupCompare : public std::binary_function<const ArMapFileLineGroup &, 
                                                               const ArMapFileLineGroup &, 
                                                               bool> {
  bool operator()(const ArMapFileLineGroup &group1,
                  const ArMapFileLineGroup &group2) 
  { 
    bool b = (strcmp(group1.myParentLine.getLineText(), 
                     group2.myParentLine.getLineText()) < 0);
    return b;
  }
  virtual ~ArMapFileLineGroupCompare() {}
}; // end struct ArMapFileLineGroupCompare


/// Comparator used to sort groups in order of ascending parent line number.
/** @internal  
 *  @swigomit
 */
struct ArMapFileLineGroupLineNumCompare : 
                  public std::binary_function<const ArMapFileLineGroup &, 
                                              const ArMapFileLineGroup &, 
                                              bool> {
  bool operator()(const ArMapFileLineGroup &group1,
                  const ArMapFileLineGroup &group2) 
  { 
    bool b = (group1.myParentLine.getLineNum() < 
              group2.myParentLine.getLineNum());
    return b;
  }
  virtual ~ArMapFileLineGroupLineNumCompare() {}
}; // end struct ArMapFileLineGroupLineNumCompare


// ============================================================================
// ArMapFileLineSet
// ============================================================================

/// A set of map file line groups.
/**
 * ArMapFileLineSet is a container of ArMapFileLineGroup objects -- i.e. a 
 * set of parent/child text lines in an Aria map.  The class has been 
 * defined to enable comparisons of map file versions.  Each section of 
 * an Aria map is written to an ArMapFileLineSet and then the standard
 * algorithm set_difference can be used to determine changes within a 
 * section.  The static method ArMapFileLineSet::calculateChanges()
 * performs this comparison.
 * @swigomit
 * @internal
**/
class ArMapFileLineSet : public std::vector<ArMapFileLineGroup>
{
public:

  // ---------------------------------------------------------------------------
  /// Determines the changes that have been made to a set of ArMapFileLines
  /**
   * @param origLines the ArMapFileLineSet that contains the original map file
   * lines
   * @param newLines the ArMapFileLineSet that contains the new map file lines
   * @param deletedLinesOut a pointer to the ArMapFileLineSet that is populated
   * with lines that have been deleted, i.e. that are contained in origLines 
   * but not in newLines
   * @param addedLinesOut a pointer to the ArMapFileLineSet that is populated
   * with lines that have been added, i.e. that are contained in newLines but
   * not in origLines
   * @param isCheckChildren a bool set to true if child lines should also be 
   * checked; if false, then only the parent lines are checked
   * @return bool true if the changes were successfully determined; false if
   * an error occurred
  **/
  static bool calculateChanges(ArMapFileLineSet &origLines,
                               ArMapFileLineSet &newLines,
                               ArMapFileLineSet *deletedLinesOut,
                               ArMapFileLineSet *addedLinesOut,
                               bool isCheckChildren = true);

  // ---------------------------------------------------------------------------

  /// Constructor 
  ArMapFileLineSet() {}

  /// Copy constructor
  ArMapFileLineSet(const ArMapFileLineSet &other) :
    std::vector<ArMapFileLineGroup>(other)
  {}

  /// Assignment operator
  ArMapFileLineSet &operator=(const ArMapFileLineSet &other) 
  {
    if (this != &other) {
      std::vector<ArMapFileLineGroup>::operator =(other);
    }
    return *this;
  }

  /// Destructor
  virtual ~ArMapFileLineSet() {}

  /// Searches the set for the given parent line.
  AREXPORT iterator find(const ArMapFileLine &groupParent);

  /// Writes the set to the Aria output log.
  AREXPORT void log(const char *prefix);

}; // end class ArMapFileLineSet


// ============================================================================
// ArMapChangeDetails
// ============================================================================

/// Helper class used to track changes to an Aria map.
/**
 * ArMapChangeDetails is a simple helper class that is used to track changes
 * to an Aria map.  These changes are determined based on set comparisons 
 * (and thus everything in the map must be ordered in a repeatable manner).
 *
 * The class itself provides very little functionality.  It is basically 
 * a repository of change information that may be accessed directly by the 
 * application. The methods return pointers to the internal data members 
 * which may be directly manipulated.  There is no error checking, 
 * thread-safety, etc.  The class's use and scope is expected to be very 
 * limited (to the Aria map and related classes).
 * @swigomit
 * @internal
**/
class ArMapChangeDetails
{
public:

	enum MapLineChangeType {
		DELETIONS,  ///< Lines that have been deleted from the Aria map
		ADDITIONS,  ///< Lines that have been added to the Aria map 
    LAST_CHANGE_TYPE = ADDITIONS  ///< Last value in the enumeration
	};

  enum {
    CHANGE_TYPE_COUNT = LAST_CHANGE_TYPE + 1 ///< Number of map change types
  };

  /// Constructor
  AREXPORT ArMapChangeDetails();

  /// Copy constructor
  AREXPORT ArMapChangeDetails(const ArMapChangeDetails &other);

  /// Assignment operator
  AREXPORT ArMapChangeDetails &operator=(const ArMapChangeDetails &other);

  /// Destructor
  AREXPORT ~ArMapChangeDetails();

  // ---------------------------------------------------------------------------
  // Map ID Methods
  // ---------------------------------------------------------------------------

  /// Returns the original pre-change map ID
  /**
   * @param mapIdOut a pointer to the ArMapId to be returned 
   * @return bool true if the map ID was successfully retrieved; false if an 
   * error occurred
  **/
  AREXPORT bool getOrigMapId(ArMapId *mapIdOut);

  /// Sets the original pre-change map ID
  /**
   * @param mapId the ArMapId of the map before it was changed
  **/
  AREXPORT void setOrigMapId(const ArMapId &mapId);

  /// Returns the new post-change map ID
  /**
   * @param mapIdOut a pointer to the ArMapId to be returned 
   * @return bool true if the map ID was successfully retrieved; false if an 
   * error occurred
  **/
  AREXPORT bool getNewMapId(ArMapId *mapIdOut);

  /// Sets the new post-change map ID
  /**
   * @param mapId the ArMapId of the map after it was changed
  **/
  AREXPORT void setNewMapId(const ArMapId &mapId);

  // ---------------------------------------------------------------------------
  // Change Info
  // ---------------------------------------------------------------------------

  /// Returns a pointer to the list of scan types that have been modified
  /**
   * This list includes the scan types in the map before and after it was changed.
   * Scan types usually won't be added or removed, but it may occur when one 
   * map is inserted into another.
   * A pointer to the actual internal attribute is returned.
   * @return a pointer to the list of scan type identifier strings
  **/
  AREXPORT std::list<std::string> *getScanTypes();

  /// Returns a pointer to the data points that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * points are to be returned
   * @param scanType the const char * identifier of the scan for which the points
   * are to be returned; must be non-NULL
   * @return a non-NULL pointer to the vector of ArPose's that have been changed
  **/
  AREXPORT std::vector<ArPose> *getChangedPoints(MapLineChangeType change,
                                                 const char *scanType);

  /// Returns a pointer to the data line segments that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * line segments are to be returned
   * @param scanType the const char * identifier of the scan for which the line
   * segments are to be returned; must be non-NULL
   * @return a non-NULL pointer to the vector of ArLineSegment's that have been changed
  **/
  AREXPORT std::vector<ArLineSegment> *getChangedLineSegments
                                                (MapLineChangeType change,
                                                 const char *scanType);

  /// Returns a pointer to the header lines that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * scan header lines are to be returned
   * @param scanType the const char * identifier of the scan for which the changes
   * are to be returned; must be non-NULL
   * @return a non-NULL pointer to the ArMapFileLineSet that describes the changes
  **/
  AREXPORT ArMapFileLineSet *getChangedSummaryLines(MapLineChangeType change,
                                                 const char *scanType);

  /// Returns a pointer to the map supplement lines that have been changed 
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * supplement lines are to be returned
   * @return a non-NULL pointer to the ArMapFileLineSet that describes the changes
  **/
  AREXPORT ArMapFileLineSet *getChangedSupplementLines(MapLineChangeType change);

  /// Returns a pointer to the map object (i.e. Cairn) lines that have been changed 
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * map object lines are to be returned
   * @return a non-NULL pointer to the ArMapFileLineSet that describes the changes
  **/
  AREXPORT ArMapFileLineSet *getChangedObjectLines(MapLineChangeType change);

  /// Returns a pointer to the specified info lines that have been changed
  /**
   * @param infoName the int identifier of the info type to be returned
   * @param change the MapLineChangeType that indicates whether added or removed
   * map info lines are to be returned
   * @return a non-NULL pointer to the ArMapFileLineSet that describes the changes
  **/
  AREXPORT ArMapFileLineSet *getChangedInfoLines(const char *infoName, 
                                                 MapLineChangeType change);


  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Determines whether the change details is empty (i.e. no actual changes were made to the map)
  AREXPORT bool isEmpty() const;


  /// Returns a list of the info types that have been changed
  /**
   * This method searches the internal changed info lines and returns the info
   * types that have non-empty change information.
   * @return list of the string info names that have non-empty changes
  **/
  AREXPORT std::list<std::string> findChangedInfoNames() const;

  /// Determines whether the given argument for the specified info type is a "child".
  /**
   * "Child" arguments have a non-NULL parent.  For example, an argument that
   * represents a robot task may be a child of one that defines a macro.  When
   * a child argument is changed, then the parent and all of its children 
   * must be included in the change details.  Note that currently only one
   * level of parent-ness is supported (i.e. there are no grandparents).
  **/
  AREXPORT bool isChildArg(const char *infoName,
                           ArArgumentBuilder *arg) const;

  /// Determines whether the given arg 0 for the info type is a "child".
  AREXPORT bool isChildArg(const char *infoName,
                           const char *arg0Text) const;

  /// Creates a map of args that are considered to be a "child" of another arg.
  AREXPORT void createChildArgMap();

  /// Writes the change details to the Aria log.
  AREXPORT void log();

  /// Locks the change details for multithreaded access.
  AREXPORT void lock();
  //// Unlocks the change details for multithreaded access.
  AREXPORT void unlock();

protected:

  /// Summary of changes for a specific map scan type.
  struct ArMapScanChangeDetails {

    std::vector<ArPose> myChangedPoints[CHANGE_TYPE_COUNT];
    std::vector<ArLineSegment> myChangedLineSegments[CHANGE_TYPE_COUNT];

    ArMapFileLineSet myChangedSummaryLines[CHANGE_TYPE_COUNT];

    ArMapScanChangeDetails();
    ~ArMapScanChangeDetails();

  }; // end struct ArMapScanChangeDetails

  ArMapScanChangeDetails *getScanChangeDetails(const char *scanType);

protected:

  /// Mutex to protect multithreaded access.
  ArMutex myMutex;

  /// Identifier of the map before the changes were made.
  ArMapId myOrigMapId;
  /// Identifier of the map after the changes were made.
  ArMapId myNewMapId;

  /// Map of info type identifiers to argument types, and whether each argument type is a child.
  std::map<std::string, std::map<std::string, bool> > myInfoNameToMapOfChildArgsMap;

  /// List of scan types included in the change details.
  std::list<std::string> myScanTypeList;
  /// Map of scan types to the changes for the scan.
  std::map<std::string, ArMapScanChangeDetails*> myScanTypeToChangesMap;
  /// Value returned when no scan data was changed.
  ArMapScanChangeDetails myNullScanTypeChanges;

  /// Change details for the map's supplemental data.
  ArMapFileLineSet myChangedSupplementLines[CHANGE_TYPE_COUNT];
  /// Change details for the map's object list.
  ArMapFileLineSet myChangedObjectLines[CHANGE_TYPE_COUNT];
  /// Change details for the map's info data.
  std::map<std::string, ArMapFileLineSet> myInfoToChangeMaps[CHANGE_TYPE_COUNT];

}; // end class ArMapChangeDetails

// ============================================================================
// ArMapFileLineSetWriter
// ============================================================================

/// Functor that populates a specified ArMapFileLineSet.
/**
 * ArMapFileLineSetWriter is used to create an ArMapFileLineSet, using the 
 * normal Aria map writeToFunctor mechanism.
 * @swigomit
 * @internal
**/
class ArMapFileLineSetWriter : public ArFunctor1<const char *>
{
public:

  /// Constructs a new writer for the given map file line set.
  ArMapFileLineSetWriter(ArMapFileLineSet *multiSet) :
    myLineNum(0),
    myChildLineNum(0),
	  myMultiSet(multiSet),
    myIsAddingChildren(false)
  {}
  
  /// Destructor
  virtual ~ArMapFileLineSetWriter()
  {}
  
  /// Returns whether children are currently being added to the map file line set.
  bool isAddingChildren() const
  {
    return myIsAddingChildren;
  }

  /// Sets whether children are currently being added to the map file line set.
  /**
   * When a child line is detected by the map file parser, setAddingChildren(true) 
   * must be called.  The new children will be added to the most recently 
   * created ArMapFileLineSet.  Likewise, when the child list is complete, 
   * setAddingChildren(false) must be called.
  **/
  void setAddingChildren(bool isAddingChildren) {
    if (myIsAddingChildren == isAddingChildren) {
      return;
    }
    myIsAddingChildren = isAddingChildren; 
    if (myIsAddingChildren) {
      myChildLineNum = 0;
    }
  } // end method setAddingChildren


  /// Invokes the functor; this method shouldn't be called.
  virtual void invoke(void) {};

  /// Invokes the functor
  /**
     @param p1 the char * map file line to be written
  */
  virtual void invoke(const char *p1)
  {
    myLineNum++;

    if ((myMultiSet != NULL) && (p1 != NULL)) {
      if (!myIsAddingChildren) {
        // Being verbose on the parameters here in order to make it more clear.
        // Create a new map file line for the parent, create a new group for 
        // that parent, and add it to the set.
	      myMultiSet->push_back(ArMapFileLineGroup(ArMapFileLine(myLineNum, p1)));
      }
      else if (!myMultiSet->empty()) {
        myChildLineNum++;
        ArMapFileLineGroup &curParent = myMultiSet->back();
        curParent.getChildLines()->push_back(ArMapFileLine(myChildLineNum, p1));
      }
	  }
  } // end method invoke
 
private:

  /// Disabled copy constructor
  ArMapFileLineSetWriter(const ArMapFileLineSetWriter &other);
  /// Disabled assignment operator
  ArMapFileLineSetWriter &operator=(const ArMapFileLineSetWriter &other);

protected:

  /// Line number currently being written
  int myLineNum;
  /// "Local" line number of the current child in relation to its parent
  int myChildLineNum;

  /// Map file line set that is being populated
  ArMapFileLineSet *myMultiSet; 
  /// Whether a child is currently being added
  bool myIsAddingChildren;

}; // end class ArMapFileLineSetWriter


// ============================================================================
// ArMapChangedHelper
// ============================================================================

/// Helper class that stores and invokes the map changed callbacks.
/** @swigomit
 * @internal 
 */
class ArMapChangedHelper
 {
 public:
 
   ///  Constructor 
   AREXPORT ArMapChangedHelper();
   /// Destructor
   AREXPORT virtual ~ArMapChangedHelper();
   
   /// Function that invokes the map changed callbacks
   AREXPORT virtual void invokeMapChangedCallbacks(bool invokePathPlanningCB = true,
                                                   bool invokeLocalizationCB = true);

   /// Adds a callback to be invoked when the map is changed
   AREXPORT virtual void addMapChangedCB(ArFunctor *functor, 
					 int position = 50);
   /// Removes a callback invoked when the map is changed
   AREXPORT virtual void remMapChangedCB(ArFunctor *functor);
   /// Adds a callback to be invoked when the map is changed

   AREXPORT virtual void addMapChangedPathPlanningCB(ArFunctor *functor,
                     int position = 50);
   /// Removes a callback invoked when the map is changed
   AREXPORT virtual void remMapChangedPathPlanningCB(ArFunctor *functor);
   /// Adds a callback to be invoked when the map is changed
   AREXPORT virtual void addMapChangedLocalizationCB(ArFunctor *functor,
                     int position = 50);
   /// Removes a callback invoked when the map is changed
   AREXPORT virtual void remMapChangedLocalizationCB(ArFunctor *functor);

   /// Adds a callback called before the map changed callbacks are called
   /**
    * Note that these callbacks are simply invoked before the "normal" map changed
    * callbacks.  They are not 
   **/
   AREXPORT virtual void addPreMapChangedCB(ArFunctor *functor,
                                            int position = 50);
   /// Removes a callback called before the map changed callbacks are called
   AREXPORT virtual void remPreMapChangedCB(ArFunctor *functor);
 
   /// Sets the level we log our map changed callback at
   AREXPORT virtual void setMapChangedLogLevel(ArLog::LogLevel level);
   /// Gets the level we log our map changed callback at
   AREXPORT virtual ArLog::LogLevel getMapChangedLogLevel(void);
   
 private:
 
   /// Disabled copy constructor
   AREXPORT ArMapChangedHelper(const ArMapChangedHelper &other);
   ///  Disabled assignment operaotr
   AREXPORT ArMapChangedHelper &operator=(const ArMapChangedHelper &other);
 
 protected:
 
   ArLog::LogLevel myMapChangedLogLevel;
 
   ArCallbackList myMapChangedCBList;
   ArCallbackList myPreMapChangedCBList;
   //Callback list used by path planning so that it ignores map changes
   //that it doesnt care about
   ArCallbackList myMapChangedPathPlanningCBList;
   //Callback list used by localization so that it ignores map changes
   //that it doesnt care about
   ArCallbackList myMapChangedLocalizationCBList;
 }; // end class ArMapChangedHelper
 
#endif // ifndef SWIG

#endif // ARMAPUTILS_H

