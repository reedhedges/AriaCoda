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
#include "ArMapObject.h"

//#define ARDEBUG_MAP_OBJECT
#ifdef ARDEBUG_MAP_OBJECT
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

AREXPORT ArMapObject *ArMapObject::createMapObject(ArArgumentBuilder *arg)
{
  if (arg->getArgc() < 7) {
    ArLog::log(ArLog::Terse, 
	             "ArMapObject: 'Cairn:' insufficient arguments '%s'", 
               arg->getFullString());
    return NULL;
  } // end if enough args

  bool isSuccess = true;
  ArMapObject *object = NULL;

  // Strip the quotes out of the name
  arg->compressQuoted();

  bool xOk = false;
  bool yOk = false;
  bool thOk = false;

  ArPose pose;
  ArPose fromPose;
  ArPose toPose;
  bool hasFromTo = false;
 
  char *fileBuffer = NULL;
  char *nameBuffer = NULL;

  if (arg->getArgc() >= 11) {
    
    hasFromTo = true;

    double x = arg->getArgDouble(7, &xOk);
    double y = arg->getArgDouble(8, &yOk);
    if (xOk & yOk) {
      fromPose.setPose(x, y);
    }
    else {
      isSuccess = false;
    }

    x = arg->getArgDouble(9, &xOk);
    y = arg->getArgDouble(10, &yOk);
    if (xOk & yOk) {
      toPose.setPose(x, y);
    }
    else {
      isSuccess = false;
    }
  } // end if from to pose

  if (isSuccess) {

    double x  = arg->getArgDouble(1, &xOk);  
    double y  = arg->getArgDouble(2, &yOk);
	  double th = arg->getArgDouble(3, &thOk);
    
    if (xOk && yOk && thOk) {
      pose.setPose(x, y, th);
    }
    else {
      isSuccess = false;
    } 
  }  // end if no error has occurred
  
  if (isSuccess) { 

    const char *fileArg = arg->getArg(4);
    int fileBufferLen = strlen(fileArg) + 1;
    fileBuffer = new char[fileBufferLen];
  
     if (!ArUtil::stripQuotes(fileBuffer, fileArg, fileBufferLen))
     {
       ArLog::log(ArLog::Terse, 
	                "ArMapObjects: 'Cairn:' couldn't strip quotes from fileName '%s'", 
	                fileArg);
       isSuccess = false;
    } // end if error stripping quotes
  } // end if no error has occurred

  if (isSuccess) {

    const char *nameArg = arg->getArg(6);
    int nameBufferLen = strlen(nameArg) + 1;
    nameBuffer = new char[nameBufferLen];
  
    if (!ArUtil::stripQuotes(nameBuffer, nameArg, nameBufferLen))
    {
      ArLog::log(ArLog::Terse, 
	               "ArMapObjects: 'Cairn:' couldn't strip quotes from name '%s'", 
	               nameArg);
      isSuccess = false;
    } // end if error stripping quotes

  } // end if no error has occurred
  
  if (isSuccess) { 

    object = new ArMapObject(arg->getArg(0), 
                             pose, 
                             fileBuffer,
			                       arg->getArg(5), 
                             nameBuffer, 
                             hasFromTo, 
                             fromPose, 
			                       toPose);

    if (!setObjectDescription(object, arg)) {
      isSuccess = false;
    }
  } // end if no error has occurred


  delete [] fileBuffer;
  delete [] nameBuffer;
  
  if (isSuccess) {
    return object;
  }
  else {
    delete object;
    return NULL;
  }

} // end method createMapObject


bool ArMapObject::setObjectDescription(ArMapObject *object,
                                       ArArgumentBuilder *arg)
{
  if ((object == NULL) || (arg == NULL)) {
    return false;
  }
  unsigned int descArg = 0;
  if (object->hasFromTo()) {
    descArg = 11;
  }
  else {
    descArg = 7;
  }

  if (arg->getArgc() >= (descArg + 1)) {
   size_t descLen = strlen(arg->getArg(descArg)) + 1;
   char *descBuffer = new char[descLen];

   if (!ArUtil::stripQuotes(descBuffer, arg->getArg(descArg), descLen))
   {
      ArLog::log(ArLog::Terse, 
	        "ArMap: 'Cairn:' couldn't strip quotes from desc '%s'", 
	        arg->getArg(descArg));
      delete [] descBuffer;
      return false;
   }
   object->setDescription(descBuffer);
   delete [] descBuffer;
  }
  return true;

} // end method setObjectDescription


AREXPORT ArMapObject::ArMapObject(const char *type, 
					                        ArPose pose, 
                                  const char *description,
 		                              const char *iconName, 
                                  const char *name,
 		                              bool hasFromTo, 
                                  ArPose fromPose, 
                                  ArPose toPose) :
  myType((type != NULL) ? type : ""),
  myBaseType(),
  myName((name != NULL) ? name : "" ),
  myDescription((description != NULL) ? description : "" ),
  myPose(pose),
  myIconName((iconName != NULL) ? iconName : "" ),
  myHasFromTo(hasFromTo),
  myFromPose(fromPose),
  myToPose(toPose),
  myFromToSegments(),
  myStringRepresentation()
{
  if (myHasFromTo)
  {
    setFromTo(fromPose, toPose);
  }
  else { // pose only
    size_t whPos = myType.rfind("WithHeading");
    size_t whLen = 11;
    if (whPos > 0) {
      if (whPos == myType.size() - whLen) {
        myBaseType = myType.substr(0, whPos);
      }
    }
  } // end else pose

  IFDEBUG(
  ArLog::log(ArLog::Normal, 
             "ArMapObject::ctor() created %s (%s)",
             myName.c_str(), myType.c_str());
  );

} // end ctor

AREXPORT void ArMapObject::setFromTo(ArPose fromPose, ArPose toPose)
{
    double angle = myPose.getTh();
    double sa = ArMath::sin(angle);
    double ca = ArMath::cos(angle);
    double fx = fromPose.getX();
    double fy = fromPose.getY();
    double tx = toPose.getX();
    double ty = toPose.getY();
    ArPose P0((fx*ca - fy*sa), (fx*sa + fy*ca));
    ArPose P1((tx*ca - fy*sa), (tx*sa + fy*ca));
    ArPose P2((tx*ca - ty*sa), (tx*sa + ty*ca));
    ArPose P3((fx*ca - ty*sa), (fx*sa + ty*ca));
    myFromToSegments.clear();
    myFromToSegments.push_back(ArLineSegment(P0, P1));
    myFromToSegments.push_back(ArLineSegment(P1, P2));
    myFromToSegments.push_back(ArLineSegment(P2, P3));
    myFromToSegments.push_back(ArLineSegment(P3, P0));
    myFromToSegment.newEndPoints(fromPose, toPose);
    myFromPose = fromPose;
    myToPose = toPose;
    myHasFromTo = true;
}

/// Copy constructor
AREXPORT ArMapObject::ArMapObject(const ArMapObject &mapObject) :
  myType(mapObject.myType),
  myBaseType(mapObject.myBaseType),
  myName(mapObject.myName),
  myDescription(mapObject.myDescription),
  myPose(mapObject.myPose),
  myIconName(mapObject.myIconName),
  myHasFromTo(mapObject.myHasFromTo),
  myFromPose(mapObject.myFromPose),
  myToPose(mapObject.myToPose),
  myFromToSegments(mapObject.myFromToSegments),
  myFromToSegment(mapObject.myFromToSegment),
  myStringRepresentation(mapObject.myStringRepresentation)
{
}


AREXPORT ArMapObject &ArMapObject::operator=(const ArMapObject &mapObject)
{
  if (&mapObject != this) {

    myType = mapObject.myType;
    myBaseType = mapObject.myBaseType;
    myName = mapObject.myName;
    myDescription = mapObject.myDescription;
    myPose = mapObject.myPose;
    myIconName = mapObject.myIconName;
    myHasFromTo = mapObject.myHasFromTo;
    myFromPose = mapObject.myFromPose;
    myToPose = mapObject.myToPose;
    myFromToSegments = mapObject.myFromToSegments;
    myFromToSegment = mapObject.myFromToSegment;
    myStringRepresentation = mapObject.myStringRepresentation;
  }
  return *this;

} // end operator=

/// Destructor
AREXPORT ArMapObject::~ArMapObject() 
{
}

/// Gets the type of the object
AREXPORT const char *ArMapObject::getType(void) const { return myType.c_str(); }


AREXPORT const char *ArMapObject::getBaseType(void) const 
{ 
  if (!myBaseType.empty()) {
    return myBaseType.c_str();
  }
  else {
    return myType.c_str(); 
  }
}

/// Gets the pose of the object 
AREXPORT ArPose ArMapObject::getPose(void) const { return myPose; }

/// Gets the fileName of the object (probably never used for maps)
/**
* This method is maintained solely for backwards compatibility.
* It now returns the same value as getDescription (i.e. any file names
* that may have been associated with an object can now be found in the
* description attribute).
* @deprecated 
**/
AREXPORT const char *ArMapObject::getFileName(void) const { return myDescription.c_str(); }

/// Gets the icon string of the object 
AREXPORT const char *ArMapObject::getIconName(void) const { return myIconName.c_str(); }

/// Returns the numerical identifier of the object, when auto-numbering is on.
AREXPORT int ArMapObject::getId() const
{
  // TODO: If this method is going to be called frequently, then the ID should be cached.

  const char *iconText = myIconName.c_str();

  // The most common case is the "normal" object that has the initial "ICON" text.
  if (strcmp(iconText, "ICON") == 0) {
    return 0;
  }
  if (strstr(iconText, "ID=") == iconText) {
    const char *idText = &iconText[3];
    if (!ArUtil::isStrEmpty(idText)) {
      return atoi(idText);
    }
  }
  return 0;

} // end method getId


/// Gets the name of the object (if any)
AREXPORT const char *ArMapObject::getName(void) const { return myName.c_str(); }
/// Gets the addition args of the object
AREXPORT bool ArMapObject::hasFromTo(void) const { return myHasFromTo; }
/// Gets the from pose (could be for line or box, depending)
AREXPORT ArPose ArMapObject::getFromPose(void) const { return myFromPose; }
/// Gets the to pose (could be for line or box, depending)
AREXPORT ArPose ArMapObject::getToPose(void) const { return myToPose; }

AREXPORT double ArMapObject::getFromToRotation(void) const
{
  if (myHasFromTo) {
    return myPose.getTh();
  }
  else {
    return 0;
  }
} // end method getFromToRotation


AREXPORT const char *ArMapObject::getDescription() const 
{
  return myDescription.c_str();
}


AREXPORT void ArMapObject::setDescription(const char *description) 
{
  if (description != NULL) {
    myDescription = description;
  }
  else {
    myDescription = "";
  }
} 


AREXPORT void ArMapObject::log(const char *intro) const { 

  std::string introString;
  if (!ArUtil::isStrEmpty(intro)) {
    introString = intro;
    introString += " ";
  }
  introString += "Cairn:";

  ArLog::log(ArLog::Terse, 
 	           "%s%s",
             introString.c_str(),
             toString());
}


AREXPORT std::list<ArLineSegment> ArMapObject::getFromToSegments(void)
{
  return myFromToSegments;
}

AREXPORT ArLineSegment ArMapObject::getFromToSegment(void)
{
  return myFromToSegment;
}

AREXPORT ArPose ArMapObject::findCenter(void) const
{
  if (!myHasFromTo) {
    return myPose;
  }
  else { // rect

    double centerX = (myFromPose.getX() + myToPose.getX()) / 2.0;
    double centerY = (myFromPose.getY() + myToPose.getY()) / 2.0;

    double angle = myPose.getTh();
    double sa = ArMath::sin(angle);
    double ca = ArMath::cos(angle);

    ArPose centerPose(centerX * ca - centerY * sa,
                      centerX * sa + centerY * ca); 

    return centerPose;

  } // end else a rect
  
} // end method findCenter





AREXPORT const char *ArMapObject::toString(void) const
{
  // Since the ArMapObject is effectively immutable, this is okay to do...
  if (myStringRepresentation.empty()) {

    // The "Cairn" intro is not included in the string representation 
    // because it may be modified (e.g. for inactive objects).
    
    char buf[1024];
    myStringRepresentation += getType();
    myStringRepresentation += " ";      
  
    // It's alright to write out the x and y without a fraction, but the
    // th value must have a higher precision since it is used to rotate
    // rectangles around the global origin.
    snprintf(buf, sizeof(buf),
             "%.0f %.0f %f", 
            myPose.getX(), myPose.getY(), myPose.getTh());
    buf[sizeof(buf) - 1] = '\0';
    myStringRepresentation += buf;
    
    myStringRepresentation += " \"";
    myStringRepresentation += getDescription();
    myStringRepresentation += "\" ";
    myStringRepresentation += getIconName();
    myStringRepresentation += " \"";
    myStringRepresentation += getName();
    myStringRepresentation += "\"";

    if (myHasFromTo)
    {
      snprintf(buf, sizeof(buf),
              " %.0f %.0f %.0f %.0f", 
              myFromPose.getX(), myFromPose.getY(), 
              myToPose.getX(), myToPose.getY());
      buf[sizeof(buf) - 1] = '\0';
      myStringRepresentation += buf;
    }
  }
  return myStringRepresentation.c_str();

} // end method toString


AREXPORT bool ArMapObject::operator<(const ArMapObject& other) const
{
  if (!myHasFromTo) {
    
    if (!other.myHasFromTo) {

      if (myPose == other.myPose) {
        // Fall through to name and type comparisons below
      }
      else {
        return myPose < other.myPose;
      }
    }
    else { // other has from/to poses

      if (myPose == other.myFromPose) {
        // If pose equals the from pose, then always put the !hasFromTo object first
        return true;
      }
      else {
        return myPose < other.myFromPose;
      }
    }
  }
  else { // has from/to poses
    if (!other.myHasFromTo) {

      if (myFromPose == other.myPose) {
        // This is the inverse of the situation above, always put the !hasFromTo object first
        return false;
      }
      else {
        return myFromPose < other.myPose;
      }
    }
    else { // other has from/to poses

      if (myFromPose == other.myFromPose) {
        // Fall through to name and type comparisons below
      }
      else {
        return myFromPose < other.myFromPose;
      }
    }
  } // end else has from/to poses


  int typeCompare = myType.compare(other.myType);

  if (typeCompare != 0) {
    return (typeCompare < 0);
  }
  
  // Equal types, try to compare names

  if (!myName.empty() && !other.myName.empty()) {

    int nameCompare = myName.compare(other.myName);

    if (nameCompare != 0) {
      return (nameCompare < 0);
    }

  } 
  else if (myName.empty() && !other.myName.empty()) {
    return true; 
  }
  else if (!myName.empty() && other.myName.empty()) {
    return false; 
  }

  // Both names empty, must be hasFromTo

  if (myToPose == other.myToPose) {
    // Fall through to name and type comparisons below
  }
  else {
    return myToPose < other.myToPose;
  }

  if (!myDescription.empty() || !other.myDescription.empty()) {
    return (myDescription.compare(other.myDescription) < 0);
  }
 
  // KMC It might be slightly more efficient to call this at the beginning of the 
  // method, but I'm trying to minimize the impact of any change.
  // If they are the same physical pointers, then they are equal. Neither is 
  // "less than" the other. 
  if ((void *) this == (void *) &other) {
    return false;
  }

  ArLog::log(ArLog::Normal,
             "ArMapObject::operator<() two nearly identical objects cannot compare");

  IFDEBUG(log("this"));
  IFDEBUG(log("other")); 

  // Using a pointer comparison for lack of any better ideas.
  return ((void *) this < (void *) &other);


} // end operator<

AREXPORT  std::vector<ArPose> ArMapObject::getRegionVertices() const 
{
  std::vector<ArPose> v;
  if(!hasFromTo())
    return v;  // return empty list
  const double a = getFromToRotation();
  const double sa = ArMath::sin(a);
  const double ca = ArMath::cos(a);
  const double fx = getFromPose().getX();
  const double fy = getFromPose().getY();
  const double tx = getToPose().getX();
  const double ty = getToPose().getY();
  v.reserve(4);
  v.push_back(ArPose((fx*ca - fy*sa), (fx*sa + fy*ca)));
  v.push_back(ArPose((tx*ca - fy*sa), (tx*sa + fy*ca)));
  v.push_back(ArPose((tx*ca - ty*sa), (tx*sa + ty*ca)));
  v.push_back(ArPose((fx*ca - ty*sa), (fx*sa + ty*ca)));
  return v;
}

