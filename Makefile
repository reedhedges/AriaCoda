
include Makefile.inc

majorlibver=3
minorlibver=0.0

ifndef JAVAC
ifdef JAVA_BIN
JAVAC:=$(JAVA_BIN)/javac
else
JAVAC:=javac
endif #ifdef JAVA_BIN
endif #ifndef JAVAC

ifndef JAR
ifdef JAVA_BIN
JAR:=$(JAVA_BIN)/jar
else
JAR:=jar
endif #ifdef JAVA_BIN
endif #ifndef JAR

ifndef JAVA_INCLUDE
JAVA_INCLUDE:=/usr/lib/jvm/default-java/include
endif

ifndef CSHARP_COMPILER
CSHARP_COMPILER:=mcs
endif

# Default targets to build in the default rule:
ifeq ($(host),MINGW32)
TARGETS:=lib/libAria.a 
  #examples/demo$(binsuffix)
else
TARGETS:=lib/libAria.$(sosuffix) 
  #examples/demo$(binsuffix)
endif

# Default static libraries and examples:
STATIC_TARGETS:=lib/libAria.a 
#examples/demoStatic$(binsuffix)

# Lots of targets, to build in the everything rule:
ALL_TARGETS:=lib/libAria.$(sosuffix) utils examples tests docs params swig $(STATIC_TARGETS)

CFILES:= \
	ArAction.cpp \
	ArActionAvoidFront.cpp \
	ArActionAvoidSide.cpp \
	ArActionBumpers.cpp \
	ArActionConstantVelocity.cpp \
	ArActionDeceleratingLimiter.cpp \
	ArActionDesired.cpp \
	ArActionDriveDistance.cpp \
	ArActionGoto.cpp \
	ArActionGotoStraight.cpp \
	ArActionGroup.cpp \
	ArActionGroups.cpp \
	ArActionInput.cpp \
	ArActionIRs.cpp \
	ArActionJoydrive.cpp \
	ArActionKeydrive.cpp \
	ArActionLimiterBackwards.cpp \
	ArActionLimiterForwards.cpp \
	ArActionLimiterRot.cpp \
	ArActionLimiterTableSensor.cpp \
	ArActionMovementParameters.cpp \
	ArActionMovementParametersDebugging.cpp \
	ArActionRatioInput.cpp \
	ArActionRobotJoydrive.cpp \
	ArActionStallRecover.cpp \
	ArActionStop.cpp \
	ArActionTriangleDriveTo.cpp \
	ArActionTurn.cpp \
	ArAnalogGyro.cpp \
	ArArg.cpp \
	ArArgumentBuilder.cpp \
	ArArgumentParser.cpp \
	ArASyncTask.cpp \
	ArBasePacket.cpp \
	ArBatteryConnector.cpp \
	ArBatteryMTX.cpp \
	ArBumpers.cpp \
	ArCondition_LIN.cpp \
	ArConfig.cpp \
	ArConfigArg.cpp \
	ArConfigGroup.cpp \
	ArDataLogger.cpp \
	ArDeviceConnection.cpp \
	ArDPPTU.cpp \
	ArFileDeviceConnection.cpp \
	ArFileParser.cpp \
	ArForbiddenRangeDevice.cpp \
	ArFunctorASyncTask.cpp \
	ArGPS.cpp \
	ArGPSConnector.cpp \
	ArGPSCoords.cpp \
	ArGripper.cpp \
	ArInterpolation.cpp \
	ArIRs.cpp \
	ArJoyHandler.cpp \
	ArKeyHandler.cpp \
	ArLaser.cpp \
	ArLaserConnector.cpp \
	ArLaserFilter.cpp \
	ArLaserLogger.cpp \
	ArLCDConnector.cpp \
	ArLCDMTX.cpp \
	ArLineFinder.cpp \
	ArLMS1XX.cpp \
	ArLMS2xx.cpp \
	ArLMS2xxPacket.cpp \
	ArLMS2xxPacketReceiver.cpp \
	ArLog.cpp \
	ArMap.cpp \
	ArMapComponents.cpp \
	ArMapInterface.cpp \
	ArMapObject.cpp \
	ArMapUtils.cpp \
	ArMD5Calculator.cpp \
	ArMutex.cpp \
	ArMutex_LIN.cpp \
	ArNMEAParser.cpp \
	ArNovatelGPS.cpp \
  ArPacketUtil.cpp \
	ArPriorityResolver.cpp \
	ArPTZ.cpp \
	ArPTZConnector.cpp \
	ArRangeBuffer.cpp \
	ArRangeDevice.cpp \
	ArRangeDeviceThreaded.cpp \
	ArRatioInputKeydrive.cpp \
	ArRatioInputJoydrive.cpp \
	ArRatioInputRobotJoydrive.cpp \
	ArRecurrentTask.cpp \
	ArRobot.cpp \
	ArRobotBatteryPacketReader.cpp \
	ArRobotConfigPacketReader.cpp \
	ArRobotConnector.cpp \
	ArRobotJoyHandler.cpp \
	ArRobotPacket.cpp \
	ArRobotPacketReceiver.cpp \
	ArRobotPacketReaderThread.cpp \
	ArRobotPacketSender.cpp \
	ArRobotParams.cpp \
	ArRobotTypes.cpp \
	ArRVisionPTZ.cpp \
	ArS3Series.cpp \
	ArSZSeries.cpp \
  ArSeekurIMU.cpp \
	ArSimulatedLaser.cpp \
	ArSocket.cpp \
	ArSonarConnector.cpp \
	ArSonarDevice.cpp \
	ArSonarMTX.cpp \
	ArSensorReading.cpp \
	ArSoundsQueue.cpp \
	ArSoundPlayer.cpp \
	ArStringInfoGroup.cpp \
	ArSyncLoop.cpp \
	ArSyncTask.cpp \
	ArSonarAutoDisabler.cpp \
	ArTcpConnection.cpp \
	ArThread.cpp \
	ArThread_LIN.cpp \
	ArTransform.cpp \
	ArTrimbleGPS.cpp \
	ArUrg.cpp \
	ArUrg_2_0.cpp \
	ArVCC4.cpp \
	Aria.cpp \
	ariaUtil.cpp \
	md5.cpp

# Files with platform-specific code:
ifeq ($(host),MINGW32)
	CFILES+=ArSocket_WIN.cpp \
		ArJoyHandler_WIN.cpp \
		ArSerialConnection_WIN.cpp \
		ArSignalHandler_WIN.cpp
else
  CFILES+=ArSocket_LIN.cpp \
		ArJoyHandler_LIN.cpp \
		ArSerialConnection_LIN.cpp \
		ArSignalHandler_LIN.cpp \
		ArSystemStatus.cpp 
  ifneq ($(host),Darwin)
		# Very Linux-specific
    CFILES+=ArMTXIO.cpp
  endif
endif



OTFILES:=$(patsubst %.$(CFILEEXT),%.o,$(CFILES))
OFILES:=$(patsubst %,obj/%,$(OTFILES))
EXAMPLES_CPP:=$(shell find examples -name "*.$(CFILEEXT)" | grep -v Mod.cpp | grep -v proprietary)
EXAMPLES:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(EXAMPLES_CPP))
EXAMPLES_STATIC:=$(patsubst %,%Static$(binsuffix),$(EXAMPLES))
MOD_EXAMPLES_CPP:=$(shell find examples -name "*.$(CFILEEXT)" | grep Mod.cpp)
MOD_EXAMPLES:=$(patsubst %.$(CFILEEXT),%.$(sosuffix),$(MOD_EXAMPLES_CPP))
TESTS_CPP:=$(shell find tests -name "*.$(CFILEEXT)" | grep -v Mod.cpp | grep -v proprietary | grep -v ^tests/Ar)
MOD_TESTS_CPP:=$(shell find tests -name "*Mod.$(CFILEEXT)")
MOD_TESTS:=$(patsubst %.$(CFILEEXT),%.$(sosuffix),$(MOD_TESTS_CPP))
TESTS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(TESTS_CPP))
TESTS_STATIC:=$(patsubst %,%Static$(binsuffix),$(TESTS))
#UTILS_CPP:=$(shell find utils -name "*.$(CFILEEXT)")
#UTILS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(UTILS_CPP))
SRC_FILES:=$(patsubst %,src/%,$(CFILES))
HEADER_FILES:=$(shell find include -type f -name \*.h)


# Metadata for documentation, installers, and providing version info in Aria.cpp
DATESTAMP=$(shell date +%Y-%m-%d)
DATE=$(shell date +'%B %d, %Y')
GITCOMMIT=$(shell git log --pretty=reference -n 1  | cut -f 1 -d ' ')
GITLONGCOMMIT=$(shell git log --pretty=oneline -n 1 | cut -f 1 -d ' ')
GIT_NUM_MODIFIED=$(shell git ls-files -m | wc -l)
ifeq ($(GIT_NUM_MODIFIED),0)
ARIACODA_VERSION_STRING=$(GITLONGCOMMIT)
else
#ARIACODA_VERSION_STRING="$(GITLONGCOMMIT) with $(GIT_NUM_MODIFIED) modified files"
ARIACODA_VERSION_STRING="$(GITLONGCOMMIT)+changes"
endif


# Default Rule
all: dirs $(TARGETS)

debug: FORCE
	$(MAKE) all DEBUG=1

# Build all targets, docs, params, etc. etc.
everything: dirs $(ALL_TARGETS) 

static: dirs $(STATIC_TARGETS)

dist-all: $(DIST_TARGETS)

examples: $(EXAMPLES) 

tests: $(TESTS) $(MOD_TESTS)

utils: 
	$(MAKE) -C utils

cleanDep:
	-rm Makefile.dep 

# Rules to generate API documentation for local/personal use (rather than part of a
# released package; that uses the packaging shell scripts instead)

ifndef DOXYGEN_CONF
DOXYGEN_CONF=doxygen.conf
endif

ifndef DOXYGEN_OUTDIR
DOXYGEN_OUTDIR=docs
endif

docs: doc
doc: $(DOXYGEN_OUTDIR)/index.html
$(DOXYGEN_OUTDIR)/index.html: $(SRC_FILES) $(HEADER_FILES) $(EXAMPLES_CPP) docs/overview.dox docs/options/all_options.dox docs/params.dox $(DOXYGEN_CONF)
	@echo
	@echo Removing old documentation...
	$(MAKE) cleanDoc
	@echo
	@echo Making new local documentation...
	sed -e 's/PROJECT_NUMBER.*/PROJECT_NUMBER = $(GITCOMMIT)_$(DATESTAMP)/' -e 's/OUTPUT_DIRECTORY.*/OUTPUT_DIRECTORY = $(DOXYGEN_OUTDIR)/' < $(DOXYGEN_CONF) >doxygen.conf.tmp
	doxygen doxygen.conf.tmp
	rm doxygen.conf.tmp
	@echo
	@echo Done autogenerating ARIA API documentation. Open docs/index.html in a web browser.
	@echo Use \"make cleanDocs\" to remove autogenerated documentation.

help:
	@echo To build the library plus 'demo' example, run \'make\' or \'make all\'
	@echo Some useful targets include: 
	@echo "  clean"
	@echo "  dep"
	@echo "  cleanDep"
	@echo "  docs"
	@echo "  cleanAll (also cleans docs, java, python, etc.)"
	@echo "  examples"
	@echo "  tests"
	@echo "  utils"
	@echo "  python" 
	@echo "  cleanPython"
	@echo "  java" 
	@echo "  cleanJava"
	@echo "  examples/<Some Example>, where examples/<Some Example>.cpp is an example program source file."
	@echo "  tests/<Some Test>, where tests/<Some Test>.cpp is a test program source file."
	@echo "  install"
	@echo "  debian"
	@echo "  clang-tidy"
	@echo "  clang-tidy-headers"
	@echo "  cppcheck"
	@echo "  cppcheck-headers"
	@echo "  tags"
	@echo "Any markdown file (.md) can be converted to RTF, text, PDF or HTML with rules with rtf, txt, pdf, html file suffixes (e.g. 'make README.html'). The 'pandoc' tool is used."
	@echo 
	@echo "Set EXTRA_CXXFLAGS to add any additional C++ compilation flags you want (e.g. optimization or profiling flags)."
	@echo "Set CXXWARNFLAGS to override default set of warning flags.  ($(CXXWARNFLAGS)). (Use EXTRA_CXXFLAGS to add more warnings in addition to the defaults.)"
	@echo "Set DEBUG to use -Og as default for optimization flags instead of -O3, and maybe other debugging friendly compilation flags. (Note, not setting DEBUG does not define the NDEBUG preprocessor symbol [NDEBUG disables assert(), possibly other effects, depending on system library or compiler.] Define NDEBUG manually in EXTRA_CXXFLAGS or CXXOPTFLAGS if desired.)"
	@echo "Set CXXOPTFLAGS to override default optimization flags (-O3 unless DEBUG is set, then -Og)."
	@echo "Set CXXDEBUGFLAGS to override default debug flags (-g)."
	@echo "Set EXTRA_CXXFLAGS_<File> to add additional compilation flag added only when compiling <File>.cpp. E.g. EXTRA_CXXFLAGS_ArRobot"
	@echo "Set EXTRA_LFLAGS to add additional link flags (not used for static link of the library)"
	@echo "Set CXXSTD to select C++ standard (i.e. value passed to -std).  Default is $(CXXSTD)"
	@echo "Set CXX to override C++ compiler command.  E.g. \"clang++\", \"ccache c++\", etc.  Default is $(CXX)."
	@echo "Set AR to override ar static linker command."
	@echo "Set DESTDIR for installation in a temporary directory (eg for packaging or testing)."

info:
	@echo ARIA=$(ARIA)
	@echo CXX=$(CXX)
	@echo CXXSTD=$(CXXSTD)
	@echo DEBUG=$(DEBUG)
	@echo CXXDEBUGFLAGS=$(CXXDEBUGFLAGS)
	@echo CXXOPTFLAGS=$(CXXOPTFLAGS)
	@echo CXXWARNFLAGS=$(CXXWARNFLAGS)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo BARECXXFLAGS=$(BARECXXFLAGS)
	@echo EXTRA_CXXFLAGS=$(EXTRA_CXXFLAGS)
	@echo CXXINC=$(CXXINC)
	@echo ARIA_CXXLINK=$(ARIA_CXXLINK)
	@echo CXXLINK=$(CXXLINK)
	@echo ARIA_CXXSTATICLINK=$(ARIA_CXXSTATICLINK)
	@echo CXXSTATICLINK=$(CXXSTATICLINK)
	@echo ALL_TARGETS=$(ALL_TARGETS)
	@echo
	@echo JAVAC=$(JAVAC)
	@echo JAR=$(JAR)
	@echo
	@echo Use \'make moreinfo\' for info about individual files, etc.

moreinfo:
	@echo CFILES=$(CFILES)
	@echo
	@echo EXAMPLES=$(EXAMPLES)
	@echo
	@echo MOD_EXAMPLES=$(MOD_EXAMPLES)
	@echo
	@echo TESTS=$(TESTS)
	@echo
	@echo MOD_TESTS=$(MOD_TESTS)
	@echo
	@echo ADVANCED=$(ADVANCED)
	@echo
	@echo UTILS=$(UTILS)
	@echo
	@echo SRC_FILES=$(SRC_FILES)
	@echo
	@echo HEADER_FILES=$(HEADER_FILES)



markdown_format:=markdown #+multiline_tables #+simple_tables
%.rtf: %.md
	pandoc -s --toc --template pandoc_template -f $(markdown_format) -t rtf -o $@ $<

%.txt: %.md
	pandoc -s --toc -f $(markdown_format) -t plain -o $@ $<

%.pdf: %.md
	pandoc -s --toc -f $(markdown_format) -o $@ $<

%.html: %.md
	pandoc -s --toc -f $(markdown_format) -o $@ $<


clean: cleanUtils cleanExamples cleanTests 
	-rm -f lib/libAria.a lib/libAria.$(sosuffix) lib/libAria.$(sosuffix).$(majorlibver) $(OFILES) 

cleanUtils:
	-rm -f $(UTILS)

cleanExamples:
	-rm -f $(EXAMPLES) $(EXAMPLES_STATIC)

cleanTests:
	-rm -f $(TESTS) $(TESTS_STATIC)

cleanDoc:
	-rm -r $(DOXYGEN_OUTDIR)/*.html $(DOXYGEN_OUTDIR)/*.png $(DOXYGEN_OUTDIR)/doxygen.css $(DOXYGEN_OUTDIR)/*.js

cleanDocs: cleanDoc

cleandoc: cleanDoc

cleandocs: cleanDoc

cleanPython:
	-rm python/_AriaPy.$(sosuffix)
	-rm python/AriaPy.py
	-rm python/AriaPy.pyc
	-rm python/AriaPy_wrap.cpp
	-rm python/AriaPy_wrap.h
	-rm obj/AriaPy_wrap.o

cleanpython: cleanPython

# Force rebuild of Makefile.dep:
dep: cleanDep clean
	$(MAKE) Makefile.dep

Makefile.dep:
	if [ -f `echo src/*.cpp | cut -d' ' -f1` ]; then \
    $(CXX) $(CXXFLAGS) $(CXXINC) -MM src/*.cpp | \
    awk '$$1 ~ /:/{printf "obj/%s\n", $$0} $$1 !~ /:/' > Makefile.dep; fi
	if [ -f `echo examples/*.cpp | cut -d' ' -f1` ]; then \
    $(CXX) $(CXXFLAGS) $(CXXINC) -MM examples/*.cpp | \
    awk '$$1 ~ /:/{printf "examples/%s\n", $$0} $$1 !~ /:/' | \
    sed 's/\.o//' >> Makefile.dep; fi
	if [ -f `echo tests/*.cpp | cut -d' ' -f1` ]; then \
    $(CXX) $(CXXFLAGS) $(CXXINC) -MM tests/*.cpp | \
    awk '$$1 ~ /:/{printf "tests/%s\n", $$0} $$1 !~ /:/' | \
    sed 's/\.o//' >> Makefile.dep; fi

#	if [ -f `echo utils/*.cpp | cut -d' ' -f1` ]; then \
#    $(CXX) $(CXXFLAGS) $(CXXINC) -MM utils/*.cpp | \
#    awk '$$1 ~ /:/{printf "utils/%s\n", $$0} $$1 !~ /:/' | \
#    sed 's/\.o//' >> Makefile.dep; fi

cleanAll: clean cleanJava cleanPython cleanDocs
	rm -f lib/lib*.$(sosuffix)*
	rm -f obj/*.o*



params: utils/makeParams$(binsuffix)
	-mkdir params
	ARIA=. LD_LIBRARY_PATH=lib:$$LD_LIBRARY_PATH utils/makeParams$(binsuffix)


# These files are generated by running a utility that outputs command line
# option information from various ARIA classes.
CommandLineOptions.txt docs/options/all_options.dox: utils/genCommandLineOptionDocs$(binsuffix) 
	-mkdir docs/options
	LD_LIBRARY_PATH=lib:$$LD_LIBRARY_PATH utils/genCommandLineOptionDocs$(binsuffix)

distclean: clean cleanUtils cleanExamples cleanDep
	-rm python/\*.pyc python/\*.$(sosuffix)

####
#### Swig wrappers
####

ifndef SWIG
SWIG=swig
endif

### Python wrapper: ###


ifdef PYTHON_INCLUDE
PYTHON_INCLUDE_FLAGS=-I$(PYTHON_INCLUDE)
else
ifdef DIST_INSTALL
$(warning WARNING using default value of /usr/include/python2.7 for PYTHON_INCLUDE directory. Set PYTHON_INCLUDE environment variable if you would like to use a different version of the Python C development library to build the Python ARIA wrapper.)
endif
PYTHON_INCLUDE:=/usr/include/python2.7
PYTHON_INCLUDE_FLAGS=-I/usr/include/python2.7
endif

python: python/_AriaPy.$(sosuffix) python/AriaPy.py

py: python

python-doc: python/AriaPy.html

python/AriaPy.html: python/AriaPy.py
	cd python; pydoc -w AriaPy

# When running swig, -DARIA_WRAPPER omits non-public API that is not needed.
# -DAREXPORT forces a definition of the AREXPORT macro to empty. The SWIG
# preprocessor symbol is also automatically defined by swig which omits things
# specifically from being seen by swig (usually things swig can't deal with
# automatically)

python/AriaPy_wrap.cpp python/AriaPy.py: include/Aria/wrapper.i include/Aria/*.h python/DiscoverWiBox.py
	#-rm -f `find python -maxdepth 1 -xtype f -name "*Aria*" | grep -v .ds | grep -v .sln | grep -v .vcproj`
	cd python; $(SWIG) -Wall -c++ -python -modern -module AriaPy -DARIA_WRAPPER -Dlinux -DAREXPORT -o AriaPy_wrap.cpp -I../include ../include/Aria/wrapper.i 

python/_AriaPy.$(sosuffix): obj/AriaPy_wrap.o lib/libAria.$(sosuffix)
	if ! test -f $(PYTHON_INCLUDE)/Python.h; then echo "Error: $(PYTHON_INCLUDE)/Python.h not found. Is the Python development package (python2.7-dev) installed on this system?"; exit 1; fi
	$(CXX) -O3 -shared -o $(@) obj/AriaPy_wrap.o -Llib -lAria -lpthread -ldl -lrt

obj/AriaPy_wrap.o: python/AriaPy_wrap.cpp
	mkdir -p obj
	@ if test -z "$(PYTHON_INCLUDE)"; then echo "*** Error: PYTHON_INCLUDE is not set, cannot build ARIA python wrapper"; fi
	if ! test -f $(PYTHON_INCLUDE)/Python.h; then echo "Error: $(PYTHON_INCLUDE)/Python.h not found. Is the Python development package (python2.7-dev) installed on this system? Is PYTHON_INCLUDE set correctly?"; exit 1; fi
	$(CXX) -O3 -c $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER $(PYTHON_INCLUDE_FLAGS) $< -o $@


### Java Wrapper: ###



java:  lib/libAriaJava.$(sosuffix) java/Aria.jar

java/Aria.jar: java/com/mobilerobots/Aria/ArRobot.class
	cd java; $(JAR) cf Aria.jar com/mobilerobots/Aria/*.class

cleanJava:
	-rm -r java/com/mobilerobots/Aria
	-rm java/Aria.jar
	-rm lib/libAriaJava.$(sosuffix)
	-rm java/AriaJava_wrap.cpp
	-rm java/AriaJava_wrap.h
	-rm obj/AriaJava_wrap.o

cleanjava: cleanJava

cleanSwigJava:
	-rm -r java/Aria.jar java/com/mobilerobots/Aria

lib/libAriaJava.$(sosuffix): obj/AriaJava_wrap.o lib/libAria.$(sosuffix) 
	$(CXX) -O3 -fPIC -shared -o $(@) obj/AriaJava_wrap.o -lpthread -ldl -lrt -Llib -lAria

obj/AriaJava_wrap.o: java/AriaJava_wrap.cpp
	@ if test -z "$(JAVA_INCLUDE)"; then echo "Error: JAVA_INCLUDE is not set, compiling AriaJava_wrap.cpp will fail!"; fi
	@ test -n "$(JAVA_INCLUDE)"
	@ if test \! -d "$(JAVA_INCLUDE)"; then echo "Error: JAVA_INCLUDE directory $(JAVA_INCLUDE) does not exist, compiling AriaJava_wrap.cpp will fail!"; fi
	@ test -d "$(JAVA_INCLUDE)" 
	mkdir -p obj
	$(CXX) -O3 -c $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER -I$(JAVA_INCLUDE) -I$(JAVA_INCLUDE)/linux $< -o $@

# When running swig, -DARIA_WRAPPER omits non-public API that is not needed.
# -DAREXPORT forces a definition of the AREXPORT macro to empty. The SWIG
# preprocessor symbol is also automatically defined by swig which omits things
# specifically from being seen by swig (usually things swig can't deal with
# automatically)

java/AriaJava_wrap.cpp java/com/mobilerobots/Aria/ArRobot.java: include/Aria/wrapper.i $(HEADER_FILES)
	-mkdir -p java/com/mobilerobots/Aria; 
	-rm java/com/mobilerobots/Aria/*.java java/AriaJava_wrap.cpp java/AriaJava_wrap.h
	$(SWIG) -Wall -c++ -java -package com.mobilerobots.Aria -outdir java/com/mobilerobots/Aria -module AriaJava -DARIA_WRAPPER -Dlinux -DAREXPORT -o java/AriaJava_wrap.cpp -Iinclude include/Aria/wrapper.i 
    
# The sed script is a hack to let subclasses outside of Aria work for ArFunctor;
# for some reason SWIG doesn't use my %typmeap(javabody) for ArFunctor. 
#    \
#	  && sed 's/protected static long getCPtr/public ArFunctor() { this(0, false); }     public static long getCPtr/' java/com/mobilerobots/Aria/ArFunctor.java > ArFunctor.java.tmp  \
#		&& mv ArFunctor.java.tmp java/com/mobilerobots/Aria/ArFunctor.java

# It is much faster to compile them all at once. Use ArRobot.java/class as a
# stand-in for all java files and all class files.
java/com/mobilerobots/Aria/ArRobot.class: java/com/mobilerobots/Aria/ArRobot.java
		rm java/com/mobilerobots/Aria/*.class; $(JAVAC) -classpath java java/com/mobilerobots/Aria/*.java 


csharp: csharp/libAriaCS.$(sosuffix) csharp/AriaCS.dll

csharp/libAriaCS.$(sosuffix): csharp/AriaCS_wrap.cpp lib/libAria.$(sosuffix)
	$(CXX) -O3 $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER -fPIC -shared -o $@ csharp/AriaCS_wrap.cpp -lpthread -ldl -lrt -Llib -lAria

# When running swig, -DARIA_WRAPPER omits non-public API that is not needed.
# -DAREXPORT forces a definition of the AREXPORT macro to empty. The SWIG
# preprocessor symbol is also automatically defined by swig which omits things
# specifically from being seen by swig (usually things swig can't deal with
# automatically)

csharp/AriaCS_wrap.cpp csharp/ArRobot.cs: include/Aria/wrapper.i $(HEADER_FILES)
	-mkdir -p csharp
	-rm csharp/*.cs
	$(SWIG) -Wall -c++ -csharp -namespace AriaCS -outdir csharp -module AriaCS -DARIA_WRAPPER -Dlinux -DAREXPORT -o csharp/AriaCS_wrap.cpp -Iinclude include/Aria/wrapper.i

csharp/AriaCS.dll: csharp/ArRobot.cs
	$(CSHARP_COMPILER) -nologo -out:$@ -target:library csharp/*.cs

cleanCSharp:
	-rm csharp/AriaCS.dll
	-rm csharp/libAriaCS.$(sosuffix)
	-rm csharp/AriaCS_wrap.cpp
	-rm csharp/AriaCS_wrap.h
	-rm csharp/*.cs

####
#### Targets to actually build binaries (libraries, programs)
####

lib/libAria.$(sosuffix): $(OFILES) 
	-mkdir lib
	$(CXX) -shared $(ARIA_LINKFLAGS) -o $(@) $(OFILES) $(ARIA_CXXLINK) $(EXTRA_LINKFLAGS)
	ln -sf libAria.$(sosuffix) lib/libAria.$(sosuffix).$(majorlibver)

lib/libAria.a: $(OFILES) 
	-mkdir lib
	$(AR) -cr $(@) $(OFILES)
	$(RANLIB) $(@)

examples/%.$(sosuffix): examples/%.$(CFILEEXT) lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) -shared $< -o $@

examples/%$(binsuffix): examples/%.$(CFILEEXT) lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

examples/%Static$(binsuffix): examples/%.$(CFILEEXT) lib/libAria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	if test -z "$$NOSTRIP"; then strip $@; fi

tests/%.$(sosuffix): tests/%.$(CFILEEXT) lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) -shared $< -o $@

tests/%$(binsuffix): tests/%.$(CFILEEXT) lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

tests/%Static$(binsuffix): tests/%.$(CFILEEXT) lib/libAria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	if test -z "$$NOSTRIP"; then strip $@; fi

utils/%$(binsuffix): FORCE
	$(MAKE) -C utils $*

#utils/%Static$(binsuffix): utils/%.$(CFILEEXT) lib/libAria.a 
#	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
#	#strip $@

#utils/convertBitmapToArMap$(binsuffix): utils/convertBitmapToArMap.$(CFILEEXT) lib/libAria.$(sosuffix)
#	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ -lnetpbm $(CXXLINK)

# This utility prints information about command line options recognized by
# various classes in ARIA (e.g. ArRobotConnector, ArLaserConnector (for various
# laser types), etc.
#utils/genCommandLineOptionDocs$(binsuffix): utils/genCommandLineOptionDocs.cpp lib/libAria.$(sosuffix) 
#	$(CXX) $(CXXFLAGS) -DFOR_ARIA $(CXXINC) $< -o $@ $(CXXLINK)

ifneq ($(binsuffix),)
examples/%: examples/%$(binsuffix)
utils/%: utils/%$(binsuffix)
tests/%: tests/$(binsuffix)
endif

obj/%.o : src/%.cpp 
	@mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS_$(*F)) $(CXXINC) -DARIABUILD $< -o $@

obj/%.o : src/%.c 
	@mjdir -p obj
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS_$(*F)) $(CXXINC) -DARIABUILD $< -o $@

obj/ArPacketUtil.o: src/ArPacketUtil.cpp
	$(CXX) -c $(BARECXXFLAGS) $(EXTRA_CXXFLAGS)  -fexceptions $(EXTRA_CXXFLAGS_$(*F)) $(CXXINC) -DARIABUILD $< -o $@

obj/Aria.o: src/Aria.cpp versionstring
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS_$(*F)) $(CXXINC) -DARIABUILD -DARIA_VCSREV=\"$(ARIACODA_VERSION_STRING)\" $< -o $@

versionstring: FORCE
	if test -f versionstring; then oldver="`cat versionstring`"; else oldver=""; fi;  newver=$(ARIACODA_VERSION_STRING); if test "$$oldver" != "$$newver"; then echo $$newver > versionstring; fi

include/Aria/%.i: include/Aria/%.h 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) -DARIABUILD $< -o $@

src/%.i: src/%.cpp 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) -DARIABUILD $< -o $@

# Don't build .o files if their library is up to date with respect to source files:
.INTERMEDIATE: $(OFILES)

# But don't delete .o files if we do make them in order to make a library:
.PRECIOUS: $(OFILES)


# This rule doesn't actually produce its target, so anything depending on it
# will always get remade.
FORCE:


tags: $(SRC_FILES) $(HEADER_FILES)
	ctags $(SRC_FILES) $(HEADER_FILES)

ctags: tags

CLANG_TIDY_CHECKS:=-fuchsia-*,-google-*,-zircon-*,-android-*,-abseil-*,-altera-*,-boost-*,-linuxkernel-*,-llvm-*,-llvmlibc-*

clang-tidy: FORCE
	clang-tidy --header-filter=include/Aria/.* $(SRC_FILES) $(HEADER_FILES) --checks='$(CLANG_TIDY_CHECKS)' -- -x c++ $(CXXFLAGS) -DARIABUILD $(CXXINC)


# clang-tidy but disable some warnings that are probably ok but happen a lot:
clang-tidy-less: FORCE
	clang-tidy --checks='-clang-analyzer-valist.Uninitialized,-clang-analyzer-optin.cplusplus.VirtualCall,$(CLANG_TIDY_CHECKS)' --header-filter=include/Aria/.* $(SRC_FILES) $(HEADER_FILES) -- -x c++ $(CXXFLAGS) -DARIABUILD $(CXXINC)

clang-tidy-%: include/Aria/%.h src/%.cpp
	clang-tidy --header-filter=$< $^ --checks='$(CLANG_TIDY_CHECKS)' -- -x c++ $(CXXFLAGS) -DARIABUILD $(CXXINC)

clang-tidy-%: include/Aria/%.h
	clang-tidy --header-filter=$< $^ --checks='$(CLANG_TIDY_CHECKS)' -- -x c++ $(CXXFLAGS) -DARIABUILD $(CXXINC)

clang-tidy-headers: FORCE
	clang-tidy --header-filter=include/Aria/.* $(HEADER_FILES) --checks='$(CLANG_TIDY_CHECKS)' -- -x c++ $(CXXFLAGS) $(CXXINC) 

clang-tidy-examples: FORCE
	clang-tidy --header-filter=include/Aria/.* $(EXAMPLES_CPP) --checks='$(CLANG_TIDY_CHECKS)' -- -x c++ $(CXXFLAGS) $(CXXINC) 

cppcheck: FORCE
	cppcheck --enable=all --language=c++ --std=$(CXXSTD) $(CXXINC) -j 4 -DAREXPORT $(SOURCE_FILES) $(HEADER_FILES)

cppcheck-headers: FORCE
	cppcheck --enable=all --language=c++ --std=$(CXXSTD) $(CXXINC) -j 4 -DAREXPORT $(HEADER_FILES)

cppclean: FORCE
	cppclean --include-path=include $(SOURCE_FILES) $(HEADER_FILES)

cppclean-headers: FORCE
	cppclean --include-path=include $(HEADER_FILES)




# If DESTDIR is set, do Debian/Ubuntu system installation in that location.
# Otherwise, install in /usr/local

ifneq ($(DESTDIR),)
  libdir=$(DESTDIR)/usr/lib
  bindir=$(DESTDIR)/usr/bin
  docdir=$(DESTDIR)/usr/share/doc
  sharedir=$(DESTDIR)/usr/share
  ariadir=/usr/share/Aria
  incdir=$(DESTDIR)/usr/include
  etcdir=$(DESTDIR)/etc
  examplesdir=$(DESTDIR)/usr/share/doc/Aria/examples
  ldconfig=
else
  libdir=/usr/local/lib
  bindir=/usr/local/bin
  docdir=/usr/local/share/doc
  sharedir=/usr/local/share
  incdir=/usr/local/include
  etcdir=/etc
  ariadir=/usr/local/share/Aria
  examplesdir=/usr/local/share/doc/Aria/examples
  ldconfig=ldconfig
endif

install: install-default install-utils install-doc 
	@echo 
	@echo libAria.so* shared libraries have been installed in $(libdir).
	@echo libAria.a static library has been installed in $(libdir).
	@echo Header files have been installed in $(incdir)/Aria.
	@echo Documentation has been installed in $(docdir)/Aria.
	@echo Robot parameter files have been installed in $(sharedir)/Aria/params
	@echo Utility programs \(ariaDemo, seekurPower, mtxPower\) have been installed in $(bindir).	
	@echo The file $(etcdir)/Aria has been created to help Aria find $(ariadir).
	@echo 
	@echo Use 'make uninstall' to delete installed files.

install-default: lib/libAria.$(sosuffix) lib/libAria.a params README.md CommandLineOptions.txt $(HEADER_FILES) 
	install -D -m 644 lib/libAria.$(sosuffix) $(libdir)/libAria.$(sosuffix).$(majorlibver).$(minorlibver)
	ln -sf libAria.$(sosuffix).$(majorlibver).$(minorlibver) $(libdir)/libAria.$(sosuffix).$(majorlibver)
	ln -sf libAria.$(sosuffix).$(majorlibver).$(minorlibver) $(libdir)/libAria.$(sosuffix)
	install -D -m 644 lib/libAria.a $(libdir)/libAria.a
	install -D -m 644 -t $(docdir)/Aria README.md CommandLineOptions.txt
	install -D -m 644 -t $(sharedir)/Aria/params params/*
	install -D -m 644 -t $(incdir)/Aria $(HEADER_FILES)
	mkdir -p $(etcdir)
	echo $(ariadir) > $(etcdir)/Aria || echo Warning unable to store location of ARIA shared resources in $(etcdir)/Aria. ARIA will use built in default search order.
	$(ldconfig)

uninstall: FORCE
	rm $(libdir)/libAria.$(sosuffix).$(majorlibver).$(minorlibver)
	rm $(libdir)/libAria.$(sosuffix).$(majorlibver)
	rm $(libdir)/libAria.$(sosuffix)
	rm $(libdir)/libAria.a
	rm -rd $(docdir)/Aria
	rm -rd $(sharedir)/Aria
	rm $(etcdir)/Aria
	rm -r $(incdir)/Aria
	rm $(bindir)/ariaDemo $(bindir)/seekurPower $(bindir)/mtxPower

install-utils: examples/demo examples/seekurPower examples/mtxPower
	install -D -m 755 -s examples/demo $(bindir)/ariaDemo
	install -D -m 755 -s examples/seekurPower $(bindir)/
	install -D -m 755 -s examples/mtxPower $(bindir)/

install-doc: doc
	find docs -type f -and -not -name .\* -exec install -D -m 644 \{\} $(docdir)/Aria/\{\} \;

install-docs: install-doc

install-java: java

install-python: python

install-examples:
	install -D -m 644 -t $(examplesdir) $(EXAMPLES_CPP) examples/README.txt examples/*.wav

debian-changelog-add-versionstring: versionstring
	debchange --append `cat versionstring`

debian-release: debian-changelog-add-versionstring
	debchange --release --force-save-on-release
	debchange --edit
	fakeroot dh binary
	debchange -D UNRELEASED --increment

debian-test: debian-changelog-add-versionstring
	debchange --edit
	fakeroot dh binary

deb: debian FORCE

debian: FORCE
	@echo Use debian-test or debian-release



# Make optimization, tell it what rules aren't files:
.PHONY: all everything examples modExamples tests utils cleanDep docs doc dirs help info moreinfo clean cleanUtils cleanExamples cleanTests cleanDoc cleanPython dep params python python-doc java cleanJava params swig help info moreinfo py python-doc cleanSwigJava dirs install  distclean ctags csharp cleanCSharp cleanAll tidy cppclean cppcheck clang-tidy debug deb debian debian-test debian-release debian-changelog-add-versionstring

# Include Autogenerated dependencies, using Makefile.dep rule above to generate
# this file if needed:
include Makefile.dep

