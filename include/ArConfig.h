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
#ifndef ARCONFIG_H
#define ARCONFIG_H

#include "ArConfigArg.h"
#include "ArFileParser.h"
#include "ArHasFileName.h"
#include <set>

class ArArgumentBuilder;
class ArConfigSection;


/// Stores configuration information which may be read to and from files or other sources
/**
 * The configuration is a set of parameters (or config arguments), organized into
 * sections.  Parameters contain a key or name (a short string), a value 
 * (which may be one of several types), a priority (important, normal, or trivial
 * to most users), a longer description, and a display hint which suggests 
 * what kind of UI control might be most appropriate for the parameter.
 * After adding a parameter to the configuration, its value may be changed
 * when the configuration is loaded or reloaded from a file.
 * Various program modules may register callbacks to be notified
 * when a shared global configuration (such as the static ArConfig object kept
 * by the Aria class) is loaded or otherwise changed.
 *
 * Classes dealing with more specialized kinds of config files 
 * inherit from this one.
 *
 * Important methods in this class are: addParam(), addProcessFileCB(),
 * remProcessFileCB(), parseFile(), writeFile().
 *
 * Usually, configuration data are read from and written to a file using
 * parseFile() and writeFile(), or are set
 * by a remote client via ArNetworking.  It is also possible to import 
 * configuration settings from an ArArgumentParser (which, for example, may 
 * contain a program's command line arguments) using useArgumentParser().
 *
 * @warning ArConfig does not escape any special characters when writing or
 * loading to/from a file. Therefore in general parameter names, values, section
 * names, and comments must not contain characters which have special meaning
 * in a config file, such as '#', ';', tab or newline. (see also ArFileParser) Parameter names may 
 * have spaces, though by convention they generally do not.  

  @ingroup OptionalClasses
*/
class ArConfig : public ArHasFileName
{
private:

  /// Current version of the output config file.
  static const char *CURRENT_CONFIG_VERSION;
  /// Parser tag that introduces the config file version.
  static const char *CONFIG_VERSION_TAG;

  /// Current version of the output config resource file.
  static const char *CURRENT_RESOURCE_VERSION;
  /// Parser tag that introduces the config resource file version.
  static const char *RESOURCE_VERSION_TAG;

public:

  /// Sections related to I/O, ARCL, display, network communication, etc.
  AREXPORT static const char *CATEGORY_ROBOT_INTERFACE;
  /// Sections related to robot behavior, docking, driving, navigation, etc.
  AREXPORT static const char *CATEGORY_ROBOT_OPERATION;
  /// Sections related to the robot hardware and its sensors/accessories.
  AREXPORT static const char *CATEGORY_ROBOT_PHYSICAL;
  /// Sections related to the fleet, enterprise manager, etc.
  AREXPORT static const char *CATEGORY_FLEET;
  /// Sections related to security.
  AREXPORT static const char *CATEGORY_SECURITY;
  /// Sections related to debug, including logging, replay, etc.
  AREXPORT static const char *CATEGORY_DEBUG;

  /// Returns NULL if category name is not one of the predefined constants 
  AREXPORT static const char *toCategoryName(const char *categoryName);

public:
  /// Constructor
  AREXPORT ArConfig(const char *baseDirectory = NULL, 
		                bool noBlanksBetweenParams = false,
		                bool ignoreBounds = false,
		                bool failOnBadSection = false,
		                bool saveUnknown = true);
  /// Destructor
  AREXPORT virtual ~ArConfig();

  /// Copy constructor
  AREXPORT ArConfig(const ArConfig &config);

  /// Assignment operator
  AREXPORT ArConfig &operator=(const ArConfig &config);

  /// Copies the given config to this one, detaching any pointers so they are not shared
  AREXPORT virtual void copyAndDetach(const ArConfig &config);

  /// Stores an optional config and robot name to be used in log messages.
  AREXPORT virtual void setConfigName(const char *configName,
                                      const char *robotName = NULL);

  /// Turn on this flag to reduce the number of verbose log messages.
  AREXPORT virtual void setQuiet(bool isQuiet);


  /// Sets an associate config that provides translations for each parameter (as read from a resource file).
  AREXPORT virtual void setTranslator(ArConfig *xlatorConfig);

  /// Returns the associated translator config, or NULL if none.
  AREXPORT virtual ArConfig *getTranslator() const;


  /// Parse a config file
  AREXPORT bool parseFile(const char *fileName, 
                          bool continueOnError = false,
                          bool noFileNotFoundMessage = false, 
                          char *errorBuffer = NULL,
                          size_t errorBufferLen = 0,
                          std::list<std::string> *sectionsToParse = NULL,
                          ArPriority::Priority highestPriority = ArPriority::FIRST_PRIORITY,
                          ArPriority::Priority lowestPriority  = ArPriority::LAST_PRIORITY,
                          ArConfigArg::RestartLevel *restartLevelNeeded = NULL);
      
  /// Write out a config file
  AREXPORT bool writeFile(const char *fileName, 
                          bool append = false,
                          std::set<std::string> *alreadyWritten = NULL,
                          bool writeExtras = false,
                          std::list<std::string> *sectionsToWrite = NULL,
                          ArPriority::Priority highestPriority = ArPriority::FIRST_PRIORITY,
                          ArPriority::Priority lowestPriority  = ArPriority::LAST_PRIORITY);

  /// Parse a set of text lines, in the same format as the config file.
  AREXPORT bool parseText(const std::list<std::string> &configLines,
                          bool continueOnErrors = false,
                          bool *parseOk = NULL,
                          bool *processOk = NULL,
                          char *errorBuffer = NULL,
                          size_t errorBufferLen = 0,
                          std::list<std::string> *sectionsToParse = NULL,
                          ArPriority::Priority highestPriority = ArPriority::FIRST_PRIORITY,
                          ArPriority::Priority lowestPriority  = ArPriority::LAST_PRIORITY,
                          ArConfigArg::RestartLevel *restartLevelNeeded = NULL);

  
  /// Parse a config resource file, for translation.
  AREXPORT bool parseResourceFile(const char *fileName, 
                                  bool continueOnError = true,
                                  char *errorBuffer = NULL,
                                  size_t errorBufferLen = 0,
                                  std::list<std::string> *sectionsToParse = NULL);
                                        
  /// Parse a config resource file with parameters suitable for custom commands.
  AREXPORT void parseResourceFile(ArArgumentBuilder *builder);
  
  /// Write a config resource file, for translation.
  AREXPORT bool writeResourceFile(const char *fileName, 
                                  bool append = false,
                                  std::set<std::string> *alreadyWritten = NULL,
                                  std::list<std::string> *sectionsToWrite = NULL);

  /// Write a config resource file with parameters suitable for custom commands.
  AREXPORT void writeResourceFile(ArArgumentBuilder *builder);
  
  /// Command to add a section and its description to the specified category.
  /**
   * The category is expected to be one of the CATEGORY_ constants defined above.
   * If the section already exists but has not yet been assigned to a category,
   * then this method can be called to add it to the specified category.
  **/
  AREXPORT bool addSection(const char *categoryName,
                           const char *sectionName,
                           const char *sectionDescription);


  /// Command to add a parameter to the given section with given priority
  AREXPORT bool addParam(const ArConfigArg &arg, 
                         const char *sectionName = "", 
                         ArPriority::Priority priority = ArPriority::NORMAL,
                         const char *displayHint = NULL,
                         ArConfigArg::RestartLevel restart = ArConfigArg::NO_RESTART);

  /// Command to add a new comment to the given section with given priority
  AREXPORT bool addComment(const char *comment, const char *sectionName = "", 
			                     ArPriority::Priority priority = ArPriority::NORMAL);

  /// Adds a parameter that has all the other information on it
  /// already set
  AREXPORT bool addParamAsIs(const ArConfigArg &arg, 
			     const char *sectionName = "");

  /// Sets the comment for a section
  AREXPORT void setSectionComment(const char *sectionName, 
				                          const char *comment);


  /// Uses this argument parser after it parses a file before it processes
  AREXPORT void useArgumentParser(ArArgumentParser *parser);

  /// for inheritors this is called after the file is processed
  /**
     For classes that inherit from ArConfig this function is called
     after parseFile and all of the processFileCBs are called... If
     you want to call something before the processFileCBs then just
     add a processFileCB... this is only called if there were no
     errors parsing the file or continueOnError was set to false when
     parseFile was called

     @return true if the config parsed was good (parseFile will return
     true) false if the config parsed wasn't (parseFile will return false)
   **/
  AREXPORT virtual bool processFile(void) { return true; }
  /// Adds a callback to be invoked when the configuration is loaded or
  /// reloaded.
  AREXPORT void addProcessFileCB(ArRetFunctor<bool> *functor, 
				 int priority = 0);
  /// Adds a callback to be invoked when the configuration is loaded
  /// or reloaded.... if you really want errors you should use
  /// addProcessFileWithErrorCB, this is just to catch mistakes
  AREXPORT void addProcessFileCB(ArRetFunctor2<bool, char *, size_t> *functor, 
				 int priority = 0);
  /// Adds a callback to be invoked when the configuration is loaded or
  /// reloaded, which may also receive error messages
  AREXPORT void addProcessFileWithErrorCB(
	  ArRetFunctor2<bool, char *, size_t> *functor, 
	  int priority = 0);
  /// Removes a processedFile callback
  AREXPORT void remProcessFileCB(ArRetFunctor<bool> *functor);
  /// Removes a processedFile callback
  AREXPORT void remProcessFileCB(
	  ArRetFunctor2<bool, char *, size_t> *functor);
  /// Call the processFileCBs
  AREXPORT bool callProcessFileCallBacks(bool continueOnError,
					 char *errorBuffer = NULL,
					 size_t errorBufferLen = 0);
  /// This parses the argument given (for parser or other use)
  AREXPORT bool parseArgument(ArArgumentBuilder *arg, 
			      char *errorBuffer = NULL,
			      size_t errorBufferLen = 0);

  /// Parses the config file version information.
  AREXPORT bool parseVersion(ArArgumentBuilder *arg, 
			                       char *errorBuffer = NULL,
			                        size_t errorBufferLen = 0);

  /// This parses the section change (for parser or other use)
  AREXPORT bool parseSection(ArArgumentBuilder *arg, 
			      char *errorBuffer = NULL,
			      size_t errorBufferLen = 0);

  AREXPORT bool parseListBegin(ArArgumentBuilder *arg,
				                       char *errorBuffer,
				                       size_t errorBufferLen);

  AREXPORT bool parseListEnd(ArArgumentBuilder *arg,
				                     char *errorBuffer,
				                     size_t errorBufferLen);

  /// This parses an unknown argument (so we can save it)
  AREXPORT bool parseUnknown(ArArgumentBuilder *arg, 
			     char *errorBuffer = NULL,
			     size_t errorBufferLen = 0);

  /// Gets the restart level needed
  AREXPORT ArConfigArg::RestartLevel getRestartLevelNeeded(void) const;

  /// Gets the restart level needed
  AREXPORT void resetRestartLevelNeeded(void);

  /// Get the base directory
  AREXPORT const char *getBaseDirectory(void) const;
  /// Set the base directory
  AREXPORT void setBaseDirectory(const char *baseDirectory);
  /// Get the file name we loaded
  AREXPORT const char *getFileName(void) const;

  /// Set whether we have blanks between the params or not
  AREXPORT void setNoBlanksBetweenParams(bool noBlanksBetweenParams);

  /// Get whether we have blanks between the params or not
  AREXPORT bool getNoBlanksBetweenParams(void);

  /// Use an argument parser to change the config
  AREXPORT bool parseArgumentParser(ArArgumentParser *parser,
				    bool continueOnError = false,
				    char *errorBuffer = NULL,
				    size_t errorBufferLen = 0);


  /// Returns the list of categories contained in this config.
  AREXPORT std::list<std::string> getCategoryNames() const;

  /// Returns the names of the sections that have been added to the specified category.
  AREXPORT std::list<std::string> getSectionNamesInCategory(const char *categoryName) const;

  /// Returns the names of all sections in the config.
  AREXPORT std::list<std::string> getSectionNames() const;

  /// Get the sections themselves (use only if you know what to do)
  AREXPORT std::list<ArConfigSection *> *getSections(void);



  /// Find the section with the given name.  
  /// @return section object, or NULL if not found.
  AREXPORT ArConfigSection *findSection(const char *sectionName) const;

  /// Set the log level used when loading or reloading the configuration
  AREXPORT void setProcessFileCallbacksLogLevel(ArLog::LogLevel level) 
    {  myProcessFileCallbacksLogLevel = level; }
  /// Get the log level used when loading or reloading the configuration
  AREXPORT ArLog::LogLevel getProcessFileCallbacksLogLevel(void)
    {  return myProcessFileCallbacksLogLevel; }

  /// Sets whether we save unknown items (if we don't save 'em we ignore 'em)
  AREXPORT void setSaveUnknown(bool saveUnknown) 
    { mySaveUnknown = saveUnknown; }
  /// Gets whether we save unknowns (if we don't save 'em we ignore 'em)
  AREXPORT bool getSaveUnknown(void) { return mySaveUnknown; }

  /// Clears out all the section information
  AREXPORT void clearSections(void);
  /// Clears out all the section information and the processFileCBs
  AREXPORT void clearAll(void);

  /// adds a flag to a section
  AREXPORT bool addSectionFlags(const char *sectionName, 
				const char *flags);
  /// Removes a flag from a section
  AREXPORT bool remSectionFlag(const char *sectionName, 
			       const char *flag);

  /// calls clearValueSet on the whole config (internal for default configs)
  AREXPORT void clearAllValueSet(void);
  /// Removes all unset values from the config (internal for default configs)
  AREXPORT void removeAllUnsetValues(void);
  /// Removes all values from sections in which no value has been set (internal)
  AREXPORT void removeAllUnsetSections(void);

  /// Logs the config
  AREXPORT void log(bool isSummary = true,
                    std::list<std::string> *sectionNameList = NULL,
                    const char *logPrefix = "");

  /// Sets permissions on some things 
  AREXPORT void setPermissions(bool allowFactory = true, 
			       bool rememberUnknowns = true);

  /// Adds a section for this config to always skip
  AREXPORT void addSectionNotToParse(const char *section);
  /// Removes a section for this config to always skip
  AREXPORT void remSectionNotToParse(const char *section);
 
  /// Adds the children of the given parent arg to the config parser. 
  AREXPORT void addListNamesToParser(const ArConfigArg &parent);

protected:

  /// Write out a section  
  AREXPORT void writeSection(ArConfigSection *section, 
                             FILE *file,
                             std::set<std::string> *alreadyWritten,
                             bool writeExtras,
                             ArPriority::Priority highestPriority,
                             ArPriority::Priority lowestPriority);

  /// Write out a section in CSV format for translation
  AREXPORT void writeSectionResource(ArConfigSection *section, 
                                     FILE *file,
                                     std::set<std::string> *alreadyWritten);

  AREXPORT void translateSection(ArConfigSection *section);

  void copySectionsToParse(std::list<std::string> *from);

  /// Removes unset values from the config.
  /**
   * @param isRemovingUnsetSectionsOnly a bool set to true if unset parameters should
   * not be removed from sections that have any set parameters; false if all unset 
   * values should be removed
  **/
  AREXPORT void removeAllUnsetValues(bool isRemovingUnsetSectionsOnly);


  /**
     This class's job is to make the two functor types largely look
     like the same one from the code's perspective, this is so we can
     store them both in the same map for order of operations purposes.
     
     The funkiness with the constructor is because the retfunctor2
     looks like the retfunctor and winds up always falling into that
     constructor.
  **/
  class ProcessFileCBType
  {
    public:
    ProcessFileCBType(
	    ArRetFunctor2<bool, char *, size_t> *functor)
    {
      myCallbackWithError = functor;
      myCallback = NULL;
    }
    ProcessFileCBType(ArRetFunctor<bool> *functor)
    {
      myCallbackWithError = NULL;
      myCallback = functor;
    }
    ~ProcessFileCBType() {}
    bool call(char *errorBuffer, size_t errorBufferLen) 
    { 
      if (myCallbackWithError != NULL) 
	      return myCallbackWithError->invokeR(errorBuffer, errorBufferLen);
      else if (myCallback != NULL) 
        return myCallback->invokeR(); 
      // if we get here there's a problem
      ArLog::log(ArLog::Terse, "ArConfig: Horrible problem with process callbacks");
      return false;
    }
    bool haveFunctor(ArRetFunctor2<bool, char *, size_t> *functor)
    { 
      if (myCallbackWithError == functor) 
        return true; 
      else 
        return false; 
    }
    bool haveFunctor(ArRetFunctor<bool> *functor)
    { 
      if (myCallback == functor) 
        return true; 
      else 
        return false; 
    }
    const char *getName(void) 
    { 
      if (myCallbackWithError != NULL)
        return myCallbackWithError->getName();
      else if (myCallback != NULL)
        return myCallback->getName();
      // if we get here there's a problem
      ArLog::log(ArLog::Terse, "ArConfig: Horrible problem with process callback names");
      return NULL;
    }
    protected:
    ArRetFunctor2<bool, char *, size_t> *myCallbackWithError;
    ArRetFunctor<bool> *myCallback;
  };

  void addParserHandlers(void);
  void remParserHandlers(void);

  /// Optional name of the robot with which the config is associated.
  std::string myRobotName;
  /// Optional name of the config instance.
  std::string myConfigName;
  /// Prefix to be inserted in log messages (contains the robot and config names).
  std::string myLogPrefix;

  ArArgumentParser *myArgumentParser;
  std::multimap<int, ProcessFileCBType *> myProcessFileCBList;
  bool myNoBlanksBetweenParams;

  /// Version information read from config file
  std::string myConfigVersion;

  ArConfig *myTranslator;
  char myCsvSeparatorChar;

  std::string mySection;
  std::list<std::string> *mySectionsToParse;
  std::set<std::string> mySectionsNotToParse;
  ArPriority::Priority myHighestPriorityToParse;
  ArPriority::Priority myLowestPriorityToParse;
  
  ArConfigArg::RestartLevel myRestartLevelNeeded;
  bool myCheckingForRestartLevel;

  bool mySectionBroken;
  bool mySectionIgnored;
  bool myUsingSections;

  std::list<std::string> myParsingListNames;
  bool myIsParsingListBroken;

  std::string myFileName;
  std::string myBaseDirectory;
  ArFileParser myParser;

  bool myIgnoreBounds;
  bool myFailOnBadSection;
  bool myDuplicateParams;
  bool mySaveUnknown;
  bool myIsQuiet;

  bool myPermissionAllowFactory;
  bool myPermissionSaveUnknown;

  ArLog::LogLevel myProcessFileCallbacksLogLevel;

  /// Map that defines the section names contained in each category. 
  std::map<std::string, std::list<std::string> > myCategoryToSectionsMap;

  // our list of sections which has in it the argument list for each
  std::list<ArConfigSection *> mySections;

  // callback for the file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> myParserCB;
  // callback for the config version in the file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> myVersionCB;
  // callback for the section in the file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> mySectionCB;

  // callback for the _listBegin in the file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> myListBeginCB;
  // callback for the _listEnd in the file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> myListEndCB;

  // callback for the unknown param (or whatever) file parser
  ArRetFunctor3C<bool, ArConfig, ArArgumentBuilder *, char *, size_t> myUnknownCB;


};


/** Represents a section in the configuration. Sections are used to
 *  group items used by separate parts of Aria.
 */
class ArConfigSection
{
public:
  AREXPORT ArConfigSection(const char *name = NULL, 
						               const char *comment = NULL,
                           bool isQuiet = false,
                           const char *categoryName = NULL);

  AREXPORT virtual ~ArConfigSection();
  AREXPORT ArConfigSection(const ArConfigSection &section);
  AREXPORT ArConfigSection &operator=(const ArConfigSection &section);

  /// Copies the given section to this one, detaching any pointers so they are not shared
  AREXPORT virtual void copyAndDetach(const ArConfigSection &section);

  /** @return The name of this section */
  const char *getName(void) const { return myName.c_str(); }

  /** @return A comment describing this section */
  const char *getComment(void) const { return myComment.c_str(); }

  /// Returns the name of the category that contains this section.
  AREXPORT const char *getCategoryName() const;

  const char *getFlags(void) const { return myFlags->getFullString(); }
  AREXPORT bool hasFlag(const char *flag) const;
  
  std::list<ArConfigArg> *getParams(void) { return &myParams; }
  
  AREXPORT void setName(const char *name);

  AREXPORT void setComment(const char *comment);

  AREXPORT bool addFlags(const char *flags, bool isQuiet = false);
  AREXPORT bool remFlag(const char *dataFlag);

  /// Finds a parameter item in this section with the given name.  Returns NULL if not found.
  AREXPORT ArConfigArg *findParam(const char *paramName,
                                  bool isAllowStringHolders = false);

  /// Finds a list member parameter with the specified name path.  Returns NULL if not found.
  AREXPORT ArConfigArg *findParam(const std::list<std::string> &paramNamePath,
                                  bool isAllowHolders = false); 

  /// Finds a list member parameter with the specified name path.  Returns NULL if not found.
  AREXPORT ArConfigArg *findParam(const char **paramNamePath, 
                                  int pathLength,
                                  bool isAllowHolders = false); 

  /// Determines whether the current section contains parameters in the specified priority ran
  AREXPORT bool containsParamsOfPriority(ArPriority::Priority highestPriority,
                                         ArPriority::Priority lowestPriority);

  /// Removes a string holder for this param, returns true if it found one
  AREXPORT bool remStringHolder(const char *paramName); 

  /// Turn on this flag to reduce the number of verbose log messages.
  AREXPORT void setQuiet(bool isQuiet); 

protected:

  /// Give the config access to the protected category name setter.
  friend class ArConfig;
  
  /// Sets the name of the category to which this section belongs.
  void setCategoryName(const char *categoryName);

protected:

  std::string myName;
  std::string myComment;
  std::string myCategoryName;
  std::string myDisplayName; // Not yet supported
  ArArgumentBuilder *myFlags;
  std::list<ArConfigArg> myParams;
  bool myIsQuiet;

}; // end class ArConfigSection

#endif // ARCONFIG

