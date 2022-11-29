

all: ../lib/libAria.so
	$(MAKE) -C .. tests

.PHONY: all clean FORCE

../lib/libAria.so: FORCE
	$(MAKE) -C .. dirs lib/libAria.so

../lib/libAria.a: FORCE
	$(MAKE) -C .. dirs lib/libAria.a

%.so: ../lib/libAria.so %.cpp 
	$(MAKE) -C .. tests/$@

%: ../lib/libAria.so %.cpp 
	$(MAKE) -C .. tests/$@

%Static: ../lib/libAria.a %.cpp
	$(MAKE) -C .. tests/$@

newFunctionObjectTemplates: ../lib/libAria.so newFunctionObjectTemplates.cpp
	$(MAKE) -C .. tests/$@ CXXSTD=c++17

clean: 
	$(MAKE) -C .. cleanTests

# Run subset of tests that automatically test for and fail on errors, and don't require any special hardware (like robot or sensors):
RUNNABLE_TESTS = poseTest lineTest arsectors mathTests lms1xxPacket angleFixTest angleTest configTest configSectionTest fileParserTest nmeaParser functorTest getValuesFromCharBuf gpsCoordsTest interpolationTest transformTest stripQuoteTest moreStringTests

SLOW_RUNNABLE_TESTS = timeTest


runTests: $(RUNNABLE_TESTS)
	for f in $(RUNNABLE_TESTS); do echo; echo ========== ; echo $$f; echo ----------; LD_LIBRARY_PATH=../lib ./$$f || echo ">>>> Test $$f failed! <<<<" ; done


runSlowTests: $(SLOW_RUNNABLE_TESTS)
	for f in $(SLOW_RUNNABLE_TESTS); do echo; echo ========== ; echo $$f; echo ----------; LD_LIBRARY_PATH=../lib ./$$f || echo ">>>> Test $$f failed! <<<<" ; done


runAllTests: runTests runSlowTests

run_%: %
	@echo
	@echo ==========
	@echo $<
	@echo ----------
	./$<
	@echo "Test $<  ok"


# Run subset of tests that automatically test for and fail on errors, but require special hardware (like robot or sensors):
RUNNABLE_ROBOT_TESTS = 


runRobotTests: $(RUNNABLE_ROBOT_TESTS)
	for f in $(RUNNABLE_ROBOT_TESTS); do echo; echo ========== ; echo $$f; echo ----------; ./$$f || echo ">>>> Test $$f failed! <<<<" ; done


FORCE:

