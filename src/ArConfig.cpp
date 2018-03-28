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
#include "ariaOSDef.h"
#include "ArConfig.h"
#include "ArArgumentBuilder.h"
#include "ArLog.h"

#include <string>


//#define ARDEBUG_CONFIG

#if (defined(ARDEBUG_CONFIG))
//#if (defined(_DEBUG) && defined(ARDEBUG_CONFIG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

const char *ArConfig::CURRENT_CONFIG_VERSION = "2.0";
const char *ArConfig::CONFIG_VERSION_TAG = "ConfigVersion";

const char *ArConfig::CURRENT_RESOURCE_VERSION = "1.1";
const char *ArConfig::RESOURCE_VERSION_TAG = "ConfigResourceVersion";

AREXPORT const char *ArConfig::CATEGORY_ROBOT_INTERFACE = "Robot Interface";
AREXPORT const char *ArConfig::CATEGORY_ROBOT_OPERATION = "Robot Operation";
AREXPORT const char *ArConfig::CATEGORY_ROBOT_PHYSICAL  = "Robot Physical";
AREXPORT const char *ArConfig::CATEGORY_FLEET     = "Enterprise";
AREXPORT const char *ArConfig::CATEGORY_SECURITY  = "Security";
AREXPORT const char *ArConfig::CATEGORY_DEBUG     = "Debug";

AREXPORT const char *ArConfig::toCategoryName(const char *categoryName)
{
  if (categoryName == NULL) {
    return NULL;
  }
  // KMC 8/15/13 Obviously this should be improved to be less cut-and-paste
  // error prone
  if (ArUtil::strcasecmp(categoryName, CATEGORY_ROBOT_INTERFACE) == 0) {
    return CATEGORY_ROBOT_INTERFACE;
  }
  if (ArUtil::strcasecmp(categoryName, CATEGORY_ROBOT_OPERATION) == 0) {
    return CATEGORY_ROBOT_OPERATION;
  }
  if (ArUtil::strcasecmp(categoryName, CATEGORY_ROBOT_PHYSICAL) == 0) {
    return CATEGORY_ROBOT_PHYSICAL;
  }
  if (ArUtil::strcasecmp(categoryName, CATEGORY_FLEET) == 0) {
    return CATEGORY_FLEET;
  }
  if (ArUtil::strcasecmp(categoryName, CATEGORY_DEBUG) == 0) {
    return CATEGORY_DEBUG;
  }
  return NULL;

} // end method toCategoryName 

/**
   @param baseDirectory a directory to search for files in
   @param noBlanksBetweenParams if there should not be blank lines between params in output
   @param ignoreBounds if this is true bounds checking will be ignored when the file is read in. this should ONLY be used by developers debugging
   @param failOnBadSection if this is true and there is a bad section, then parseFile will fail
   @param saveUnknown if this is true and there are unknown parameters or sections then they will be saved, if false then they will be ignored (can also be set with setSaveUnknown())
 **/
AREXPORT ArConfig::ArConfig(const char *baseDirectory,
			                      bool noBlanksBetweenParams,
			                      bool ignoreBounds, 
			                      bool failOnBadSection,
			                      bool saveUnknown) :
  myRobotName(""),
  myConfigName(""),
  myLogPrefix("ArConfig: "),

  myArgumentParser(NULL),
  myProcessFileCBList(),
  myNoBlanksBetweenParams(noBlanksBetweenParams),
  
  myConfigVersion(),
  myTranslator(NULL),
  myCsvSeparatorChar('\t'), // Or tab?

  mySection(),
  mySectionsToParse(NULL),
  mySectionsNotToParse(),
  myHighestPriorityToParse(ArPriority::FIRST_PRIORITY),
  myLowestPriorityToParse(ArPriority::LAST_PRIORITY),

  myRestartLevelNeeded(ArConfigArg::NO_RESTART),
  myCheckingForRestartLevel(true),

  mySectionBroken(false),
  mySectionIgnored(false),
  myUsingSections(false),

  myParsingListNames(),
  myIsParsingListBroken(false),

  myFileName(),
  myBaseDirectory(), // setBaseDirectory() is called in ctor

  myParser(NULL),
  
  myIgnoreBounds(ignoreBounds),
  myFailOnBadSection(failOnBadSection),

  myDuplicateParams(false),

  mySaveUnknown(saveUnknown),
  myIsQuiet(false),

  myPermissionAllowFactory(true),
  myPermissionSaveUnknown(saveUnknown),
  myProcessFileCallbacksLogLevel(ArLog::Verbose),

  myCategoryToSectionsMap(),
  mySections(),

  myParserCB(this, &ArConfig::parseArgument),
  myVersionCB(this, &ArConfig::parseVersion),
  mySectionCB(this, &ArConfig::parseSection),
  myListBeginCB(this, &ArConfig::parseListBegin),
  myListEndCB(this, &ArConfig::parseListEnd),
  myUnknownCB(this, &ArConfig::parseUnknown)
{
  /* taking this part out and just calling 'addParsrHandler' so all the codes in one spot
  if (!myParser.addHandlerWithError("section", &mySectionCB))
  {
    ArLog::log(ArLog::Normal, "Could not add section to file parser, horrific failure");
    }*/
  addParserHandlers();
  
  myPermissionAllowFactory = true;
  myPermissionSaveUnknown = saveUnknown;

  myArgumentParser = NULL;
  mySaveUnknown = saveUnknown;
  myIsQuiet = false;
  setBaseDirectory(baseDirectory);
  myIgnoreBounds = ignoreBounds;
  myFailOnBadSection = failOnBadSection;
  myProcessFileCallbacksLogLevel = ArLog::Verbose;
  myUsingSections = false;
  mySectionBroken = false;
  mySectionIgnored = false;
  myDuplicateParams = false;

  myParserCB.setName("ArConfig::parseArgument");
  myVersionCB.setName("ArConfig::parseVersion");
  mySectionCB.setName("ArConfig::parseSection");
  myListBeginCB.setName("ArConfig::parseListBegin");
  myListEndCB.setName("ArConfig::parseListEnd");
  myUnknownCB.setName("ArConfig::parseUnknown");

  mySection = "";
}

AREXPORT ArConfig::~ArConfig()
{
  clearAll();
}


AREXPORT ArConfig::ArConfig(const ArConfig &config) :
  myRobotName(""),
  myConfigName(""),
  myLogPrefix("ArConfig: "),

  myArgumentParser(NULL),
  myProcessFileCBList(),
  myNoBlanksBetweenParams(config.myNoBlanksBetweenParams),
  
  myConfigVersion(config.myConfigVersion),
  myTranslator(config.myTranslator),
  myCsvSeparatorChar(config.myCsvSeparatorChar),

  mySection(config.mySection),
  mySectionsToParse(NULL),
  mySectionsNotToParse(config.mySectionsNotToParse),
  myHighestPriorityToParse(config.myHighestPriorityToParse),
  myLowestPriorityToParse(config.myLowestPriorityToParse),

  myRestartLevelNeeded(ArConfigArg::NO_RESTART),
  myCheckingForRestartLevel(true),

  mySectionBroken(config.mySectionBroken),
  mySectionIgnored(config.mySectionIgnored),
  myUsingSections(config.myUsingSections),

  myParsingListNames(),
  myIsParsingListBroken(false),

  myFileName(),
  myBaseDirectory(), // setBaseDirectory() is called in ctor

  myParser(NULL),
  
  myIgnoreBounds(config.myIgnoreBounds),
  myFailOnBadSection(config.myFailOnBadSection),

  myDuplicateParams(config.myDuplicateParams),

  mySaveUnknown(config.mySaveUnknown),
  myIsQuiet(config.myIsQuiet),

  myPermissionAllowFactory(config.myPermissionAllowFactory),
  myPermissionSaveUnknown(config.myPermissionSaveUnknown),
  myProcessFileCallbacksLogLevel(config.myProcessFileCallbacksLogLevel),

  myCategoryToSectionsMap(config.myCategoryToSectionsMap),
  mySections(),

  myParserCB(this, &ArConfig::parseArgument),
  myVersionCB(this, &ArConfig::parseVersion),
  mySectionCB(this, &ArConfig::parseSection),
  myListBeginCB(this, &ArConfig::parseListBegin),
  myListEndCB(this, &ArConfig::parseListEnd),
  myUnknownCB(this, &ArConfig::parseUnknown)
{
  myArgumentParser = NULL;
  setBaseDirectory(config.getBaseDirectory());
  
  std::list<ArConfigSection *>::const_iterator it;
  for (it = config.mySections.begin(); 
       it != config.mySections.end(); 
       it++) 
  {
    mySections.push_back(new ArConfigSection(*(*it)));
  }
  copySectionsToParse(config.mySectionsToParse);

  myParserCB.setName("ArConfig::parseArgument");
  myVersionCB.setName("ArConfig::parseVersion");
  mySectionCB.setName("ArConfig::parseSection");
  myListBeginCB.setName("ArConfig::parseListBegin");
  myListEndCB.setName("ArConfig::parseListEnd");
  myUnknownCB.setName("ArConfig::parseUnknown");

  
  myParser.setQuiet(myIsQuiet);
  remParserHandlers();
  addParserHandlers();

}


AREXPORT ArConfig &ArConfig::operator=(const ArConfig &config)
{
  if (this != &config) {
    
    // Note that the name is not copied...
    IFDEBUG(ArLog::log(ArLog::Verbose,
                       "%soperator= (from %s) begins",
                       myLogPrefix.c_str(),
                       config.myLogPrefix.c_str()));

    // TODO: The following attributes also need to be copied (or 
    // something) -- ditto for copy ctor:
    //     myProcessFileCBList
    //     myParser
    // The callbacks should not be copied
    //     myParserCB
    //     myVersionCB
    //     mySectionCB
    myArgumentParser = NULL;
    setBaseDirectory(config.getBaseDirectory());
    myNoBlanksBetweenParams = config.myNoBlanksBetweenParams;
    myConfigVersion = config.myConfigVersion;
    myIgnoreBounds = config.myIgnoreBounds;    
    myFailOnBadSection = config.myFailOnBadSection;
    mySection = config.mySection;
    mySectionBroken = config.mySectionBroken;
    mySectionIgnored = config.mySectionIgnored;
    myUsingSections = config.myUsingSections;
    myDuplicateParams = config.myDuplicateParams;

    // KMC 7/13/12 Not copying the myParsingListNames because they are 
    // transient internal pointers only used while parsing.
    myIsParsingListBroken = config.myIsParsingListBroken;

    myPermissionAllowFactory = config.myPermissionAllowFactory;
    myPermissionSaveUnknown = config.myPermissionSaveUnknown;
    
    myCategoryToSectionsMap.clear();
    myCategoryToSectionsMap = config.myCategoryToSectionsMap;
    
    clearSections();

    std::list<ArConfigSection *>::const_iterator it;
    for (it = config.mySections.begin(); 
	       it != config.mySections.end(); 
	       it++) 
    {
      mySections.push_back(new ArConfigSection(*(*it)));
    }

    
    copySectionsToParse(config.mySectionsToParse);

    mySectionsNotToParse = config.mySectionsNotToParse;
   
    myHighestPriorityToParse = config.myHighestPriorityToParse;
    myLowestPriorityToParse  = config.myLowestPriorityToParse;
    myRestartLevelNeeded = config.myRestartLevelNeeded;
    myCheckingForRestartLevel = config.myCheckingForRestartLevel;

    remParserHandlers();

    addParserHandlers();
    
    IFDEBUG(ArLog::log(ArLog::Verbose,
                       "%soperator= (from %s) ends",
                       myLogPrefix.c_str(),
                       config.myLogPrefix.c_str()));
  }
  return *this;

}

AREXPORT void ArConfig::copyAndDetach(const ArConfig &config)
{
  if (this != &config) {
    
    // Note that the name is not copied...
    IFDEBUG(ArLog::log(ArLog::Verbose,
                       "%soperator= (from %s) begins",
                       myLogPrefix.c_str(),
                       config.myLogPrefix.c_str()));

    // TODO: The following attributes also need to be copied (or 
    // something) -- ditto for copy ctor:
    //     myProcessFileCBList
    //     myParser
    // The callbacks should not be copied
    //     myParserCB
    //     myVersionCB
    //     mySectionCB
    myArgumentParser = NULL;
    setBaseDirectory(config.getBaseDirectory());
    myNoBlanksBetweenParams = config.myNoBlanksBetweenParams;
    myConfigVersion = config.myConfigVersion;
    myIgnoreBounds = config.myIgnoreBounds;    
    myFailOnBadSection = config.myFailOnBadSection;
    mySection = config.mySection;
    mySectionBroken = config.mySectionBroken;
    mySectionIgnored = config.mySectionIgnored;
    myUsingSections = config.myUsingSections;
    myDuplicateParams = config.myDuplicateParams;

   // KMC 7/13/12 Not copying the myParsingListNames because they are 
    // transient internal pointers only used while parsing.
     myIsParsingListBroken = config.myIsParsingListBroken;

    myPermissionAllowFactory = config.myPermissionAllowFactory;
    myPermissionSaveUnknown = config.myPermissionSaveUnknown;

    myCategoryToSectionsMap.clear();
    myCategoryToSectionsMap = config.myCategoryToSectionsMap;
 
    clearSections();

    std::list<ArConfigSection *>::const_iterator it;
    for (it = config.mySections.begin(); 
	       it != config.mySections.end(); 
	       it++) 
    {
      ArConfigSection *sectionCopy = new ArConfigSection();
      sectionCopy->setQuiet(myIsQuiet);
      sectionCopy->copyAndDetach(*(*it));
      mySections.push_back(sectionCopy);
      //mySections.push_back(new ArConfigSection(*(*it)));
    }

    
    copySectionsToParse(config.mySectionsToParse);
    myHighestPriorityToParse = config.myHighestPriorityToParse;
    myLowestPriorityToParse = config.myLowestPriorityToParse;
    myRestartLevelNeeded = config.myRestartLevelNeeded;
    myCheckingForRestartLevel = config.myCheckingForRestartLevel;
    mySectionsNotToParse = config.mySectionsNotToParse;

    remParserHandlers();
    addParserHandlers();
    
    IFDEBUG(ArLog::log(ArLog::Verbose,
                       "%soperator= (from %s) ends",
                       myLogPrefix.c_str(),
                       config.myLogPrefix.c_str()));
  }
  // Hmmm... Is there any use in returning... ? return *this;

} // end method copyAndDetach


  
/**
 * These names are used solely for log messages.
 * 
 * @param configName a char * descriptive name of the ArConfig instance
 * (e.g. Server or Default)
 * @param robotName an optional char * identifier of the robot that has 
 * the config
**/
AREXPORT void ArConfig::setConfigName(const char *configName,
                                      const char *robotName)
{
  myConfigName = ((configName != NULL) ? configName : "");
  myRobotName = ((robotName != NULL) ? robotName : "");
  myLogPrefix = "";

  if (!myRobotName.empty()) {
    myLogPrefix = myRobotName + ": ";
  }
  myLogPrefix += "ArConfig";

  if (!myConfigName.empty()) {
    myLogPrefix += " (" + myConfigName + ")";
  }

  myLogPrefix += ": ";
} 

AREXPORT void ArConfig::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
  myParser.setQuiet(isQuiet);
}


AREXPORT void ArConfig::setTranslator(ArConfig *xlatorConfig)
{
  myTranslator = xlatorConfig;

  if (myTranslator != NULL) {
    
    if (!mySections.empty()) {
      ArLog::log(ArLog::Normal,
                 "%sArConfig::setTranslator() sections already created",
                 myLogPrefix.c_str());

      for (std::list<ArConfigSection *>::iterator iter = mySections.begin();
           iter != mySections.end();
           iter++) {
        translateSection(*iter);
       }


    }
  }

} // end method setTranslator


AREXPORT ArConfig *ArConfig::getTranslator() const
{
  return myTranslator;

} // end method getTranslator



/** 
 * @param isSummary a bool set to true if only the section names and parameter counts
 * should be logged; false to log detailed parameter information
 * @param sectionNameList a list of the string section names to be logged; if NULL,
 * then the data for all sections are logged
 * @param logPrefix String to prepend to log messages. E.g. "ArConfig::"
**/
AREXPORT void ArConfig::log(bool isSummary,
                            std::list<std::string> *sectionNameList,
                            const char *logPrefix)
{
  std::string origLogPrefix = myLogPrefix;

  if (!ArUtil::isStrEmpty(logPrefix)) {
    myLogPrefix = logPrefix;
  }

  std::list<ArConfigArg> *params = NULL;

  ArLog::log(ArLog::Normal, 
	           "%slog",
             myLogPrefix.c_str());

  for (std::list<ArConfigSection *>::const_iterator it = mySections.begin(); 
       it != mySections.end(); 
       it++) 
  {
    params = (*it)->getParams();

    // If the section names were specified and the current section isn't in the
    // list, then skip the section...
    if (sectionNameList != NULL) {
      if (!ArUtil::isStrInList((*it)->getName(), *sectionNameList)) {
        continue;
      }
    } // end if section names specified


    if (params == NULL) {
      ArLog::log(ArLog::Normal, "    Section %s has NULL params",
                 (*it)->getName());

      continue;
    }

    if (isSummary) {
      ArLog::log(ArLog::Normal, "    Section %s has %i params",
                 (*it)->getName(), params->size());
      continue;
    }

    ArLog::log(ArLog::Normal, "    Section %s:",
                 (*it)->getName());

    for (std::list<ArConfigArg>::iterator pit = params->begin(); pit != params->end(); pit++)
    {
       (*pit).log(false, 1, myLogPrefix.c_str());
    }

  } // end for each section

    ArLog::log(ArLog::Normal, 
	           "%send",
             myLogPrefix.c_str());
  
  myLogPrefix = origLogPrefix;

} // end method log


AREXPORT void ArConfig::clearSections(void)
{
  IFDEBUG(ArLog::log(ArLog::Verbose, 
                     "%sclearSections() begin",
                     myLogPrefix.c_str()));

  while (mySections.begin() != mySections.end())
  {
    delete mySections.front();
    mySections.pop_front();
  }
  // Clear this just in case...
  if (mySectionsToParse != NULL)
  {
    delete mySectionsToParse;
    mySectionsToParse = NULL;
  }

  IFDEBUG(ArLog::log(ArLog::Verbose, 
                     "%sclearSections() end",
                     myLogPrefix.c_str()));

}

AREXPORT void ArConfig::clearAll(void)
{
  clearSections();
  ArUtil::deleteSetPairs(myProcessFileCBList.begin(), 
			                   myProcessFileCBList.end());
  myProcessFileCBList.clear();
}

void ArConfig::addParserHandlers(void)
{
  std::list<ArConfigSection *>::const_iterator it;
  std::list<ArConfigArg> *params;
  std::list<ArConfigArg>::iterator pit;

 if (!myParser.addHandlerWithError(CONFIG_VERSION_TAG, &myVersionCB)) {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, 
	               "%sCould not add ConfigVersion parser (probably unimportant)",
                 myLogPrefix.c_str());
    }
  }
  if (!myParser.addHandlerWithError("section", &mySectionCB)) {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, 
	               "%sCould not add section parser (probably unimportant)",
                 myLogPrefix.c_str());
    }
  }
  if (!myParser.addHandlerWithError(ArConfigArg::LIST_BEGIN_TAG, &myListBeginCB)) {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, 
	               "%sCould not add _listBegin parser",
                 myLogPrefix.c_str());
    }
  }
  if (!myParser.addHandlerWithError(ArConfigArg::LIST_END_TAG, &myListEndCB)) {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, 
	               "%sCould not add _listEnd parser",
                 myLogPrefix.c_str());
    }
  }


  for (it = mySections.begin(); 
       it != mySections.end(); 
       it++) 
  {
    params = (*it)->getParams();
    if (params == NULL)
      continue;

    for (pit = params->begin(); pit != params->end(); pit++)
    {

      (*pit).addToFileParser(&myParser,
                             &myParserCB,                              
                             myLogPrefix.c_str(),
                             myIsQuiet); 

      //if (!myParser.addHandlerWithError((*pit).getName(), &myParserCB)) {
      //  if (!myIsQuiet) {
      //    ArLog::log(ArLog::Verbose, 
		    //             "%sCould not add keyword %s (probably unimportant)",
      //               myLogPrefix.c_str(),
		    //             (*pit).getName());
      //  }
      //}
    }
  }

  // add our parser for unknown params
  if (!myParser.addHandlerWithError(NULL, &myUnknownCB)) {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, 
	               "%sCould not add unknown param parser (probably unimportant)",
                 myLogPrefix.c_str());
    }
  }
}

void ArConfig::remParserHandlers(void)
{
  myParser.remHandler(&myParserCB);
  myParser.remHandler(&myVersionCB);
  myParser.remHandler(&mySectionCB);
  myParser.remHandler(&myListBeginCB);
  myParser.remHandler(&myListEndCB);
  myParser.remHandler(&myUnknownCB);

} // end method remParserHandlers


AREXPORT bool  ArConfig::addSection(const char *categoryName,
                                    const char *sectionName,
                                    const char *sectionDescription)
{
  // KMC 8/10/12 Allowing empty category names for unclassified sections
  if ((categoryName == NULL) || ArUtil::isStrEmpty(sectionName)) {
//  if (ArUtil::isStrEmpty(categoryName) || ArUtil::isStrEmpty(sectionName)) {
    ArLog::log(ArLog::Normal,
               "%sFailed to add section '%s' to category '%s'", 
               myLogPrefix.c_str(), 
               ((sectionName != NULL) ? sectionName : "NULL"),
               ((categoryName != NULL) ? categoryName : "NULL"));
    return false;
  }

  ArConfigSection *section = findSection(sectionName);
  std::string origCategoryName;

  if ((section != NULL) && (!ArUtil::isStrEmpty(section->getCategoryName()))) 
  {
    translateSection(section);

    ArLog::log(ArLog::Normal,
               "%sFailed to add section '%s' to category '%s', already defined in category '%s'", 
               myLogPrefix.c_str(), 
               ((sectionName != NULL) ? sectionName : "NULL"),
               ((categoryName != NULL) ? categoryName : "NULL"),
               section->getCategoryName());
    return false;
  }

  if (section == NULL) {
    ArLog::log(ArLog::Verbose, "%sMaking new section '%s' in category '%s'", 
               myLogPrefix.c_str(), sectionName, categoryName);

    section = new ArConfigSection(sectionName, 
                                  sectionDescription, 
                                  myIsQuiet,
                                  categoryName);
    mySections.push_back(section);
  }
  else {
    ArLog::log(ArLog::Verbose, "%sAssigning existing section '%s' to category '%s'", 
               myLogPrefix.c_str(), sectionName, categoryName);
  
    origCategoryName = (section->getCategoryName() != NULL ? 
                                 section->getCategoryName() : "");
    
    section->setCategoryName(categoryName);
    section->setComment(sectionDescription);
  } 
  translateSection(section);

  // If the section was in a different category, then remove it. 
  if (!origCategoryName.empty() && 
      (ArUtil::strcasecmp(origCategoryName, categoryName) != 0))
  { 
    std::map<std::string, std::list<std::string> >::iterator origCatIter = myCategoryToSectionsMap.find(origCategoryName);
    if (origCatIter != myCategoryToSectionsMap.end()) {
      std::list<std::string> *origSectionList = &(origCatIter->second);
      if (origSectionList != NULL) {
        for (std::list<std::string>::iterator remIter = origSectionList->begin();
             remIter != origSectionList->end();
             remIter++) {
          if (ArUtil::strcasecmp(sectionName, *remIter) == 0) {
            origSectionList->erase(remIter);
            break;
          }
        } // end for each 
      }
               
    }
  } // end if already in a different category 

  // Make sure that the section is not already stored in the category map
  std::list<std::string> *sectionList = NULL;

  std::map<std::string, std::list<std::string> >::iterator catIter = myCategoryToSectionsMap.find(categoryName);
  if (catIter != myCategoryToSectionsMap.end()) {
    sectionList = &(catIter->second);
  }

  bool isSectionFound = false;
  if (sectionList != NULL) {
    isSectionFound = ArUtil::isStrInList(sectionName, *sectionList);
  }

  // If the section is not already in the map, then add it.
  if (!isSectionFound) {
    myCategoryToSectionsMap[categoryName].push_back(sectionName);
  }

  return true;

} // end method addSection


/**
   Set the comment string associated with a section. If the section doesn't 
   exist then it is created.
   @deprecated use addSection instead
    @warning The section name must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  The comment must not contain tab or newline, but '#' and
    ';' are OK within a comment.
**/
AREXPORT void ArConfig::setSectionComment(const char *sectionName, 
					  const char *comment)
{
  ArConfigSection *section = findSection(sectionName);

  if (section == NULL)
  {
    ArLog::log(ArLog::Verbose, "%sMaking new section '%s' (for comment)", 
               myLogPrefix.c_str(), sectionName);

    section = new ArConfigSection(sectionName, comment, myIsQuiet);


    mySections.push_back(section);
  }
  else {
    section->setComment(comment);
  }

  translateSection(section);

}


/**
   Add a flag to a section. If the section doesn't 
   exist then it is created.
    @warning The section name and flags must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
**/
AREXPORT bool ArConfig::addSectionFlags(const char *sectionName, 
					                              const char *flags)
{
  ArConfigSection *section = findSection(sectionName);

  if (section == NULL)
  {
    ArLog::log(ArLog::Verbose, "%sMaking new section '%s' (flags)", 
               myLogPrefix.c_str(), sectionName);
    section = new ArConfigSection(sectionName, NULL, myIsQuiet);
    section->addFlags(flags, myIsQuiet);

    translateSection(section);
    mySections.push_back(section);
  }
  else
    section->addFlags(flags, myIsQuiet);
  return true;
}

/**
   Add a flag to a section. If the section doesn't 
   exist then it is created.
**/
AREXPORT bool ArConfig::remSectionFlag(const char *sectionName, 
				       const char *flag)
{
  ArConfigSection *section = findSection(sectionName);

  if (section == NULL)
    return false;
  
  section->remFlag(flag);
  return true;
}

/** Add a parameter.  
 * @param arg Object containing key, description and value type of this parameter. This object will be copied.... it must already have the priority, display hint, restart level, and other things like that already set.
 * @param sectionName Name of the section to put this parameter in.
 */
AREXPORT bool ArConfig::addParamAsIs(const ArConfigArg &arg, 
				 const char *sectionName)
{
  return addParam(arg, sectionName,
		  arg.getConfigPriority(), 
		  arg.getDisplayHint(),
		  arg.getRestartLevel());
}

/** Add a parameter.  
 * @param arg Object containing key, description and value type of this parameter. This object will be copied.
 * @param sectionName Name of the section to put this parameter in.
 * @param priority Priority or importance of this parameter to a user.
 * @param displayHint Suggest an appropriate UI control for editing this parameter's value. See ArConfigArg::setDisplayHint() for description of display hint format.
 * @param restart restart level
    @warning The parameter name and value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
 */
AREXPORT bool ArConfig::addParam(const ArConfigArg &arg, 
				                         const char *sectionName,
				                         ArPriority::Priority priority,
                                 const char *displayHint,
                                 ArConfigArg::RestartLevel restart)
{
  ArConfigSection *section = findSection(sectionName);

  ArConfigSection *xltrSection = NULL;
  ArConfigArg *xltrArg = NULL;
  if ((myTranslator != NULL) &&
      (arg.getType() != ArConfigArg::SEPARATOR))  
  {
    xltrSection = myTranslator->findSection(sectionName);

    if (xltrSection != NULL) {
      xltrArg = xltrSection->findParam(arg.getName(), 
                                         true); // allow string holders

      if (xltrArg != NULL) {
        IFDEBUG(ArLog::log(ArLog::Normal,
                           "ArConfig::addParam() found translation arg for %s, %s",
                           sectionName, 
                           arg.getName()));
        
      }
      else {
        IFDEBUG(ArLog::log(ArLog::Normal,
                           "ArConfig::addParam() cannot find translation arg for %s, %s",
                           sectionName, 
                           arg.getName()));

      }
    }
    else {
      IFDEBUG(ArLog::log(ArLog::Normal,
                         "ArConfig::addParam() cannot find translation section for %s",
                         sectionName));
    }
  }

  //printf("SECTION '%s' name '%s' desc '%s'\n", sectionName, arg.getName(), arg.getDescription());
  if (section == NULL)
  {
    ArLog::log(ArLog::Verbose, "ArConfigArg %s: Making new section '%s' (for param)", 
               myLogPrefix.c_str(), sectionName);
    section = new ArConfigSection(sectionName, NULL, myIsQuiet);
   
    translateSection(section);

    mySections.push_back(section);
  }
   
  std::list<ArConfigArg> *params = section->getParams();

  if (params == NULL)
  {
    ArLog::log(ArLog::Terse, "%sSomething has gone hideously wrong in ArConfig::addParam",
               myLogPrefix.c_str());
    return false;
  }
  
  // Don't add consecutive separators
  if (arg.getType() == ArConfigArg::SEPARATOR && 
      !params->empty() && params->back().getType() == ArConfigArg::SEPARATOR)
  {
    //ArLog::log(ArLog::Verbose, "Last parameter a sep, so is this one, ignoring it");
    return true;
  }

  // see if we have this parameter in another section so we can require sections
  std::list<ArConfigSection *>::iterator sectionIt;
  
  for (sectionIt = mySections.begin(); 
       sectionIt != mySections.end(); 
       sectionIt++)
  {
    ArConfigSection *curSection = *sectionIt;

    ArConfigArg *existingParam = NULL;
    if (!ArUtil::isStrEmpty(arg.getName())) {
      existingParam = curSection->findParam(arg.getName());
    }

    // if we have an argument of this name but we don't have see if
    // this section is our own, if its not then note we have
    // duplicates
    if (existingParam != NULL)  
    {
      if (strcasecmp(curSection->getName(), section->getName()) != 0) {
        ArLog::log(ArLog::Verbose, 
                  "%sParameter %s (type %s) name duplicated in section %s and %s",
                  myLogPrefix.c_str(),
                  arg.getName(), 
                  ArConfigArg::toString(arg.getType()),
                  curSection->getName(), section->getName());
        myDuplicateParams = true;
      }
      else {
        ArLog::log(((!existingParam->isPlaceholder()) ? ArLog::Normal : ArLog::Verbose), 
                  "%sParameter %s (type %s) already exists in section %s (placeholder = %i)",
                  myLogPrefix.c_str(),
		              arg.getName(), 
                  ArConfigArg::toString(arg.getType()),
                  section->getName(),
                  existingParam->isPlaceholder());
        
      }
    }
  } // end for each section
  
  // now make sure we can add it to the file parser (with the section
  // stuff its okay if we can't)
  if (!myParser.addHandlerWithError(arg.getName(), &myParserCB))
  {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, "%sCould not add parameter '%s' to file parser, probably already there.", 
	               myLogPrefix.c_str(), arg.getName());
    }
    //return false;
  }

  // now we add all the list names to the parser too (the function
  // makes sure it's a list)
  addListNamesToParser(arg);

  // remove any string and list holders for this param
  section->remStringHolder(arg.getName());
  
  // KMC 12/8/13 This comment is incorrect.  I believe that we intentionally allow duplicate
  // names so that multiple objects can reference the same param value. (True?) 
  // we didn't have a parameter with this name so add it
  params->push_back(arg);

  if (xltrArg != NULL) {
    params->back().copyTranslation(*xltrArg);
  }

  params->back().setConfigPriority(priority);
  params->back().setDisplayHint(displayHint);
  params->back().setRestartLevel(restart);

  params->back().setIgnoreBounds(myIgnoreBounds);
  params->back().replaceSpacesInName();

  IFDEBUG(ArLog::log(ArLog::Verbose, "%sAdded parameter '%s' to section '%s'", 
                      myLogPrefix.c_str(), arg.getName(), section->getName()));
  //arg.log();
  return true;
}

/**
 * Add a comment to a section.
   @param comment Text of the comment.
   @param sectionName Name of the section to add the comment to. If the section does not exist, it will be created.
   @param priority Priority or importance.
    @warning The section name must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  The comment must not contain tab or newline, but '#' and
    ';' are OK within a comment.
**/
AREXPORT bool ArConfig::addComment(const char *comment, const char *sectionName, 
				   ArPriority::Priority priority)
{
  return addParam(ArConfigArg(comment), sectionName, priority);
}
  
AREXPORT bool ArConfig::parseVersion(ArArgumentBuilder *arg, 
			                               char *errorBuffer,
			                               size_t errorBufferLen)
{
  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    errorBuffer[0] = '\0';
  }
  if ((arg->getArgc() < 0) || (arg->getArg(0) == NULL)) {
    if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
      snprintf(errorBuffer, errorBufferLen,
               "Configuration version error (blank");
      errorBuffer[errorBufferLen - 1] = '\0';
    }
    return false;
  } // end if no version

  myConfigVersion = arg->getArg(0);
  ArLog::log(ArLog::Normal,
              "%sConfig version: %s",
              myLogPrefix.c_str(),
              myConfigVersion.c_str());

  // Someday this may do something more interesting... like check
  // for compatibility in the versions.

  return true;

} // end method parseVersion

/**
   The extra string of the parser should be set to the 'Section'
   command while the rest of the arg should be the arguments to the
   section command. Its case insensitive.

   @param arg Should contain the 'Section' keyword as its "extra" string, and section name as argument(s).
   
   @param errorBuffer if this is NULL it is ignored, otherwise the
   string for the error is put into the buffer, the first word should
   be the parameter that has trouble
   
   @param errorBufferLen the length of the error buffer
 **/
AREXPORT bool ArConfig::parseSection(ArArgumentBuilder *arg,
				                             char *errorBuffer,
				                             size_t errorBufferLen)
{
  if (myFailOnBadSection && errorBuffer != NULL)
    errorBuffer[0] = '\0';

  std::list<ArConfigSection *>::iterator sectionIt;
  ArConfigSection *section = NULL;

  if (myFailOnBadSection && errorBuffer != NULL)
    errorBuffer[0] = '\0';
  for (sectionIt = mySections.begin(); 
    sectionIt != mySections.end(); 
    sectionIt++)
  {
    section = (*sectionIt);
    if (ArUtil::strcasecmp(section->getName(), arg->getFullString()) == 0)
    {
      bool isParseSection = true;
      if (mySectionsToParse != NULL) {
        isParseSection = false;
        for (std::list<std::string>::iterator sIter = mySectionsToParse->begin();
          sIter != mySectionsToParse->end();
          sIter++) {
            std::string sp = *sIter;
            if (ArUtil::strcasecmp(section->getName(), sp.c_str()) == 0) {
              isParseSection = true;
              break;
            } // end if section 
        } // end for each section to parse

      } // end else sections to parse specified
      if (mySectionsNotToParse.find(section->getName()) != 
        mySectionsNotToParse.end())
      {
        isParseSection = false;
      }

      if (isParseSection) {
        ArLog::log(ArLog::Verbose, "%sConfig switching to section '%s'",
          myLogPrefix.c_str(),
          arg->getFullString());
        //printf("Config switching to section '%s'\n", 
        //arg->getFullString());
        mySection = arg->getFullString();
        mySectionBroken = false;
        mySectionIgnored = false;
        myUsingSections = true;
        return true;
      }
      else { // section is valid but shouldn't be parsed
        ArLog::log(ArLog::Verbose, "%signoring section '%s'", 
          myLogPrefix.c_str(),
          arg->getFullString());
        //printf("Config switching to section '%s'\n", 
        //arg->getFullString());
        mySection = arg->getFullString();
        mySectionBroken = false;
        mySectionIgnored = true;
        myUsingSections = true;
        return true;

      } // end else don't parse section
    } // end if section found
  } // end for each section


  if (myFailOnBadSection)
  {
    mySection = "";
    mySectionBroken = true;
    mySectionIgnored = false;
    snprintf(errorBuffer, errorBufferLen,  "ArConfig: Could not find section '%s'", 
      arg->getFullString());

    ArLog::log(ArLog::Terse,  
      "%sCould not find section '%s', failing", 
      myLogPrefix.c_str(),
      arg->getFullString());
    return false;
  }
  else
  {
    if (mySaveUnknown && mySectionsToParse == NULL && 
      myPermissionSaveUnknown)
    {
      ArLog::log(ArLog::Verbose, "%smaking new section '%s' to save unknown", 
        myLogPrefix.c_str(), arg->getFullString());
      mySection = arg->getFullString();
      mySectionBroken = false;
      mySectionIgnored = false;
      section = new ArConfigSection(arg->getFullString(), NULL, myIsQuiet);

      translateSection(section);

      mySections.push_back(section);
    }
    else
    {
      mySection = "";
      mySectionBroken = false;
      mySectionIgnored = true;
      ArLog::log(ArLog::Normal,  
        "%sIgnoring unknown section '%s'", 
        myLogPrefix.c_str(),
        arg->getFullString());
    }
    return true;
  }
}


AREXPORT bool ArConfig::parseListBegin(ArArgumentBuilder *arg,
				                               char *errorBuffer,
				                               size_t errorBufferLen)
{
  if ((arg == NULL) || (arg->getArgc() < 1)) {
    ArLog::log(ArLog::Normal,
               "ArConfig::parseListBegin() invalid arg input");
    return false;
  }
  if (mySectionIgnored) {
    return true;
  }
  if (mySectionBroken) {
    if (myFailOnBadSection)
    {
      return false;
    }
    else {
      return true;
    }
  }


  if (myIsParsingListBroken) {
    // KMC 10/25/12 Changing this under the theory that when the containing 
    // list is not found, true is still returned.  I believe that indicates
    // it's a recoverable error ... i.e. the corresponding list parameter has
    // not been added to the config.
    return true; // false;
  }

  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    errorBuffer[0] = '\0';
  }

  ArConfigArg *param = NULL;

  ArConfigSection *section = findSection(mySection.c_str());

  if (section == NULL) {
    ArLog::log(ArLog::Normal,
                "ArConfig::parseListBegin() cannot find section %s for list %s",
                mySection.c_str(),
                arg->getFullString());
    return true;
  }

  ArConfigArg *parentParam = section->findParam(myParsingListNames, true);

  myParsingListNames.push_back(arg->getFullString());
  param = section->findParam(myParsingListNames, true);

  if (param == NULL) {

    ArLog::log(ArLog::Normal,
               "ArConfig::parseListBegin() cannot find param for list %s, creating LIST_HOLDER",
               arg->getFullString());

    param = new ArConfigArg(ArConfigArg::LIST_HOLDER,
                            arg->getFullString(),
                            ""); // empty description 
   
    if (parentParam == NULL) {
      addParam(*param, mySection.c_str());
    }
    else {
      parentParam->addArg(*param);
    }

    delete param;
    param = NULL;
 
    // myIsParsingListBroken = true;
    return true;

  } // end if param not found

  return true;

} // end method parseListBegin


AREXPORT bool ArConfig::parseListEnd(ArArgumentBuilder *arg,
				                             char *errorBuffer,
				                             size_t errorBufferLen)
{
  if ((arg == NULL) || (arg->getArgc() < 1)) {
    ArLog::log(ArLog::Normal,
               "ArConfig::parseListBegin() invalid arg input");
    return false;
  }
  if (mySectionIgnored) {
    return true;
  }
  if (mySectionBroken) {
    return false;
  }

  if ((myParsingListNames.empty()) ||
      (ArUtil::strcasequotecmp(myParsingListNames.back(),
                               arg->getFullString()) != 0)) {

    // If previous list was broken, this really isn't an error
    ArLog::log(ArLog::Normal,
                "ArConfig::parseListEnd() found end of different list %s",
                arg->getFullString());
    if (myIsParsingListBroken) {
      return true;
    }
    else {
      return false;
    }
  }

  // Found the end of the current list
  if (!myIsParsingListBroken) {
  
    ArConfigArg *param = NULL;
    ArConfigSection *section = findSection(mySection.c_str());

    if (section != NULL) {
      param = section->findParam(myParsingListNames, true);   
    }
    if (param != NULL) {
      param->setValueSet();
    }
  } // end if list is fine

  myIsParsingListBroken = false;
  // KMC 11/7/12 Note test for empty() above.
  myParsingListNames.pop_back();

  return true;

} // end method parseListEnd



/**
   The extra string of the parser should be set to the command wanted,
   while the rest of the arg should be the arguments to the command.
   Its case insensitive.

   @param arg Obtain parameter name from this object's "extra string"
   and value(s) from its argument(s).
   
   @param errorBuffer If this is NULL it is ignored, otherwise the
   string for the error is put into the buffer, the first word should
   be the parameter that has trouble
   
   @param errorBufferLen the length of @a errorBuffer
 **/
AREXPORT bool ArConfig::parseArgument(ArArgumentBuilder *arg, 
                                      char *errorBuffer,
                                      size_t errorBufferLen)
{

  
  bool ret = true;
 

  if (mySectionBroken)
  {
    ArLog::log(ArLog::Verbose, "%sSkipping parameter %s because section broken",
               myLogPrefix.c_str(),
	             arg->getExtraString());
    if (myFailOnBadSection)
    {
      snprintf(errorBuffer, errorBufferLen, 
	       "Failed because broken config section");
      return false;
    }
    else
    {
      return true;
    }
  }
  else if (mySectionIgnored) {
    return true;
  }


  if (myIsParsingListBroken) {

    ArLog::log(ArLog::Normal, "%sSkipping parameter %s because list broken",
               myLogPrefix.c_str(),
	             arg->getExtraString());

    return true; // ?? 

  } // end if list broken


  // if we have duplicate params and don't have sections don't trash anything
  if (myDuplicateParams && myUsingSections && mySection.size() <= 0)
  {
    snprintf(errorBuffer, errorBufferLen, 
	     "%s not in section, client needs an upgrade",
	     arg->getExtraString());

    ArLog::log(ArLog::Normal, 
               "%s%s not in a section, client needs an upgrade",
               myLogPrefix.c_str(),
	             arg->getExtraString());
    return false;
  }

  // see if we found this parameter
  bool found = false;

  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';

  //for (std::list<ArConfigSection *>::iterator sectionIt = mySections.begin(); 
  //     sectionIt != mySections.end(); 
  //     sectionIt++)
  //{
  //  section = (*sectionIt);

  ArConfigSection *section = findSection(mySection.c_str());

  if (section != NULL) {

    std::list<ArConfigArg*> parseParamList;

    // MPL took out the part where if the param wasn't in a section at
    // all it checked all the sections, I took this out since
    // everything is generally in sections these days

    // KMC Note that duplicate parameter names can and do exist within 
    // a section.  Therefore it is necessary to iterate through all of 
    // the section contents and check each parameter against the extra
    // string value.
    std::list<ArConfigArg> *paramList = section->getParams();
    if (paramList != NULL) {

      for (std::list<ArConfigArg>::iterator pIter = paramList->begin();
           pIter != paramList->end();
           pIter++) {
    
        ArConfigArg *param = &(*pIter);
        ArConfigArg *parseParam = NULL;
    
        if (myParsingListNames.empty()) {
          if (ArUtil::strcasecmp(param->getName(),arg->getExtraString()) != 0) {
            continue;
          }
          parseParamList.push_back(param);

        }
        else { // parameter is in a list

          if (ArUtil::strcasecmp(param->getName(), myParsingListNames.front()) != 0) {
            continue;
          }
          std::list<std::string>::iterator listIter = myParsingListNames.begin();
          listIter++; // skip the one already parsed

          std::list<ArConfigArg*> matchParamList;
          matchParamList.push_back(param);

          std::list<ArConfigArg*> tempParamList;

          while (listIter != myParsingListNames.end()) {

            for (std::list<ArConfigArg*>::iterator mIter = matchParamList.begin();
                 mIter != matchParamList.end();
                 mIter++) {
              ArConfigArg *matchParam = *mIter;
              if (matchParam == NULL) {
                continue;
              }
              for (int i = 0; i < matchParam->getArgCount(); i++) {
            
                ArConfigArg *childArg = matchParam->getArg(i);
                if (childArg == NULL) {
                  continue;
                }
                if (ArUtil::strcasecmp(childArg->getName(), *listIter) != 0) {
                  continue;
                }
                tempParamList.push_back(childArg);
              }
            } // end for each matching parameter list

            matchParamList.clear();
            matchParamList = tempParamList;
            tempParamList.clear();

            listIter++;

          } // end for each list level
          
          for (std::list<ArConfigArg*>::iterator matchIter = matchParamList.begin();
               matchIter != matchParamList.end();
               matchIter++) {

             ArConfigArg *matchParam = *matchIter;
             if (matchParam == NULL) {
               continue;
             }

             for (int i = 0; i < matchParam->getArgCount(); i++) {
            
                ArConfigArg *childArg = matchParam->getArg(i);
                if (childArg == NULL) {
                  continue;
                }
                if (ArUtil::strcasecmp(childArg->getName(), arg->getExtraString()) != 0) {
                  continue;
                }
                parseParamList.push_back(childArg);
              } // end for each child
          } // end for each matching parameter 
        } // end else parameter is in a list


        for (std::list<ArConfigArg*>::iterator parseIter = parseParamList.begin();
             parseIter != parseParamList.end();
             parseIter++) {

          ArConfigArg *parseParam = *parseIter;
          if (parseParam == NULL) {
            continue;
          }

          found = true;

          // Make sure that the parameter is within the specified priority range,
          // primarily to avoid accidentally overwriting factory and calibration
          // parameters.  Return true because it is not an error condition.
          //
          if ((parseParam->getConfigPriority() < myHighestPriorityToParse) ||
              (parseParam->getConfigPriority() > myLowestPriorityToParse)) {
            return true;
          }

          // KMC 7/11/12 Changed this from an equality check to accomodate the new 
          // calibration priority
          if ((myPermissionAllowFactory) ||
              (parseParam->getConfigPriority() < ArPriority::FACTORY)) {
      
	    bool changed = false;
            if (!parseParam->parseArgument(arg, errorBuffer, errorBufferLen, myLogPrefix.c_str(), myIsQuiet, &changed)) {

              ArLog::log(ArLog::Normal,
                         "ArConfig::parseArgument() error parsing %s",
                         arg->getFullString());
              ret = false;
            }
	    if (changed)
	    {
	      /*
	      ArLog::log(ArLog::Normal, "%sParameter '%s' changed with restart level (%d)", 
			 myLogPrefix.c_str(), 
	      	 parseParam->getName(), parseParam->getRestartLevel());
	      */
	      if (parseParam->getRestartLevel() > myRestartLevelNeeded)
	      {
		myRestartLevelNeeded = parseParam->getRestartLevel();
		// don't print out the warning if nothings checking for it
		if (myCheckingForRestartLevel)
		  ArLog::log(ArLog::Normal, 
			     "%sParameter '%s' in section '%s' changed, bumping restart level needed to %s (%d)", 
			     myLogPrefix.c_str(), parseParam->getName(),
			     section->getName(),
			     ArConfigArg::toString(myRestartLevelNeeded),
			     myRestartLevelNeeded);
	      }
	    }
	      
	      
          }
          else { // factory parameter and no permission to change
          
            /// MPL we'll want to do something here, but kathleen and i still have to work out what...
            /*
            if (errorBuffer != NULL)
            snprintf(errorBuffer, errorBufferLen, 
            "%s is a factory level parameter but those are not allowed to be changed", 
            param->getName());
            */      

          } // end else factory parameter and no permission to change
        } // end if parameter found

      } // end for each parameter in section

    } // end if parameter list
  } // end if section found

  // if we didn't find this param its because its a parameter in another section, so pass this off to the parser for unknown things
  if (!found)
  {
    ArArgumentBuilder unknown;
    unknown.setQuiet(myIsQuiet);
    unknown.addPlain(arg->getExtraString());
    unknown.addPlain(arg->getFullString());
    return parseUnknown(&unknown, errorBuffer, errorBufferLen);
  }
  return ret;
}

/**
   The extra string of the parser should be set to the command wanted,
   while the rest of the arg should be the arguments to the command.
   Its case insensitive.

   @param arg Obtain parameter name from this argument builder's "exra" string and value(s) from its argument(s).
   
   @param errorBuffer if this is NULL it is ignored, otherwise the
   string for the error is put into the buffer, the first word should
   be the parameter that has trouble
   
   @param errorBufferLen the length of the error buffer
 **/
AREXPORT bool ArConfig::parseUnknown(ArArgumentBuilder *arg, 
				     char *errorBuffer,
				     size_t errorBufferLen)
{
  if (arg->getArgc() < 1)
  {
    if (!myIsQuiet) {
      ArLog::log(ArLog::Verbose, "%sEmpty arg in section '%s', ignoring it", 
                 myLogPrefix.c_str(), mySection.c_str());
    }
    return true;
  }

  if (mySaveUnknown && mySectionsToParse == NULL && 
      myPermissionSaveUnknown)
  {
    if (arg->getArgc() < 2)
    {

      if (!myIsQuiet) {
        ArLog::log(ArLog::Verbose, "%sNo arg for param '%s' in section '%s', saving it anyways", 
                  myLogPrefix.c_str(), arg->getArg(0), mySection.c_str());
      }
      
      if (myParsingListNames.empty() || myIsParsingListBroken) {
  
        addParam(ArConfigArg(arg->getArg(0), ""),
	               mySection.c_str());
      }
      else { 
        ArConfigSection *section = findSection(mySection.c_str());
        if (section == NULL) {
          ArLog::log(ArLog::Normal,
                     "%sArConfigArg::parseUnknown() cannot find section %s",
                     myLogPrefix.c_str(),
                     mySection.c_str());
          return false;
        }  
        ArConfigArg *parentParam = section->findParam(myParsingListNames, true);
        if (parentParam == NULL) {
          ArLog::log(ArLog::Normal,
                     "%sArConfigArg::parseUnknown() cannot find parent param %s for %s",
                     myLogPrefix.c_str(),
                     myParsingListNames.front().c_str(),
                     arg->getArg(0));
          return false;
        }
        parentParam->addArg(ArConfigArg(arg->getArg(0), ""));
  


      } // end else list member
    }
    else
    {
      std::string str;
      // int i;
      for (unsigned int i = 1; i < arg->getArgc(); i++)
      {
        if (i != 1)
	        str += ' ';
	      str += arg->getArg(i);
      }

      if (!myIsQuiet) {
        ArLog::log(ArLog::Verbose, "%sUnknown '%s %s' in section '%s', saving it", 
		              myLogPrefix.c_str(), arg->getArg(0), str.c_str(), 
		              mySection.c_str());
      }
      if (myParsingListNames.empty() || myIsParsingListBroken) {
  
        addParam(ArConfigArg(arg->getArg(0), str.c_str()),
	               mySection.c_str());
      }
      else {

        ArConfigSection *section = findSection(mySection.c_str());
        if (section == NULL) {
          ArLog::log(ArLog::Normal,
                     "%sArConfigArg::parseUnknown() cannot find section %s",
                     myLogPrefix.c_str(),
                     mySection.c_str());
          return false;
        }  
        ArConfigArg *parentParam = section->findParam(myParsingListNames, true);
        if (parentParam == NULL) {
          ArLog::log(ArLog::Normal,
                     "%sArConfigArg::parseUnknown() cannot find parent param '%s' for %s",
                     myLogPrefix.c_str(),
                     myParsingListNames.front().c_str(),
                     arg->getArg(0));
          return false;
        }
       
        parentParam->addArg(ArConfigArg(arg->getArg(0), str.c_str()));

      } // end else list member
    }
  }
  else
  {
    IFDEBUG(ArLog::log(ArLog::Normal,
                       "ArConfig::parseUnknown() mySaveUnknown = %i, myPermissionSaveUnknown = %i mySectionsToParse %s NULL",
                       mySaveUnknown,
                       myPermissionSaveUnknown,
                       ((mySectionsToParse == NULL) ? "==" : "!=")));

    ArLog::log(ArLog::Verbose, "%sUnknown '%s' in section '%s', ignoring it", 
	             myLogPrefix.c_str(), arg->getFullString(), mySection.c_str());
  }
  return true;

} // end method parseUnknown

/**
   @param fileName the file to load

   @param continueOnErrors whether to continue parsing if we get
   errors (or just bail)

   @param noFileNotFoundMessage if the file isn't found and this param
   is true it won't complain, otherwise it will

   @param errorBuffer If an error occurs and this is not NULL, copy a description of the error into this buffer
   
   @param errorBufferLen the length of @a errorBuffer

   @param sectionsToParse if NULL, then parse all sections; otherwise,
   a list of the section names that should be parsed (sections not in 
   the list are ignored)

   @param highestPriority Any configuration settings with a priority before 
this are ignored

   @param lowestPriority Any configuration settings with a priority after
this are ignored.

   @param restartLevelNeeded for internal use, leave as NULL
 **/
AREXPORT bool ArConfig::parseFile(const char *fileName, 
                                  bool continueOnErrors,
                                  bool noFileNotFoundMessage, 
                                  char *errorBuffer,
                                  size_t errorBufferLen,
                                  std::list<std::string> *sectionsToParse,
                                  ArPriority::Priority highestPriority,
                                  ArPriority::Priority lowestPriority,
                                  ArConfigArg::RestartLevel *restartLevelNeeded)
{
  bool ret = true;

  if(fileName)
    myFileName = fileName;
  else
    myFileName = "";

  ArLog::log(myProcessFileCallbacksLogLevel,
	     "Starting parsing file %s (%s)", myFileName.c_str(), fileName);


  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';

  // Should be null, but just in case...
  if (mySectionsToParse != NULL)
  {
    delete mySectionsToParse; 
    mySectionsToParse = NULL;
  }

  copySectionsToParse(sectionsToParse);
  myHighestPriorityToParse = highestPriority;
  myLowestPriorityToParse  = lowestPriority;
  myRestartLevelNeeded = ArConfigArg::NO_RESTART;
  if (restartLevelNeeded != NULL)
    myCheckingForRestartLevel = true;
  else
    myCheckingForRestartLevel = false;
  

  // parse the file (errors will go into myErrorBuffer from the functors)
  ret = myParser.parseFile(fileName, continueOnErrors, noFileNotFoundMessage);

  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
  //printf("file %s\n", ArUtil::convertBool(ret));

  // if we have a parser and haven't failed (or we continue on errors)
  // then parse the arguments from the parser
  if (myArgumentParser != NULL && (ret || continueOnErrors))
    ret = parseArgumentParser(myArgumentParser, continueOnErrors, 
			      errorBuffer, errorBufferLen) && ret;

  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
  //printf("parser %s\n", ArUtil::convertBool(ret));
  
  // if we haven't failed (or we continue on errors) then call the
  // process file callbacks
  if (ret || continueOnErrors)
    ret = callProcessFileCallBacks(continueOnErrors, errorBuffer,
					  errorBufferLen) && ret;
  
  // copy our error if we have one and haven't copied in yet
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }

  if (restartLevelNeeded != NULL)
    *restartLevelNeeded = myRestartLevelNeeded;

  // Done with the temp parsing info, so delete it...
  delete mySectionsToParse; 
  mySectionsToParse = NULL;

  myHighestPriorityToParse = ArPriority::FIRST_PRIORITY;
  myLowestPriorityToParse  = ArPriority::LAST_PRIORITY;
  myRestartLevelNeeded = ArConfigArg::NO_RESTART;
  myCheckingForRestartLevel = true;

  ArLog::log(myProcessFileCallbacksLogLevel,
	     "Done parsing file %s (ret %s)", fileName,
	     ArUtil::convertBool(ret));
  return ret;
}

/**
   @param fileName the name of the file to write out

   @param append if true then text will be appended to the file if it exists, otherwise any existing file will be overwritten.

   @param alreadyWritten if non-NULL, a list of strings that have already been
   written out, don't write it again if it's in this list; when 
   something is written by this function, then it is put it into this list 

   @param writeExtras if this is true then the priority, display hint, restart level, and other later extras of each parameter will 
   also be written to the file, if it is false they will not be

   @param sectionsToWrite if NULL, then write all sections; otherwise,
   a list of the section names that should be written 
   
   @param highestPriority the ArPriority::Priority that specifies the minimum (numerical)
   priority that a parameter must have in order to be written; not related to the 
   writeExtras flag

   @param lowestPriority the ArPriority::Priority that specifies the maximum (numerical)
   priority that a parameter must have in order to be written; not related to the 
   writeExtras flag
 **/
AREXPORT bool ArConfig::writeFile(const char *fileName, 
                                  bool append, 
                                  std::set<std::string> *alreadyWritten,
                                  bool writeExtras,
                                  std::list<std::string> *sectionsToWrite,
                                  ArPriority::Priority highestPriority,
                                  ArPriority::Priority lowestPriority)
{
  FILE *file;

  std::set<std::string> writtenSet;
  std::set<std::string> *written;
  if (alreadyWritten != NULL)
    written = alreadyWritten;
  else
    written = &writtenSet;
  //std::list<ArConfigArg>::iterator it;

  // later this'll have a prefix
  std::string realFileName;
  if (fileName[0] == '/' || fileName[0] == '\\')
  {
    realFileName = fileName;
  }
  else
  {
    realFileName = myBaseDirectory;
    realFileName += fileName;
  }

  std::string mode;

  if (append)
    mode = "a";
  else
    mode = "w";

  if ((file = ArUtil::fopen(realFileName.c_str(), mode.c_str())) == NULL)
  {
    ArLog::log(ArLog::Terse, "%sCannot open file '%s' for writing",
	       myLogPrefix.c_str(), realFileName.c_str());
    return false;
  }

  // Start the config file with version information
  fprintf(file, "%s %s\n", CONFIG_VERSION_TAG, CURRENT_CONFIG_VERSION);

  bool firstSection = true;
  std::list<ArConfigSection *>::iterator sectionIt;
  ArConfigSection *section = NULL;
  
  // first write out the generic section (ie sectionless stuff, mostly
  // for backwards compatibility)
  if ( ((section = findSection("")) != NULL) &&
       (sectionsToWrite == NULL) )
  {
    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;
    writeSection(section, file, written, writeExtras, highestPriority, lowestPriority);
  }

  // then write out the rest (skip the generic section if we have one)
  for (sectionIt = mySections.begin(); 
       sectionIt != mySections.end(); 
       sectionIt++)
  {
    section = (*sectionIt);
    if (strcmp(section->getName(), "") == 0)
      continue;

    if (sectionsToWrite != NULL) {
      bool isSectionFound = false;
      for (std::list<std::string>::iterator swIter = sectionsToWrite->begin();
           swIter != sectionsToWrite->end();
           swIter++) {
        std::string sp = *swIter;
        if (ArUtil::strcasecmp(section->getName(), sp.c_str()) == 0) {
          isSectionFound = true;
          break;
        }
      } // end for each section to write

      if (!isSectionFound) {
        continue;
      }
    
    } // end if sections specified

    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;

    writeSection(section, file, written, writeExtras, highestPriority, lowestPriority);
  }
  fclose(file);
  return true;
}
  

AREXPORT bool ArConfig::parseText(const std::list<std::string> &configLines,
                                  bool continueOnErrors,
                                  bool *parseOk,
                                  bool *processOk,
                                  char *errorBuffer,
                                  size_t errorBufferLen,
                                  std::list<std::string> *sectionsToParse,
                                  ArPriority::Priority highestPriority,
                                  ArPriority::Priority lowestPriority,
                                  ArConfigArg::RestartLevel *restartLevelNeeded)
{
  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    errorBuffer[0] = '\0';
  }
 
  if (parseOk != NULL) {
    *parseOk = true;
  }
  if (processOk != NULL) {
    *processOk = true;
  }


  bool ret = true;
  
  // Should be null, but just in case...
  if (mySectionsToParse != NULL)
  {
    delete mySectionsToParse; 
    mySectionsToParse = NULL;
  }
  

  copySectionsToParse(sectionsToParse);
  myHighestPriorityToParse = highestPriority;
  myLowestPriorityToParse  = lowestPriority;
  myRestartLevelNeeded = ArConfigArg::NO_RESTART;
  

  if (restartLevelNeeded != NULL)
    myCheckingForRestartLevel = true;
  else
    myCheckingForRestartLevel = false;

  // KMC 8/9/13 I think that the original ARCL command handler acted 
  // differently. Maybe stopped on first parse error, but allowed 
  // the callbacks to continue on errors.  TODO Why??!

  char lineBuf[10000];
  size_t lineBufLen = sizeof(lineBuf);
  
  for (std::list<std::string>::const_iterator iter = configLines.begin();
       iter != configLines.end();
       iter++) {
    const std::string &curLine = *iter;

    if (curLine.empty()) {
      continue;
    }
    snprintf(lineBuf, lineBufLen,
             curLine.c_str());

    if (!myParser.parseLine(lineBuf, errorBuffer, errorBufferLen))
    {
      ret = false;
      if (parseOk != NULL) {
        *parseOk = false;
      }
      if (!continueOnErrors)
        break;
    }
  } // end for each text line
  
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
  //printf("parser %s\n", ArUtil::convertBool(ret));
  
  // if we haven't failed (or we continue on errors) then call the
  // process file callbacks
  if (ret || continueOnErrors) {
    if (!callProcessFileCallBacks(continueOnErrors, 
                                  errorBuffer,
					                        errorBufferLen)) {
      ret = false;
      if (processOk != NULL) {
        *processOk = false;
      }
    } 
  }
 
  // copy our error if we have one and haven't copied in yet
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }

  if (restartLevelNeeded != NULL)
    *restartLevelNeeded = myRestartLevelNeeded;

  // Done with the temp parsing info, so delete it...
  delete mySectionsToParse; 
  mySectionsToParse = NULL;

  myHighestPriorityToParse = ArPriority::FIRST_PRIORITY;
  myLowestPriorityToParse  = ArPriority::LAST_PRIORITY;
  myRestartLevelNeeded = ArConfigArg::NO_RESTART;
  myCheckingForRestartLevel = true;

  ArLog::log(myProcessFileCallbacksLogLevel,
      	     "Done parsing text list (ret %s)", 
	           ArUtil::convertBool(ret));

  return ret;

} // end method parseText 

// -----------------------------------------------------------------------------


/// Parse a config resource file, for translation.
AREXPORT bool ArConfig::parseResourceFile(const char *fileName, 
                                  bool continueOnError,
                                  char *errorBuffer,
                                  size_t errorBufferLen,
                                  std::list<std::string> *sectionsToParse)
{
  bool ret = true;

  if(fileName)
    myFileName = fileName;
  else
    myFileName = "";


  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';

 
  // stat(fileName, &myReadFileStat);
  FILE *file = NULL;

  char line[10000];

  ArLog::log(ArLog::Normal, 
             "%sArConfig::parseResourceFile() Opening file %s", 
             myLogPrefix.c_str(),
             fileName);


  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.
  if ((file = ArUtil::fopen(fileName, "rb")) == NULL)
  {
    ArLog::log(ArLog::Terse, "Cannot open file '%s'", fileName);
    // TODO This used to put the config param name into the error buffer
    if (errorBuffer != NULL) {
      snprintf(errorBuffer, errorBufferLen, 
               "Resource file invalid: cannot open file '%s'",
               fileName);
    }
    return false;
  }


  bool isSuccess = true;

  char *localErrorBuffer = NULL;
  size_t localErrorBufferLen = 0;

  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    localErrorBufferLen = errorBufferLen;
    localErrorBuffer = new char[localErrorBufferLen];
    localErrorBuffer[0] = '\0';
  }

  // KMC TODO Parse and use the version info someday.
  // Parse resource version. 
  fgets(line, sizeof(line), file);

  // KMC TODO This needs to be improved... made more generic... possibly test
  // that this is really header information.
  //
  // Skipping two lines for the header.  The third one is blank.
  for (int h = 0; h < 2; h++) {
    if (fgets(line, sizeof(line), file) == NULL) {
      ArLog::log(ArLog::Terse, "%sArConfig::parseResourceFile() Header missing in '%s'", 
                 myLogPrefix.c_str(),
                 fileName);
      return false;
    }
  }

  bool ignoreNormalSpaces = true;
   
  while ((fgets(line, sizeof(line), file) != NULL)) 
    {
      if (ArUtil::isStrEmpty(line)) {
        continue;
      }
     
      ArArgumentBuilder builder(512, myCsvSeparatorChar, ignoreNormalSpaces); // , true, true);
 
      builder.addPlain(line);

      if ((builder.getArgc() == 0) ||
          (ArUtil::isStrEmpty(builder.getArg(0)))) {
        continue; // More whitespace possibilities
      }
      if (builder.getArgc() < ArConfigArg::RESOURCE_INDEX_OF_DESCRIPTION) {
        // KMC Commented this out because Ctrl-M frequently shows in log
        IFDEBUG(ArLog::log(ArLog::Normal,
                  "ArConfig::parseResourceFile() no section for %s",
                  builder.getFullString()));
                   
        continue;
      }

      std::string sectionName = ArConfigArg::parseResourceSectionName(&builder,
                                                                      myLogPrefix.c_str());
      
      ArConfigSection *section = findSection(sectionName.c_str());
      if (section == NULL) {
        IFDEBUG(ArLog::log(ArLog::Normal,
                           "ArConfig::parseResourceFile() adding section %s",
                           sectionName.c_str()));        
        if (!addSection(CATEGORY_DEBUG, sectionName.c_str(), "")) {
          ArLog::log(ArLog::Normal,
                     "ArConfig::parseResourceFile() error adding section %s",
                     sectionName.c_str());    
          continue;
        }

        section = findSection(sectionName.c_str());
        if (section == NULL) {
          ArLog::log(ArLog::Normal,
                     "ArConfig::parseResourceFile() error finding new section %s",
                     sectionName.c_str());    
          continue;
        }
      }

      std::string argName = ArConfigArg::parseResourceArgName(&builder,
                                                              myLogPrefix.c_str());

     // An empty argument name means that the section comment should be set.
     if (argName.empty()) {
      
       std::string desc = ArConfigArg::parseResourceDescription(&builder,
                                                                myLogPrefix.c_str());
   
       if (!desc.empty()) {

         IFDEBUG(ArLog::log(ArLog::Normal,
                    "ArConfig::parseResourceFile() setting comment for section %s",
                    section->getName()));

         section->setComment(desc.c_str());
       }
       continue;

     } // end if no parameter name


     ArConfigArg::Type argType = ArConfigArg::parseResourceType(&builder,
                                                                 myLogPrefix.c_str());

      bool isTopLevel = ArConfigArg::isResourceTopLevel(&builder, 
                                                        myLogPrefix.c_str());
 
      ArConfigArg *curArg = NULL;
      ArConfigArg *parentArg = NULL;

      if (isTopLevel) {
        curArg = section->findParam(argName.c_str());
      }
      else {
        std::list<std::string> parentPath = ArConfigArg::parseResourceParentPath(&builder, 
                                                                                 '|', // TODO Constant
                                                                                 myLogPrefix.c_str());
        parentArg = section->findParam(parentPath, true); // allow holders
        if (parentArg != NULL) {
          curArg = parentArg->findArg(argName.c_str());
        }
      }

      if (curArg == NULL) {

        IFDEBUG(ArLog::log(ArLog::Normal,
                           "ArConfig::parseResourceFile() creating arg for %s",
                           argName.c_str()));

        // This is a hack that should be fixed someday. If the arg type is a
        // list, then create a list arg -- to which children can be added.
        // If the arg is a normal arg, then create a string holder.
        if ((argType != ArConfigArg::LIST) && (argType != ArConfigArg::LIST_HOLDER)) {
          curArg = new ArConfigArg(ArConfigArg::STRING_HOLDER, argName.c_str(), "");
        }
        else {
          curArg = new ArConfigArg(ArConfigArg::LIST_HOLDER, argName.c_str(), "");
        }
        
        if (curArg->parseResource(&builder, NULL, 0, myLogPrefix.c_str())) {
        
          if (isTopLevel) {
            addParamAsIs(*curArg, sectionName.c_str());
          }
          else if (parentArg != NULL) {
            parentArg->addArg(*curArg);
          }
        }
      }
      else {
        IFDEBUG(ArLog::log(ArLog::Normal,
                           "ArConfig::parseResourceFile() found arg for %s",
                           argName.c_str()));
        curArg->parseResource(&builder, NULL, 0, myLogPrefix.c_str());
      }
    
 
    } // end while more lines to read


  fclose(file);


  return ret;

} // end method parseResourceFile
  
/// Parse a config resource file with parameters suitable for custom commands.
AREXPORT void ArConfig::parseResourceFile(ArArgumentBuilder *builder)
{
  if ((builder == NULL) || 
      (builder->getArgc() <= 0) ||
      (builder->getArg(0) == NULL)) {
    ArLog::log(ArLog::Normal,
               "ArConfig::parseResourceFile() invalid input");
    return;
  }
  parseResourceFile(builder->getArg(0));
} // end method parseResourceFile


      
/// Write a config resource file, for translation.
AREXPORT bool ArConfig::writeResourceFile(const char *fileName, 
                                 bool append,
                                 std::set<std::string> *alreadyWritten,
                                 std::list<std::string> *sectionsToWrite)
{
  FILE *file = NULL;
  char headerBuf[1024];

  std::set<std::string> writtenSet;
  std::set<std::string> *written = NULL;
  if (alreadyWritten != NULL)
    written = alreadyWritten;
  else
    written = &writtenSet;

  // later this'll have a prefix
  std::string realFileName;
  if (fileName[0] == '/' || fileName[0] == '\\')
  {
    realFileName = fileName;
  }
  else
  {
    realFileName = myBaseDirectory;
    realFileName += fileName;
  }

  std::string mode = ((!append) ? "wb" : "a");

  if ((file = ArUtil::fopen(realFileName.c_str(), mode.c_str())) == NULL)
  {
    ArLog::log(ArLog::Terse, "%sCannot open file '%s' for writing",
	       myLogPrefix.c_str(), realFileName.c_str());
    return false;
  }

  // Start the resource file with version information
  fprintf(file, "%s %s\n", RESOURCE_VERSION_TAG, CURRENT_RESOURCE_VERSION);

  int headerLineCount = ArConfigArg::writeResourceHeader(file, 
                                                    headerBuf, 
                                                    sizeof(headerBuf), 
                                                    myCsvSeparatorChar, 
                                                    "SECTION",
                                                    myLogPrefix.c_str());

  ArLog::log(ArLog::Normal,
             "%sArConfig::writeResourceFile() file %s, %s %s, header contains %i lines",
              myLogPrefix.c_str(), 
              realFileName.c_str(),
              RESOURCE_VERSION_TAG,
              CURRENT_RESOURCE_VERSION,
              headerLineCount);
  
  bool firstSection = true;
  ArConfigSection *section = NULL;
  
  // first write out the generic section (ie sectionless stuff, mostly
  // for backwards compatibility)
  if ( ((section = findSection("")) != NULL) &&
       (sectionsToWrite == NULL) )
  {
    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;
    writeSectionResource(section, file, written);
  }

  // then write out the rest (skip the generic section if we have one)
  for (std::list<ArConfigSection *>::iterator sectionIt = mySections.begin(); 
       sectionIt != mySections.end(); 
       sectionIt++)
  {
    section = (*sectionIt);
    if (strcmp(section->getName(), "") == 0)
      continue;

    if (sectionsToWrite != NULL) {
      bool isSectionFound = false;
      for (std::list<std::string>::iterator swIter = sectionsToWrite->begin();
           swIter != sectionsToWrite->end();
           swIter++) {
        std::string sp = *swIter;
        if (ArUtil::strcasecmp(section->getName(), sp.c_str()) == 0) {
          isSectionFound = true;
          break;
        }
      } // end for each section to write

      if (!isSectionFound) {
        continue;
      }
    
    } // end if sections specified

    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;

    writeSectionResource(section, file, written);
  }
  fclose(file);
  return true;

} // end method writeResourceFile

/// Write a config resource file with parameters suitable for custom commands.
AREXPORT void ArConfig::writeResourceFile(ArArgumentBuilder *builder)
{
  if ((builder == NULL) || 
      (builder->getArgc() <= 0) ||
      (builder->getArg(0) == NULL)) {
    ArLog::log(ArLog::Normal,
               "ArConfig::writeResourceFile() invalid input");
    return;
  }
  writeResourceFile(builder->getArg(0));

} // end method writeResourceFile

// -----------------------------------------------------------------------------


AREXPORT void ArConfig::writeSection(ArConfigSection *section, 
                                     FILE *file,
                                     std::set<std::string> *alreadyWritten,
                                     bool writeExtras,
                                     ArPriority::Priority highestPriority,
                                     ArPriority::Priority lowestPriority)
{
  // holds each line
  char line[1024];
  //// holds the fprintf
  //char startLine[128];
  bool commented = false;
  unsigned int startCommentColumn = 25;

 
  /// clear out our written ones between sections
  alreadyWritten->clear();
  
  if (!ArUtil::isStrEmpty(section->getName())) {
    //fprintf(file, "; %s\n", section->getName());
    fprintf(file, "Section %s\n", section->getName());
  }

  sprintf(line, "; ");

  if (!ArUtil::isStrEmpty(section->getComment())) 
  {
     ArConfigArg::writeMultiLineComment(section->getComment(),
                                        file,
                                        line,
                                        sizeof(line),
                                        "; ");
  }
  
  fprintf(file, ";SectionFlags for %s: %s\n", 
	        section->getName(), section->getFlags());
  
  std::list<ArConfigArg> *params = section->getParams();
  
  if (params == NULL) {
    return;
  }

  for (std::list<ArConfigArg>::iterator paramIt = params->begin(); 
       paramIt != params->end(); 
       paramIt++)
  {
    commented = false;
    
    ArConfigArg *param = &(*paramIt);
    
    if (param->getType() == ArConfigArg::SEPARATOR) {
      continue;
    }
    if (alreadyWritten != NULL && 
	      param->getType() != ArConfigArg::DESCRIPTION_HOLDER &&
	      alreadyWritten->find(param->getName()) != alreadyWritten->end()) {
      continue;
      
    }
    else if (alreadyWritten != NULL && 
	           param->getType() != ArConfigArg::DESCRIPTION_HOLDER)
    {
      alreadyWritten->insert(param->getName());
    }

    // If the parameter's priority does not fall in the specified bounds,
    // then do not write it.
    if ((param->getConfigPriority() < highestPriority) ||
        (param->getConfigPriority() > lowestPriority)) {
      continue;
    }

    // Write the parameter to the file.
    param->writeArguments(file,
                          line,
                          sizeof(line),
                          startCommentColumn,
                          writeExtras,
                          myLogPrefix.c_str());
                           

    // now put a blank line between params if we should
    if (!myNoBlanksBetweenParams)
      fprintf(file, "\n");
  }
}




AREXPORT void ArConfig::writeSectionResource(ArConfigSection *section, 
                                        FILE *file,
                                        std::set<std::string> *alreadyWritten)
{
  if ((section == NULL) || (file == NULL)) {
    ArLog::log(ArLog::Normal,
               "ArConfig::writeSectionResource() error, invalid input");
    return;
  }

  // holds each line
  char line[10000]; // KMC TODO Reasonable max?
  const size_t lineSize = sizeof(line);
 
  /// clear out our written ones between sections
  if (alreadyWritten != NULL) {
    alreadyWritten->clear();
  }

  ArConfigArg::writeResourceSectionHeader(file,
                                          line,
                                          lineSize,
                                          myCsvSeparatorChar,
                                          section->getName(),
                                          section->getComment(),
                                          "",   
                                          "",   // TODO Display name
                                          true, // TODO,
                                          myLogPrefix.c_str());
 
  std::list<ArConfigArg> *params = section->getParams();
  
  if (params == NULL) {
    return;
  }

  for (std::list<ArConfigArg>::iterator paramIt = params->begin(); 
       paramIt != params->end(); 
       paramIt++)
  {
    
    ArConfigArg *param = &(*paramIt);
    
    if (param->getType() == ArConfigArg::SEPARATOR) {
      continue;
    }
    if (ArUtil::isStrEmpty(param->getName())) {
      continue;
    }
    if (alreadyWritten != NULL && 
	      param->getType() != ArConfigArg::DESCRIPTION_HOLDER &&
	      alreadyWritten->find(param->getName()) != alreadyWritten->end())
      continue;

    else if (alreadyWritten != NULL && 
	           param->getType() != ArConfigArg::DESCRIPTION_HOLDER)
    {
      alreadyWritten->insert(param->getName());
    }
   
    // Write the parameter to the file.
    param->writeResource(file,
                    line,
                    lineSize,
                    myCsvSeparatorChar,
                    section->getName(),
                    myLogPrefix.c_str());
    
  }
} // end method writeSectionResource

AREXPORT void ArConfig::translateSection(ArConfigSection *section)
{
  if ((section == NULL) || (ArUtil::isStrEmpty(section->getName()))) {
    return;
  }

  IFDEBUG(ArLog::log(ArLog::Normal,
             "%sArConfig::translateSection() %stranslator for %s",
             myLogPrefix.c_str(),
             ((myTranslator != NULL) ? "" : "NULL "),
             section->getName()));
  
  if (myTranslator != NULL) 
  {
    ArConfigSection *xltrSection = myTranslator->findSection(section->getName());

    if (xltrSection != NULL) {
      if (!ArUtil::isStrEmpty(xltrSection->getComment())) {
        IFDEBUG(ArLog::log(ArLog::Normal,
                 "ArConfig::addSection() translator section for %s",
                 section->getName()));
        section->setComment(xltrSection->getComment());
      }
      else { 
        IFDEBUG(ArLog::log(ArLog::Normal,
                 "ArConfig::addSection() no comment in translator section for %s",
                 section->getName()));
      }
    }
    else {
      ArLog::log(ArLog::Normal,
                 "ArConfig::addSection() did not find translator section for %s",
                 section->getName());
    }
  }

} // end method translateSection


AREXPORT const char *ArConfig::getBaseDirectory(void) const
{
  return myBaseDirectory.c_str();
}

AREXPORT void ArConfig::setBaseDirectory(const char *baseDirectory)
{
  if (baseDirectory != NULL && strlen(baseDirectory) > 0)
    myBaseDirectory = baseDirectory;
  else
    myBaseDirectory = "";
   
  myParser.setBaseDirectory(baseDirectory);
}

AREXPORT const char *ArConfig::getFileName(void) const
{
  return myFileName.c_str();
}

/**
   After a file has been read all the way these processFileCBs are
   called in the priority (higher numbers first)... these are only
   called if there were no errors parsing the file or
   continueOnError was set to false when parseFile was called

   The functor should return true if the config parsed was good
   (parseFile will return true) false if the config parsed wasn't
   (parseFile will return false)

   @param functor the functor to call 

   @param priority the functors are called in descending order, if two
   things have the same number the first one added is the first one
   called
**/
AREXPORT void ArConfig::addProcessFileCB(ArRetFunctor<bool> *functor,
					 int priority)
{
  myProcessFileCBList.insert(
	  std::pair<int, ProcessFileCBType *>(-priority, 
					      new ProcessFileCBType(functor)));
}

/** 
    Removes a processFileCB, see addProcessFileCB for details
 **/
AREXPORT void ArConfig::remProcessFileCB(ArRetFunctor<bool> *functor)
{
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *cb;

  for (it = myProcessFileCBList.begin(); it != myProcessFileCBList.end(); ++it)
  {
    if ((*it).second->haveFunctor(functor))
    {
      cb = (*it).second;
      myProcessFileCBList.erase(it);
      delete cb;
      remProcessFileCB(functor);
      return;
    }
  }
}

/**
   After a file has been read all the way these processFileCBs are
   called in the priority (higher numbers first)... these are only
   called if there were no errors parsing the file or
   continueOnError was set to false when parseFile was called

   The functor should return true if the config parsed was good
   (parseFile will return true) false if the config parsed wasn't
   (parseFile will return false)

   @param functor the functor to call 

   @param priority the functors are called in descending order, if two
   things have the same number the first one added is the first one
   called
**/
AREXPORT void ArConfig::addProcessFileCB(
	ArRetFunctor2<bool, char *, size_t> *functor,
	int priority)
{
  if (functor->getName() != NULL && functor->getName()[0] != '\0')
    ArLog::log(ArLog::Normal, "ArConfig::addProcessFileCB: Adding error handler callback %s that should have used addProcessFileWithErrorCB", functor->getName());
  else
    ArLog::log(ArLog::Normal, "ArConfig::addProcessFileCB: Adding anonymous error handler callback that should have used addProcessFileWithErrorCB");
  myProcessFileCBList.insert(
	  std::pair<int, ProcessFileCBType *>(-priority, 
					      new ProcessFileCBType(functor)));
}
/**
   This function has a different name than addProcessFileCB just so
   that if you mean to get this function but have the wrong functor
   you'll get an error.  The rem's are the same though since that
   shouldn't matter.

   After a file has been read all the way these processFileCBs are
   called in the priority (higher numbers first)... these are only
   called if there were no errors parsing the file or
   continueOnError was set to false when parseFile was called

   The functor should return true if the config parsed was good
   (parseFile will return true) false if the config parsed wasn't
   (parseFile will return false)

   @param functor the functor to call (the char * and unsigned int
   should be used by the functor to put an error message into that
   buffer)

   @param priority the functors are called in descending order, if two
   things have the same number the first one added is the first one
   called
**/
AREXPORT void ArConfig::addProcessFileWithErrorCB(
	ArRetFunctor2<bool, char *, size_t> *functor,
	int priority)
{
  myProcessFileCBList.insert(
	  std::pair<int, ProcessFileCBType *>(-priority, 
				      new ProcessFileCBType(functor)));
}

/** 
    Removes a processFileCB, see addProcessFileCB for details
 **/
AREXPORT void ArConfig::remProcessFileCB(
	ArRetFunctor2<bool, char *, size_t> *functor)
{
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *cb;

  for (it = myProcessFileCBList.begin(); it != myProcessFileCBList.end(); ++it)
  {
    if ((*it).second->haveFunctor(functor))
    {
      cb = (*it).second;
      myProcessFileCBList.erase(it);
      delete cb;
      remProcessFileCB(functor);
    }
  }
}

AREXPORT bool ArConfig::callProcessFileCallBacks(bool continueOnErrors,
						 char *errorBuffer,
						 size_t errorBufferLen)
{
  bool ret = true;
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *callback;
  ArLog::LogLevel level = myProcessFileCallbacksLogLevel;

  // reset our section to nothing again
  mySection = "";

  // in windows, can't simply declare an array of errorBufferLen -- so
  // allocate one.
  
  // empty the buffer, we're only going to put the first error in it
  if (errorBuffer != NULL && errorBufferLen > 0)
    errorBuffer[0] = '\0';

  ArLog::log(level, "%sProcessing file", myLogPrefix.c_str());
  for (it = myProcessFileCBList.begin(); 
       it != myProcessFileCBList.end(); 
       ++it)
  {
    callback = (*it).second;
    if (callback->getName() != NULL && callback->getName()[0] != '\0')
      ArLog::log(level, "%sProcessing functor '%s' (%d)", 
                 myLogPrefix.c_str(),
		             callback->getName(), -(*it).first);
    else
      ArLog::log(level, "%sProcessing unnamed functor (%d)", 
                 myLogPrefix.c_str(),
                 -(*it).first);
    if (!(*it).second->call(errorBuffer, errorBufferLen))
    {
      //printf("# %s\n", scratchBuffer); 

      // if there is an error buffer and it got filled get rid of our
      // pointer to it
      if (errorBuffer != NULL && errorBuffer[0] != '\0')
      {
	errorBuffer = NULL;
	errorBufferLen = 0;
      }
      ret = false;
      if (!continueOnErrors)
      {
	if (callback->getName() != NULL && callback->getName()[0] != '\0')
	  ArLog::log(ArLog::Normal, "ArConfig: Failed, stopping because the '%s' process file callback failed", 
		     callback->getName());
	else
	  ArLog::log(ArLog::Normal, "ArConfig: Failed, stopping because unnamed process file callback failed");
	break;
      }
      else
      {
	if (callback->getName() != NULL && callback->getName()[0] != '\0')
	  ArLog::log(ArLog::Normal, "ArConfig: Failed but continuing, the '%s' process file callback failed", 
		     callback->getName());
	else
	  ArLog::log(ArLog::Normal, "ArConfig: Failed but continuing, an unnamed process file callback failed");
      }

    }
  }
  if (ret || continueOnErrors)
  {
    ArLog::log(level, "%sProcessing with own processFile",
                 myLogPrefix.c_str());
    if (!processFile())
      ret = false;
  }
  ArLog::log(level, "%sDone processing file, ret is %s", myLogPrefix.c_str(),
	     ArUtil::convertBool(ret));

  return ret;
}

  
AREXPORT std::list<std::string> ArConfig::getCategoryNames() const
{
  std::list<std::string> retList;

  for (std::map<std::string, std::list<std::string> >::const_iterator iter =
                                    myCategoryToSectionsMap.begin();
       iter != myCategoryToSectionsMap.end();
       iter++) {
    retList.push_back(iter->first);
  }
  return retList;

} // end method getCategoryNames


/**
 * @param categoryName the char * name of the category for which to find the 
 * section names; if empty/null, then the method returns the names of all
 * sections that have not been included in any category
**/
AREXPORT std::list<std::string> ArConfig::getSectionNamesInCategory
                                              (const char *categoryName) const
{
  std::list<std::string> retList;
  
  if (!ArUtil::isStrEmpty(categoryName)) {
 
    std::map<std::string, std::list<std::string> >::const_iterator iter =
                                      myCategoryToSectionsMap.find(categoryName);
    if (iter != myCategoryToSectionsMap.end()) {
      retList = iter->second;
    }
  }
  else { // find sections that have no category
   
    for (std::list<ArConfigSection *>::const_iterator sIter = mySections.begin(); 
         sIter != mySections.end();
         sIter++) {
                           
      const ArConfigSection *section = *sIter;
      if ((section != NULL) &&
          (!ArUtil::isStrEmpty(section->getName())) &&
          (ArUtil::isStrEmpty(section->getCategoryName()))) {
        retList.push_back(section->getName());
      }
    } // end for each section

  } // end else find sections that have no category

  return retList;
  
} // end method getSectionNamesInCategory
  

AREXPORT std::list<std::string> ArConfig::getSectionNames() const
{
   std::list<std::string> retList;

   for (std::list<ArConfigSection *>::const_iterator sIter = mySections.begin(); 
        sIter != mySections.end();
        sIter++) {
                           
    const ArConfigSection *section = *sIter;
    if ((section != NULL) &&
        (!ArUtil::isStrEmpty(section->getName()))) {
      retList.push_back(section->getName());
    }
  } // end for each section
   
  return retList;

} // end method getSectionNames


AREXPORT std::list<ArConfigSection *> *ArConfig::getSections(void)
{
  return &mySections;
}


AREXPORT void ArConfig::setNoBlanksBetweenParams(bool noBlanksBetweenParams)
{
  myNoBlanksBetweenParams = noBlanksBetweenParams;
}

AREXPORT bool ArConfig::getNoBlanksBetweenParams(void)
{
  return myNoBlanksBetweenParams;
}

/**
   This argument parser the arguments in to check for parameters of
   this name, note that ONLY the first parameter of this name will be
   used, so if you have duplicates only the first one will be set.
 **/
AREXPORT void ArConfig::useArgumentParser(ArArgumentParser *parser)
{
  myArgumentParser = parser;
}

AREXPORT bool ArConfig::parseArgumentParser(ArArgumentParser *parser, 	
					    bool continueOnError,
					    char *errorBuffer,
					    size_t errorBufferLen)
{
  std::list<ArConfigSection *>::iterator sectionIt;
  std::list<ArConfigArg>::iterator paramIt;
  ArConfigSection *section = NULL;
  ArConfigArg *param = NULL;
  std::list<ArConfigArg> *params = NULL;

  bool ret;
  size_t i;
  std::string strArg;
  std::string strUndashArg;
  ArArgumentBuilder builder;
  builder.setQuiet(myIsQuiet);

  bool plainMatch;

  for (i = 0; i < parser->getArgc(); i++)
  {
    if (parser->getArg(i) == NULL)
    {
      ArLog::log(ArLog::Terse, "%sset up wrong (parseArgumentParser broken).",
                 myLogPrefix.c_str());
      if (errorBuffer != NULL)
        strncpy(errorBuffer, 
        "ArConfig set up wrong (parseArgumentParser broken).", 
        errorBufferLen);
      return false;
    }
    strArg = parser->getArg(i); 
    if (parser->getArg(i)[0] == '-')
      strUndashArg += &parser->getArg(i)[1]; 
    else
      strUndashArg = "";
    //printf("normal %s undash %s\n", strArg.c_str(), strUndashArg.c_str());
    for (sectionIt = mySections.begin(); 
         sectionIt != mySections.end(); 
         sectionIt++)
    {
      section = (*sectionIt);
      params = section->getParams();

      for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
      {
        param = &(*paramIt);
        /*
        printf("### normal %s undash %s %d %d\n", strArg.c_str(), 
        strUndashArg.c_str(),
        ArUtil::strcasecmp(param->getName(), strArg),
        ArUtil::strcasecmp(param->getName(), strUndashArg));
        */
        if (strlen(param->getName()) > 0 &&
          ((plainMatch = ArUtil::strcasecmp(param->getName(),strArg)) == 0 ||
          ArUtil::strcasecmp(param->getName(), strUndashArg) == 0))
        {
          if (plainMatch == 0)
            builder.setExtraString(strArg.c_str());
          else
            builder.setExtraString(strUndashArg.c_str());
          if (i+1 < parser->getArgc())
          {
            builder.addPlain(parser->getArg(i+1));
            parser->removeArg(i+1);
          }
          parser->removeArg(i);

          // set us to use the section this is in and then parse the argument
          std::string oldSection = mySection;
          bool oldSectionBroken = mySectionBroken;
          bool oldSectionIgnored = mySectionIgnored;
          bool oldUsingSections = myUsingSections;
          bool oldDuplicateParams = myDuplicateParams;
          mySection = section->getName();
          mySectionBroken = false;
          mySectionIgnored = false; // ?? TODO
          myUsingSections = true;
          myDuplicateParams = false;
          ret = parseArgument(&builder, errorBuffer, errorBufferLen);
          mySection = oldSection;
          mySectionBroken = oldSectionBroken;
          mySectionIgnored = oldSectionIgnored;
          myUsingSections = oldUsingSections;
          myDuplicateParams = oldDuplicateParams;

          // if we parsed the argument right or are continuing on
          // errors call ourselves again (so we don't hose iterators up above)
          if (ret || continueOnError)
          {
            //printf("Ret %s\n", ArUtil::convertBool(ret));
            return ret && parseArgumentParser(parser, continueOnError, 
              errorBuffer, errorBufferLen);
          }
          else
            return false;
        }
      }
    }
  }

  return true;
}

AREXPORT ArConfigSection *ArConfig::findSection(const char *sectionName) const
{
  // KMC 6/26/13 Added this to prevent exception in call to strcasecmp (at 
  // least in its current implementation). Still allowing a search for an 
  // empty string because that would have previously worked, and do not want
  // to break any existing functionality.
  if (sectionName == NULL) {
    ArLog::log(ArLog::Normal,
               "ArConfig::findSection() cannot find NULL section name");
    return NULL;
  }

  ArConfigSection *section = NULL;
  ArConfigSection *tempSection = NULL;

  for (std::list<ArConfigSection *>::const_iterator sectionIt = mySections.begin(); 
       sectionIt != mySections.end(); 
       sectionIt++)
  {
    tempSection = (*sectionIt);
    if (tempSection == NULL) {
      ArLog::log(ArLog::Normal,
                 "ArConfig::findSection(%s) unexpected null section in config",
                 sectionName);
      continue;
    }

   
    if (ArUtil::strcasecmp(tempSection->getName(), sectionName) == 0)
    {
      section = tempSection;
      // KMC 7/11/12 Added the break here (hoping there's not a compelling
      // reason to always search the config for null sections.
      break;
    }
  }
  return section;

} // end method findSection


void ArConfig::copySectionsToParse(std::list<std::string> *from)
{
  // MPL inserted these two lines trying to track down a memory leak
  if (mySectionsToParse != NULL)
    delete mySectionsToParse;
  std::string sections;
  mySectionsToParse = NULL;
  if (from != NULL) {
    mySectionsToParse = new std::list<std::string>();
    for (std::list<std::string>::const_iterator spIter = from->begin();
         spIter != from->end();
         spIter++) {
      sections += "'";
      sections += (*spIter);
      sections += "' ";
      mySectionsToParse->push_back(*spIter);
    } // end for each section to parse
    ArLog::log(ArLog::Verbose, "Will parse section(s) %s", sections.c_str());
  } // end if copy sections to parse
  else
  {
    ArLog::log(ArLog::Verbose, "Will parse all sections");
  }
} // end method copySectionsToParse

AREXPORT void ArConfig::addSectionNotToParse(const char *section)
{
  ArLog::log(ArLog::Normal, "%sWill not parse section %s", 
	     myLogPrefix.c_str(), section);
  mySectionsNotToParse.insert(section);
} // end method copySectionsToParse



AREXPORT void ArConfig::remSectionNotToParse(const char *section)
{
  ArLog::log(ArLog::Normal, "%sWill not not parse section %s", 
	     myLogPrefix.c_str(), section);
  mySectionsNotToParse.erase(section);
} // end method copySectionsToParse



AREXPORT void ArConfig::clearAllValueSet(void)
{
  std::list<ArConfigSection *> *sections;
  ArConfigSection *section;
  std::list<ArConfigSection *>::iterator sectionIt;
  
  std::list<ArConfigArg> *params;
  ArConfigArg *param;
  std::list<ArConfigArg>::iterator paramIt;

  sections = getSections();
  for (sectionIt = sections->begin(); 
       sectionIt != sections->end(); 
       sectionIt++)
  {
    section = (*sectionIt);
    params = section->getParams();
    for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
    {
      param = &(*paramIt);
      param->clearValueSet();
    }
  }
}


AREXPORT void ArConfig::removeAllUnsetValues(void)
{
  removeAllUnsetValues(false);
}

AREXPORT void ArConfig::removeAllUnsetSections(void)
{
  removeAllUnsetValues(true);
}


AREXPORT void ArConfig::removeAllUnsetValues(bool isRemovingUnsetSectionsOnly)
{
  std::list<ArConfigSection *> *sections;
  ArConfigSection *section;
  std::list<ArConfigSection *>::iterator sectionIt;
  
  std::list<ArConfigArg> *params;
  ArConfigArg *param;
  std::list<ArConfigArg>::iterator paramIt;
  std::list<std::list<ArConfigArg>::iterator> removeParams;
  std::list<std::list<ArConfigArg>::iterator>::iterator removeParamsIt;

  sections = getSections();
  for (sectionIt = sections->begin(); 
       sectionIt != sections->end(); 
       sectionIt++)
  {
    section = (*sectionIt);
    params = section->getParams();
    for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
    {
      param = &(*paramIt);
      if (param->getType() != ArConfigArg::SEPARATOR && 
	        param->getType() != ArConfigArg::STRING_HOLDER && 
	        param->getType() != ArConfigArg::LIST_HOLDER && 
	        param->getType() != ArConfigArg::DESCRIPTION_HOLDER)
      {
        if (!param->isValueSet()) {
	        removeParams.push_back(paramIt);
        }
        else if (isRemovingUnsetSectionsOnly) {
          removeParams.clear();
          break;
        }
      }

    }
    while ((removeParamsIt = removeParams.begin()) != removeParams.end())
    {
      ArLog::log(ArLog::Verbose, 
		 "%s:removeAllUnsetValues: Removing %s:%s", 
     myLogPrefix.c_str(),
		 section->getName(), (*(*removeParamsIt)).getName());
      section->getParams()->erase((*removeParamsIt));
      removeParams.pop_front();      
    }
  }
}



/**
   This is more temporary than some of the others like 'setSaveUknown'
   and such, this should be called by anything that's going to parse
   the config...
**/
AREXPORT void ArConfig::setPermissions(
	bool allowFactory, bool saveUnknown)
{
  myPermissionAllowFactory = allowFactory;
  myPermissionSaveUnknown = saveUnknown;
  
  ArLog::log(ArLog::Normal, "%s: Setting permissions of allowFactory to %s and saveUnknown to %s",
	     myLogPrefix.c_str(), 
	     ArUtil::convertBool(myPermissionAllowFactory),
	     ArUtil::convertBool(myPermissionSaveUnknown));
}

AREXPORT ArConfigArg::RestartLevel ArConfig::getRestartLevelNeeded(void) const
{
  return myRestartLevelNeeded;
}

AREXPORT void ArConfig::resetRestartLevelNeeded(void) 
{
  myRestartLevelNeeded = ArConfigArg::NO_RESTART;
}


AREXPORT void ArConfig::addListNamesToParser(const ArConfigArg &parent)
{
  if (parent.getType() != ArConfigArg::LIST)
    return;
  
  std::list<ArConfigArg> children = parent.getArgs();
  std::list<ArConfigArg>::iterator it;
  
  for (it = children.begin(); it != children.end(); it++)
  {
    ArConfigArg child = (*it);
    //ArLog::log(ArLog::Normal, "@@@ Adding %s", child.getName());
    if (!myParser.addHandlerWithError(child.getName(), &myParserCB))
    {
      if (!myIsQuiet) {
	ArLog::log(ArLog::Verbose, "%sCould not add parameter '%s' to file parser, probably already there.", 
		   myLogPrefix.c_str(), child.getName());
      }
    }
    addListNamesToParser(child);
  }
}

AREXPORT ArConfigSection::ArConfigSection(const char *name, 
					                                const char *comment,
                                          bool isQuiet,
                                          const char *categoryName) :
  myName((name != NULL) ? name : ""),
  myComment((comment != NULL) ? comment : ""),
  myCategoryName((categoryName != NULL) ? categoryName : ""),
  myDisplayName(""),
  myFlags(NULL),
  myParams(),
  myIsQuiet(isQuiet)
{
  myFlags = new ArArgumentBuilder(512, '|');
  myFlags->setQuiet(myIsQuiet);
}

AREXPORT ArConfigSection::~ArConfigSection()
{
  delete myFlags;
}


AREXPORT ArConfigSection::ArConfigSection(const ArConfigSection &section) 
{
  myName = section.myName;
  myComment = section.myComment;
  myCategoryName = section.myCategoryName;
  myDisplayName = section.myDisplayName;

  myFlags = new ArArgumentBuilder(512, '|');
  // Since any messages were logged when the first section was created,
  // it doesn't seem necessary to log them again.
  myFlags->setQuiet(true);
  myFlags->addPlain(section.myFlags->getFullString());

  for (std::list<ArConfigArg>::const_iterator it = section.myParams.begin(); 
       it != section.myParams.end(); 
       it++) 
  {
    myParams.push_back(*it);
  }

  myIsQuiet = section.myIsQuiet;
}

AREXPORT ArConfigSection &ArConfigSection::operator=(const ArConfigSection &section) 
{
  if (this != &section) 
  {
    
    myName = section.getName();
    myComment = section.getComment();
    myCategoryName = section.getCategoryName();
    myDisplayName = section.myDisplayName;
  
    delete myFlags;
    myFlags = new ArArgumentBuilder(512, '|');
    //myFlags->setQuiet(myIsQuiet);
    myFlags->addPlain(section.myFlags->getFullString());
    
    for (std::list<ArConfigArg>::const_iterator it = section.myParams.begin(); 
	 it != section.myParams.end(); 
	 it++) 
    {
      myParams.push_back(*it);
    }
      
    myIsQuiet = section.myIsQuiet;

  }
  return *this;
}


AREXPORT void ArConfigSection::copyAndDetach(const ArConfigSection &section) 
{
  if (this != &section) 
  {
    
    myName = section.getName();
    myComment = section.getComment();
    myCategoryName = section.getCategoryName();
    myDisplayName = section.myDisplayName;

    delete myFlags;
    myFlags = new ArArgumentBuilder(512, '|');
    myFlags->setQuiet(myIsQuiet);

    myFlags->addPlain(section.myFlags->getFullString());
    
    for (std::list<ArConfigArg>::const_iterator it = section.myParams.begin(); 
	       it != section.myParams.end(); 
	       it++) 
    {
      ArConfigArg paramCopy;
      paramCopy.copyAndDetach(*it);
      myParams.push_back(paramCopy);
    }

    myIsQuiet = section.myIsQuiet;

  }
  //return *this;

}



AREXPORT ArConfigArg *ArConfigSection::findParam(const char *paramName,
                                                 bool isAllowStringHolders)
{
  ArConfigArg *param = NULL;
  ArConfigArg *tempParam = NULL;

  for (std::list<ArConfigArg>::iterator pIter = myParams.begin(); 
       pIter != myParams.end(); 
       pIter++)
  {
    tempParam = &(*pIter);
    // ignore string holders 
    if (!isAllowStringHolders &&
        ((tempParam->getType() == ArConfigArg::STRING_HOLDER) || 
         (tempParam->getType() == ArConfigArg::LIST_HOLDER)))
      continue;
    if (ArUtil::strcasecmp(tempParam->getName(), paramName) == 0)
    {
      param = tempParam;
    }
  }
  return param;

} // end method findParam

/**
 * This method provides a shortcut for looking up child parameters 
 * in a list type parameter that is contained in the section.
 * For example:
 * <code>
 *     std::list<std::string> maxRangeP;
 *     maxRangeP.push_back(laserName);
 *     maxRangeP.push_back("LMS2xxInfo");
 *     maxRangeP.push_back("MaxRange");
 *     ArConfigArg *maxRangeArg = section->findParam(maxRangeP);
 *
 *     if(maxRangeArg)
 *     {
 *          ...
 * 
 * </code>
 *
 * This is functionally equivalent to the following, but better for long
 * sequences.
 * <code>
 *     ArConfigArg *laserListArg = section->findParam(laserName);
 *     if(laserListArg)
 *     {
 *        ArConfigArg *lms2xxArg   = laserListArg->findArg("LMS2xxInfo");
 *        if(lms2xxArg)
 *        {
 *            ArConfigArg *maxRangeArg  = lms2xxxArg->findArg("MaxRange");
 *            if(maxRangeArg)
 *            {
 *                 ...
 *            }
 *            else
 *            {
 *                ...
 *         ...
 *     ...
 *     etc.
 * </code>
 *
 * @param paramNamePath a list of strings that specifies the sequence of parameter
 * names, from "top" to "bottom"
 * @return ArConfigArg *, a pointer to the requested parameter, or NULL
 * if not found
**/ 
AREXPORT ArConfigArg *ArConfigSection::findParam
                                      (const std::list<std::string> &paramNamePath,
                                       bool isAllowHolders)
{
  if (paramNamePath.empty()) {
    return NULL;
  }

  ArConfigArg *topParam = findParam(paramNamePath.front().c_str(), isAllowHolders);
  if (topParam == NULL) {
    return NULL;
  }

  // Handle the degenerate case where this method is used in a non-list lookup
  if (paramNamePath.size() == 1) {
    return topParam;
  }

  ArConfigArg *curParam = topParam;

  std::list<std::string>::const_iterator iter = paramNamePath.begin();
  iter++; // skipping the front one from above
  
  for (iter; iter != paramNamePath.end(); iter++) {

    // If curParam is not a LIST type, then findArg will return NULL.
    curParam = curParam->findArg((*iter).c_str());
  
  } // end while not found

  // At this point, curParam should either be NULL (because the path 
  // was not found)... or... nameIndex should equal pathLength and 
  // curParam should be the requested one.

  return curParam;
  
} // end method findParam


AREXPORT ArConfigArg *ArConfigSection::findParam(const char **paramNamePath, 
                                                 int pathLength,
                                                 bool isAllowHolders)
{
  if ((paramNamePath == NULL) || (pathLength <= 0)) {
    ArLog::log(ArLog::Normal,
               "ArConfigSection::findParam() invalid input, paramNamePath = %p pathLength = %i",
               paramNamePath,
               pathLength);
    return NULL;
  }

  std::list<std::string> nameList;
  for (int i = 0; i < pathLength; i++) {
    nameList.push_back(paramNamePath[i]);
  } // end for each path component

  return findParam(nameList, isAllowHolders);


} // end method findParam


AREXPORT bool ArConfigSection::containsParamsOfPriority
                                        (ArPriority::Priority highestPriority,
                                         ArPriority::Priority lowestPriority)
{
  if (highestPriority > lowestPriority) {
    return false;
  }
  for (std::list<ArConfigArg>::iterator iter = myParams.begin();
       iter != myParams.end();
       iter++) {
    const ArConfigArg &arg = *iter;

    if (arg.getType() == ArConfigArg::SEPARATOR) {
      continue;
    }
    if ((arg.getConfigPriority() >= highestPriority) &&
        (arg.getConfigPriority() <= lowestPriority)) {
      return true;
    }
  } // end for each parameter

  return false;

} // end method containsParamsOfPriority


// This will also remove list holders
AREXPORT bool ArConfigSection::remStringHolder(const char *paramName)
{
  ArConfigArg *tempParam = NULL;
  
  for (std::list<ArConfigArg>::iterator pIter = myParams.begin(); 
       pIter != myParams.end(); 
       pIter++)
  {
    tempParam = &(*pIter);
    
     // pay attention to only string holders
    if (ArUtil::isStrEmpty(paramName)) {
      continue;
    }
    if ((tempParam->getType() != ArConfigArg::STRING_HOLDER) &&
        (tempParam->getType() != ArConfigArg::LIST_HOLDER)) { 
      continue;
    }

    if (ArUtil::strcasecmp(tempParam->getName(), paramName) == 0)
    {
      myParams.erase(pIter);
      // Recurse to ensure that all occurrences of the string holder
      // are removed.
      remStringHolder(paramName);
      return true;
    }
  }
  return false;
}

AREXPORT bool ArConfigSection::hasFlag(const char *flag) const
{
  size_t i;
  for (i = 0; i < myFlags->getArgc(); i++)
  {
    if (strcmp(myFlags->getArg(i), flag) == 0)
    {
      return true;
    }
  }
  return false;
}

AREXPORT bool ArConfigSection::addFlags(const char *flags, bool isQuiet)
{
  if (myFlags == NULL) {
    return false;
  }
  myFlags->setQuiet(isQuiet);

  // MPL replacing this line with the stuff from after it until the
  // return, this is so that if the section already has a flag we
  // don't add it again

  //myFlags->addPlain(flags); 

  size_t i;
  ArArgumentBuilder theseFlags(512, '|');
  theseFlags.setQuiet(isQuiet);

  theseFlags.addPlain(flags);
  
  for (i = 0; i < theseFlags.getArgc(); i++)
  {
    if (!hasFlag(theseFlags.getArg(i)))
      myFlags->addPlain(theseFlags.getArg(i));
  }
  return true; 
}

AREXPORT bool ArConfigSection::remFlag(const char *flag)
{
  size_t i;
  
  for (i = 0; i < myFlags->getArgc(); i++)
  {
    if (strcmp(myFlags->getArg(i), flag) == 0)
    {
      myFlags->removeArg(i, true);
      return remFlag(flag);
    }
  }

  return true;
}
  
AREXPORT void ArConfigSection::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
}


AREXPORT void ArConfigSection::setName(const char *name) 
{ 
  myName = ((name != NULL) ? name : "");
}
 
AREXPORT void ArConfigSection::setComment(const char *comment) 
{ 
  myComment = ((comment != NULL) ? comment : ""); 
}


AREXPORT const char *ArConfigSection::getCategoryName() const
{
  return myCategoryName.c_str();
}

void ArConfigSection::setCategoryName(const char *categoryName)
{
  myCategoryName = ((categoryName != NULL) ? categoryName : "");
}


