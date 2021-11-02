# A simple $(MAKE)file to cause $(MAKE) to go look in the top directory. A simple
# convenience.

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

FORCE:

