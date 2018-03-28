
#ifndef ARTESTCONFIG_H
#define ARTESTCONFIG_H

struct ArConfigPrimitiveData;

struct ArConfigDisplayHintData;


#include <ArConfig.h>
#include <ArConfigArg.h>

class ArTestConfig
{
public:
  
	ArTestConfig(ArConfig *config,
                const char *configSectionName);

	virtual ~ArTestConfig();

protected:

  bool addToConfig(ArConfig *config,
                   const char *configSectionName);

private:

  ArConfigPrimitiveData *myTopLevelConfigPrimitives;

  ArConfigPrimitiveData *myListMemberPrimitives;

  ArConfigDisplayHintData *myDisplayHintData;

}; // end class ArTestConfig



struct ArConfigPrimitiveData {

  int myIntInt;
  int myIntIntWithMin;
  int myIntIntWithMax;
  int myIntIntWithMinMax;

  short myIntShort;
  short myIntShortWithMin;
  short myIntShortWithMax;
  short myIntShortWithMinMax;

  unsigned short myIntUShort;
  unsigned short myIntUShortWithMin;
  unsigned short myIntUShortWithMax;
  unsigned short myIntUShortWithMinMax;

  unsigned char myIntUChar;
  unsigned char myIntUCharWithMin;
  unsigned char myIntUCharWithMax;
  unsigned char myIntUCharWithMinMax;
    
  double myDouble;
  double myDoubleWithMin;
  double myDoubleWithMax;
  double myDoubleWithMinAndMax;

  bool myBool;

  char myStringBuf[256];

  std::list<ArConfigArg> myArgList;
  
  ArConfigPrimitiveData();

}; // end struct ArConfigPrimitiveData

ArConfigPrimitiveData::ArConfigPrimitiveData() :
  myIntInt(1),
  myIntIntWithMin(2),
  myIntIntWithMax(3),
  myIntIntWithMinMax(4),

  myIntShort(5),
  myIntShortWithMin(6),
  myIntShortWithMax(7),
  myIntShortWithMinMax(8),

  myIntUShort(9),
  myIntUShortWithMin(10),
  myIntUShortWithMax(11),
  myIntUShortWithMinMax(12),

  myIntUChar(13),
  myIntUCharWithMin(12),
  myIntUCharWithMax(11),
  myIntUCharWithMinMax(10),
    
  myDouble(17.1),
  myDoubleWithMin(18.2),
  myDoubleWithMax(19.3),
  myDoubleWithMinAndMax(20.4),

  myBool(false),

  myStringBuf(),

  myArgList()

{
  myStringBuf[0] = '\0';

  /// Constructor for making an integer argument by pointer (4 bytes)

  myArgList.push_back(ArConfigArg("PointerIntInt", 
                                  &myIntInt,
                                  "Pointer to an INT_INT, no bounds")); 

  myArgList.push_back(ArConfigArg("PointerIntIntWithMin", 
                                  &myIntIntWithMin,
                                  "Pointer to an INT_INT, min bound",
                                  1)); 

  myArgList.push_back(ArConfigArg("PointerIntIntWithMax", 
                                  &myIntIntWithMax,
                                  "Pointer to an INT_INT, max bounds",
                                  INT_MIN,
                                  9999)); 

  myArgList.push_back(ArConfigArg("PointerIntIntWithMinMax", 
                                  &myIntIntWithMinMax,
                                  "Pointer to an INT_INT, min and max bounds",
                                  -1,
                                  9999)); 


  /// Constructor for making an int argument thats a short (2 bytes)
  //ArConfigArg(const char * name, short *pointer, 
  //            const char * description = "", 
	//            int minInt = SHRT_MIN, 
	//            int maxInt = SHRT_MAX); 

  myArgList.push_back(ArConfigArg("PointerIntShort", 
                                  &myIntShort,
                                  "Pointer to an INT_SHORT, no bounds")); 

  myArgList.push_back(ArConfigArg("PointerIntShortWithMin", 
                                  &myIntShortWithMin,
                                  "Pointer to an INT_SHORT, min bound",
                                  1)); 

  myArgList.push_back(ArConfigArg("PointerIntShortWithMax", 
                                  &myIntShortWithMax,
                                  "Pointer to an INT_SHORT, max bounds",
                                  SHRT_MIN,
                                  9999)); 

  myArgList.push_back(ArConfigArg("PointerIntShortWithMinMax", 
                                  &myIntShortWithMinMax,
                                  "Pointer to an INT_SHORT, min and max bounds",
                                  -1,
                                  9999)); 



  /// Constructor for making an int argument thats a ushort (2 bytes)
  // ArConfigArg(const char * name, unsigned short *pointer, 
	//	           const char * description = "", 
	//	           int minInt = 0, 
	//	           int maxInt = USHRT_MAX);


  myArgList.push_back(ArConfigArg("PointerIntUShort", 
                                  &myIntUShort,
                                  "Pointer to an INT_USHORT, no bounds")); 
  myArgList.back().setRestartLevel(ArConfigArg::NO_RESTART);

  myArgList.push_back(ArConfigArg("PointerIntUShortWithMin", 
                                  &myIntUShortWithMin,
                                  "Pointer to an INT_USHORT, min bound",
                                  1)); 
  myArgList.back().setRestartLevel(ArConfigArg::RESTART_CLIENT);

  myArgList.push_back(ArConfigArg("PointerIntUShortWithMax", 
                                  &myIntUShortWithMax,
                                  "Pointer to an INT_USHORT, max bounds",
                                  SHRT_MIN,
                                  9999)); 
  myArgList.back().setRestartLevel(ArConfigArg::RESTART_SOFTWARE);

  myArgList.push_back(ArConfigArg("PointerIntUShortWithMinMax", 
                                  &myIntUShortWithMinMax,
                                  "Pointer to an INT_USHORT, min and max bounds",
                                  1,
                                  9999)); 
  myArgList.back().setRestartLevel(ArConfigArg::RESTART_HARDWARE);


  ///// Constructor for making an char (1 byte) argument by pointer (treated as int)
  // ArConfigArg(const char * name, unsigned char *pointer, 
  //             const char * description = "", 
  //             int minInt = 0,
  //             int maxInt = 255); 

  myArgList.push_back(ArConfigArg("PointerIntUChar", 
                                  &myIntUChar,
                                  "Pointer to an INT_UCHAR, no bounds")); 

  myArgList.push_back(ArConfigArg("PointerIntUCharWithMin", 
                                  &myIntUCharWithMin,
                                  "Pointer to an INT_UCHAR, min bound",
                                  1)); 

  myArgList.push_back(ArConfigArg("PointerIntUCharWithMax", 
                                  &myIntUCharWithMax,
                                  "Pointer to an INT_UCHAR, max bounds",
                                  0,
                                  11)); 

  myArgList.push_back(ArConfigArg("PointerIntUCharWithMinMax", 
                                  &myIntUCharWithMinMax,
                                  "Pointer to an INT_UCHAR, min and max bounds",
                                  1,
                                  11)); 




  /// Constructor for making a double argument by pointer
  //  ArConfigArg(const char * name, double *pointer,
	//              const char * description = "", 
	//              double minDouble = -HUGE_VAL,
	//              double maxDouble = HUGE_VAL); 

  myArgList.push_back(ArConfigArg("PointerDouble", 
                                  &myDouble,
                                  "Pointer to a DOUBLE, no bounds")); 

  myArgList.push_back(ArConfigArg("PointerDoubleWithMin", 
                                  &myDoubleWithMin,
                                  "Pointer to a DOUBLE, min bound and a precision set to 2 plus a really, really long description that should require word wrapping",
                                  -1.2,
                                  HUGE_VAL,
                                  2)); 
  myArgList.back().setExtraExplanation("A bit more information regarding the double with min.");

  myArgList.push_back(ArConfigArg("PointerDoubleWithMax", 
                                  &myDoubleWithMax,
                                  "Pointer to a DOUBLE, max bounds",
                                  -HUGE_VAL,
                                  9999.8)); 
  myArgList.back().setExtraExplanation("A bit more information regarding the double with max.");

  myArgList.push_back(ArConfigArg("PointerDoubleWithMinMax", 
                                  &myDoubleWithMinAndMax,
                                  "Pointer to a DOUBLE, min and max bounds",
                                  -1.2,
                                  9999.8)); 


  /// Constructor for making a boolean argument by pointer

  //ArConfigArg(const char * name, bool *pointer,
	//            const char * description = ""); 

  myArgList.push_back(ArConfigArg("PointerBool", 
                                  &myBool,
                                  "Pointer to a BOOL")); 


  /// Constructor for making an argument of a string by pointer (see details)
  // ArConfigArg(const char *name, char *str, 
  //             const char *description,
  //             size_t maxStrLen);

  myArgList.push_back(ArConfigArg("PointerString", 
                                  myStringBuf, 
                                  "Pointer to a STRING",
                                  sizeof(myStringBuf)));


  myArgList.push_back(ArConfigArg(ArConfigArg::SEPARATOR));

  myArgList.push_back(ArConfigArg("ValueString", 
                                  "InputText", 
                                  "A STRING value",
                                  0));

  /// Constructor for making an argument of a string by pointer (see details)
  //  ArConfigArg(const char *name, const char *str, 
  //              const char *description);
  myArgList.push_back(ArConfigArg("ValueStringNoMaxLength", 
                                  "InputTextNoMaxLength", 
                                  "A STRING value, no max string length"));

  /// Constructor for making an integer argument
  //  ArConfigArg(const char * name, 
  //              int val, 
	//              const char * description = "", 
	//              int minInt = INT_MIN, 
	//              int maxInt = INT_MAX);
  myArgList.push_back(ArConfigArg("ValueIntInt", 
                                  1,
                                  "An INT_INT value, no bounds")); 

  /// Constructor for making a double argument
  // ArConfigArg(const char * name, 
  //             double val,
	//             const char * description = "", 
	//             double minDouble = -HUGE_VAL,
	//             double maxDouble = HUGE_VAL);
  myArgList.push_back(ArConfigArg("ValueDouble", 
                                  2.3,
                                  "A DOUBLE value, no bounds")); 

  /// Constructor for making a boolean argument
  // ArConfigArg(const char * name, 
  //             bool val,
	//             const char * description = "");
  myArgList.push_back(ArConfigArg("ValueBool", 
                                  false,
                                  "A BOOL value")); 

  /// Constructor for making an argument that has functors to handle things
  // ArConfigArg(const char *name, 
  //             ArRetFunctor1<bool, ArArgumentBuilder *> *setFunctor, 
  //             ArRetFunctor<const std::list<ArArgumentBuilder *> *> *getFunctor,
  //             const char *description);

  /// Constructor for just holding a description (for ArConfig)
  // ArConfigArg(const char *str, Type type = DESCRIPTION_HOLDER);
  myArgList.push_back(ArConfigArg("DescriptionHolder")); 
  myArgList.push_back(ArConfigArg("Description Holder With Blanks and a really long description that might need to employ truncation or word wrapping upon display")); 

  /// Constructor for holding an unknown argument (STRING_HOLDER)
  // ArConfigArg(const char *name, const char *str);
  myArgList.push_back(ArConfigArg("StringHolderName", 
                                  "StringHolderText")); 
  myArgList.push_back(ArConfigArg("String Holder Name With Blanks", 
                                  "String Holder Text With Blanks"));

  /// Constructs a new named argument of the specified type.
  //AREXPORT ArConfigArg(Type type,
  //                     const char *name, 
	//	                   const char *description);

  myArgList.push_back(ArConfigArg(ArConfigArg::LIST,
                                  "EmptyList", 
		                              "List with no children"));

  /// Constructs a new argument of the specified type.
  // ArConfigArg(Type type);
  myArgList.push_back(ArConfigArg(ArConfigArg::SEPARATOR));


} // end method addToConfig

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct ArConfigDisplayHintData
{
 
  std::list<ArConfigArg> myArgList;
  
  ArConfigDisplayHintData();

}; // end struct ArConfigDisplayHintData

ArConfigDisplayHintData::ArConfigDisplayHintData() :
  myArgList()
{

  // STRING
  // <code>Choices:</code><i>choice1</i>(<code>;;</code><i>choice#i</i>)*
  myArgList.push_back(ArConfigArg("PrimaryColor", 
                                  "Red", 
                                  "Choices display hint, for a primary color"));
  myArgList.back().setDisplayHint("Choices:Red;;Green;;Blue");
 
  // <code>MapItem:</code><i>type</i><code>|</code>(<code>Parent</code>|<code>SubType</code>|<code>SubTypeAndParent</code>|<code>SubTypeGlob</code>)<code>|</code>(<code>Optional</code>|<code>Required</code>)
  myArgList.push_back(ArConfigArg("Goals", 
                                  "", 
                                  "MapItem display hint, for goals"));
  myArgList.back().setDisplayHint("MapItem:Goal");


  // <code>ChildObject:</code><i>type</i><code>|</code>(<code>Parent</code>|<code>SubType</code>|<code>SubTypeAndParent</code>|<code>SubTypeGlob</code>)<code>|</code>(<code>Optional</code>|<code>Required</code>)
  // TODO

  // <code>Macro:</code><i>taskClass1</i>(<code>;;</code><i>taskClass#i</i>)*|(<code>Optional</code>|<code>Required</code>)
  // TODO

  // <code>Macro</code>
  myArgList.push_back(ArConfigArg("Macros", 
                                  "", 
                                  "Macro display hint, no task classes"));
  myArgList.back().setDisplayHint("Macro");

  // <code>Time:</code><i>format</i>
  // TODO

  // <code>Time</code>
  myArgList.push_back(ArConfigArg("PlainTime", 
                                  "", 
                                  "Time display hint, no formatting"));
  myArgList.back().setDisplayHint("Time");


  // <code>RobotFile:</code><i>fileFilterNames</i><code>|</code><i>fileFilters</i>
  // KMC Not sure that wil work
  myArgList.push_back(ArConfigArg("RobotFile", 
                                  "", 
                                  "RobotFile display hint, no filters"));
  myArgList.back().setDisplayHint("RobotFile");

  // INT
  // <code>Color</code>
  myArgList.push_back(ArConfigArg("Color", 
                                  0xAAAAAA, 
                                  "Color display hint, for chooser"));
  myArgList.back().setDisplayHint("Color");

  // BOOL
  // <code>Checkbox</code>
  myArgList.push_back(ArConfigArg("PlainCheckbox", 
                                  false, 
                                  "Checkbox display hint (plain)"));
  myArgList.back().setDisplayHint("Checkbox");

  
  // VISIBLE =
  // <code>Visible:</code><i>OtherParamName=OtherParamValue</i>

  // Creation of variable lists of parameters (a set of lasers, some parameters
  // depending on the selected laser type)

  // Data regarding a single laser unit.
  //
  ArConfigArg laserUnitInfo(ArConfigArg::LIST,
                           "LaserUnitInfo",
                           "Data for a laser on the robot.");

  ArConfigArg laserNumberArg("LaserNumber", 
                             0,
                             "ID of laser unit"); // ?? TODO Redundant with laserUnit name
  laserNumberArg.setDisplayHint("Editable:false");

  laserUnitInfo.addArg(laserNumberArg);

  laserUnitInfo.addArg(ArConfigArg("LaserX", 
                                    0,
                                   "X position of laser on robot, measured from center of rotation (mm)"));
  laserUnitInfo.addArg(ArConfigArg("LaserY", 
                                    0,
                                   "Y position of laser on robot, measured from center of rotation (mm)"));
  laserUnitInfo.addArg(ArConfigArg("LaserZ", 
                                    0,
                                   "Z position of laser on robot, measured from floor (mm)"));

  ArConfigArg laserTypeArg("LaserType",
                           "",
                           "Type of the laser unit",
                           0);
  laserTypeArg.setDisplayHint("Choices:LMS1xxx;;LMS2xxx;;URG");

  laserUnitInfo.addArg(laserTypeArg);  


  // Data that is displayed only when LaserType is set to lms2xxx
  //
  ArConfigArg lms2xxInfo(ArConfigArg::LIST,
                         "LMS2xxInfo",
                         "Data for an LMS2xxx laser");
  lms2xxInfo.setDisplayHint("Visible:LaserType=lms2xxx");

  lms2xxInfo.addArg(ArConfigArg("LaserMaxRange", 
                                32000,
                                "Maximum range of the laser (mm)"));
  lms2xxInfo.addArg(ArConfigArg("LaserCumulativeBufSize", 
                                200,
                                "Needs a description"));
  
  laserUnitInfo.addArg(lms2xxInfo);
		 

  // Placeholders for all possible laser units on the robot
  //
  ArConfigArg laserUnitCount("LaserCount", 
                             1,
                             "Number of lasers on the robot", 
                             0, 
                             3);
  laserUnitCount.setDisplayHint("SpinBox");
  myArgList.push_back(laserUnitCount);


  // VISIBLE >
  // <code>Visible:</code><i>OtherParamName>OtherParamValue</i>

  int maxCount = 3;
  char nameBuf[256];
  char displayHintBuf[256];

  for (int i = 1; i <= maxCount; i++) {

    snprintf(nameBuf, sizeof(nameBuf), "LaserUnit%i", i);  
    nameBuf[sizeof(nameBuf) - 1] = '\0';

    ArConfigArg curLaserUnit(nameBuf, laserUnitInfo);
    
    // TODO Set laserNumber too?
    ArConfigArg *laserNumberArg = curLaserUnit.findArg("LaserNumber");
    if (laserNumberArg != NULL) {
      laserNumberArg->setInt(i); 
    }
    // This info is visible only when the laserUnitCount is big enough
    //
    snprintf(displayHintBuf, sizeof(displayHintBuf), 
             "Visible:LaserCount>%i", i - 1);
    displayHintBuf[sizeof(displayHintBuf) - 1] = '\0';

    curLaserUnit.setDisplayHint(displayHintBuf);

    myArgList.push_back(curLaserUnit);
  }


  // <code>Visible:</code><i>OtherParamName!=OtherParamValue</i>  
  // <code>Visible:</code><code>false</code>

  // <code>Editable:</code><i>OtherParamName=OtherParamValue</i>
  // <code>Editable:</code><i>OtherParamName!=OtherParamValue</i>  
  // <code>Editable:</code><i>OtherParamName>OtherParamValue</i>
  // <code>Editable:</code><code>false</code>



} // end ctor




// --------------------------------------------------------------------------

// Simple parameter that is editable only when a feature is enabled
//
//config.addParam("MicroCycleDockEnabled");
//config.addParam("MicroCycleDockStateOfCharge", ...
//	          "Editable:MicroCycleDockEnabled=true")  // DisplayHint
//


// -------------------------------------------------------------------------





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


ArTestConfig::ArTestConfig(ArConfig *config,
						                 const char *configSectionName) :
  myTopLevelConfigPrimitives(new ArConfigPrimitiveData()),
  myListMemberPrimitives(new ArConfigPrimitiveData()),
  myDisplayHintData(new ArConfigDisplayHintData())
{
  // KMC Was thinking that we may someday want flags to control these things
  // so left them in the body.
 
  addToConfig(config, configSectionName);
 

} // end ctor

ArTestConfig::~ArTestConfig()
{
  delete myTopLevelConfigPrimitives;
	delete myListMemberPrimitives;
  delete myDisplayHintData;

}
  
bool ArTestConfig::addToConfig(ArConfig *config,
                                const char *configSectionName)
{
 
  if (config == NULL) {
    return false;
  }

  bool isSuccess = true;

  if (myTopLevelConfigPrimitives != NULL) {
    
    std::string fullSectionName;
    if (configSectionName != NULL) {
      fullSectionName = configSectionName;
    }
    fullSectionName += "Primitives";
  
    for (std::list<ArConfigArg>::iterator iter = 
                      myTopLevelConfigPrimitives->myArgList.begin();
         (iter != myTopLevelConfigPrimitives->myArgList.end());
         iter++) {

           if ((*iter).getRestartLevel() != ArConfigArg::NO_RESTART) {
             ArLog::log(ArLog::Normal,
               " %s", (*iter).getName());
           }
      if (!config->addParam(*iter, 
                            fullSectionName.c_str(),
                            ArPriority::NORMAL, 
                            (*iter).getDisplayHint(),
                            (*iter).getRestartLevel())) {
        isSuccess = false;
      }

    } // end for each top level arg 

  } // end if 

  if (myListMemberPrimitives != NULL) {

    std::string fullSectionName;
    if (configSectionName != NULL) {
      fullSectionName = configSectionName;
    }
    fullSectionName += "ListMembers";

    ArConfigArg listArg(ArConfigArg::LIST,
                        "PrimitiveList",
                        "List that contains all of the primitives");

    for (std::list<ArConfigArg>::iterator iter = 
                      myListMemberPrimitives->myArgList.begin();
         (iter != myListMemberPrimitives->myArgList.end());
         iter++) {

      // KMC This is temp just because the separator was the first item in 
      // the list.
      if ((*iter).getType() == ArConfigArg::SEPARATOR) {
        continue;
      }
      if (!listArg.addArg(*iter)) {

         // Not necessarily failure, cannot add pointers ... 
 
      }

    } // end for each list member arg 

    if (!config->addParam(listArg, 
                           fullSectionName.c_str(),
                           ArPriority::NORMAL)) {
       isSuccess = false;
    }

  } // end if list member primitives


  if (myDisplayHintData != NULL) {

    std::string fullSectionName;
    if (configSectionName != NULL) {
      fullSectionName = configSectionName;
    }
    fullSectionName += "DisplayHints";
  
    for (std::list<ArConfigArg>::iterator iter = 
                      myDisplayHintData->myArgList.begin();
         (iter != myDisplayHintData->myArgList.end());
         iter++) {

      if (!config->addParam(*iter, 
                            fullSectionName.c_str(),
                            ArPriority::NORMAL, 
                            (*iter).getDisplayHint())) {
        isSuccess = false;
      }

    } // end for each top level arg 
  } // end if display hint data
  

  {
    std::string fullSectionName;
    if (configSectionName != NULL) {
      fullSectionName = configSectionName;
    }
    fullSectionName += "RestartTypes";

    config->addParam(ArConfigArg("RestartClientBool", 
				 false,
				 "A BOOL to flip to cause a client restart request to be sent"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_CLIENT);

    config->addParam(ArConfigArg("RestartServerBool", 
				 false,
				 "A BOOL to flip to cause the software to restart"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_SOFTWARE);

    config->addParam(ArConfigArg("RestartServerInt", 
				 5,
				 "An INT to flip to cause the software to restart"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_SOFTWARE);

    config->addParam(ArConfigArg("RestartServerDouble", 
				 5.0,
				 "A DOUBLE to flip to cause the software to restart"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_SOFTWARE);

    config->addParam(ArConfigArg("RestartRobotBool", 
				 false,
				 "A BOOL to flip to cause the complete robot to restart (really this just turns it off right now)"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_HARDWARE);

    config->addParam(ArConfigArg("RestartServerBool2", 
				 false,
				 "A BOOL to flip to cause the software to restart"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_SOFTWARE);

    config->addParam(ArConfigArg("RestartClientBool2", 
				 false,
				 "A BOOL to flip to cause a client restart request to be sent"),
		     fullSectionName.c_str(), 
		     ArPriority::NORMAL, "", ArConfigArg::RESTART_CLIENT);


  }

  return isSuccess;

} // end method addToConfig

#endif // ARTESTCONFIG_H
