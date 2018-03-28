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
/*! \file ArMapComponents.h
 *  \brief Contains the set of component classes used to implement Aria maps.
 *  \date 06/27/08
 *  \author K. Cunningham
 * 
 * The ArMap class (defined in ArMap.h) is composed of several smaller 
 * classes that are defined in this header file.  These include:
 * 
 *  - ArMapScan: An implementation of the ArMapScanInterface.  This 
 *    contains all of the data related to the sensed obstacles (i.e.
 *    data points and lines).  An instance of this class is created 
 *    for each scan type that is defined in the map.
 *
 *  - ArMapObjects: An implementation of the ArMapObjectsInterface.
 *    This stores all of the map objects for the Aria map.
 *
 *  - ArMapInfo: An implementation of the ArMapInfoInterface.  This
 *    contains all of the info (ArArgumentBuilder) tags defined for 
 *    the map, including MapInfo, TaskInfo, and RouteInfo.
 *
 *  - ArMapSupplement: An implementation of the ArMapSupplementInterface.
 *    This is a repository for all of the extra data that does not fit
 *    into any of the above categories.
 * 
 *  - ArMapSimple:  The RealSubject of the ArMap Proxy.  This implements
 *    the ArMapInterface and is an aggregate of all of the above map
 *    components.
 * 
 * The following "diagram" illustrates the basic hierarchy:
 *
 * <pre>  
 *
 *          ________ArMapSupplementInterface________
 *         ^                                        ^
 *         |   ________ArMapInfoInterface________   |
 *         |   ^                                ^   |
 *         |   |   __ArMapObjectsInterface___   |   |
 *         |   |   ^                        ^   |   |
 *         |   |   |   ArMapScanInterface   |   |   |
 *         |   |   |   ^                ^   |   |   |
 *         |   |   |   |                |   |   |   |
 *        ArMapInterface                |   |   |   |  
 *        ^ (extends)                   |   |   |   | (extends)
 *        |                             |   |   |   |
 *        |                             |   |   |   |
 *     ArMapSimple +----------> ArMapScan   |   |   |
 *                 | (contains)             |   |   |
 *                 +-----------> ArMapObjects   |   |
 *                 |                            |   |
 *                 +------------------> ArMapInfo   |
 *                 |                                |
 *                 +----------------> ArMapSupplement
 *
 * </pre>

 * @see ArMapInterface
 * @see ArMap
**/
#ifndef ARMAPCOMPONENTS_H
#define ARMAPCOMPONENTS_H

#include "ArMapInterface.h"

class ArMapChangeDetails;
class ArMapFileLineSet;
class ArFileParser;
class ArMD5Calculator;


// ============================================================================
// ArMapScan 
// ============================================================================

/// The map data related to the sensable obstacles in the environment.
/**
 * ArMapScan encapsulates the data for a particular sensor that is generated 
 * during the scanning process (i.e. during the creation of a .2d file).  
 * The class's primary attributes are the points and line segments that 
 * were detected during the scan.  It contains methods to get and set these 
 * coordinates, and to read and write the data from and to a file.
 * <p>
 * The <code>scanType</code> parameters identify the sensor used for scanning. 
 * The parameter is used in the constructor, but it is generally disregarded
 * in the other methods.  (The method signatures are defined in 
 * ArMapScanInterface, which is also implemented by ArMap.  The map provides
 * access to the scan data for all of the sensors -- and therefore uses the 
 * <code>scanType</code> parameters.  This interface was chosen in order
 * to maintain backwards compatibility with the original map.)
 * <p>
 * If the scanType is specified, then it is used as a prefix to the DATA and
 * LINES tags that are contained in the map file.
**/
class ArMapScan : public ArMapScanInterface 
{
public:

  /// Constructor
  /**
   * Creates a new map scan object for the specified scan type.
   * @param scanType the const char * identifier of the scan; must be 
   * non-NULL and must not contain whitespaces
  **/
  AREXPORT ArMapScan(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  /// Copy constructor
  AREXPORT ArMapScan(const ArMapScan &other);

  /// Assignment operator
  AREXPORT ArMapScan &operator=(const ArMapScan &other);

  /// Destructor
  AREXPORT virtual ~ArMapScan();


  // --------------------------------------------------------------------------
  // ArMapScanInterface Methods
  // --------------------------------------------------------------------------

  AREXPORT virtual const char *getDisplayString
                                  (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual std::vector<ArPose> *getPoints
                          (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual std::vector<ArLineSegment> *getLines
                          (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual ArPose getMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual ArPose getMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual int getNumPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual bool isSortedPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;

  AREXPORT virtual void setPoints(const std::vector<ArPose> *points,
                                  const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedPoints = false,
                                  ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual ArPose getLineMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual ArPose getLineMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual int getNumLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual bool isSortedLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;

  AREXPORT virtual void setLines(const std::vector<ArLineSegment> *lines,
                                 const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                 bool isSortedLines = false,
                                 ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual int getResolution(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void setResolution(int resolution,
                                      const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                      ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual void writeScanToFunctor
                                (ArFunctor1<const char *> *functor, 
 			                          const char *endOfLineChars,
                                const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void writePointsToFunctor
 		                        (ArFunctor2<int, std::vector<ArPose> *> *functor,
                             const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                             ArFunctor1<const char *> *keywordFunctor = NULL);

  AREXPORT virtual void writeLinesToFunctor
 	                          (ArFunctor2<int, std::vector<ArLineSegment> *> *functor,
                             const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                             ArFunctor1<const char *> *keywordFunctor = NULL);

  AREXPORT virtual bool addToFileParser(ArFileParser *fileParser);

  AREXPORT virtual bool remFromFileParser(ArFileParser *fileParser);

  AREXPORT virtual bool readDataPoint( char *line);

  AREXPORT virtual bool readLineSegment( char *line);

  AREXPORT virtual void loadDataPoint(double x, double y);

  AREXPORT virtual void loadLineSegment(double x1, double y1, double x2, double y2);
  
  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------
 
  /// Resets the scan data, clearing all points and line segments
  AREXPORT virtual void clear();
  
  /// Combines the given other scan with this one.
  /**
   * @param other the ArMapScan * to be united with this one
   * @param isIncludeDataPointsAndLines a bool set to true if the other scan's
   * data points and lines should be copied to this scan; if false, then only
   * the summary (bounding box, counts, etc) information is copied.
  **/
  AREXPORT virtual bool unite(ArMapScan *other,
                              bool isIncludeDataPointsAndLines = false);
  
  /// Returns the time at which the scan data was last changed.
  AREXPORT virtual ArTime getTimeChanged() const;

  // TODO: Which of these need to be in the ArMapScanInterface?

  /// Returns the unique string identifier of the associated scan type.
  AREXPORT virtual const char *getScanType() const;

  /// Returns the keyword that designates the scan's data points in the map file.
  AREXPORT virtual const char *getPointsKeyword() const;
  /// Returns the keyword that designates the scan's data lines in the map file.
  AREXPORT virtual const char *getLinesKeyword() const;

  /// Writes the scan's data points (and introductory keyword) to the given functor.
  AREXPORT virtual void writePointsToFunctor
                                (ArFunctor1<const char *> *functor, 
 			                          const char *endOfLineChars,
                                const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  /// Writes the scan's data lines (and introductory keyword) to the given functor.
  AREXPORT virtual void writeLinesToFunctor
                                (ArFunctor1<const char *> *functor, 
 			                           const char *endOfLineChars,
                                 const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  
  /// Adds the handlers for the data points and lines keywords to the given file parser.
  /**
   * These handlers are "extra" because they are added after all of the summary
   * keyword have been parsed.
  **/
  AREXPORT virtual bool addExtraToFileParser(ArFileParser *fileParser,
                                            bool isAddLineHandler);

  /// Removes the handlers for the data points and lines keywords from the given file parser.
  AREXPORT virtual bool remExtraFromFileParser(ArFileParser *fileParser);


protected:

  /// Writes the list of data lines to the given functor.
  /**
   * @param functor the ArFunctor1<const char *> * to which to write the 
   * data lines
   * @param lines the vector of ArLineSegments to be written to the functor
   * @param endOfLineChars an optional string to be appended to the end of 
   * each text line written to the functor
   * @param scanType the unique string identifier of the scan type associated
   * with the data lines
  **/
  AREXPORT virtual void writeLinesToFunctor
                                (ArFunctor1<const char *> *functor, 
                                 const std::vector<ArLineSegment> &lines,
 			                           const char *endOfLineChars,
                                 const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);


  // Function to read the minimum pos
  bool handleMinPos(ArArgumentBuilder *arg);
  // Function to read the maximum pos
  bool handleMaxPos(ArArgumentBuilder *arg);
  // Function to read the number of points
  bool handleNumPoints(ArArgumentBuilder *arg);
  // Function to read whether the points are sorted
  bool handleIsSortedPoints(ArArgumentBuilder *arg);

  // Function to read the line minimum pos
  bool handleLineMinPos(ArArgumentBuilder *arg);
  // Function to read the line maximum pos
  bool handleLineMaxPos(ArArgumentBuilder *arg);
  // Function to read the number of lines
  bool handleNumLines(ArArgumentBuilder *arg);
  // Function to read whether the lines are sorted
  bool handleIsSortedLines(ArArgumentBuilder *arg);

  // Function to handle the resolution
  bool handleResolution(ArArgumentBuilder *arg);
  /// Callback to handle the Display string.
  bool handleDisplayString(ArArgumentBuilder *arg);

  // Function to snag the map points (mainly for the getMap over the network)
  bool handlePoint(ArArgumentBuilder *arg);
  // Function to snag the line segments (mainly for the getMap over the network)
  bool handleLine(ArArgumentBuilder *arg);
  
  /// Adds the specified argument handler to the given file parser.
  bool addHandlerToFileParser(ArFileParser *fileParser,
                              const char *keyword,
                              ArRetFunctor1<bool, ArArgumentBuilder *> *handler);

  /// Returns the keyword prefix for this scan type.
  const char *getKeywordPrefix() const;

  /// Parses a pose from the given arguments.
  bool parsePose(ArArgumentBuilder *arg,
                 const char *keyword,
                 ArPose *poseOut);

  /// Parses an integer from the given text line.
  bool parseNumber(char *line, 
                   size_t lineLen, 
                   size_t *charCountOut,
                   int *numOut) const;

  /// Parses whitespace from the given text line.
  bool parseWhitespace(char *line,
                       size_t lineLen,
                       size_t *charCountOut) const;


private:

  /// Constant appended to the end of each scan data text line.
  static const char *EOL_CHARS;

protected:

  /// The unique string identifier of this scan type.
  std::string myScanType;
  /// Whether this is a special summary of the other scans.
  bool myIsSummaryScan;

  /// The prefix prepended to the output log file messages.
  std::string myLogPrefix;
  /// The prefix prepended to the map file keywords (e.g. DATA and LINES)
  std::string myKeywordPrefix;
  /// The keyword that designates this scan's data points in the map file.
  std::string myPointsKeyword;
  /// The keyword that designates this scan's data lines in the map file.
  std::string myLinesKeyword;

  /// Time that this scan data was last modified.
  ArTime myTimeChanged;

  /// Displayable text for this scan type.
  std::string myDisplayString;

  /// Number of data points in the scan.
  int myNumPoints;
  /// Number of data lines in the scan.
  int myNumLines;
  /// Resolution of the data points (in mm).
  int myResolution;
  /// Maximum x/y values of all of the data points in the scan.
  ArPose myMax;
  /// Minimum x/y values of all of the data points in the scan.
  ArPose myMin;
  /// Maximum x/y values of all of the data lines in the scan.
  ArPose myLineMax;
  /// Minimum x/y values of all of the data lines in the scan.
  ArPose myLineMin;

  /// Whether the data points in myPoints have been sorted in ascending order.
  bool myIsSortedPoints;
  /// Whether the data lines in myLines have been sorted in ascending order.
  bool myIsSortedLines;

  /// List of data points contained in this scan data.
  std::vector<ArPose> myPoints;
  /// List of data lines contained in this scan data.
  std::vector<ArLineSegment> myLines;

  /// Callback to parse the minimum poise from the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myMinPosCB;
  /// Callback to parse the maximum pose from the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myMaxPosCB;
  /// Callback to parse whether the points in the map file have been sorted.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myIsSortedPointsCB;
  /// Callback to parse the number of data points in the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myNumPointsCB;

  /// Callback to parse the minimum line pose from the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myLineMinPosCB;
  /// Callback to parse the maximum line pose from the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myLineMaxPosCB;
  /// Callback to parse whether the lines in the map file have been sorted.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myIsSortedLinesCB;
  /// Callback to parse the number of data lines in the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myNumLinesCB;

  /// Callback to parse the resolution in the map file.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myResolutionCB;
  /// Callback to parse the displayable text for this scan type.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myDisplayStringCB;

  /// Callback to parse a data point.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myPointCB;
  /// Callback to parse a data line.
  ArRetFunctor1C<bool, ArMapScan, ArArgumentBuilder *> myLineCB;

}; // end class ArMapScan

// ============================================================================
// ArMapObjects
// ============================================================================

/// The collection of map objects that are contained in an Aria map.
/**
 * ArMapObjects contains a list of objects defined in an Aria map.  There are
 * two basic classes of objects:  user-defined objects such as goals and 
 * forbidden areas; and, special data objects that are usually automatically
 * generated during the scanning process.
**/
class ArMapObjects : public ArMapObjectsInterface				
{

public :

  /// Default keyword that prefixes each map object line in the map file
  AREXPORT static const char *DEFAULT_KEYWORD;

  /// Constructor
  /**
   * @param keyword the char * keyword that prefixes each map object line in
   * the map file
  **/
  AREXPORT ArMapObjects(const char *keyword = "Cairn:");

  /// Copy constructor
  AREXPORT ArMapObjects(const ArMapObjects &other);

  /// Assignment operator
  AREXPORT ArMapObjects &operator=(const ArMapObjects &other);

  /// Destructor
  AREXPORT virtual ~ArMapObjects();


  // ---------------------------------------------------------------------------
  // ArMapObjectsInterface Methods
  // ---------------------------------------------------------------------------

  AREXPORT virtual ArMapObject *findFirstMapObject(const char *name, 
                                                   const char *type,
                                                   bool isIncludeWithHeading = false);

  AREXPORT virtual ArMapObject *findMapObject(const char *name, 
 				                                      const char *type = NULL, 
                                              bool isIncludeWithHeading = false);

  AREXPORT virtual std::list<ArMapObject *> findMapObjectsOfType
                                                    (const char *type,
                                                     bool isIncludeWithHeading = false);

  AREXPORT virtual std::list<ArMapObject *> *getMapObjects(void);

  AREXPORT virtual void setMapObjects(const std::list<ArMapObject *> *mapObjects,
                                      bool isSortedObjects = false,
                                      ArMapChangeDetails *changeDetails = NULL); 
   
  AREXPORT void writeObjectListToFunctor(ArFunctor1<const char *> *functor, 
		                                     const char *endOfLineChars);


  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Clears the map objects and deletes them.
  AREXPORT virtual void clear();

  /// Adds the keyword and handler for the map objects to the given file parser.
  AREXPORT virtual bool addToFileParser(ArFileParser *fileParser);

  /// Removes the keyword and handler for the map objects from the given file parser.
  AREXPORT virtual bool remFromFileParser(ArFileParser *fileParser);

  /// Returns the time at which the map objects were last changed.
  AREXPORT virtual ArTime getTimeChanged() const;
 
protected:

  // Function to handle the cairns
  bool handleMapObject(ArArgumentBuilder *arg);

  /// Sorts the given list of map objects in order of increasing object pose.
  void sortMapObjects(std::list<ArMapObject *> *mapObjects);

  /// Writes the map objects to the given ArMapFileLineSet.
  void createMultiSet(ArMapFileLineSet *multiSet);

  /// Writes the given ArMapFileLineSet to the output log with the specified prefix.
  void logMultiSet(const char *prefix,
                   ArMapFileLineSet *multiSet);

protected:

  /// Time at which the map objects were last changed.
  ArTime myTimeChanged;
  /// Whether the myMapObjects list has been sorted in increasing (pose) order.
  bool myIsSortedObjects;

  /// Keyword that prefixes each map object in the map file.
  std::string myKeyword;

  /// List of map objects contained in the Aria map.
  std::list<ArMapObject *> myMapObjects;

  /// Callback to parse the map object from the map file.
  ArRetFunctor1C<bool, ArMapObjects, ArArgumentBuilder *> myMapObjectCB;

}; // end class ArMapObjects


// ============================================================================
// ArMapInfo
// ============================================================================


/// A container for the various "info" tags in an Aria map.
/**
 * ArMapInfo is an implementation of ArMapInfoInterface that provides access
 * to a collection of "info" arguments (such as MapInfo and RouteInfo).  An Aria 
 * map may have one or more categories of info, each implemented by an ordered 
 * list of ArArgumentBuilder's.
 *
 * Info types are currently identified by a unique integer.  The default types
 * are defined in ArMapInfoInterface::InfoType, but applications may define 
 * additional types.  (See ArMapInfo::ArMapInfo(int*, char**, size_t))
**/
class ArMapInfo : public ArMapInfoInterface
{
public:

  /// Contructor
  /**
   * @param infoNameList an array of the char * keywords for each of the
   * standard ArMapInfo::InfoType's; if NULL, then the default keywords are
   * used
   * @param infoNameCount the size_t length of the infoNameList array
   * @param keywordPrefix optional prefix to add to keywords.
  **/ 
  AREXPORT ArMapInfo(const char **infoNameList = NULL,
                     size_t infoNameCount = 0,
                     const char *keywordPrefix = NULL);

  /// Copy contructor
  AREXPORT ArMapInfo(const ArMapInfo &other);
  /// Assignment operator
  AREXPORT ArMapInfo &operator=(const ArMapInfo &other);
  /// Destructor
  AREXPORT virtual ~ArMapInfo();

  // ---------------------------------------------------------------------------
  // ArMapInfoInterface Methods
  // ---------------------------------------------------------------------------

  AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(const char *infoName);

  AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(int infoType);
  AREXPORT virtual std::list<ArArgumentBuilder *> *getMapInfo(void);

  AREXPORT virtual int getInfoCount() const;

  AREXPORT virtual std::list<std::string> getInfoNames() const;

  AREXPORT virtual bool setInfo(const char *infoName,
                                const std::list<ArArgumentBuilder *> *infoList,
                                ArMapChangeDetails *changeDetails);


  AREXPORT virtual bool setInfo(int infoType,
 						                    const std::list<ArArgumentBuilder *> *infoList,
                                ArMapChangeDetails *changeDetails = NULL); 
  AREXPORT virtual bool setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                   ArMapChangeDetails *changeDetails = NULL); 


  AREXPORT virtual const char *getInfoName(int infoType);

  AREXPORT virtual void writeInfoToFunctor(ArFunctor1<const char *> *functor, 
 			                                     const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Clears all info arguments and deletes them.
  AREXPORT virtual void clear();
 
  /// Adds handlers for all of the info types to the given file parser.
  AREXPORT virtual bool addToFileParser(ArFileParser *fileParser);
  /// Removes handlers for all of the info types from the given file parser.
  AREXPORT virtual bool remFromFileParser(ArFileParser *fileParser);

  /// Returns the time at which the info were last changed.
  AREXPORT virtual ArTime getTimeChanged() const;

protected:

  /// Processes the given argument for the specified info.
  bool handleInfo(ArArgumentBuilder *arg);

  /// Give ArMapSimple access to the createMultiSet() and  setChanged() methods
  friend class ArMapSimple;
  
  /// Writes the specified info arguments to the given ArMapFileLineSet.
  /**
   * @param infoName unique identifier for the info to be written
   * @param multiSet the ArMapFileLineSet * to which to write the info; 
   * must be non-NULL
   * @param changeDetails the ArMapChangeDetails * that specifies the 
   * parent/child relationship amongst info lines 
   * @see ArMapChangeDetails::isChildArg
  **/
  void createMultiSet(const char *infoName, 
                      ArMapFileLineSet *multiSet,
                      ArMapChangeDetails *changeDetails);

  /// Basically updates the timeChanged to now.
  void setChanged();
  
  /// Populates this object with the default info names / keywords
  void setDefaultInfoNames();



protected:

  struct ArMapInfoData {

    ArMapInfo *myParent;
    int myType;
    std::string myKeyword;
    std::list<ArArgumentBuilder *> myInfo;
    ArRetFunctor1C<bool, ArMapInfo, ArArgumentBuilder *> *myInfoCB;

    ArMapInfoData(ArMapInfo *parent,
                  const char *name = NULL,
                  int type = -1);
    ~ArMapInfoData();
    ArMapInfoData(ArMapInfo *parent,
                  const ArMapInfoData &other);
    ArMapInfoData &operator=(const ArMapInfoData &other);

  }; // end struct ArMapInfoData

  typedef std::map<std::string, ArMapInfoData *, ArStrCaseCmpOp> ArInfoNameToDataMap;


  AREXPORT ArMapInfoData *findData(const char *infoName);

  AREXPORT ArMapInfoData *findDataByKeyword(const char *keyword);


   /// Time at which the info was last changed
  ArTime myTimeChanged;

  // Sigh... In retrospect, this should have been structured differently
  // and we probably should've used a string for the info identifier...

  /// Number of info types contained in this collection
  int myNumInfos;
  std::string myPrefix;

  std::map<int, std::string> myInfoTypeToNameMap;
  ArInfoNameToDataMap myInfoNameToDataMap;
  std::map<std::string, std::string, ArStrCaseCmpOp> myKeywordToInfoNameMap;

}; // end class ArMapInfo


// ============================================================================
// ArMapSupplement
// ============================================================================

/// Supplemental data associated with an Aria map.
/**
 * ArMapSupplement is a repository for extra, miscellaneous data that is 
 * associated with an Aria map but which does not fit neatly into any of the 
 * other components.  
**/
class ArMapSupplement : public ArMapSupplementInterface 
{
public:

  /// Constructor
  AREXPORT ArMapSupplement();

  /// Copy constructor
  AREXPORT ArMapSupplement(const ArMapSupplement &other);

  /// Assignment operator
  AREXPORT ArMapSupplement &operator=(const ArMapSupplement &other);

  /// Destructor
  AREXPORT virtual ~ArMapSupplement();


  // --------------------------------------------------------------------------
  // ArMapSupplementInterface Methods
  // --------------------------------------------------------------------------

  AREXPORT virtual bool hasOriginLatLongAlt();
  
  AREXPORT virtual ArPose getOriginLatLong();
  
  AREXPORT virtual double getOriginAltitude();

  AREXPORT virtual void setOriginLatLongAlt(bool hasOriginLatLong,
                                            const ArPose &originLatLong,
                                            double altitude,
                                            ArMapChangeDetails *changeDetails = NULL);
  
  AREXPORT virtual void writeSupplementToFunctor
                                (ArFunctor1<const char *> *functor, 
 			                           const char *endOfLineChars);

  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------

  /// Resets the map supplement to its default values.
  AREXPORT virtual void clear();

  /// Adds handlers for all of the supplement keywords to the given file parser.
  AREXPORT virtual bool addToFileParser(ArFileParser *fileParser);
  /// Removes handlers for all of the supplement keywords from the given file parser.
  AREXPORT virtual bool remFromFileParser(ArFileParser *fileParser);
  
  /// Returns the time at which the supplement data were last changed.
  AREXPORT virtual ArTime getTimeChanged() const;

protected:

  // Function to get the origin lat long altitude
  bool handleOriginLatLongAlt(ArArgumentBuilder *arg);
  
private:

  /// Constant appended to the end of each supplement text line.
  static const char *EOL_CHARS;

protected:

  /// Time at which the supplement was last changed
  ArTime myTimeChanged;

  /// Whether the supplement data contains latitude/longitude information for the origin
  bool myHasOriginLatLongAlt;
  /// The latitude/longitude of the origin; only if myHasOriginLatLongAlt is true
  ArPose myOriginLatLong;
  /// The altitude (in m) of the origin; only if myHasOriginLatLongAlt is true
  double myOriginAltitude;

  /// Callback that parses the origin latitude/longitude/altitude information
  ArRetFunctor1C<bool, ArMapSupplement, ArArgumentBuilder *> myOriginLatLongAltCB;

}; // end class ArMapSupplement


// =============================================================================
// ArMapSimple
// =============================================================================
 
/// Comparator used to sort scan data types in a case-insensitive manner. 
struct ArDataTagCaseCmpOp 
{
public:
  bool operator() (const std::string &s1, const std::string &s2) const
  {
    size_t s1Len = s1.length();
    size_t s2Len = s2.length();

    if (s1Len < s2Len) {
      return strncasecmp(s1.c_str(), s2.c_str(), s1Len) < 0;
    }
    else {
      return strncasecmp(s1.c_str(), s2.c_str(), s2Len) < 0;
    }
  }
}; // end struct ArDataTagCaseCmpOp

/// Type definition for a map of scan types to scan data.
typedef std::map<std::string, ArMapScan *, ArStrCaseCmpOp> ArTypeToScanMap;

/// Type definition for a map of data tags to scan types
typedef std::map<std::string, std::string, ArDataTagCaseCmpOp> ArDataTagToScanTypeMap;


/// Simple map that can be read from and written to a file
/**
 * ArMapSimple is the real subject of the ArMap proxy.  Functionally, it is identical
 * to the ArMap, @b except that it is not well-suited for for loading from a file at
 * runtime and therefore doesn't provide any hooks into the Aria config.  In general,
 * ArMap should be used instead.  The exception to this rule may be in off-line 
 * authoring tools where error checking can be performed at a higher level.
**/
class ArMapSimple : public ArMapInterface
{
public:

  /// Constructor
  /**
   * @param baseDirectory the name of the directory in which to search for map
   * files that are not fully qualified
   * @param tempDirectory the name of the directory in which to write temporary
   * files when saving a map; if NULL, then the map file is written directly.  
   * Note that using a temp file reduces the risk that the map will be corrupted
   * if the application crashes.
   * @param overrideMutexName an optional name to be used for the map object's
   * mutex; useful for debugging when multiple maps are active
  **/
  AREXPORT ArMapSimple(const char *baseDirectory = "./",
                       const char *tempDirectory = NULL,
                       const char *overrideMutexName = NULL);

  /// Copy constructor
  AREXPORT ArMapSimple(const ArMapSimple &other);
  /// Assignment operator
  AREXPORT ArMapSimple &operator=(const ArMapSimple &other);
  /// Destructor
  AREXPORT virtual ~ArMapSimple(void);


  AREXPORT virtual void clear();

  AREXPORT virtual bool set(ArMapInterface *other);

  AREXPORT virtual ArMapInterface *clone();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Scan Types Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   
  AREXPORT virtual std::list<std::string> getScanTypes() const;

  AREXPORT virtual bool setScanTypes(const std::list<std::string> &scanTypeList);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Locking / Semaphore Method
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual int lock();

  AREXPORT virtual int tryLock();

  AREXPORT virtual int unlock();


  // ---------------------------------------------------------------------------
  // ArMapInfoInterface
  // ---------------------------------------------------------------------------

  AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(const char *infoName);
  AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(int infoType);
  AREXPORT virtual std::list<ArArgumentBuilder *> *getMapInfo(void);
  
  AREXPORT virtual int getInfoCount() const;
  
  AREXPORT virtual std::list<std::string> getInfoNames() const;

  AREXPORT virtual bool setInfo(const char *infoName,
 						                    const std::list<ArArgumentBuilder *> *infoList,
                                ArMapChangeDetails *changeDetails = NULL);

  AREXPORT virtual bool setInfo(int infoType,
 						                    const std::list<ArArgumentBuilder *> *infoList,
                                ArMapChangeDetails *changeDetails = NULL);

  AREXPORT virtual bool setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                   ArMapChangeDetails *changeDetails = NULL);

  AREXPORT virtual void writeInfoToFunctor
 				(ArFunctor1<const char *> *functor, 
 			        const char *endOfLineChars);

  AREXPORT virtual const char *getInfoName(int infoType);

  // ---------------------------------------------------------------------------
  // ArMapObjectsInterface
  // ---------------------------------------------------------------------------

  AREXPORT virtual ArMapObject *findFirstMapObject(const char *name, 
                                                   const char *type,
                                                   bool isIncludeWithHeading = false);

  AREXPORT virtual ArMapObject *findMapObject(const char *name, 
 				                                      const char *type = NULL,
                                              bool isIncludeWithHeading = false);

  AREXPORT virtual std::list<ArMapObject *> findMapObjectsOfType
                                                    (const char *type,
                                                     bool isIncludeWithHeading = false);

  AREXPORT virtual std::list<ArMapObject *> *getMapObjects(void);

  AREXPORT virtual void setMapObjects(const std::list<ArMapObject *> *mapObjects,
                                      bool isSortedObjects = false,
                                      ArMapChangeDetails *changeDetails = NULL); 


   AREXPORT virtual void writeObjectListToFunctor(ArFunctor1<const char *> *functor, 
			                                            const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // ArMapSupplementInterface
  // ---------------------------------------------------------------------------

  AREXPORT virtual bool hasOriginLatLongAlt();

  AREXPORT virtual ArPose getOriginLatLong();

  AREXPORT virtual double getOriginAltitude();

  AREXPORT virtual void setOriginLatLongAlt
                                        (bool hasOriginLatLong,
                                         const ArPose &originLatLong,
                                         double altitude,
                                         ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual void writeSupplementToFunctor(ArFunctor1<const char *> *functor, 
			                                          const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // ArMapScanInterface
  // ---------------------------------------------------------------------------

  AREXPORT virtual const char *getDisplayString
                                 (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual std::vector<ArPose> *getPoints
                                 (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual ArPose getMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual ArPose getMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual int getNumPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual bool isSortedPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;

  AREXPORT virtual void setPoints(const std::vector<ArPose> *points,
                                  const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedPoints = false,
                                  ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual std::vector<ArLineSegment> *getLines
                                 (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual ArPose getLineMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual ArPose getLineMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual int getNumLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual bool isSortedLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;

  AREXPORT virtual void setLines(const std::vector<ArLineSegment> *lines,
                                 const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                 bool isSortedLines = false,
                                 ArMapChangeDetails *changeDetails = NULL);


  AREXPORT virtual int getResolution(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void setResolution(int resolution,
                                      const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                      ArMapChangeDetails *changeDetails = NULL);

  
  AREXPORT virtual void writeScanToFunctor
                              (ArFunctor1<const char *> *functor, 
			                         const char *endOfLineChars,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void writePointsToFunctor
                              (ArFunctor2<int, std::vector<ArPose> *> *functor,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                               ArFunctor1<const char *> *keywordFunctor = NULL);

  AREXPORT virtual void writeLinesToFunctor
 	                            (ArFunctor2<int, std::vector<ArLineSegment> *> *functor,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                               ArFunctor1<const char *> *keywordFunctor = NULL);

  AREXPORT virtual bool addToFileParser(ArFileParser *fileParser);
  AREXPORT virtual bool remFromFileParser(ArFileParser *fileParser);


  AREXPORT virtual bool readDataPoint( char *line);
  AREXPORT virtual bool readLineSegment( char *line);


  /** Public for ArQClientMapProducer **/

  AREXPORT virtual void loadDataPoint(double x, double y);
  AREXPORT virtual void loadLineSegment(double x1, double y1, double x2, double y2);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Map Changed / Callback Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void mapChanged(void);
  AREXPORT virtual void mapChanged(bool invokePathPlanningCB,
                                   bool invokeLocalizationCB);

  AREXPORT virtual void addMapChangedCB(ArFunctor *functor, 
					int position = 50);

  AREXPORT virtual void remMapChangedCB(ArFunctor *functor);

  AREXPORT virtual void addMapChangedPathPlanningCB(ArFunctor *functor,
                    int position = 50);

  AREXPORT virtual void remMapChangedPathPlanningCB(ArFunctor *functor);

  AREXPORT virtual void addMapChangedLocalizationCB(ArFunctor *functor,
                    int position = 50);

  AREXPORT virtual void remMapChangedLocalizationCB(ArFunctor *functor);


  AREXPORT virtual void addPreMapChangedCB(ArFunctor *functor,
                                           int position = 50);

  AREXPORT virtual void remPreMapChangedCB(ArFunctor *functor);

  AREXPORT virtual void setMapChangedLogLevel(ArLog::LogLevel level);

  AREXPORT virtual ArLog::LogLevel getMapChangedLogLevel(void);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Persistence
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  

  AREXPORT virtual void writeToFunctor(ArFunctor1<const char *> *functor, 
 			                                 const char *endOfLineChars);

  AREXPORT virtual void writeObjectsToFunctor(ArFunctor1<const char *> *functor, 
 			                                        const char *endOfLineChars,
                                              bool isOverrideAsSingleScan = false,
                                              const char *maxCategory = NULL);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File I/O Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  AREXPORT virtual void addPreWriteFileCB(ArFunctor *functor,
                                          ArListPos::Pos position = ArListPos::LAST);
  AREXPORT virtual void remPreWriteFileCB(ArFunctor *functor);
 
  AREXPORT virtual void addPostWriteFileCB(ArFunctor *functor,
                                           ArListPos::Pos position = ArListPos::LAST);
  AREXPORT virtual void remPostWriteFileCB(ArFunctor *functor);
  AREXPORT virtual void addPreParseFileCB(ArFunctor1<std::string> *functor);
  AREXPORT virtual void remPreParseFileCB(ArFunctor1<std::string> *functor);

  AREXPORT virtual bool readFile(const char *fileName, 
 			                           char *errorBuffer = NULL, 
                                 size_t errorBufferLen = 0,
                                 unsigned char *md5DigestBuffer = NULL,
                                 size_t md5DigestBufferLen = 0);

  AREXPORT virtual bool writeFile(const char *fileName, 
                                  bool internalCall = false,
                                  unsigned char *md5DigestBuffer = NULL,
                                  size_t md5DigestBufferLen = 0,
                                  time_t fileTimestamp = -1);

#ifndef SWIG
  /// @swigomit
  AREXPORT virtual struct stat getReadFileStat() const;
#endif

  AREXPORT virtual bool getMapId(ArMapId *mapIdOut,
                                 bool isInternalCall = false);

  AREXPORT virtual bool calculateChecksum(unsigned char *md5DigestBuffer,
                                          size_t md5DigestBufferLen);
  
  AREXPORT virtual const char *getBaseDirectory(void) const;

  AREXPORT virtual void setBaseDirectory(const char *baseDirectory);

  AREXPORT virtual const char *getTempDirectory(void) const;

  AREXPORT virtual void setTempDirectory(const char *tempDirectory);

  AREXPORT virtual std::string createRealFileName(const char *fileName);

  AREXPORT virtual const char *getFileName(void) const;

  AREXPORT virtual void setSourceFileName(const char *sourceName,
                                          const char *fileName,
                                          bool isInternalCall = false);

  AREXPORT virtual bool refresh();


  virtual void setIgnoreEmptyFileName(bool ignore);
  virtual bool getIgnoreEmptyFileName(void);
  virtual void setIgnoreCase(bool ignoreCase = false);
  virtual bool getIgnoreCase(void);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Inactive Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual ArMapInfoInterface *getInactiveInfo();

  AREXPORT virtual ArMapObjectsInterface *getInactiveObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Child Objects Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual ArMapObjectsInterface *getChildObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  AREXPORT virtual ArArgumentBuilder *findMapObjectParams(const char *mapObjectName);

  AREXPORT virtual bool setMapObjectParams(const char *mapObjectName,
                                           ArArgumentBuilder *params, 
                                           ArMapChangeDetails *changeDetails = NULL);



  AREXPORT virtual std::list<ArArgumentBuilder *> *getRemainder();

  AREXPORT virtual void setQuiet(bool isQuiet);
 	
  AREXPORT bool parseLine(char *line);
  AREXPORT void parsingComplete(void);

  AREXPORT bool isLoadingDataStarted(); 
  AREXPORT bool isLoadingLinesAndDataStarted(); 

  AREXPORT virtual const char *getMapCategory();


  // ---------------------------------------------------------------------
#ifndef SWIG
  /// Searches the given CairnInfo list for an entry that matches the given mapObject.
  /**
   * The CairnInfo list stores the parameter information (if any) for map 
   * objects. If a map object is removed (or activated), then the CairnInfo 
   * must also be updated.
   * @param mapObjectName the ArMapObject for which to find the parameters
   * @param cairnInfoList the list of ArArgumentBuilder *'s that contain the
   * map object parameters (also may be set to the inactive section)
   * @return iterator that points to the parameter information for the map
   * object, or cairnInfoList.end() if not found
  **/
  AREXPORT static std::list<ArArgumentBuilder *>::iterator findMapObjectParamInfo
             (const char *mapObjectName,
              std::list<ArArgumentBuilder*> &cairnInfoList);
#endif
protected:

  AREXPORT bool setInactiveInfo(const char *infoName,
 						                    const std::list<ArArgumentBuilder *> *infoList,
                                ArMapChangeDetails *changeDetails = NULL);

  AREXPORT void setInactiveObjects(const std::list<ArMapObject *> *mapObjects,
                                   bool isSortedObjects = false,
                                   ArMapChangeDetails *changeDetails = NULL); 

  AREXPORT void setChildObjects(const std::list<ArMapObject *> *mapObjects,
                                bool isSortedObjects = false,
                                ArMapChangeDetails *changeDetails = NULL); 

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /// Callback that handles the different types of map categories (e.g. 2D-Map, 2D-Map-Ex)
  /**
   * This method replaces the old handle2DMap method.  It determines which category
   * was detected and sets the myMapCategory attribute accordingly.
   * @param arg a pointer to the parsed ArArgumentBuilder; no arguments are expected
  **/
  bool handleMapCategory(ArArgumentBuilder *arg);
  
  /// Callback that handles the Sources keyword
  /**
   * @param arg a pointer to the parsed ArArgumentBuilder; a list of string scan type
   * arguments are expected
  **/
  bool handleSources(ArArgumentBuilder *arg);


  /// Callback that handles the different types of data introductions (e.g. DATA, LINES)
  /**
   * This method replaces the old handleData and handleLines methods.  It determines
   * which keyword was detected and updates the myLoadingDataTag and myLoadingScan
   * attributes accordingly.
   * @param arg a pointer to the parsed ArArgumentBuilder; no arguments are expected
  **/
  bool handleDataIntro(ArArgumentBuilder *arg);


  bool handleRemainder(ArArgumentBuilder *arg);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns the ArMapScan for the specified scan type.
  AREXPORT virtual ArMapScan *getScan(const char *scanType) const;

  /// Sets up the map to contain teh specified scan types.
  /**
   * Any scans which are currently in the map are cleared and removed.
   * This method is not virtual because it is called by the constructor.
   * @param scanTypeList a list of the scan type string identifiers to be 
   * created; the list must be non-empty and must not contain duplicates;
   * if the list contains more than one entry, then they all must be 
   * non-empty
   * @return bool true if the scans were successfully created; false otherwise
  **/
  bool createScans(const std::list<std::string> &scanTypeList);

  /// Adds all of the map's scan types to the current file parser.
  /**
   * This method calls addToFileParser() on each of the map's scans.  It also
   * adds handlers for each of the scans' data point and line introduction 
   * keywords.
   * @return bool true if the scans were successfully added to the current
   * file parser
  **/
  bool addScansToParser();

  /// Removes all of the map's scan types from the current file parser.
  bool remScansFromParser(bool isRemovePointsAndLinesKeywords = true);

  AREXPORT void writeScanTypesToFunctor(ArFunctor1<const char *> *functor, 
			                                  const char *endOfLineChars);

  AREXPORT ArTime findMaxMapScanTimeChanged();

  AREXPORT ArMapScan *findScanWithDataKeyword(const char *myLoadingDataTag,
                                              bool *isLineDataTagOut);
  
  AREXPORT void updateSummaryScan();
  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  

  AREXPORT virtual void updateMapCategory(const char *updatedInfoName = NULL);

  /// Determines whether the MapInfo contains the given arg text. 
  AREXPORT virtual bool mapInfoContains(const char *arg0Text);
 
  /// Determines whether the RouteInfo contains the given arg text. 
  AREXPORT virtual bool routeInfoContains(const char *arg0Text);

  /// Determines whether the specified info contains the specified arg text.
  AREXPORT virtual bool infoContains(const char *infoName,
                                     const char *argText,
                                     int argIndex,
                                     const char *secondaryArgText = NULL,
                                     int secondaryArgIndex = -1);

  AREXPORT bool isDataTag(const char *line); 

  AREXPORT void reset();

  AREXPORT void updateMapFileInfo(const char *realFileName);



  AREXPORT static int getNextFileNumber();

  AREXPORT void invokeCallbackList(std::list<ArFunctor*> *cbList);
  AREXPORT void invokeCallbackList(std::list<ArFunctor1<std::string>*> *cbList, std::string mapFormat);
  AREXPORT void addToCallbackList(ArFunctor *functor,
                                  ArListPos::Pos position,
                                  std::list<ArFunctor*> *cbList);

  AREXPORT void remFromCallbackList(ArFunctor *functor,
                                    std::list<ArFunctor*> *cbList);

protected:

  // static const char *ourDefaultInactiveInfoNames[INFO_COUNT];


  static int ourTempFileNumber;
  static ArMutex ourTempFileNumberMutex;

  // lock for our data
  ArMutex myMutex;

  std::list<std::string> myMapCategoryList;
  std::string myMapCategory;


  ArMD5Calculator *myChecksumCalculator;

  std::string myBaseDirectory;
  std::string myFileName;
  struct stat myReadFileStat;

  std::list<ArFunctor*> myPreWriteCBList;
  std::list<ArFunctor*> myPostWriteCBList;
  std::list<ArFunctor1<std::string>*> myPreParseCBList;

  bool myIsWriteToTempFile;
  std::string myTempDirectory;

  ArMapId myMapId;

  ArFileParser *myLoadingParser;

  // std::string myConfigParam;
  bool myIgnoreEmptyFileName;
  bool myIgnoreCase;

  ArMapChangedHelper *myMapChangedHelper;

  /***
  // things for our config
  bool myConfigProcessedBefore;
  char myConfigMapName[MAX_MAP_NAME_LENGTH];
  ***/

  bool myLoadingGotMapCategory; 
  // TODO: Need to change for multi scans
  bool myLoadingDataStarted;
  bool myLoadingLinesAndDataStarted;

  ArMapInfo       * const myMapInfo;
  ArMapObjects    * const myMapObjects;
  ArMapSupplement * const myMapSupplement;

  std::list<std::string> myScanTypeList;
  ArTypeToScanMap myTypeToScanMap;
  ArMapScan    * mySummaryScan;

  ArDataTagToScanTypeMap myDataTagToScanTypeMap;

  std::string    myLoadingDataTag;
  ArMapScan    * myLoadingScan;

  ArMapInfo    * const myInactiveInfo;
  ArMapObjects * const myInactiveObjects;

  ArMapObjects * const myChildObjects;

  std::map<std::string, ArArgumentBuilder *, ArStrCaseCmpOp> myMapObjectNameToParamsMap;

  /// List of map file lines that were not recognized
  std::list<ArArgumentBuilder *> myRemainderList;

  ArTime myTimeMapInfoChanged;
  ArTime myTimeMapObjectsChanged;
  ArTime myTimeMapScanChanged;
  ArTime myTimeMapSupplementChanged;

  // callbacks
  ArRetFunctor1C<bool, ArMapSimple, ArArgumentBuilder *> myMapCategoryCB;
  ArRetFunctor1C<bool, ArMapSimple, ArArgumentBuilder *> mySourcesCB;
  ArRetFunctor1C<bool, ArMapSimple, ArArgumentBuilder *> myDataIntroCB;

  // Handler for unrecognized lines
  ArRetFunctor1C<bool, ArMapSimple, ArArgumentBuilder *> myRemCB;

  bool myIsQuiet;
  bool myIsReadInProgress;
  bool myIsCancelRead;

}; // end class ArMapSimple

/// --------------------------------------------------------------------------- 

#endif // ARMAPCOMPONENTS_H

