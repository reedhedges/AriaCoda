

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

# Run subset of tests that fail on errors, and don't require any special hardware (like robot or sensors):
RUNNABLE_TESTS = poseTest lineTest arsectors mathTests lms1xxPacket

runTests: $(RUNNABLE_TESTS)
	for f in $(RUNNABLE_TESTS); do echo; echo ========== ; echo $$f; echo ----------; ./$$f || echo ">>>> Test $$f failed! <<<<" ; done

run_%: %
	@echo
	@echo ==========
	@echo $<
	@echo ----------
	./$<
	@echo "Test $<  ok"

FORCE:

