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
 * 
 * 
 *
 * Aria maps are implemented by a collection of classes and interfaces.  The 
 * most important is the ArMapInterface, which is defined in ArMapInterface.h
 * and specifies the methods that all Aria maps must provide.  
 * 
 * This file contains the top-level concrete implementation of the 
 * ArMapInterface.  ArMap is basically the map that is used by the robot 
 * to navigate its environment.  In addition to implementing the methods of 
 * ArMapInterface, ArMap also provides a means of hooking into the Aria config.  
 * When the map file name is specified in the Aria config and is changed during 
 * runtime, ArMap loads the new file and notifies all listeners that the map 
 * has been changed.
 * 
 * In order to accomplish this, ArMap has been implemented using the Proxy 
 * design pattern (refer to "Design Patterns", Gamma et al, 1995).  The 
 * participants are as follows:
 * 
 *   - Subject: ArMapInterface.  The common interface for both the RealSubject
 *     and the Proxy.  It is defined in ArMapInterface.h and is actually 
 *     the combination of several smaller interfaces:  ArMapScanInterface,
 *     ArMapObjectsInterface, ArMapInfoInterface, and ArMapSupplementInterface.
 * 
 *   - RealSubject: ArMapSimple.  The real object that the Proxy represents.
 *     It is defined in ArMapComponents.h, and is implemented by a set of 
 *     helper classes that are also defined in that header file. 
 *
 *   - Proxy: ArMap.  Creates and controls access to the RealSubject.  ArMap
 *     actually contains references to two ArMapSimple objects.  One is
 *     permanent and is the map object that is used by the robot.  The other
 *     is transient and is the map object that is currently being read from 
 *     a file following an Aria configuration update.  If the file is 
 *     successfully read, then the loading ArMapSimple object is copied to 
 *     the main permanent one and mapChanged notifications are sent to
 *     registered listeners (see addMapChangedCB()).
 * 
 * The following "diagram" illustrates the basic hierarchy:
 *
 * <pre>  
 *        ArMapInterface                          , Defined in ArMapInterface.h
 *        ^ (extends)  ^ 
 *        |            |
 *        |            |
 *     ArMap +-------> ArMapSimple * +-------> ArMapScan *
 *                 (contains)        |
 *                                   +-------> ArMapObjects *
 *                                   |
 *                                   +-------> ArMapInfo *
 *                                   |
 *                                   +-------> ArMapSupplement *
 *
 *                                              * : Defined in ArMapComponents.h
 * </pre>
 * 
 *
 * This header file originally also contained the definition of ArMapObject.  
 * That class has been moved into its own header file (ArMapObject.h).
 * 
 *
 * Maintenance Note #1: In this case, the use of the Proxy design pattern implies 
 * that modifying the ArMap external interface requires multiple changes:  
 * First, the ArMapInterface (or sub-interface) must be updated.  Then the ArMap 
 * and ArMapSimple classes must be modified.  Depending on the change, one of the
 * helper map components may also need to be modified.  It is expected that 
 * such changes to the interface will be infrequent.
 *
 * @see ArMapInfo
 * @see ArMapObject
 * @see ArMapObjects
 * @see ArMapScan
 * @see ArMapSupplement
**/
#ifndef ARMAP_H
#define ARMAP_H
 
#include "ariaTypedefs.h"
#include "ariaUtil.h"

#include "ArMapComponents.h"
#include "ArMapInterface.h"
#include "ArMapUtils.h"

#include "ArFunctor.h"
#include "ArArgumentBuilder.h"
#include "ArMutex.h"

#include "ArGPSCoords.h" // for ArLLACoords

#include <vector>

class ArFileParser;

/// A map of a two-dimensional space the robot can navigate within, and which can be updated via the Aria config
/**
* ArMap contains data that represents the operating space of the robot, and can
* be used for space searching, localizing, navigating etc.  MobileRobots' ARNL
* and SONARNL localization and navigation libraries use ArMap objects.
* ArMap also provides methods to read and write the map from and to a file,
* along with a mechanism for setting the map file via the global Aria config.
* 
* Types of data stored in a map include sensable obstacles (e.g. walls and 
* furniture in a room) that are represented either as a collection of data 
* points or lines.  The data points are similar to a raster or bit map and 
* are useful for high resolution sensors like lasers.  The lines create a 
* vector map that is useful for low resolution sensors like sonar.  For 
* advanced applications, the sensable obstacle data can be categorized according
* to the type of scan or sensor; see ArMapScanInterface for more information.
* 
* In addition to the obstacle data, the Aria map may contain goals, forbidden
* areas, and other points and regions of interest (a.k.a. "map objects").  
* Advanced applications can extend the set of predefined map object types.
* See @ref MapObjects for more information.
*
* If the application needs to be aware of any changes that are made to the 
* Aria map at runtime, then it should install "mapChanged" callbacks on the map
* using addMapChangedCB().
* If the map file is re-read while the robot is running, then the callbacks 
* are automatically invoked.  If the application makes other changes to the 
* map by calling any of the set methods, then it should call mapChanged() when
* it is finished.
* 
* See @ref MapFileFormat for the exact format of the actual .map file.
* 
* @section MapThreading Thread issuses, and changing the map
* 
* Different threads will need to access the same map data (for example ARNL, 
* ArForbiddenRangeDevice, networking). However, the ArMap class is not 
* inherently thread-safe.  You must call lock() and  unlock() methods, before 
* and after any access to the map data (e.g. calls to getMapObjects(), 
* getPoints(), setMapObjects(), setPoints()).  
* 
* If you are going to use setMapObjects(), setPoints(), setLines(), or
* setMapInfo(), then you should lock() the map beforehand, call the methods,
* then call mapChanged() to invoke the callbacks, and then finally 
* unlock() the map when done.  Note that mapChanged() will only invoke
* the callbacks if the data has actually changed.
* 
* However, there is an exception: the readFile() and writeFile() 
* methods @b do automatically lock the map while they read and write.
* 
* @section MapObjects Map Objects
* 
* In addition to lines and points, maps may contain "map objects", points
* or regions in space that have special meaning. 
*
* Certain types of objects are predefined for all maps.  These include
* Goal, GoalWithHeading, Dock, ForbiddenLine, ForbiddenArea, and RobotHome.  
* 
*  - Goal and GoalWithHeading are basically named ArPoses, the difference being
*    that the "th" (heading) value is only valid for GoalWithHeading.
*  - Dock is an ArPose that must always have a heading.
*  - ForbiddenLine is a boundary line, and ForbiddenArea is a rectangular
*    "sector". The extents of these objects are given as a pair of 
*    poses, a "from" point and a "to" point.
*  - RobotHome may be either an ArPose or a rectangular sector.
* 
* Rectangular objects may also have an associated angle of rotation, which is 
* stored in the object pose theta value (ArMapObject::getPose().getTh()).
* The actual global coordinates of the rectangle must be calculated
* using this angle and its "from-to" values. You can get a list of the 4
* ArLineSegment objects that comprise the rectangle's edges using
* ArMapObject::getFromToSegments(). If you want to do your own calculations,
* see ArMapObject::ArMapObject().
*
* You can get a pointer to the current list of map objects with getMapObjects(),
* and directly modify the list.  You can also replace the current list of 
* map objects with a new one by calling setMapObjects(); this will destroy 
* the old list of map objects.  Call mapChanged() to notify other components 
* that the map has changed.
*
* In addition to the standard map object types,  is also possible to define 
* custom types of map objects using the "MapInfo" metadata section of the
* map file.  For example, if you wished to program some special behavior that 
* would only occur upon reaching certain goals, you could define a new 
* "SpecialGoal" type in the map file, and check for it in your program.  
* See @ref MapFileFormat for the syntax for defining new object types in the 
* map file.  Custom types will appear in Mapper3 and MobilePlanner in 
* drop-down menus, and instances of the custom types will be displayed in
* MobileEyes.  
*
* <i>Important Note</i>: if a map defines special GoalType or DockType items, 
* then it must define <b>all</b> possible goal or dock types, including the 
* default "Goal", "GoalWithHeading", and "Dock" types if you want those
* types to remain available.
*
 * @ingroup OptionalClasses
*/
class ArMap : public ArMapInterface
{
   
public:

  /// Constructor
  /**
   * @param baseDirectory the name of the directory in which to search for map
   * files that are not fully qualified
   * @param addToGlobalConfig a bool set to true if the map file name parameter 
   * should be added to the global config, Aria::getConfig(); false, otherwise
   * @param configSection the char * name of the config section to which to 
   * add the map file name parameter name; applicable only if addToGlobalConfig 
   * is true
   * @param configParam the char * name of the parameter to be added to the 
   * specified configSection; applicable only if addToGlobalConfig is true
   * @param configDesc the char * description of the configParam; applicable
   * only if addToGlobalConfig is true
   * @param ignoreEmptyFileName a bool set to true if an empty file name is a 
   * valid config parameter value; set to false if a failure should be reported
   * when the file name is empty; applicable only if addToGlobalConfig is true
   * @param priority the ArPriority::Priority of the config parameter; 
   * applicable only if addToGlobalConfig is true
   * @param tempDirectory the name of the directory in which to write temporary
   * files when saving a map; if NULL, then the map file is written directly.  
   * Note that using a temp file reduces the risk that the map will be corrupted
   * if the application crashes.
   * @param configProcessFilePriority priority at which ArMap's configuration
   * parameters should be processed by ArConfig.
  **/
  AREXPORT ArMap(const char *baseDirectory = "./",
 		             bool addToGlobalConfig = true, 
 		             const char *configSection = "Files",
 		             const char *configParam = "Map",
 		             const char *configDesc = 
 		                  "Map of the environment that the robot uses for navigation",
 		             bool ignoreEmptyFileName = true,
                 ArPriority::Priority priority = ArPriority::IMPORTANT,
                 const char *tempDirectory = NULL,
		             int configProcessFilePriority = 100);

  /// Copy constructor
  AREXPORT ArMap(const ArMap &other);

  /// Assignment operator
  AREXPORT ArMap &operator=(const ArMap &other);

  /// Destructor
  AREXPORT virtual ~ArMap(void);


  // ===========================================================================
  // ArMapInterface Methods
  // ===========================================================================

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Scan Types Methods 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   
  AREXPORT virtual std::list<std::string> getScanTypes() const;

  AREXPORT virtual bool setScanTypes(const std::list<std::string> &scanTypeList);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Locking / Mutex Methods 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual int lock();

  AREXPORT virtual int tryLock();

  AREXPORT virtual int unlock();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // ArMapScanInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
  // ArMapObjectsInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
 
   AREXPORT virtual void writeObjectListToFunctor
                              (ArFunctor1<const char *> *functor, 
 			                         const char *endOfLineChars);

   /// Translate positions of all map points, lines and objects by the given
   /// @a offset.
   /// Map is not rotated, Theta value of @a offset is ignored.
   /// Make sure to call mapChanged() after recentering map and making any other changes to
   /// notify other modules and network clients.
   AREXPORT virtual void offsetMapContents(const ArPose& offset);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // ArMapInfoInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

   AREXPORT virtual const char *getInfoName(int infoType);

   AREXPORT virtual void writeInfoToFunctor(ArFunctor1<const char *> *functor, 
 			                                      const char *endOfLineChars);
 

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // ArMapSupplementInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual bool hasOriginLatLongAlt();
  bool hasOriginLatLonAlt() { return hasOriginLatLongAlt(); }
  AREXPORT virtual ArPose getOriginLatLong();
  ArPose getOriginLatLon() { return getOriginLatLong(); }
  AREXPORT virtual double getOriginAltitude();
  double getOriginLatitude() {
    return getOriginLatLong().getX();
  }
  double getOriginLongitude() {
    return getOriginLatLong().getY();
  }


  ArLLACoords getOriginLLA() {
    return ArLLACoords(getOriginLatLon(), getOriginAltitude());
  }


  AREXPORT virtual void setOriginLatLongAlt
                                        (bool hasOriginLatLong,
                                         const ArPose &originLatLong,
                                         double altitude,
                                         ArMapChangeDetails *changeDetails = NULL);

  void setOriginLatLonAlt
                                        (bool hasOriginLatLong,
                                         const ArPose &originLatLong,
                                         double altitude,
                                         ArMapChangeDetails *changeDetails = NULL)
  {
    setOriginLatLongAlt(hasOriginLatLong, originLatLong, altitude, changeDetails);
  }


  AREXPORT virtual void writeSupplementToFunctor(ArFunctor1<const char *> *functor, 
			                                          const char *endOfLineChars);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Remaining ArMapInterface Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void clear();

  AREXPORT virtual bool set(ArMapInterface *other);

  AREXPORT virtual ArMapInterface *clone();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Map Changed / Callback Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void mapChanged(void);
  AREXPORT virtual void mapChanged(bool invokePathPlanningCB,
                                   bool invokeLocalizationCB);

  AREXPORT virtual void addMapChangedCB(ArFunctor *functor, int position = 50);

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

  /// Forces the map to reload if the config is changed/reloaded
  AREXPORT void forceMapLoadOnConfigProcessFile(void) 
    { myForceMapLoad = true; }

#ifndef SWIG
  /** @swigomit (can't write to arguments yet) */
  AREXPORT virtual bool readFile(const char *fileName, 
 			                           char *errorBuffer,
                                 size_t errorBufferLen,
                                 unsigned char *md5DigestBuffer,
                                 size_t md5DigestBufferLen);
  AREXPORT bool readFile(const char *fileName, char *errorBuffer, size_t errorBufferLen);
#endif
  AREXPORT bool readFile(const char *fileName);

#ifndef SWIG
  /** @swigomit (can't write to arguments yet) */
  AREXPORT virtual bool writeFile(const char *fileName, 
                                  bool internalCall,
                                  unsigned char *md5DigestBuffer = NULL,
                                  size_t md5DigestBufferLen = 0,
                                  time_t fileTimestamp = -1);
#endif

  virtual bool writeFile(const char *fileName) 
  {
    return writeFile(fileName, false, NULL, 0, -1);
  }

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

  AREXPORT virtual void setIgnoreEmptyFileName(bool ignore);

  AREXPORT virtual bool getIgnoreEmptyFileName(void);

  AREXPORT virtual void setIgnoreCase(bool ignoreCase = false);

  AREXPORT virtual bool getIgnoreCase(void);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Inactive Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual ArMapInfoInterface *getInactiveInfo();

  AREXPORT virtual ArMapObjectsInterface *getInactiveObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Child Objects Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual ArMapObjectsInterface *getChildObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  AREXPORT virtual ArArgumentBuilder *findMapObjectParams
                                          (const char *mapObjectName);

  AREXPORT virtual bool setMapObjectParams(const char *mapObjectName,
                                           ArArgumentBuilder *params,
                                           ArMapChangeDetails  *changeDetails = NULL);


  AREXPORT virtual std::list<ArArgumentBuilder *> *getRemainder();

  AREXPORT virtual void setQuiet(bool isQuiet);
 	
  AREXPORT virtual bool parseLine(char *line);

  AREXPORT virtual void parsingComplete(void);

  AREXPORT virtual bool isLoadingDataStarted();

  AREXPORT virtual bool isLoadingLinesAndDataStarted();

  AREXPORT virtual const char *getMapCategory() {
    return ((myCurrentMap != NULL)? myCurrentMap->getMapCategory(): NULL);
  }

  // ===========================================================================
  // End of ArMapInterface
  // ===========================================================================



  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // ArMap Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Reads a map and changes the config map name to that file
  AREXPORT bool readFileAndChangeConfig(const char *fileName);
  /// Changes the config map name
  AREXPORT void changeConfigMapName(const char *fileName);



 protected:
 
   /// Processes changes to the Aria configuration; loads a new map file if necessary
   bool processFile(char *errorBuffer, size_t errorBufferLen);
 
 protected:
 
   // Lock to protect data during file I/O operations
   ArMutex myMutex;
 
   /// File path in which to find the map file name
   std::string myBaseDirectory;
   /// Name of the map file 
   std::string myFileName;
   /// File statistics for the map file
   struct stat myReadFileStat;

   /// Name of the Aria config parameter that specifies the map file name
   std::string myConfigParam;
   /// Whether to ignore (not process) an empty Aria config parameter
   bool myIgnoreEmptyFileName;
   /// Whether to ignore case when comparing map file names
   bool myIgnoreCase;
 
   /// Whether the Aria config has already been processed at least once
   bool myConfigProcessedBefore;
   /// The name of the map file specified in the Aria config parameter
   char myConfigMapName[MAX_MAP_NAME_LENGTH];
   /// Whether we want to force loading the map for some reasing
   bool myForceMapLoad;
 
   /// The current map used by the robot
   ArMapSimple * const myCurrentMap;
   /// The map that is being loaded, i.e. read from a file; will be copied to the current map if successful
   ArMapSimple * myLoadingMap;
   
   /// Whether to run in "quiet mode", i.e. logging less information
   bool myIsQuiet;
  
   /// Callback that processes changes to the Aria config.
   ArRetFunctor2C<bool, ArMap, char *, size_t> myProcessFileCB;

   std::list<ArFunctor1<std::string>*> myPreParseCBList;

}; // end class ArMap

 
#endif // ARMAP_H
 
 
