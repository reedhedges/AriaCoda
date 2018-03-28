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
#ifndef ARCONFIGARG_H
#define ARCONFIGARG_H

#include <map>

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArFunctor.h"

class ArArgumentBuilder;
class ArFileParser;
class ArSocket;

/// Argument class for ArConfig
/** 
    ArConfigArg stores information about a parameter (name, description, type),
    and a pointer to the variable that will actually store the value. This
    variable is normally stored and used by whatever class or module has added the
    parameter to ArConfig.  (In addition, there are some special types of
    ArConfigArg that behave differently such as the "holder" types and separator,
    these are used internally or in special cases.)

    Which constructor you use determines the value type of the ArConfigArg object.
  
    Example:
    @code
    config->addParam(ArConfigArg("MyParameter", &myTarget, "Example Parameter"), "Example Section");
    @endcode

    Where: <tt>config</tt> is a pointer to an ArConfig object or subclass (e.g.  use
    <tt>ArConfig* config = Aria::getConfig()</tt> to use the global Aria ArConfig
    object);
    <tt>myTarget</tt> is a variable (e.g. int) that is a class member whose instance will not
    be destroyed before the end of the program, or which will remove the parameter
    from ArConfig before being destroyed (the pointer to <tt>myTarget</tt> that is stored
    in ArConfig must not become invalid.)  The ArConfigArg object passed to
    addParam() will be copied and stored in ArConfig.  The type of the target
    variable (<tt>myTarget</tt>) determines the type of the parameter.


    @swignote Swig cannot determine the correct constructor to use
     based on most target langugages types, so you must use subclasses
     defined for various types. Or, use the constructor that accepts 
     functors for dealing with arguments.  Also, Swig cannot use pointers
     to change variables, so you must create ArConfigArg objects, passing
     in default values, and retain references to those objects,
     in addition to passing them to ArConfig, and read new values from those
     objects if ArConfig changes; or pass functors to ArConfigArg instead
     of the initial value.

    Special features:

    You can add a separator line to the section by adding an ArConfigArg with type SEPARATOR: 
    @code
      config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), "My Section");
    @endcode

    You can specify a list of possible values by setting a "display hint".  For
    example, for a string parameter with possible values "A", "B" and "C":
    @code
      ArConfigArg arg("Example", exampleString, "Example string parameter");
      arg.setDisplayHint("Choices:A;;B;;C");
      config->addParam(arg);
    @endcode
    This information is provided to clients, but you should still verify the
    value is not unexpected when it changes or when you use the value.
*/
class ArConfigArg
{
public:

  typedef enum 
  { 
    INVALID, ///< An invalid argument, the argument wasn't created correctly
    INT, ///< Integer argument
    DOUBLE, ///< Double argument
    STRING, ///< String argument
    BOOL, ///< Boolean argument
    FUNCTOR, ///< Argument that handles things with functors
    DESCRIPTION_HOLDER, ///< Argument that just holds a description
    STRING_HOLDER, ///< this one is for holding strings and reading them in and writing them out but not really letting them get sent anywhere (its for unknown config parameters (so they don't get lost if a feature is turned off)
    SEPARATOR, ///< Empty argument that merely acts as a separator within a (large) section.
    CPPSTRING, ///< Pointer to std::string, use like STRING.
    LIST, ///< Composite argument that contains an ordered list of other args
    LIST_HOLDER, ///< Placeholder for composite argument that are not currently active
    LAST_TYPE = LIST_HOLDER ///< Last value in the enumeration
  } Type;


  /// Indicates the components that must be restarted if the parameter is changed
  enum RestartLevel {
    NO_RESTART,       //< No restart necessary (default value)
    RESTART_CLIENT,   //< Client software must be restarted (TODO Can this be eliminated?)
    RESTART_IO,   //< IO has changed and needs to be restarted
    RESTART_SOFTWARE,   //< Server software must be restarted
    RESTART_HARDWARE,    //< Physical robot must be rebooted
    LAST_RESTART_LEVEL = RESTART_HARDWARE
  };

  enum {
    DEFAULT_DOUBLE_PRECISION = 5, ///< Default double precision, originates from previous saving behavior
    TYPE_COUNT = LAST_TYPE + 1, ///< Number of argument types
    RESTART_LEVEL_COUNT = LAST_RESTART_LEVEL + 1, ///< Number of restart levels
  };


  enum SocketIndices {

    SOCKET_INDEX_OF_SECTION_NAME = 0,
    SOCKET_INDEX_OF_ARG_NAME = 1,
    SOCKET_INDEX_OF_DESCRIPTION = 2,
    SOCKET_INDEX_OF_PRIORITY = 3,
    SOCKET_INDEX_OF_TYPE = 4,

    SOCKET_INDEX_OF_VALUE = 5,
    SOCKET_INDEX_OF_MIN_VALUE = 6,
    SOCKET_INDEX_OF_MAX_VALUE = 7,

    SOCKET_INDEX_OF_DISPLAY = 8, 
    SOCKET_INDEX_OF_PARENT_PATH = 9,
    SOCKET_INDEX_OF_SERIALIZABLE = 10

  }; // end enum SocketIndices


  enum ResourceIndices {

    RESOURCE_INDEX_OF_SECTION_NAME = 0,
    RESOURCE_INDEX_OF_ARG_NAME = 1,
    RESOURCE_INDEX_OF_TYPE = 2,
    RESOURCE_INDEX_OF_PRIORITY = 3,
    RESOURCE_INDEX_OF_RESTART_LEVEL = 4,
    RESOURCE_INDEX_OF_PARENT_PATH = 5,
    RESOURCE_INDEX_OF_DESCRIPTION = 6,
    RESOURCE_INDEX_OF_EXTRA = 7,
    RESOURCE_INDEX_OF_DISPLAY = 8, // not yet supported
    RESOURCE_INDEX_OF_NEW = 9 

  }; // end enum ResourceIndices



  /// Keyword that indicates the start of an ArConfigArg LIST object, for ArFileParser.
  AREXPORT static const char *LIST_BEGIN_TAG;
  /// Keyword that indicates the end of an ArConfigArg LIST object, for ArFileParser.
  AREXPORT static const char *LIST_END_TAG;

  /// Resource file keyword that indicates an empty string (cannot write empty for csv).
  AREXPORT static const char *NULL_TAG;
  /// Resource file keyword that indicates a new entry.
  AREXPORT static const char *NEW_RESOURCE_TAG;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Static Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns a text representation of the given type, suitable for logging.
  AREXPORT static const char *toString(Type t);

  /// Returns a text representation of the given RestartLevel, suitable for logging.
  AREXPORT static const char *toString(RestartLevel r);


  /// Given a text representation, returns the appropriate Type.
  AREXPORT static Type typeFromString(const char *text);

  /// Given a text representation, returns the appropriate RestartLevel.
  AREXPORT static RestartLevel restartLevelFromString(const char *text);



  /// Returns the section name contained in the given resource arg.
  AREXPORT static std::string parseResourceSectionName(ArArgumentBuilder *arg, 
                                                       const char *logPrefix = "");

  /// Returns the param name contained in the given resource arg.
  AREXPORT static std::string parseResourceArgName(ArArgumentBuilder *arg, 
                                                   const char *logPrefix = "");

  /// Returns the arg type contained in the given resource arg.
  AREXPORT static Type parseResourceType(ArArgumentBuilder *arg, 
                                         const char *logPrefix = "");

  /// Returns true if the given resource arg is "top-level", i.e. not a list member.
  AREXPORT static bool isResourceTopLevel(ArArgumentBuilder *arg, 
                                          const char *logPrefix = "");

  /// Returns the parent path contained in the resource arg for list members.
  AREXPORT static std::list<std::string> parseResourceParentPath(ArArgumentBuilder *arg,
                                                                 char separator = '|',
                                                                 const char *logPrefix = "");
  
  /// Returns the description contained in the given resource arg.
  AREXPORT static std::string parseResourceDescription(ArArgumentBuilder *arg, 
                                                       const char *logPrefix = "");
  /// Returns the extra explanation contained in the given resource arg.
  AREXPORT static std::string parseResourceExtra(ArArgumentBuilder *arg, 
                                                       const char *logPrefix = "");

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Instance Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Default empty contructor
  AREXPORT ArConfigArg();
  /// Constructor for making an integer argument by pointer (4 bytes)
  AREXPORT ArConfigArg(const char * name, int *pointer, 
		       const char * description = "", 
		       int minInt = INT_MIN, 
		       int maxInt = INT_MAX); 
  /// Constructor for making an int argument thats a short (2 bytes)
  AREXPORT ArConfigArg(const char * name, short *pointer, 
		       const char * description = "", 
		       int minInt = SHRT_MIN, 
		       int maxInt = SHRT_MAX); 
  /// Constructor for making an int argument thats a ushort (2 bytes)
  AREXPORT ArConfigArg(const char * name, unsigned short *pointer, 
		       const char * description = "", 
		       int minInt = 0, 
		       int maxInt = USHRT_MAX); 
  /// Constructor for making an char (1 byte) argument by pointer (treated as int)
  AREXPORT ArConfigArg(const char * name, unsigned char *pointer, 
		       const char * description = "", 
		       int minInt = 0,
		       int maxInt = 255); 
  /// Constructor for making a double argument by pointer
  AREXPORT ArConfigArg(const char * name, 
                       double *pointer,
		                   const char * description = "", 
		                   double minDouble = -HUGE_VAL,
		                   double maxDouble = HUGE_VAL,
                       int precision = DEFAULT_DOUBLE_PRECISION); 
  /// Constructor for making a boolean argument by pointer
  AREXPORT ArConfigArg(const char * name, bool *pointer,
		       const char * description = ""); 
  /// Constructor for making an argument of a string by pointer (see details)
  AREXPORT ArConfigArg(const char *name, char *str, 
		       const char *description,
		       size_t maxStrLen);
  /// Constructor for making an argument of a string by pointer (see details)
  AREXPORT ArConfigArg(const char *name, const char *str, 
		       const char *description);

  /// Constructor for making an argument of a C++ std::string 
  AREXPORT ArConfigArg(const char *name, std::string *str, const char *description);
  
  /// Constructor for making an integer argument
  AREXPORT ArConfigArg(const char * name, int val, 
		       const char * description = "", 
		       int minInt = INT_MIN, 
		       int maxInt = INT_MAX); 
  /// Constructor for making a double argument
  AREXPORT ArConfigArg(const char * name, 
                       double val,
		                   const char * description = "", 
		                   double minDouble = -HUGE_VAL,
		                   double maxDouble = HUGE_VAL,
                       int precision = DEFAULT_DOUBLE_PRECISION); 
  /// Constructor for making a boolean argument
  AREXPORT ArConfigArg(const char * name, bool val,
		       const char * description = ""); 
  /// Constructor for making an argument that has functors to handle things
  AREXPORT ArConfigArg(const char *name, 
		 ArRetFunctor1<bool, ArArgumentBuilder *> *setFunctor, 
		 ArRetFunctor<const std::list<ArArgumentBuilder *> *> *getFunctor,
		 const char *description);

  /// Constructor for just holding a description (for ArConfig)
  AREXPORT ArConfigArg(const char *str, Type type = DESCRIPTION_HOLDER);
  /// Constructor for holding an unknown argument (STRING_HOLDER)
  AREXPORT ArConfigArg(const char *name, const char *str);

  /// Constructs a new named argument of the specified type.
  AREXPORT ArConfigArg(Type type,
                       const char *name, 
		                   const char *description);

  /// Constructs a new argument of the specified type.
  AREXPORT ArConfigArg(Type type);

  /// Destructor
  AREXPORT virtual ~ArConfigArg();

  /// Copy constructor that allows a new name to be assigned
  AREXPORT ArConfigArg(const char *argName,
                       const ArConfigArg & arg);

  /// Copy constructor
  AREXPORT ArConfigArg(const ArConfigArg & arg);
  /// Assignment operator
  AREXPORT ArConfigArg &operator=(const ArConfigArg &arg);

  /// Copies the given arg to this one, detaching any pointers so they are not shared
  AREXPORT void copyAndDetach(const ArConfigArg &arg);


  /// Copies the translation data from given arg to this one.
  AREXPORT bool copyTranslation(const ArConfigArg &arg);

  /// Converts a list holder argument to an actual list and copies the children from arg.
  AREXPORT bool promoteList(const ArConfigArg &arg);
  
  /// Whether the arg type is LIST or LIST_HOLDER
  AREXPORT bool isListType() const;

  /// Gets the type of the argument
  AREXPORT ArConfigArg::Type getType(void) const;
  /// Gets the name of the argument
  AREXPORT const char *getName(void) const;
  /// Gets the brief description of the argument
  AREXPORT const char *getDescription(void) const;

  /// Sets the description (normally given in the constructor)
  AREXPORT void setDescription(const char *description);

  /// For arguments that require more than a brief description, set the extra explanation.
  AREXPORT void setExtraExplanation(const char *extraExplanation);

  /// Returns the extra explanation, if any, for this argument
  AREXPORT const char *getExtraExplanation() const;


  /// Sets the argument value, for int arguments
  AREXPORT bool setInt(int val, 
                       char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                       bool doNotSet = false);

  /// Sets the argument value, for double arguments
  AREXPORT bool setDouble(double val, 
                          char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                          bool doNotSet = false);

  /// Sets the argument value, for bool arguments
  AREXPORT bool setBool(bool val, 
                        char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                        bool doNotSet = false);

  /// Sets the argument value for string arguments
  AREXPORT bool setString(const char *str, 
                          char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                          bool doNotSet = false);

  AREXPORT bool setCppString(const std::string &str,
                          char *errorBuffer = NULL, size_t errorBufferLen = 0,
                          bool doNotSet = false);

  /// Sets the argument by calling the setFunctor callback
  AREXPORT bool setArgWithFunctor(ArArgumentBuilder *argument, 
				  char *errorBuffer = NULL,
				  size_t errorBufferLen = 0,
				  bool doNotSet = false);



  /// Gets the argument value, for int arguments
  AREXPORT int getInt(bool *ok = NULL) const; 

  /// Gets the minimum int value
  AREXPORT int getMinInt(bool *ok = NULL) const;
  /// Gets the maximum int value
  AREXPORT int getMaxInt(bool *ok = NULL) const;


  /// Gets the argument value, for double arguments
  AREXPORT double getDouble(bool *ok = NULL) const;
  /// Gets the minimum double value
  AREXPORT double getMinDouble(bool *ok = NULL) const;
  /// Gets the maximum double value
  AREXPORT double getMaxDouble(bool *ok = NULL) const;
  /// Gets the decimal precision of the double
  AREXPORT int getDoublePrecision(bool *ok = NULL) const;

  /// Gets the argument value, for bool arguments
  AREXPORT bool getBool(bool *ok = NULL) const;

  /// Gets the argument value, for string (and string holder) arguments
  AREXPORT const char *getString(bool *ok = NULL) const;

  /// Gets the argument value, which is a list of argumentbuilders here
  AREXPORT const std::list<ArArgumentBuilder *> *getArgsWithFunctor(bool *ok = NULL) const;

  AREXPORT std::string getCppString(bool *ok = NULL) const;

  AREXPORT const std::string* getCppStringPtr(bool *ok = NULL) const;

  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Methods for LIST Type 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Adds a child arg to this arg.  Valid only for LIST type; otherwise returns false.
  AREXPORT bool addArg(const ArConfigArg &arg); 

  /// Removes the child arg that has the same name as the specified one. Valid only for LIST type.
  AREXPORT bool removeArg(const ArConfigArg  &arg);

  /// Returns whether the list contains child args. Valid only for LIST type; otherwise returns false.
  AREXPORT bool hasArgs() const;

  /// Returns the number of child args. Valid only for LIST type; otherwise returns 0.
  AREXPORT size_t getArgCount() const;   

  /// Returns the total number of descendent args (children, grandchildren, etc). Valid only for LIST type; otherwise returns 0.
  AREXPORT size_t getDescendantArgCount() const;

  // KMC 7/9/12 Right now, the returned args will not have the parent set to this arg.
  // I suspect that this may present an implementation issue later but am not sure.
  // Perhaps the addition of an iterator would suffice.
  //
  /// Returns a list of all child args in this arg.  Valid only for LIST type; otherwise returns an empty list.
  AREXPORT std::list<ArConfigArg> getArgs(bool *ok = NULL) const;

  /// Returns the child arg at the specified index (between 0 and getArgCount()).
  AREXPORT const ArConfigArg *getArg(size_t index) const;

  /// Returns the child arg at the specified index (between 0 and getArgCount()).
  AREXPORT ArConfigArg *getArg(size_t index);

  /// Finds the specified child arg.  Valid only for LIST type; otherwise returns NULL.
  AREXPORT const ArConfigArg *findArg(const char *childParamName) const;

  /// Finds the specified child arg.  Valid only for LIST type; otherwise returns NULL.
  AREXPORT ArConfigArg *findArg(const char *childParamName);

  /// If the arg is a list member, returns all ancestors in order.
  AREXPORT bool getAncestorList(std::list<ArConfigArg*> *ancestorListOut);

  /// If the arg is a list member, returns the top-most arg. Otherwise, returns this.
  AREXPORT const ArConfigArg *getTopLevelArg() const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous Attributes
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  /// Gets the priority (only used by ArConfig)
  AREXPORT ArPriority::Priority getConfigPriority(void) const;
  /// Sets the priority (only used by ArConfig)
  AREXPORT void setConfigPriority(ArPriority::Priority priority);

  /// Returns the display hint for this arg, or NULL if none is defined.
  AREXPORT const char *getDisplayHint() const;
  /// Sets the display hint for this arg.
  AREXPORT void setDisplayHint(const char *hintText);

  /// Gets the restart level of this parameter (only used by ArConfig)
  AREXPORT RestartLevel getRestartLevel() const;
  /// Sets the restart level of this parameter (only used by ArConfig)
  AREXPORT void setRestartLevel(RestartLevel level);

  /// Gets if notifications of changes are suppressed (for the central
  /// server config)
  AREXPORT bool getSuppressChanges(void) const;
  /// Sets if notifications of changes are suppressed (for the central
  /// server config)
  AREXPORT void setSuppressChanges(bool suppressChanges);


  /// Returns whether the configuration parameter should be saved in the file (default is true).
  AREXPORT bool isSerializable() const;

  /// Sets whether the configuration parameter should be saved in the file (default is true).
  AREXPORT void setSerializable(bool isSerializable);


  /// Returns a pointer to the immediate parent arg.  If this is not a child ArConfigArg, then returns NULL.
  AREXPORT ArConfigArg *getParentArg() const;

  /// If getParentArg() is not null, then returns the path to the top level, as a single string
  AREXPORT std::string getParentPathName(char separator = '|') const;

  /// Given a parent path, as a single string, splits it in a format useable by ArConfigSection findParam
  AREXPORT static std::list<std::string> splitParentPathName(const char *parentPathName,
                                                             char separator = '|');


  /// Replaces spaces in the name with underscores
  AREXPORT void replaceSpacesInName(void);

  /// Returns whether the arg has a minimum value, currently applicable to INTs and DOUBLEs
  AREXPORT bool hasMinBound() const;

  /// Returns whether the arg has a maximum value, currently applicable to INTs and DOUBLEs
  AREXPORT bool hasMaxBound() const;

  /// Sets whether to ignore bounds or not (default is to not to)
  AREXPORT void setIgnoreBounds(bool ignoreBounds = false);

  /// Returns true if this arg points to a member of another object, false if arg is self-contained.
  AREXPORT bool hasExternalDataReference() const;

  /// Returns true if this is a special placeholder arg, i.e. string, list, or description.
  AREXPORT bool isPlaceholder() const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Generic (Type-Independent) Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File Parsing 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Adds given parserCB handler for the appropriate keyword(s) to the given file parser.
  AREXPORT bool addToFileParser(ArFileParser *parser,
                                ArRetFunctor3C<bool, 
                                               ArConfig, 
                                               ArArgumentBuilder *, 
                                               char *, 
                                               size_t> *parserCB,                              
                                const char *logPrefix = "",
                                bool isQuiet = false) const;



  /// Sets the value of this arg to the data described in the given ArArgumentBuilder
  AREXPORT bool parseArgument(ArArgumentBuilder *arg, 
				                      char *errorBuffer,
				                      size_t errorBufferLen,
                              const char *logPrefix = "",
                              bool isQuiet = false,
                              bool *changed = NULL);

  /// Writes this arg to the given file, in a format suitable for reading by parseArgument.
  AREXPORT bool writeArguments(FILE *file,
                               char *lineBuf,
                               int lineBufSize,
                               int startCommentColumn,
                               bool isWriteExtra = false,
                               const char *logPrefix = "",
                               int indentLevel = 0) const;

  /// Writes the name of this arg to the given buffer, indenting as specified.
  AREXPORT bool writeName(char *lineBuf,
                          int lineBufSize,
                          int indentLevel) const;

  /// Writes the min/max values of this arg to the given buffer, as applicable.
  AREXPORT bool writeBounds(char *line,
                            size_t lineLen,
                            const char *logPrefix = "") const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Sockets
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Determines whether the given arg text is empty or the special "None" identifier. 
  AREXPORT static bool isNone(const char *argText);

  /// Parses a new config arg definition from the given arguments (generally received on a text socket).
  /**
   * The given args are formatted according to the SocketIndices defined above.
  **/
  AREXPORT bool parseSocket(const ArArgumentBuilder &args,
                            char *errorBuffer,
                            size_t errorBufferLen);

  /// Writes the value of this parameter, and all child parameters, to the given text socket.
  /**
   *  This method is primarily intended for limited, internal use. The format of the 
   *  output string is:
   *     <i>intro</i> <i>paramName</i> <i>paramValue</i>
  **/
  AREXPORT bool writeValue(ArSocket *socket,
                           const char *intro) const;

  /// Writes the definition of this parameter, and all child parameters, to the given text socket.
  /**
   * This method is primarily intended for limited, internal use. The format of the 
   * output string is:
   *    <i>intro</i> <i>type</i> <i>paramName</i> <i>priority</i> <i>min</i> <i>max</i> <i>description</i> <i>displayHint</i> <i>listDelimiter</i>
  **/
  AREXPORT bool writeInfo(ArSocket *socket,
                          const char *intro) const;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Misc
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
  /// Logs the type, name, and value of this argument
  AREXPORT void log(bool verbose = false,
                    int indentCount = 1,
                    const char *logPrefix = "") const;

  /// Checks only the name, type, and value attributes and returns whether they are equal.
  AREXPORT bool isValueEqual(const ArConfigArg &other) const;
  
  /// If the given source is of the same type, copies its value to this arg
  /**
   * Note that this method currently only works for the primitive arg
   * types (i.e. int, bool, etc.).  It doesn't copy functors or description
   * holders.
   *
   * @param source the ArConfigArg whose value is to be copied to this arg
   * @param isVerifyArgNames a bool set to true if the argument value should
   * be set only if the given source has the same name as this argument;
   * the default value is false
   * @return bool true if the value was copied; false if the source was of a 
   * different (or non-copyable) type
  **/
  AREXPORT bool setValue(const ArConfigArg &source,
                         bool isVerifyArgNames = false);

  /// Gets whether this value has been set since it was last cleared or not
  bool isValueSet(void) { return myValueSet; }
  
  /// Tells the configArg that the value hasn't been set
  void clearValueSet(void) { myValueSet = false; }

  /// For special arg types (such as LIST), sets the flag to indicate the value was set.
  void setValueSet(void) { myValueSet = true; }


  /// Returns whether the arg has been translated by a resource file.
  AREXPORT bool isTranslated() const;
  
  /// Sets whether the arg has been translated by a resource file.
  AREXPORT void setTranslated(bool b);
  

  enum {
    MAX_RESOURCE_ARG_TEXT_LENGTH = 1024
  };

  /// Sets the value of this arg to the resource data described in the given ArArgumentBuilder
  AREXPORT bool parseResource(ArArgumentBuilder *arg, 
                              char *errorBuffer,
				                      size_t errorBufferLen,
                              const char *logPrefix = "",
                              bool isQuiet = false);



  /// Parses the text obtained from the resource arg, strips surrounding quotes, blanks, and funny chars.
  AREXPORT static bool parseResourceArgText(const char *argText,
                                            char *bufOut,
                                            size_t bufLen);

  /// Writes this arg to the given file, in a format suitable for reading by parseResource.
  AREXPORT bool writeResource(FILE *file,
                         char *lineBuf,
                         int lineBufSize,
                         char separatorChar,
                         const char *sectionName,
                         const char *logPrefix = "") const;
 

  /// Writes column labels and edit info to the given resource file, returns number of lines written
  AREXPORT static int writeResourceHeader(FILE *file,
                                          char *lineBuf,
                                          int lineBufSize,
                                          char separatorChar,
                                          const char *sectionTitle,
                                          const char *logPrefix = "");
  
  /// Writes the section description to the resource file.
  AREXPORT static int writeResourceSectionHeader(FILE *file,
                                                 char *lineBuf,
                                                 int lineBufSize,
                                                 char separatorChar,
                                                 const char *sectionName,
                                                 const char *sectionDesc,
                                                 const char *sectionExtra,
                                                 const char *sectionDisplayName,
                                                 bool isTranslated,
                                                 const char *logPrefix = "");

protected:

  // Giving ArConfig access to the write method
  friend class ArConfig;

  /// Writes the given comment to the specified file, spanning multiple lines as necessary.
  AREXPORT static bool writeMultiLineComment(const char *comment,
                                    FILE *file,
                                    char *lineBuf,
                                    int lineBufSize,
                                    const char *startComment);


private:

 enum IntType {
    INT_NOT, ///< Not an int
    INT_INT, ///< An int (4 bytes) 
    INT_SHORT, ///< A short (2 bytes)
    INT_UNSIGNED_SHORT, ///< An unsigned short (2 bytes)
    INT_UNSIGNED_CHAR ///< An unsigned char (1 byte)
  };

  /// Internal helper function
  void clear(bool initial, 
             Type type = INVALID, 
             IntType intType = INT_NOT,
             bool isDelete = true);

  /// Copies the given arg to this one, optionally detaching any internal pointer
  void copy(const ArConfigArg &arg, bool isDetach = false);
  
  void set(ArConfigArg::Type type,
           const char *name,
           const char *description,
           IntType intType = INT_NOT);

  /// Sets the parent pointer of this arg.
  void setParent(ArConfigArg *parentArg);


// KMC 7/11/12 Changed from protected to private so future changes are less
// of a concern
// protected:
private:

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to INT args
  struct ArConfigIntData {

    // int data
    ArConfigArg::IntType myIntType;

    // If unions need to be avoided for some reason, then this can simply 
    // be turned into a struct.
    //
    // begin union (based on myIntType)
    union {
      int            *myIntPointer;
      short          *myIntShortPointer;
      unsigned short *myIntUnsignedShortPointer;
      unsigned char  *myIntUnsignedCharPointer;
    };
    // end union (based on myIntType)

    int myMinInt;
    int myMaxInt;

  }; // end struct ArConfigIntData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to DOUBLE args
  struct ArConfigDoubleData {
   
    double *myDoublePointer;
    double  myMinDouble;
    double  myMaxDouble;
    int     myPrecision;

  }; // end struct ArConfigDoubleData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to BOOL args
  struct ArConfigBoolData {
    
    bool *myBoolPointer;

  }; // end struct ArConfigBoolData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to STRING and STRING_HOLDER args
  struct ArConfigStringData {

    // string data
    char *myStringPointer;
    size_t myMaxStrLen;
    // KMC own string change
    // Not seeing the functional difference between this and myOwnPointedTo 
    // (which was previously applied to all but strings)
    // bool myUsingOwnedString;
    std::string *myString; ///< Used if myOwnedString is true. (Union member cannot have a copy constructor so using a pointer)

  }; // end struct ArConfigStringData

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to CPPSTRING
  struct ArConfigCppStringData {
    std::string *myCppStringPtr;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to LIST args
  struct ArConfigListData {

    std::list<ArConfigArg> *myChildArgList;

  }; // end struct ArConfigListData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to FUNCTOR args
  struct ArConfigFunctorData {

    ArRetFunctor1<bool, ArArgumentBuilder *> *mySetFunctor;
    ArRetFunctor<const std::list<ArArgumentBuilder *> *> *myGetFunctor;

  }; // end struct ArConfigFunctorData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // If unions need to be avoided for some reason, then this can simply 
  // be turned into a struct.
  //
  /// Data that varies according to type of arg
  union ArConfigArgData { // begin union (based on myType)
    ArConfigIntData     myIntData;
    ArConfigDoubleData  myDoubleData;
    ArConfigBoolData    myBoolData;
    ArConfigStringData  myStringData;
    ArConfigListData    myListData;
    ArConfigFunctorData myFunctorData;
    ArConfigCppStringData myCppStringData;
  }; // end union (based on myType)
  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Number of spaces to indent each level of list contents
  static int ourIndentSpaceCount;

  static std::map<std::string, Type, ArStrCaseCmpOp> *ourTextToTypeMap;
  static std::map<std::string, RestartLevel, ArStrCaseCmpOp> *ourTextToRestartLevelMap;

  /// Type of this arg
  ArConfigArg::Type myType;
  /// Name of this arg (may be empty for SEPARATOR args)
  std::string myName;
  /// Brief description of this arg
  std::string myDescription;
  /// An optional extra explanation for args that need clarification
  std::string myExtraExplanation;

  /// Name to be displayed in client applications. (Not yet supported.)
  std::string myDisplayName;
  
  ArConfigArgData myData;

  /// Priority of this arg
  ArPriority::Priority myConfigPriority;
  /// Optional display hint used by clients
  std::string myDisplayHint;
  /// Indicates whether modifying the arg will result in a system restart
  RestartLevel myRestartLevel;

  /// Pointer to the parent arg, or NULL if this arg is not a member of a LIST 
  ArConfigArg *myParentArg;

  /// Whether this arg "owns" its data, i.e. does not point to a member of another object
  bool myOwnPointedTo : 1;
  /// Whether this arg has been set to its initial value
  bool myValueSet     : 1;
  /// Whether any min/max bounds requirements on this arg should be ignored
  bool myIgnoreBounds : 1;
  /// Whether this arg has been translated by a resource file
  bool myIsTranslated : 1;
  /// Supressses change notification (for central server config params that should cause a restart on a robot, but not on the central server)
  bool mySuppressChanges : 1;
  /// Whether this arg should be written to the configuration file
  bool myIsSerializable : 1;

}; // end class ArConfigArg

#endif // ARCONFIGARG_H
