# A simple Makefile to cause make to go look in the top directory. A simple
# convenience.

host:=$(shell uname | cut -d _ -f 1)
ifeq ($(host),Darwin)
  sosuffix:=dylib
else
  sosuffix:=so
endif

all: lib 
	$(MAKE) -C .. examples

lib:
	$(MAKE) -C .. lib/libAria.$(sosuffix)

%.$(sosuffix): ../lib/libAria.$(sosuffix) %.cpp 
	$(MAKE) -C .. examples/$@

%: ../lib/libAria.$(sosuffix) %.cpp 
	$(MAKE) -C .. examples/$@

%Static: ../lib/libAria.a %.cpp 
	$(MAKE) -C .. examples/$@

clean: 
	$(MAKE) -C .. cleanExamples

../lib/libAria.$(sosuffix): FORCE
	$(MAKE) -C .. lib/libAria.$(sosuffix)

../lib/libAria.a: FORCE	
	$(MAKE) -C .. lib/libAria.a

FORCE:

.PHONY: all FORCE clean lib
