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

#include "ArMapUtils.h"

#include "ariaUtil.h"
#include "ArBasePacket.h"
#include "ArMapComponents.h"
#include "ArMD5Calculator.h"

#include <iterator>

//#define ARDEBUG_MAPUTILS
#ifdef ARDEBUG_MAPUTILS
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

// -----------------------------------------------------------------------------
// ArMapId
// -----------------------------------------------------------------------------

AREXPORT ArMapId::ArMapId() :
  mySourceName(),
  myFileName(),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(0),
  myTimestamp(-1)
{
}

AREXPORT ArMapId::ArMapId(const char *sourceName,
						              const char *fileName,
						              const unsigned char *checksum,
                          size_t checksumLength,
						              long int size,
						              const time_t timestamp) :
  mySourceName((sourceName != NULL) ? sourceName : ""),
  myFileName((fileName != NULL) ? fileName : ""),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(size),
  myTimestamp(timestamp)
{
  if (checksumLength > 0) {
    setChecksum(checksum,
                checksumLength);
  }
}

AREXPORT ArMapId::ArMapId(const ArMapId &other) :
  mySourceName(other.mySourceName),
  myFileName(other.myFileName),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(other.mySize),
  myTimestamp(other.myTimestamp)
{
  if (other.myChecksumLength > 0) {
    setChecksum(other.myChecksum,
                other.myChecksumLength);
  }
}

AREXPORT ArMapId &ArMapId::operator=(const ArMapId &other)
{
  if (&other != this) {

    mySourceName = other.mySourceName;
    myFileName = other.myFileName;

    delete [] myChecksum;
    myChecksum = NULL;
    myChecksumLength = 0;

    delete [] myDisplayChecksum;
    myDisplayChecksum = NULL;
    myDisplayChecksumLength = 0;

    if (other.myChecksumLength > 0) {
      setChecksum(other.myChecksum,
                  other.myChecksumLength);
    }


    mySize = other.mySize;
    myTimestamp = other.myTimestamp;
  }
  return *this;
}

AREXPORT ArMapId::~ArMapId()
{
  delete [] myChecksum;
  myChecksum = NULL;
  myChecksumLength = 0;

  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;
}

AREXPORT bool ArMapId::isNull() const
{
  // TODO Any need to check others?
  bool b = (ArUtil::isStrEmpty(mySourceName.c_str()) &&
            ArUtil::isStrEmpty(myFileName.c_str()));

  return b; 
}
  
AREXPORT void ArMapId::clear()
{
  mySourceName = "";
  myFileName = "";

  delete [] myChecksum;
  myChecksum = NULL;
  myChecksumLength = 0;

  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;

  mySize = 0;
  myTimestamp = -1;

} // end method clear


AREXPORT const char *ArMapId::getSourceName() const
{
  return mySourceName.c_str();
}

AREXPORT const char *ArMapId::getFileName() const
{
  return myFileName.c_str();
}

AREXPORT const unsigned char *ArMapId::getChecksum() const
{
  return myChecksum;
}
  
AREXPORT size_t ArMapId::getChecksumLength() const
{
  return myChecksumLength;
} 
  
AREXPORT const char *ArMapId::getDisplayChecksum() const
{
  if ((myDisplayChecksum == NULL) && (myChecksumLength > 0)) {

    myDisplayChecksumLength = ArMD5Calculator::DISPLAY_LENGTH;
    myDisplayChecksum = new char[myDisplayChecksumLength];

    ArMD5Calculator::toDisplay(myChecksum,
                               myChecksumLength,
                               myDisplayChecksum,
                               myDisplayChecksumLength);

  }

  return myDisplayChecksum;
} 

AREXPORT long int ArMapId::getSize() const
{
  return mySize;
}

AREXPORT time_t ArMapId::getTimestamp() const
{
  return myTimestamp;
}
    
AREXPORT bool ArMapId::isSameFile(const ArMapId &other) const
{
  if (ArUtil::strcasecmp(myFileName, other.myFileName) != 0) {
    return false;
  }
  if (mySize != other.mySize) {
    return false;
  }

  // If both timestamps are specified, then they must be identical...
  // KMC 8/29/13 Is this true?  Running CS and sim, had a scenario
  // where everything was the same except the timestamps. Presumably
  // sim wrote file even though nothing had changed. I think that 
  // the file name, size and checksum comparison is probably sufficient 
  // for this purpose.
  /***
  if ((myTimestamp != other.myTimestamp) &&
      (isValidTimestamp()) &&
      (other.isValidTimestamp())) {
    return false;
  }
  ***/
  if (myChecksumLength != other.myChecksumLength) {
    return false;
  }
  if ((myChecksum != NULL) && (other.myChecksum != NULL)) {
    return (memcmp(myChecksum, other.myChecksum, myChecksumLength) == 0);
  }
  return true; // ??
}


AREXPORT bool ArMapId::isVersionOfSameFile(const ArMapId &other) const
{
  if ((ArUtil::strcasecmp(mySourceName, other.mySourceName) == 0) && 
      (ArUtil::strcasecmp(myFileName, other.myFileName) == 0)) {
    return true;
  }
  return false;

} // end method isVersionOfSameFile
  
AREXPORT bool ArMapId::isValidTimestamp() const
{
  bool b = ((myTimestamp != -1) &&
            (myTimestamp != 0));
  return b;
}


AREXPORT void ArMapId::setSourceName(const char *sourceName)
{
  if (sourceName != NULL) {
    mySourceName = sourceName;
  }
  else {
    mySourceName = "";
  }
}

AREXPORT void ArMapId::setFileName(const char *fileName)
{
  if (fileName != NULL) {
    myFileName = fileName;
  }
  else {
    myFileName = "";
  }
}

AREXPORT void ArMapId::setChecksum(const unsigned char *checksum,
                                   size_t checksumLen)
{
  if (checksumLen < 0) {
    checksumLen = 0;
  }
  if (checksumLen != myChecksumLength) {
    delete [] myChecksum;
    myChecksum = NULL;
    myChecksumLength = 0;
    //AB 01/05/2017 - earlier if checksum length was the same,
    //we used to allocate myChecksum even if it was previously allocated
    if (checksumLen > 0) {
      myChecksumLength = checksumLen;
      myChecksum = new unsigned char[myChecksumLength];
      if (myChecksum != NULL)
        memcpy(myChecksum, checksum, myChecksumLength);
    }
  }
  else
  {
    //if checksum length is the same as myChecksumLength, just
    //copy over the new values
    if (myChecksum != NULL)
      memcpy(myChecksum, checksum, myChecksumLength);
  }
  // Clear this so that it is calculated if necessary....
  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;



}

AREXPORT void ArMapId::setSize(long int size)
{
  mySize = size;
}

AREXPORT void ArMapId::setTimestamp(const time_t &timestamp)
{
  IFDEBUG(ArLog::log(ArLog::Normal,
                     "ArMapId::setTimestamp() time = %i", 
                     timestamp));

  myTimestamp = timestamp;
}

/// Determines whether two IDs are equal.
AREXPORT bool operator==(const ArMapId & id1, const ArMapId & id2)
{  
  // The mirror opposite of operator!=

  // Filename is compared last just because it takes longer
  if (id1.mySize != id2.mySize) {
    return false;
  }
  // A null timestamp  (-1) can be "equal" to any other timestamp
  if ((id1.isValidTimestamp()) &&
      (id2.isValidTimestamp()) &&
      (id1.myTimestamp != id2.myTimestamp)) {
    return false;
  }
  if (id1.myChecksumLength != id2.myChecksumLength) {
    return false;
  }

  // TODO: Compare only if sources are not null (like timestamps)?
  if (ArUtil::strcasecmp(id1.mySourceName, id2.mySourceName) != 0) {
    return false;
  }
  if (ArUtil::strcasecmp(id1.myFileName, id2.myFileName) != 0) {
    return false;
  }
  if ((id1.myChecksum != NULL) && (id2.myChecksum != NULL)) {
    if (memcmp(id1.myChecksum, id2.myChecksum, id1.myChecksumLength) != 0) {
      return false;
    }
  }
  else if (id1.myChecksum != id2.myChecksum) {
    // The above says that if one of them is null, then both of them
    // must be null.
    return false;
  }

  return true;

} // end method operator==

/// Determines whether two IDs are not equal.
AREXPORT bool operator!=(const ArMapId & id1, const ArMapId & id2)
{
  // The mirror opposite of operator==
  
  // Filename is compared last just because it takes longer
  if (id1.mySize != id2.mySize) {
    return true;
  }
  // A null timestamp  (-1) can be "equal" to any other timestamp
  if ((id1.isValidTimestamp()) &&
      (id2.isValidTimestamp()) &&
      (id1.myTimestamp != id2.myTimestamp)) {
    return true;
  }
  if (id1.myChecksumLength != id2.myChecksumLength) {
    return true;
  }

  // TODO: Compare only if sources are not null (like timestamps)?
  if (ArUtil::strcasecmp(id1.mySourceName, id2.mySourceName) != 0) {
    return true;
  }
  if (ArUtil::strcasecmp(id1.myFileName, id2.myFileName) != 0) {
    return true;
  }
  if ((id1.myChecksum != NULL) && (id2.myChecksum != NULL)) {
    if (memcmp(id1.myChecksum, id2.myChecksum, id1.myChecksumLength) != 0) {
      return true;
    }
  }
  else if (id1.myChecksum != id2.myChecksum) {
    // The above says that if one of them is null, then both of them
    // must be null.
    return true;
  }

  return false;

} // end method operator!=
  

AREXPORT void ArMapId::log(const char *prefix) const
{
  time_t idTime = getTimestamp();

  char timeBuf[500];

  struct tm *idTm = NULL;
  
  if (idTime != -1) {
    idTm = localtime(&idTime);
  }
  if (idTm != NULL) {
    strftime(timeBuf, sizeof(timeBuf), "%c", idTm);
  }
  else {
    snprintf(timeBuf, sizeof(timeBuf), "NULL");
  }

  ArLog::log(ArLog::Normal,
             "%s%smap %s %s%s checksum = \"%s\" size = %i  time = %s (%i)",
             ((prefix != NULL) ? prefix : ""),
             ((prefix != NULL) ? " " : ""),
             getFileName(),
             (!ArUtil::isStrEmpty(getSourceName()) ? "source " : ""),
             (!ArUtil::isStrEmpty(getSourceName()) ? getSourceName() : ""),
             getDisplayChecksum(),
             getSize(),
             timeBuf,
             idTime);
  
}
  
AREXPORT bool ArMapId::fromPacket(ArBasePacket *packetIn,
                                  ArMapId *mapIdOut)
{
  if ((packetIn == NULL) || (mapIdOut == NULL)) {
    return false;
  }

  char sourceBuffer[512];
	packetIn->bufToStr(sourceBuffer, sizeof(sourceBuffer));
	
  char fileNameBuffer[512];
	packetIn->bufToStr(fileNameBuffer, sizeof(fileNameBuffer));

  ArUtil::fixSlashes(fileNameBuffer, sizeof(fileNameBuffer));

  size_t checksumLength = packetIn->bufToUByte4();
 
  unsigned char *checksum = NULL;
  if (checksumLength > 0) {
    checksum = new unsigned char[checksumLength];
    packetIn->bufToData(checksum, checksumLength);
  }

  size_t fileSize = packetIn->bufToUByte4();
  time_t fileTime = packetIn->bufToByte4();

  IFDEBUG(ArLog::log(ArLog::Normal,
                     "ArMapId::fromPacket() time = %i", 
                     fileTime));

  *mapIdOut = ArMapId(sourceBuffer,
                      fileNameBuffer,
                      checksum,
                      checksumLength,
                      fileSize,
                      fileTime);

  IFDEBUG(mapIdOut->log("ArMapId::fromPacket()"));

  delete [] checksum;

  return true;

} // end method fromPacket


AREXPORT bool ArMapId::toPacket(const ArMapId &mapId,
                                ArBasePacket *packetOut)
{
  
  IFDEBUG(mapId.log("ArMapId::toPacket()"));
 
  if (packetOut == NULL) {
    return false;
  }

  if (!ArUtil::isStrEmpty(mapId.getSourceName())) {
    packetOut->strToBuf(mapId.getSourceName());  
  }
  else {
    packetOut->strToBuf("");
  }

  if (!ArUtil::isStrEmpty(mapId.getFileName())) {
    packetOut->strToBuf(mapId.getFileName());
  }
  else {
    packetOut->strToBuf("");
  }

  packetOut->uByte4ToBuf(mapId.getChecksumLength());
  if (mapId.getChecksumLength() > 0) {
    packetOut->dataToBuf(mapId.getChecksum(), mapId.getChecksumLength()); 
  }
  packetOut->uByte4ToBuf(mapId.getSize());
  packetOut->byte4ToBuf(mapId.getTimestamp());
  
  IFDEBUG(ArLog::log(ArLog::Normal,
                     "ArMapId::toPacket() time = %i", 
                     mapId.getTimestamp()));
  
  return true;

} // end method toPacket
  
  
  
AREXPORT bool ArMapId::create(const char *mapFileName,
                              ArMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    ArLog::log(ArLog::Normal,
               "Cannot create null map ID");
    return false;
  }
  if (ArUtil::isStrEmpty(mapFileName)) {
    ArLog::log(ArLog::Verbose,
               "Returning null map ID for null file name");
    mapIdOut->clear();
    return true;
  }

  struct stat mapFileStat;
  if (ArUtil::filestat(mapFileName, &mapFileStat) != 0)
  {
    ArLog::log(ArLog::Normal, 
               "Map file %s not not found", mapFileName);
    mapIdOut->clear();
    return false;
  }

  unsigned char buffer[ArMD5Calculator::DIGEST_LENGTH];
  bool isSuccess = ArMD5Calculator::calculateChecksum(mapFileName,
                                                      buffer,
                                                      sizeof(buffer));

  if (!isSuccess) {
    ArLog::log(ArLog::Normal,
               "Error calculating checksum for map file %s",
               mapFileName);
    mapIdOut->clear();
    return false;
  }

  mapIdOut->setFileName(mapFileName);
	mapIdOut->setChecksum(buffer, sizeof(buffer));
	mapIdOut->setSize(mapFileStat.st_size);
	mapIdOut->setTimestamp(mapFileStat.st_mtime);

  return true;

} // end method create


// -----------------------------------------------------------------------------
// ArMapFileLineSet
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// ArMapFileLineGroup
// -----------------------------------------------------------------------------


void ArMapFileLineGroup::log() 
{
  ArLog::log(ArLog::Normal,
             "#%-3i : %s",
             myParentLine.getLineNum(),
             myParentLine.getLineText());
  for (std::vector<ArMapFileLine>::iterator iter = myChildLines.begin();
       iter != myChildLines.end();
       iter++) {
    ArMapFileLine &fileLine = *iter;
    ArLog::log(ArLog::Normal,
                "     #%-3i : %s",
                fileLine.getLineNum(),
                fileLine.getLineText());
  }
} // end method log



AREXPORT void ArMapFileLineSet::log(const char *prefix)
{
  if (prefix != NULL) {
    ArLog::log(ArLog::Normal,
               prefix);
  }
 
  int i = 0;
  ArMapFileLineSet::iterator mIter = begin();

  for (;((mIter != end()) && (i < 100));
       mIter++, i++) {
    ArMapFileLineGroup &group = *mIter;
    group.log();
  }
 
  if (mIter != end()) {
    ArLog::log(ArLog::Normal,
               "..... (cont.)");
    ArLog::log(ArLog::Normal,
               "Size = %i", size());

  }
} // end method log


AREXPORT ArMapFileLineSet::iterator ArMapFileLineSet::find(const ArMapFileLine &groupParent) {
  for (iterator iter = begin(); iter != end(); iter++) {
    ArMapFileLineGroup &group = *iter;
    if ((group.getParentLine()->getLineNum() == groupParent.getLineNum()) &&
        true) {

        if (strcmp(group.getParentLine()->getLineText(),
                   groupParent.getLineText()) != 0) {
          ArLog::log(ArLog::Normal,
                     "Line #%i text does not match:",
                     group.getParentLine()->getLineNum());
          ArLog::log(ArLog::Normal,
                      "\"%s\"",
                      group.getParentLine()->getLineText());
          ArLog::log(ArLog::Normal,
                      "\"%s\"",
                      groupParent.getLineText());

        }
        //(strcmp(group.getParentLine()->getLineText(),
        //        groupParent.getLineText()) == 0)) {
                  ArLog::log(ArLog::Normal,
                              "Found #%i : %s",
                              groupParent.getLineNum(),
                              groupParent.getLineText());
      return iter;
    }
  }
  return end();
}

bool ArMapFileLineSet::calculateChanges(ArMapFileLineSet &origLines,
                                     ArMapFileLineSet &newLines,
                                     ArMapFileLineSet *deletedLinesOut,
                                     ArMapFileLineSet *addedLinesOut,
                                     bool isCheckChildren)
{
  if ((deletedLinesOut == NULL) || (addedLinesOut == NULL)) {
    return false;
  }
  ArMapFileLineGroupCompare compare;
  ArMapFileLineGroupLineNumCompare compareLineNums;

	std::sort(origLines.begin(), origLines.end(), compare);
	std::sort(newLines.begin(), newLines.end(), compare);

  set_difference(origLines.begin(), origLines.end(), 
                 newLines.begin(), newLines.end(),
                 std::inserter(*deletedLinesOut, 
                          deletedLinesOut->begin()), 
                 compare);

  set_difference(newLines.begin(), newLines.end(),
                 origLines.begin(), origLines.end(), 
                 std::inserter(*addedLinesOut, 
                          addedLinesOut->begin()),
                 compare);

  if (isCheckChildren) {

    ArMapFileLineSet unchangedOrigLines;
    ArMapFileLineSet unchangedNewLines;

    set_difference(origLines.begin(), origLines.end(), 
                   deletedLinesOut->begin(), deletedLinesOut->end(),
                   std::inserter(unchangedOrigLines, 
                            unchangedOrigLines.begin()), 
                   compare);

    set_difference(newLines.begin(), newLines.end(), 
                   addedLinesOut->begin(), addedLinesOut->end(),
                   std::inserter(unchangedNewLines, 
                            unchangedNewLines.begin()), 
                   compare);

    ArMapFileLineCompare compareLine;

    for (ArMapFileLineSet::iterator iterO = unchangedOrigLines.begin(),
                                 iterN = unchangedNewLines.begin();
         ( (iterO != unchangedOrigLines.end()) && 
           (iterN != unchangedNewLines.end()) );
         iterO++, iterN++) {

       ArMapFileLineGroup &origGroup = *iterO;
       ArMapFileLineGroup &newGroup = *iterN;

	     std::sort(origGroup.getChildLines()->begin(), 
                 origGroup.getChildLines()->end(), 
                 compareLine);
	     std::sort(newGroup.getChildLines()->begin(), 
                 newGroup.getChildLines()->end(), 
                 compareLine);

       ArMapFileLineSet tempDeletedLines;
       ArMapFileLineSet tempAddedLines;
     
       set_difference(origGroup.getChildLines()->begin(), 
                      origGroup.getChildLines()->end(), 
                      newGroup.getChildLines()->begin(), 
                      newGroup.getChildLines()->end(),
                      std::inserter(tempDeletedLines, 
                               tempDeletedLines.begin()), 
                      compareLine);

       set_difference(newGroup.getChildLines()->begin(), 
                      newGroup.getChildLines()->end(),
                      origGroup.getChildLines()->begin(), 
                      origGroup.getChildLines()->end(), 
                      std::inserter(tempAddedLines, 
                              tempAddedLines.begin()),
                      compareLine);

        // TODO: Right now just sending the entire group -- but someday
        // we may just want to send the lines that have changed within
        // the group (plus the group heading).
        if (!tempDeletedLines.empty() || !tempAddedLines.empty()) {

          deletedLinesOut->push_back(origGroup);
          addedLinesOut->push_back(newGroup);

        } // end if child changes

    } // end for each unchanged line

  } // end if check children

	std::sort(deletedLinesOut->begin(), deletedLinesOut->end(), compareLineNums);
	std::sort(addedLinesOut->begin(), addedLinesOut->end(), compareLineNums);

  return true;

} // end method calculateChanges


// -----------------------------------------------------------------------------
// ArMapChangeDetails
// -----------------------------------------------------------------------------

void ArMapChangeDetails::createChildArgMap()
{
  myInfoNameToMapOfChildArgsMap["MapInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["TaskInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["MacroInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["Task"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["GoalTask"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["MacroTask"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goto"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goalBefore"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goalAfter"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_everyBefore"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_everyAfter"] = true;

  myInfoNameToMapOfChildArgsMap["SchedTaskInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["SchedInfo:"]["Route"] = true;
  myInfoNameToMapOfChildArgsMap["SchedInfo:"]["SchedTask"] = true;

} // end method createChildArgMap

ArMapChangeDetails::ArMapScanChangeDetails::ArMapScanChangeDetails() :
  myChangedPoints(),
  myChangedLineSegments(),
  myChangedSummaryLines()
{
} // end constructor

ArMapChangeDetails::ArMapScanChangeDetails::~ArMapScanChangeDetails()
{
  // TODO

} // end destructor

AREXPORT ArMapChangeDetails::ArMapChangeDetails() :
  myMutex(),
  myOrigMapId(),
  myNewMapId(),
  myInfoNameToMapOfChildArgsMap(),
  myScanTypeList(),
  myScanTypeToChangesMap(),
  myNullScanTypeChanges(),
  myChangedSupplementLines(),
  myChangedObjectLines(),
  myInfoToChangeMaps()
{
  myMutex.setLogName("ArMapChangeDetails");

  createChildArgMap();

} // end ctor

AREXPORT ArMapChangeDetails::ArMapChangeDetails
                                (const ArMapChangeDetails &other) :
  myMutex(),
  myOrigMapId(other.myOrigMapId),
  myNewMapId(other.myNewMapId),
  myInfoNameToMapOfChildArgsMap(),
  myScanTypeList(other.myScanTypeList),
  myScanTypeToChangesMap(),
  myNullScanTypeChanges(),
  myChangedSupplementLines(),
  myChangedObjectLines(),
  myInfoToChangeMaps()
{
  myMutex.setLogName("ArMapChangeDetails");

  createChildArgMap();

  for (std::map<std::string, ArMapScanChangeDetails*>::const_iterator iter =
                                 other.myScanTypeToChangesMap.begin();
       iter != other.myScanTypeToChangesMap.end();
       iter++) {
    ArMapScanChangeDetails *otherScan = iter->second;
    if (otherScan == NULL) {
      continue;
    }
    myScanTypeToChangesMap[iter->first] = new ArMapScanChangeDetails(*otherScan);
  }

  for (int i = 0; i < CHANGE_TYPE_COUNT; i++) {
    myChangedSupplementLines[i] = other.myChangedSupplementLines[i];
    myChangedObjectLines[i] = other.myChangedObjectLines[i];
    myInfoToChangeMaps[i] = other.myInfoToChangeMaps[i];
  }
  
} // end copy ctor

AREXPORT ArMapChangeDetails &ArMapChangeDetails::operator=
                                  (const ArMapChangeDetails &other)
{
  if (this != &other) {

    myOrigMapId = other.myOrigMapId;
    myNewMapId  = other.myNewMapId;
    myScanTypeList = other.myScanTypeList;

    ArUtil::deleteSetPairs(myScanTypeToChangesMap.begin(),
                           myScanTypeToChangesMap.end());
    myScanTypeToChangesMap.clear();

    for (std::map<std::string, ArMapScanChangeDetails*>::const_iterator iter =
                                  other.myScanTypeToChangesMap.begin();
        iter != other.myScanTypeToChangesMap.end();
        iter++) {
      ArMapScanChangeDetails *otherScan = iter->second;
      if (otherScan == NULL) {
        continue;
      }
      myScanTypeToChangesMap[iter->first] = new ArMapScanChangeDetails(*otherScan);
    }

    for (int i = 0; i < CHANGE_TYPE_COUNT; i++) {
      myChangedSupplementLines[i] = other.myChangedSupplementLines[i];
      myChangedObjectLines[i] = other.myChangedObjectLines[i];
      myInfoToChangeMaps[i] = other.myInfoToChangeMaps[i];
    }
  
  }
  return *this;

} // end operator=


AREXPORT ArMapChangeDetails::~ArMapChangeDetails()
{
  ArUtil::deleteSetPairs(myScanTypeToChangesMap.begin(),
                         myScanTypeToChangesMap.end());
}


AREXPORT bool ArMapChangeDetails::getOrigMapId(ArMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    return false;
  }
  *mapIdOut = myOrigMapId;
  return true;
}

AREXPORT bool ArMapChangeDetails::getNewMapId(ArMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    return false;
  }
  *mapIdOut = myNewMapId;
  return true;
}

AREXPORT void ArMapChangeDetails::setOrigMapId(const ArMapId &mapId)
{
  myOrigMapId = mapId;
  myOrigMapId.log("ArMapChangeDetails::setOrigMapId");
}

AREXPORT void ArMapChangeDetails::setNewMapId(const ArMapId &mapId)
{
  myNewMapId = mapId;
  myNewMapId.log("ArMapChangeDetails::setNewMapId");
}


AREXPORT std::list<std::string> *ArMapChangeDetails::getScanTypes() 
{
  return &myScanTypeList;
}

AREXPORT std::vector<ArPose> *ArMapChangeDetails::getChangedPoints
                                                    (MapLineChangeType change,
                                                     const char *scanType) 
{
  ArMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedPoints[change];
  //return &myChangedPoints[change];
}

AREXPORT std::vector<ArLineSegment> *ArMapChangeDetails::getChangedLineSegments
                                                           (MapLineChangeType change,
                                                            const char *scanType) 
{
  ArMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedLineSegments[change];
  //return &myChangedLineSegments[change];
}

AREXPORT ArMapFileLineSet *ArMapChangeDetails::getChangedSummaryLines
                                               (MapLineChangeType change,
                                                const char *scanType) 
{
  ArMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedSummaryLines[change];
  //return &myChangedSummaryLines[change];
}

AREXPORT ArMapFileLineSet *ArMapChangeDetails::getChangedSupplementLines
                                              (MapLineChangeType change) 
{
  return &myChangedSupplementLines[change];
}

AREXPORT ArMapFileLineSet *ArMapChangeDetails::getChangedObjectLines
                                              (MapLineChangeType change) 
{
  return &myChangedObjectLines[change];
}

AREXPORT ArMapFileLineSet *ArMapChangeDetails::getChangedInfoLines
                                                (const char *infoName,
                                                 MapLineChangeType change) 
{
  if (ArUtil::isStrEmpty(infoName)) {
    ArLog::log(ArLog::Normal, "ArMapChangeDetails::getChangedInfoLines() null info name");
    return NULL;
  }

  std::map<std::string, ArMapFileLineSet>::iterator iter = myInfoToChangeMaps[change].find(infoName);
  if (iter == myInfoToChangeMaps[change].end()) {
    myInfoToChangeMaps[change][infoName] = ArMapFileLineSet();
    iter = myInfoToChangeMaps[change].find(infoName);
  }
  return &(iter->second);
}


bool ArMapChangeDetails::isEmpty() const
{
  if (!myScanTypeToChangesMap.empty()) {
    for (std::map<std::string, ArMapScanChangeDetails*>::const_iterator iter =
                                myScanTypeToChangesMap.begin();
         iter != myScanTypeToChangesMap.end();
         iter++) {
      ArMapScanChangeDetails *scanDetails = iter->second;
      if (scanDetails == NULL) {
        continue;
      }
      for (int c = 0; c < CHANGE_TYPE_COUNT; c++) {
        if (!scanDetails->myChangedLineSegments[c].empty()) {
          return false;
        }
        if (!scanDetails->myChangedPoints[c].empty()) {
          return false;
        }
        if (!scanDetails->myChangedSummaryLines[c].empty()) {
          return false;
        }
      } // end for each change type
    } // end for each scan type
  } // end if scan types

  for (int c = 0; c < CHANGE_TYPE_COUNT; c++) {

    if (!myChangedSupplementLines[c].empty()) {
      return false;
    }
    if (!myChangedObjectLines[c].empty()) {
      return false;
    }

    for (std::map<std::string, ArMapFileLineSet>::const_iterator iter = myInfoToChangeMaps[c].begin();
         iter != myInfoToChangeMaps[c].end();
         iter++) {
      const ArMapFileLineSet &set = iter->second;
      if (!set.empty()) {
        return false;
      }

    }
  } // end for each change type

  // No non-empty member found.
  return true;

} // end method isEmpty



AREXPORT bool ArMapChangeDetails::isChildArg(const char *infoName,
                                             ArArgumentBuilder *arg) const 
{
  if ((arg == NULL) || 
      (arg->getArgc() < 1)) {
    return false;
  }
  const char *argText = arg->getArg(0);

  return isChildArg(infoName, argText);
}


AREXPORT bool ArMapChangeDetails::isChildArg(const char *infoName,
                                             const char *argText) const
{
  if (ArUtil::isStrEmpty(infoName) || ArUtil::isStrEmpty(argText)) {
    return false;
  }
  std::map<std::string, std::map<std::string, bool> >::const_iterator iter1 =
                              myInfoNameToMapOfChildArgsMap.find(infoName);
  if (iter1 == myInfoNameToMapOfChildArgsMap.end()) {
    return false;
  }

  std::map<std::string, bool>::const_iterator iter2 = iter1->second.find(argText);
  if (iter2 != iter1->second.end()) {
    return iter2->second;
  }

  return false;

} // end method isChildArg

ArMapChangeDetails::ArMapScanChangeDetails *ArMapChangeDetails::getScanChangeDetails
                                                      (const char *scanType)
{
  ArMapScanChangeDetails *scanChanges = NULL;

  if (scanType != NULL) {
      
    std::map<std::string, ArMapScanChangeDetails*>::iterator iter = 
                              myScanTypeToChangesMap.find(scanType);
    if (iter != myScanTypeToChangesMap.end()) {
      scanChanges = iter->second;
    }
    else {
/**
      ArLog::log(ArLog::Normal,
        "ArMapChangeDetails::getScanChangeDetails() adding details for scan type %s",
        scanType);
**/

      if (ArUtil::isStrEmpty(scanType)) {
        ArLog::log(ArLog::Verbose,
                   "ArMapChangeDetails::getScanChangeDetails() adding empty scan type%s",
                   scanType);
      }
      
      scanChanges = new ArMapScanChangeDetails();
      myScanTypeToChangesMap[scanType] = scanChanges;

      myScanTypeList.push_back(scanType);
    }

  } // end if scanType not null

  if (scanChanges == NULL) {
    scanChanges = &myNullScanTypeChanges;
  }
  return scanChanges;
} // end method getScanChangeDetails


AREXPORT std::list<std::string> ArMapChangeDetails::findChangedInfoNames() const 
{
  std::list<std::string> changedInfoNames;
  std::map<std::string, bool> infoNameToBoolMap;

  for (int change = 0; change < CHANGE_TYPE_COUNT; change++) {
    for (std::map<std::string, ArMapFileLineSet>::const_iterator iter = myInfoToChangeMaps[change].begin();
          iter != myInfoToChangeMaps[change].end();
          iter++) {
      const ArMapFileLineSet &fileLineSet = iter->second;
      if (!fileLineSet.empty()) {
        infoNameToBoolMap[iter->first] = true;
      }
    }
  }  // end for each change type

  for (std::map<std::string, bool>::const_iterator iter2 = infoNameToBoolMap.begin();
        iter2 != infoNameToBoolMap.end();
        iter2++) {
    changedInfoNames.push_back(iter2->first);
  } // end for each info type

  return changedInfoNames;

} // end method getChangedInfoTypes


AREXPORT void ArMapChangeDetails::log()
{

  ArLog::log(ArLog::Normal,
             "");

  myOrigMapId.log("ArMapChangeDetails Original Map ID:");
  myNewMapId.log("ArMapChangeDetails New Map ID:");
  ArLog::log(ArLog::Normal,
             "");

  for (int t = 0; t < CHANGE_TYPE_COUNT; t++) {

    MapLineChangeType change = (MapLineChangeType) t;

    switch (t) {
    case DELETIONS:
      ArLog::log(ArLog::Normal,
                "---- DELETED MAP LINES --------------");
      break;
    case ADDITIONS:
      ArLog::log(ArLog::Normal,
                "---- ADDED MAP LINES ----------------");
      break;

    default:
      return;
    }

    for (std::list<std::string>::iterator iter2 = myScanTypeList.begin();
         iter2 != myScanTypeList.end();
         iter2++) {

      const char *scanType = (*iter2).c_str();

      ArLog::log(ArLog::Normal,
                "%s Point Count:  %i",
                scanType,
                getChangedPoints(change, scanType)->size());
      ArLog::log(ArLog::Normal,
                "%s Line Segment Count:  %i",
                scanType,
                getChangedLineSegments(change, scanType)->size());


      ArLog::log(ArLog::Normal,
                "");

      ArMapFileLineSet *changedSummaryLines = getChangedSummaryLines(change,
                                                                  scanType);
      if (!changedSummaryLines->empty()) {
        std::string scanTypeSummary = scanType;
        scanTypeSummary += " ";
        scanTypeSummary += "Summary Lines";

        changedSummaryLines->log(scanTypeSummary.c_str());
      }
    } // end for each scan type

    if (!myChangedSupplementLines[t].empty()) {
      myChangedSupplementLines[t].log("Map Supplement Lines");
    }

    if (!myChangedObjectLines[t].empty()) {
      myChangedObjectLines[t].log("Map Object Lines");
    }

    for (std::map<std::string, ArMapFileLineSet>::iterator iter = myInfoToChangeMaps[t].begin();
         iter != myInfoToChangeMaps[t].end();
         iter++) {
      ArMapFileLineSet &fileLineSet = iter->second;
      if (!fileLineSet.empty()) {
        fileLineSet.log("      ");
      }
    }
  } // end for each change type 
} // end method log

AREXPORT void ArMapChangeDetails::lock()
{
  myMutex.lock();
}

AREXPORT void ArMapChangeDetails::unlock()
{
  myMutex.unlock();
}

// ------------------------------------------------------------------------------
// ArMapChangedHelper
// ------------------------------------------------------------------------------

AREXPORT ArMapChangedHelper::ArMapChangedHelper() :
  myMapChangedLogLevel(ArLog::Verbose),
  myMapChangedCBList(),
  myPreMapChangedCBList()
{
  myMapChangedCBList.setName("MapChangedHelper");
  myPreMapChangedCBList.setName("PreMapChangedHelper");
}

AREXPORT ArMapChangedHelper::~ArMapChangedHelper()
{
}

AREXPORT void ArMapChangedHelper::invokeMapChangedCallbacks(bool invokePathPlanningCB,
                                                            bool invokeLocalizationCB)
{
  ArLog::LogLevel level = myMapChangedLogLevel;

  myPreMapChangedCBList.setLogLevel(level);
  myPreMapChangedCBList.invoke();

  //keeping this here so that hopefully the ordering is not messed up
  myMapChangedCBList.setLogLevel(level);
  myMapChangedCBList.invoke();

  if (invokePathPlanningCB) {
    myMapChangedPathPlanningCBList.setLogLevel(level);
    myMapChangedPathPlanningCBList.invoke();
  }

  if (invokeLocalizationCB) {
    myMapChangedLocalizationCBList.setLogLevel(level);
    myMapChangedLocalizationCBList.invoke();
  }
} // end method invokeMapChangedCallbacks


AREXPORT void ArMapChangedHelper::addMapChangedCB(ArFunctor *functor, 
						  int position)
{
  myMapChangedCBList.addCallback(functor, position);
} // end method addMapChangedCB

AREXPORT void ArMapChangedHelper::remMapChangedCB(ArFunctor *functor)
{
  myMapChangedCBList.remCallback(functor);

} // end method remMapChangedCB

AREXPORT void ArMapChangedHelper::addMapChangedPathPlanningCB(ArFunctor *functor, 
                          int position)
{
  myMapChangedPathPlanningCBList.addCallback(functor, position);
} // end method addMapChangedPathPlanningCB

AREXPORT void ArMapChangedHelper::remMapChangedPathPlanningCB(ArFunctor *functor)
{
  myMapChangedPathPlanningCBList.remCallback(functor);

} // end method remMapChangedPathPlanningCB

AREXPORT void ArMapChangedHelper::addMapChangedLocalizationCB(ArFunctor *functor, 
                          int position)
{
  myMapChangedLocalizationCBList.addCallback(functor, position);
} // end method addMapChangedLocalizationCB

AREXPORT void ArMapChangedHelper::remMapChangedLocalizationCB(ArFunctor *functor)
{
  myMapChangedLocalizationCBList.remCallback(functor);
} // end method remMapChangedLocalizationCB

AREXPORT void ArMapChangedHelper::addPreMapChangedCB(ArFunctor *functor,
                                                     int position)
{
  myPreMapChangedCBList.addCallback(functor, position);
} // end method addPreMapChangedCB


AREXPORT void ArMapChangedHelper::remPreMapChangedCB(ArFunctor *functor)
{
  myPreMapChangedCBList.remCallback(functor);

} // end method remPreMapChangedCB


AREXPORT void ArMapChangedHelper::setMapChangedLogLevel(ArLog::LogLevel level)
{
  myMapChangedLogLevel = level;

} // end method setMapChangedLogLevel


AREXPORT ArLog::LogLevel ArMapChangedHelper::getMapChangedLogLevel(void)
{
  return myMapChangedLogLevel;

} // end method getMapChangedLogLevel







