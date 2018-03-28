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
/*  \file ArMapObject.h
 *  \brief Contains the definition of the ArMapObject class.
 *  \date 06/27/08
 *  \author K. Cunningham
*/
#ifndef ARMAPOBJECT_H
#define ARMAPOBJECT_H
 
#include "ariaTypedefs.h"
#include "ariaUtil.h"

/// A point or region of interest in an Aria map.
/**
 * ArMapObject contains the data related to an Aria map object, i.e a point or
 * region of interest in an Aria map that is not part of the sensed obstacle 
 * data.  Examples of map objects include goals, docks, forbidden lines, and 
 * forbidden areas.  Applications may define their own custom ArMapObject types 
 * using the ArMap MapInfo mechanism.  See @ref ArMap for more information.
 * 
 * The basic attributes of an ArMapObject include the type of the map object,
 * an optional name of the object, and its position information.  As mentioned 
 * above, there are two basic categories of ArMapObjects:  
 *
 *  - Points:  A single ArPose in the map. By convention, if a map object
 *    can have an optional heading, then "WithHeading" appears at the end of 
 *    the object type when the heading is specified.  For example, "Goal" 
 *    designates an (x,y) location in the map (any heading or theta value should
 *    be ignored) and "GoalWithHeading" designates
 *    an (x,y,th) location.
 *
 *  - Regions: A set of two ArPoses ("from" and "to") which define a rectangle
 *    or a line.  Rectangles may have an associated rotation value. It is the
 *    rotation to be applied to the "from" and "to" poses <em> around
 *    the global origin </em>.  To retrieve the list of line segments that 
 *    comprise the rotated rectangle's perimeter, use the getFromToSegments()
 *    method.
 * 
 * Note that the ArMapObject is generally immutable.  If an object needs to be 
 * changed, then the original version should simply be replaced with a new one.  
 * See ArMap::getMapObjects(), ArMap::setMapObjects(), and ArMap::mapChanged().
 * 
**/
class ArMapObject
{
public: 

  /// Creates a new ArMapObject whose attributes are as specified in the given arguments
  /**
   * @param arg the ArArgumentBuilder * from which to create the ArMapObject; this
   * should correspond to a parsed line in the ArMap file
   * @return ArMapObject * the newly created map object, or NULL if an error 
   * occurred
  **/
  AREXPORT static ArMapObject *createMapObject(ArArgumentBuilder *arg);
  

  /// ArArgumentBuilder indices for the various map object attributes
  enum ArgIndex {
    TYPE_ARG_INDEX = 0,
    POSE_X_ARG_INDEX = 1,
    POSE_Y_ARG_INDEX = 2,
    TH_ARG_INDEX = 3,
    DESC_ARG_INDEX = 4,
    ICON_ARG_INDEX = 5,
    NAME_ARG_INDEX = 6,
    LAST_POSE_ARG_INDEX = NAME_ARG_INDEX,
    FROM_X_ARG_INDEX = 7,
    FROM_Y_ARG_INDEX = 8,
    TO_X_ARG_INDEX = 9,
    TO_Y_ARG_INDEX = 10,
    LAST_ARG_INDEX = TO_Y_ARG_INDEX
  };

  enum {
    ARG_INDEX_COUNT = LAST_ARG_INDEX + 1
  };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Instance Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Constructor
  /**
   * @param type the char * type of the map object (e.g. "Goal", "ForbiddenLine");
   * must be non-empty
   * @param pose the primary ArPose of the map object; for points, this is its
   * location; for rectangles, this specifies the rotation of the rectangle (in
   * pose.getTh())
   * @param description an optional char * description of the object.
   * @param iconName char * currently unused: use "ICON" or NULL as a dummy value. Must
   * be a non-empty, non-whitespace string.
   * @param name the char * name of the map object; depending on the object type,
   * this may be optional or required
   * @param hasFromTo a bool set to true if the object is a region (i.e. line or
   * rectangle); false if the object is a point
   * @param fromPose the ArPose that defines the start point of the region object;
   * applicable only when hasFromTo is true
   * @param toPose the ArPose that defines the end point of the region object;
   * applicable only when hasFromTo is true
  **/
  AREXPORT ArMapObject(const char *type, 
                       ArPose pose, 
                       const char *description,
 		                   const char *iconName, 
                       const char *name,
 		                   bool hasFromTo, 
                       ArPose fromPose, 
                       ArPose toPose);

  /// Copy constructor
  AREXPORT ArMapObject(const ArMapObject &mapObject);

  /// Assignment operator
  AREXPORT ArMapObject &operator=(const ArMapObject &mapObject);

  /// Destructor
  AREXPORT virtual ~ArMapObject();

  // --------------------------------------------------------------------------
  // Text Attributes:
  // --------------------------------------------------------------------------

  /// Returns the type of the map object
  AREXPORT const char *getType(void) const;

  /// Returns the "base" (or root) type of the map object
  /**
   * If the map object type ends with "WithHeading", then the base is the
   * corresponding heading-less type.  For example, "GoalWithHeading" has a 
   * base type of "Goal".  
   * 
   * If the map object type does not end with "WithHeading", then the base 
   * is the same as the type.
  **/
  AREXPORT const char *getBaseType(void) const;

  /// Returns the name of the map object (if any)
  AREXPORT const char *getName(void) const;

  /// Returns the optional description of the map object
  AREXPORT const char *getDescription() const ;

  /// Returns the icon string of the object 
  /**
   * The use of the ICON field is application-dependent.  It currently contains
   * either the string "ICON" or "ID=<n>".  The ID is used only when auto-numbering
   * has been turned on in the MapInfo.
  **/
  AREXPORT const char *getIconName(void) const;

  /// Returns the numerical identifier of the object, when auto-numbering is on.
  /**
   * This method returns 0 when auto-numbering is off.
  **/
  AREXPORT int getId() const;

  /// Sets the description of the map object
  /**
   * This method really should only be called immediately after the object
   * is created, and before it is added to the map.  (Since the map object
   * isn't intended to be mutable.)  It exists for backwards compatibility.
  **/
  AREXPORT void setDescription(const char *description);


  // --------------------------------------------------------------------------
  // Position Attributes:
  // --------------------------------------------------------------------------

  /// Returns the primary pose of the object 
  /**
   * For points, this is the map object's location; for rectangles, this 
   * specifies the rotation of the rectangle (in getPose().getTh())
  **/
  AREXPORT ArPose getPose(void) const;

  /// Returns true if the map object has valid "from/to" poses (i.e. is a line or rectangle)
  AREXPORT bool hasFromTo(void) const;

  /// Returns the "from" pose for lines and rectangles; valid only if hasFromTo() 
  AREXPORT ArPose getFromPose(void) const;
  /// Returns the "to" pose for lines and rectangles; valid only if hasFromTo() 
  AREXPORT ArPose getToPose(void) const;

  void setPose(ArPose p) { myPose = p; }
  AREXPORT void setFromTo(ArPose from, ArPose to);

  /// Returns the optional rotation of a rectangle; or 0 if none
  /**
   * Note that this function doesn't know whether it actually makes sense 
   * for this map object to have the rotation.  (For example, it makes sense
   * on a ForbiddenArea but not a ForbiddenLine.)
   *
  **/
  AREXPORT double getFromToRotation(void) const;

  /// Gets a list of fromTo line segments that have been rotated
  /**
   * Note that this function doesn't know whether it actually makes sense 
   * for this map object to have the rotation.  (For example, it makes sense
   * on a ForbiddenArea but not a ForbiddenLine.)  This is just here so
   * that everyone doesn't have to do the same calculation.  Note that
   * this might be a little more CPU/Memory intensive transfering
   * these around, so you may want to keep a copy of them if you're
   * using them a lot (but make sure you clear the copy if the map
   * changes).  It may not make much difference on a modern processor
   * though (its set up this way for safety).
  **/
  AREXPORT std::list<ArLineSegment> getFromToSegments(void);

  /// Gets a line segment that goes from the from to the to
  /**
   * Note that this function doesn't know whether this is supposed to
   * be a rectangle or a line.  (For example, it makes sense on a
   * ForbiddenLine but not a ForbiddenAra.)  This is just here to
   * store it.  Note that this might be a little more CPU/Memory
   * intensive transfering these around, so you may want to keep a
   * copy of them if you're using them a lot (but make sure you clear
   * the copy if the map changes).  It may not make much difference on
   * a modern processor though (its set up this way for safety).
  **/
  AREXPORT ArLineSegment getFromToSegment(void);

  /// Computes the center pose of the map object.
  /**
   * This method determines the center of map objects that have a "from" and a 
   * "to" pose (i.e. lines and rectangles).  For map objects that are poses, 
   * this method simply returns the pose.
  **/
  AREXPORT ArPose findCenter(void) const;

  /** Return true if the given point is inside the region of this object,
   * assuming that this object is a region or sector.  False if not.
   */
  bool isPointInside(const ArPose& p) const { 
    if(!hasFromTo()) return false;
    const std::vector<ArPose> v = getRegionVertices();
    if(v.size() > 2)
      return p.isInsidePolygon(v);
    else
      return false;
  }

  /** If this object is a region or sector type, return a std::vector containing
   * the position (in global map coordinate frame) of each corner. I.e. taking
   * object rotation into account, find each corner. If this object is not a
   * region or sector (i.e. does not have "from" and "to" corners), then 
   * an empty std::vector is returned.  The "Theta" components of the vertex
   * ArPose objects is not set or used.
   */
  AREXPORT std::vector<ArPose> getRegionVertices() const;

  // --------------------------------------------------------------------------
  // I/O Methods
  // --------------------------------------------------------------------------

  /// Returns the text representation of the map object
  /**
   * The returned string is suitable for writing to the ArMap file.  Note that
   * the string does NOT include the map object's optional parameters.
  **/
  AREXPORT const char *toString() const;

  /// Returns the text representation of the map object 
  /**
   * This method is equivalent to toString();
  **/
  const char *getStringRepresentation() const {
    return toString();
  }

  /// Writes the map object to the ArLog.
  /**
   * @param intro an optional string that should appear before the object
  **/
  AREXPORT void log(const char *intro = NULL) const;


  // --------------------------------------------------------------------------
  // Miscellaneous Methods
  // --------------------------------------------------------------------------

  /// Less than operator (for sets), orders by position
  AREXPORT bool operator<(const ArMapObject& other) const;


  /// Gets the fileName of the object (probably never used for maps)
  /**
  * This method is maintained solely for backwards compatibility.
  * It now returns the same value as getDescription (i.e. any file names
  * that may have been associated with an object can now be found in the
  * description attribute).
  * @deprecated 
  **/
  AREXPORT const char *getFileName(void) const;

private:

  /// Parses the given arguments and sets the description of the given ArMapObject
  static bool setObjectDescription(ArMapObject *object,
                                   ArArgumentBuilder *arg);

protected:

  /// The type of the map object
  std::string myType;
  /// If non-empty, then myType ends with "WithHeading" and this is the "root"
  std::string myBaseType;

  /// The name of the map object, if any
  std::string myName;
  /// The description of the map object
  std::string myDescription;

  /// For pose objects, THE pose; For rectangle objects, contains the optional rotation
  ArPose myPose;

  /// Reserved for future use
  std::string myIconName;

  /// Whether the map object is a region (line or rect) with "from/to" poses
  bool myHasFromTo;
  /// The "from" pose of a region map object; valid only if myHasFromTo is true
  ArPose myFromPose;
  /// The "to" pose of a region map object; valid only if myHasFromTo is true
  ArPose myToPose;

  /// For rectangle objects, the line segments that comprise the perimeter (even if rotated)
  std::list<ArLineSegment> myFromToSegments;
  /// For line objects, the line
  ArLineSegment myFromToSegment;
  
  /// Text representation written to the map file
  mutable std::string myStringRepresentation;

}; // end class ArMapObject


// =============================================================================

#ifndef SWIG
/// Comparator for two pointers to map objects
/** @swigomit */
struct ArMapObjectCompare : 
  public std::binary_function<const ArMapObject *,
                              const ArMapObject *,
                              bool> 
 {
   /// Returns true if obj1 is less than obj2; NULL pointers are greater than non-NULL
   bool operator()(const ArMapObject *obj1,
                   const ArMapObject *obj2)
   {
     if ((obj1 != NULL) && (obj2 != NULL)) {
       return *obj1 < *obj2;
     }
     else if ((obj1 == NULL) && (obj2 == NULL)) {
       return false;
     }
     else {
       return (obj1 == NULL);
     }
   } // end operator()

 }; // end struct ArMapObjectCompare

#endif //ifndef SWIG

#endif // ARMAPOBJECT_H

