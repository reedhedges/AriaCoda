
include ../Makefile.inc

# Redefine these to change paths:
CXXINC=-I../include
CXXLINK=-L../lib -lAria $(ARIA_CXXLINK) 
CXXSTATICLINK=-L../lib -Xlinker -Bstatic -lAria -Xlinker -Bdynamic $(ARIA_CXXSTATICLINK)

UTILS_CPP:=$(shell find . -name "*.$(CFILEEXT)")
UTILS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(UTILS_CPP))

all: $(UTILS)

cleanDep:
	-rm Makefile.dep 

clean:
	-rm -f $(UTILS)

# Force rebuild of Makefile.dep:
dep: cleanDep clean
	$(MAKE) Makefile.dep

Makefile.dep:
	if [ -f `echo *.cpp | cut -d' ' -f1` ]; then \
    $(CXX) $(CXXFLAGS) $(CXXINC) -MM *.cpp | \
    awk '$$1 ~ /:/{printf "%s\n", $$0} $$1 !~ /:/' | \
    sed 's/\.o//' >> Makefile.dep; fi

../lib/libAria.$(sosuffix): FORCE
	$(MAKE) -C .. lib/libAria.$(sosuffix)

../lib/libAria.a: FORCE
	$(MAKE) -C .. lib/libAria.a

%$(binsuffix): %.$(CFILEEXT) ../lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

%Static$(binsuffix): %.$(CFILEEXT) ../lib/libAria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	#strip $@

convertBitmapToArMap$(binsuffix): convertBitmapToArMap.$(CFILEEXT) ../lib/libAria.$(sosuffix)
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ -lnetpbm $(CXXLINK)

# This utility prints information about command line options recognized by
# various classes in ARIA (e.g. ArRobotConnector, ArLaserConnector (for various
# laser types), etc.
utils/genCommandLineOptionDocs$(binsuffix): utils/genCommandLineOptionDocs.cpp lib/libAria.$(sosuffix) 
	$(CXX) $(CXXFLAGS) -DFOR_ARIA $(CXXINC) $< -o $@ $(CXXLINK)


# This rule doesn't actually produce its target, so anything depending on it
# will always get remade.
FORCE:

-include Makefile.dep
