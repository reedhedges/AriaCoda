
ifndef CXX
CXX:=g++
endif

CFILEEXT:=cpp
# BARECXXFLAGS is used when compiling python and java wrapper code
# CXXFLAGS is used when compiling everything else (libraries, static libraries,
# examples), and adds -fno-exceptions
BARECXXFLAGS=-g -Wall -D_REENTRANT  #-pg -fprofile-arcs
CXXFLAGS+=$(BARECXXFLAGS) -fno-exceptions $(EXTRA_CXXFLAGS)
CXXINC:=-Iinclude # used in all targets
CXXLINK=-Llib -lAria $(ARIA_CXXLINK) # used for examples, tests etc.
CXXSTATICLINK=-Llib -Xlinker -Bstatic -lAria -Xlinker -Bdynamic $(ARIA_CXXSTATICLINK)
# ARIA_CXXLINK and ARIA_CXXSTATICLINK are defined in next section

host:=$(shell uname | cut -d _ -f 1)
ifeq ($(host),MINGW32)
  $(info Building on MinGW)
  #CXXFLAGS+=-mwindows -mms-bitfields -D__MINGW__ -DMINGW
  BARECXXFLAGS+=-mms-bitfields -D__MINGW__ -DMINGW
  ARIA_CXXLINK=-lpthreadGC2 -lwinmm -lws2_32 -lstdc++
  ARIA_CXXSTATICLINK=-Wl,-Bstatic -lpthread -Wl,-Bdynamic -lwinmm -lws2_32 -lstdc++
  binsuffix:=.exe
else
  BARECXXFLAGS+=-fPIC
  ARIA_CXXLINK=-lpthread -ldl 
  binsuffix:=
  ifeq ($(host),Darwin)
    $(info Bulding on MacOSX (Darwin))
    CXXFLAGS+=-DMACOSX
    ARIA_CXXSTATICLINK+=-lpthread -ldl -lrt -lstdc++
    sosuffix:=dylib
  else
    ARIA_CXXLINK+=-lrt
    ARIA_CXXSTATICLINK+=-Xlinker -Bdynamic -lpthread -ldl -lrt -Xlinker -Bstatic -lstdc++ -Xlinker -Bdynamic
    sosuffix:=so
  endif
endif


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
TARGETS:=lib/libAria.$(sosuffix) examples/demo$(binsuffix)

# Default static libraries and examples:
STATIC_TARGETS:=lib/libAria.a examples/demoStatic$(binsuffix)

# Lots of targets, to build in the everything rule:
ALL_TARGETS:=lib/libAria.$(sosuffix) utils examples tests docs params swig $(STATIC_TARGETS)

DIST_TARGETS:=lib/libAria.$(sosuffix) params docs CommandLineOptions.txt \
  examples/demo \
  examples/demoStatic examples/seekurPower examples/mtxPower

ifndef NO_DIST_WRAPPERS
DIST_TARGETS:=$(DIST_TARGETS) java python
endif

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
	ArRobotConfig.cpp \
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
UTILS_CPP:=$(shell find utils -name "*.$(CFILEEXT)")
UTILS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(UTILS_CPP))
SRC_FILES:=$(patsubst %,src/%,$(CFILES))
HEADER_FILES:=$(shell find include -type f -name \*.h)




# Default Rule
all: dirs $(TARGETS)

# Build all targets, docs, params, etc. etc.
everything: dirs $(ALL_TARGETS) 

static: dirs $(STATIC_TARGETS)

dist-all: $(DIST_TARGETS)

examples: $(EXAMPLES) 

tests: $(TESTS) $(MOD_TESTS)

utils: $(UTILS)

cleanDep:
	-rm Makefile.dep `find . -name Makefile.dep`

# Rules to generate API documentation for local/personal use (rather than part of a
# released package; that uses the packaging shell scripts instead)
DATESTAMP=$(shell date +%Y%m%d)
DATE=$(shell date +'%B %d, %Y')
docs: doc
doc: docs/index.html
docs/index.html: $(SRC_FILES) $(HEADER_FILES) $(EXAMPLES_CPP) docs/overview.dox docs/options/all_options.dox docs/params.dox doxygen.conf
	@echo
	@echo Removing old documentation...
	$(MAKE) cleanDoc
	@echo
	@echo Making new local documentation...
	doxygen doxygen.conf
	@echo
	@echo Done autogenerating ARIA API documentation. Open docs/index.html in a web browser.
	@echo Use \"make cleanDocs\" to remove autogenerated documentation.

help:
	@echo To make most things, run \'make\' or \'make all\'
	@echo Some useful targets include: 
	@echo "  allLibs, cleanAllLibs, depAllLibs (do all subdirectories)"
	@echo "  clean"
	@echo "  dep"
	@echo "  cleanDep"
	@echo "  docs"
	@echo "  cleanAll (also cleans java, python, etc.)"
	@echo "  examples"
	@echo "  tests"
	@echo "  utils"
	@echo "  python" 
	@echo "  cleanPython"
	@echo "  java" 
	@echo "  cleanJava"
	@echo "  examples/<Some Example>, where examples/<Some Example>.cpp is an example program source file."
	@echo "  tests/<Some Test>, where tests/<Some Test>.cpp is a test program source file."
	@echo "  allLibs (try to build all auxilliary libraries that you have installed by running make in each directory starting with \"Ar\")"
	@echo "  cleanAllLibs, depAllLibs (do make clean or make dep in the \"Ar*\" auxilliary libraries)"
	@echo "  install (if this is a source tar.gz package)"

info:
	@echo ARIA=$(ARIA)
	@echo CXX=$(CXX)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo BARECXXFLAGS=$(BARECXXFLAGS)
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
	-rm -f lib/libAria.a lib/libAria.$(sosuffix) $(OFILES) `find . -name core` `find . -name '*~'` obj/AriaPy.o obj/AriaJava.o

cleanUtils:
	-rm -f $(UTILS)

cleanExamples:
	-rm -f $(EXAMPLES) $(EXAMPLES_STATIC)

cleanTests:
	-rm -f $(TESTS) $(TESTS_STATIC)

cleanDoc:
	-rm  docs/*.html docs/*.png docs/doxygen.css

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
	if [ -f `echo utils/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM utils/*.cpp | \
	awk '$$1 ~ /:/{printf "utils/%s\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi
	if [ -f `echo tests/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM tests/*.cpp | \
	awk '$$1 ~ /:/{printf "tests/%s\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi

depAll: cleanDep
	make dep;
	for dir in `find . -maxdepth 1 -name "Ar*" -xtype d`; do cd $$dir; make dep; cd ..; done

cleanAll: clean cleanJava cleanPython
	rm -f lib/lib*.$(sosuffix)*
	rm -f obj/*.o*



params: utils/makeParams$(binsuffix)
	-mkdir params
	utils/makeParams$(binsuffix)

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

python/AriaPy_wrap.cpp python/AriaPy.py: include/wrapper.i include/*.h python/DiscoverWiBox.py
	#-rm -f `find python -maxdepth 1 -xtype f -name "*Aria*" | grep -v .ds | grep -v .sln | grep -v .vcproj`
	cd python; $(SWIG) -Wall -c++ -python -modern -module AriaPy -DARIA_WRAPPER -Dlinux -DAREXPORT -o AriaPy_wrap.cpp -I../include ../include/wrapper.i 

python/_AriaPy.$(sosuffix): obj/AriaPy_wrap.o lib/libAria.$(sosuffix)
	if ! test -f $(PYTHON_INCLUDE)/Python.h; then echo "Error: $(PYTHON_INCLUDE)/Python.h not found. Is the Python development package (python2.7-dev) installed on this system?"; exit 1; fi
	$(CXX) -O2 -shared -o $(@) obj/AriaPy_wrap.o -Llib -lAria -lpthread -ldl -lrt

obj/AriaPy_wrap.o: python/AriaPy_wrap.cpp
	mkdir -p obj
	@ if test -z "$(PYTHON_INCLUDE)"; then echo "*** Error: PYTHON_INCLUDE is not set, cannot build ARIA python wrapper"; fi
	if ! test -f $(PYTHON_INCLUDE)/Python.h; then echo "Error: $(PYTHON_INCLUDE)/Python.h not found. Is the Python development package (python2.7-dev) installed on this system? Is PYTHON_INCLUDE set correctly?"; exit 1; fi
	$(CXX) -O2 -c $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER $(PYTHON_INCLUDE_FLAGS) $< -o $@


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
	$(CXX) -O2 -fPIC -shared -o $(@) obj/AriaJava_wrap.o -lpthread -ldl -lrt -Llib -lAria

obj/AriaJava_wrap.o: java/AriaJava_wrap.cpp
	@ if test -z "$(JAVA_INCLUDE)"; then echo "Error: JAVA_INCLUDE is not set, compiling AriaJava_wrap.cpp will fail!"; fi
	@ test -n "$(JAVA_INCLUDE)"
	@ if test \! -d "$(JAVA_INCLUDE)"; then echo "Error: JAVA_INCLUDE directory $(JAVA_INCLUDE) does not exist, compiling AriaJava_wrap.cpp will fail!"; fi
	@ test -d "$(JAVA_INCLUDE)" 
	mkdir -p obj
	$(CXX) -O2 -c $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER -I$(JAVA_INCLUDE) -I$(JAVA_INCLUDE)/linux $< -o $@

java/AriaJava_wrap.cpp java/com/mobilerobots/Aria/ArRobot.java: include/wrapper.i $(HEADER_FILES)
	-mkdir -p java/com/mobilerobots/Aria; 
	-rm java/com/mobilerobots/Aria/*.java java/AriaJava_wrap.cpp java/AriaJava_wrap.h
	$(SWIG) -Wall -c++ -java -package com.mobilerobots.Aria -outdir java/com/mobilerobots/Aria -module AriaJava -DARIA_WRAPPER -Dlinux -DAREXPORT -o java/AriaJava_wrap.cpp -Iinclude include/wrapper.i 
    
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
	$(CXX) -O2 $(BARECXXFLAGS) $(CXXINC) -DARIA_WRAPPER -fPIC -shared -o $@ csharp/AriaCS_wrap.cpp -lpthread -ldl -lrt -Llib -lAria

csharp/AriaCS_wrap.cpp csharp/ArRobot.cs: include/wrapper.i $(HEADER_FILES)
	-mkdir -p csharp
	-rm csharp/*.cs
	$(SWIG) -Wall -c++ -csharp -namespace AriaCS -outdir csharp -module AriaCS -DARIA_WRAPPER -Dlinux -DAREXPORT -o csharp/AriaCS_wrap.cpp -Iinclude include/wrapper.i

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
	$(CXX) -shared -o $(@) $(OFILES) $(ARIA_CXXLINK)

lib/libAria.a: $(OFILES) 
	-mkdir lib
	ar -cr $(@) $(OFILES)
	ranlib $(@)

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

utils/%$(binsuffix): utils/%.$(CFILEEXT) lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

utils/%Static$(binsuffix): utils/%.$(CFILEEXT) lib/libAria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	#strip $@

utils/convertBitmapToArMap$(binsuffix): utils/convertBitmapToArMap.$(CFILEEXT) lib/libAria.$(sosuffix)
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ -lnetpbm $(CXXLINK)

# This utility prints information about command line options recognized by
# various classes in ARIA (e.g. ArRobotConnector, ArLaserConnector (for various
# laser types), etc.
utils/genCommandLineOptionDocs$(binsuffix): utils/genCommandLineOptionDocs.cpp lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) -DFOR_ARIA $(CXXINC) $< -o $@ $(CXXLINK)

ifneq ($(binsuffix),)
examples/%: examples/%$(binsuffix)
utils/%: utils/%$(binsuffix)
tests/%: tests/$(binsuffix)
endif

obj/%.o : src/%.cpp 
	@mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $(CXXINC) $< -o $@

obj/%.o : src/%.c 
	@mjdir -p obj
	$(CXX) -c $(CXXFLAGS) $(CXXINC) $< -o $@

obj/ArPacketUtil.o: src/ArPacketUtil.cpp
	$(CXX) $(BARECXXFLAGS) -fexceptions $(EXTRA_CXXFLAGS) $(CXXINC) -c $< -o $@

include/%.i: include/%.h 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) $< -o $@

src/%.i: src/%.cpp 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) $< -o $@

# Don't build .o files if their library is up to date with respect to source files:
.INTERMEDIATE: $(OFILES)

# But don't delete .o files if we do make them in order to make a library:
.PRECIOUS: $(OFILES)


# This rule doesn't actually produce its target, so anything depending on it
# will always get remade.
FORCE:


####
#### Installation and distribution 
####


ifndef INSTALL_DIR
INSTALL_DIR=/usr/local/Aria
endif

ifndef SYSTEM_ETC_DIR
SYSTEM_ETC_DIR=/etc
endif

# What to put in /etc/Aria:
ifndef STORED_INSTALL_DIR
STORED_INSTALL_DIR=$(INSTALL_DIR)
endif

# How to run 'install' for the install rule:
ifndef INSTALL
INSTALL:=install --preserve-timestamps
endif


dist: FORCE
	dist/dist.sh

dist-install: install

# Install rule.  This can be used by users or ARIA developers; in the latter
# case it also installs various files needed to make a release distribution.
# Override installation locations with INSTALL_DIR environment variable.
# Things are installed group-writable so as to be hacked upon.
install: all
	@echo      	--------------------------------------
	@echo		    Installing ARIA in $(DESTDIR)$(INSTALL_DIR)...
	@echo      	--------------------------------------
	$(INSTALL) -m 775 -d $(DESTDIR)$(INSTALL_DIR)
	find    include src tests utils params docs examples maps \
	        java javaExamples python pythonExamples matlab rust \
	        obj rust \
	        \( -name \*.o -or -name core -or -name CVS -or -name .\* -or -name \*~ -or -name tmp -or -name proprietary* -or -name \*.bak -or -name \*.class -or -name \*.lib -or -name \*.dll -or -name \*.exe \) -prune  \
	        -or -type d   -exec $(INSTALL) -d -m 777 $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -type l   -exec cp --no-dereference \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -name \*.a -exec $(INSTALL) -D -m 666 \{\}  $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -perm /u=x  -exec $(INSTALL) -D --strip -m 777 \{\}  $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or           -exec $(INSTALL) -D -m 666 \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \;
	$(INSTALL) -D -m 664 README.md LICENSE.txt Makefile icon.png $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 666  Makefile.dep doxygen.conf $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -d -m 777 $(DESTDIR)$(INSTALL_DIR)/lib/
	$(INSTALL) -D --strip -m 666 lib/libAria.$(sosuffix) $(DESTDIR)$(INSTALL_DIR)/lib/
	if test -f lib/libAriaJava.$(sosuffix); then $(INSTALL) --strip -m 666 lib/libAriaJava.$(sosuffix) $(DESTDIR)$(INSTALL_DIR)/lib/; else echo Warning: lib/libAriaJava.$(sosuffix) not found. Use \"make java\" to build.; fi
	@if test -z "$(DIST_INSTALL)"; then \
		echo "if test \! -d $(DESTDIR)$(SYSTEM_ETC_DIR); then install -d $(DESTDIR)$(SYSTEM_ETC_DIR); fi" ;\
		if test \! -d $(DESTDIR)$(SYSTEM_ETC_DIR); then install -d $(DESTDIR)$(SYSTEM_ETC_DIR); fi ;\
		echo "echo $(STORED_INSTALL_DIR) > $(DESTDIR)$(SYSTEM_ETC_DIR)/Aria" ;\
		echo $(STORED_INSTALL_DIR) > $(DESTDIR)$(SYSTEM_ETC_DIR)/Aria ;\
		echo       ------------------------------------------------------------------------------------ ;\
		echo       ARIA has been installed in $(DESTDIR)$(INSTALL_DIR). ;\
		echo ;\
		echo       To be able to use the ARIA libraries, you must now add $(DESTDIR)$(INSTALL_DIR)/lib ;\
		if test "$(host)" = "Darwin"; \
		then \
			echo       to your DYLD_LIBRARY_PATH environment variable.; \
		else \
			echo       to your LD_LIBRARY_PATH environment variable, or to the /etc/ld.so.conf system file, ;\
			echo       then run \'ldconfig\';\
		fi ;\
		echo     	 ------------------------------------------------------------------------------------ ;\
	fi

#		echo ;\
#		echo       The files are owned by the group \"users\", and all members of that group ;\
#		echo       can enter the directory, read files, and modify the \"examples\" directory. ;\


# Source Code Install rule.  This is used to make source-code only packages.
# It includes files needed for all platforms.
srcdist-install: src-install

src-install:
	$(INSTALL) -m 775 -d $(DESTDIR)$(INSTALL_DIR)
	find    include src tests utils params docs examples maps \
	        java javaExamples python pythonExamples matlab rust \
	        \( -name java/com -or -name \*.jar -or -name \*.a -or -name \*.$(sosuffix) -or -name \*.o -or -name core -or -name CVS -or -name .\* -or -name \*~ -or -name tmp -or -name proprietary* -or -name \*.bak -or -name \*.class \) -prune  \
	        -or -type d   -exec $(INSTALL) -d -m 744 $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -type l   -exec cp --no-dereference \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -perm /u=x  -prune \
	        -or           -exec $(INSTALL) -D -m 644 \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \;
	$(INSTALL) -D -m 644 LICENSE.txt INSTALL.txt README.txt Makefile Aria-Reference.html version.txt Changes.txt CommandLineOptions.txt icon.png $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 644  Makefile.dep doxygen.conf $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 644 bin/SIMULATOR_README.txt $(DESTDIR)$(INSTALL_DIR)/bin/SIMULATOR_README.txt

tags: $(SRC_FILES) $(HEADER_FILES)
	ctags $(SRC_FILES) $(HEADER_FILES)

ctags: tags

# Make optimization, tell it what rules aren't files:
.PHONY: all everything examples modExamples tests utils cleanDep docs doc dirs help info moreinfo clean cleanUtils cleanExamples cleanTests cleanDoc cleanPython dep depAll cleanAll params allLibs python python-doc java cleanJava alllibs params swig help info moreinfo py python-doc cleanSwigJava checkAll dirs install srcdist-install dist-install src-install distclean ctags csharp cleanCSharp

# Include Autogenerated dependencies, using Makefile.dep rule above to generate
# this file if needed:
include Makefile.dep

