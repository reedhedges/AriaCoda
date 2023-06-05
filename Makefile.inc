

# File included by Makefile, examples/Makefile, tests/Makefile and
# utils/Makefile

ifndef CXX
CXX:=g++
endif

ifndef AR
AR:=ar
endif

ifndef RANLIB
RANLIB:=ranlib
endif

CFILEEXT:=cpp

# BARECXXFLAGS is used when compiling python and java wrapper code
# CXXFLAGS is used when compiling everything else (libraries, static libraries,
# examples) and adds EXTRA_CXXFLAGS and possibly other options.
# You can set CXXWARNFLAGS to select specific warnings, if unset a default set
# of warnings will be used.
# You can set EXTRA_CXXFLAGS to just add more flags. For example ,add -pg
# -fprofile-arcs to generate profiling information or add extra optimization flags
# here during "release" builds, or to add more warnings in addition to the
# defaults.
# Set CXXSTD to set value of -std option (to enable a different C++ version).  Default is c++14.
# Set CXXDEBBUGFLAGS and CXXOPTFLAGS to set debug and optimization flags

CXXWARNFLAGS ?= -Wall -Wextra -Wshadow -Wsign-conversion -Wconversion -Woverloaded-virtual
# -Wpedantic -Wno-deprecated-declarations
# -Wfloat-conversion (Note one of the flags above appears to turn this on with clang on MacOSX)
# -Wnon-virtual-dtor
# -Wduplicated-cond -Wduplicated-branches -Wlogical-op
# -Wnull-dereference
# -Wcast-align -Wuseless-cast 
# -Wlifetime (clang only)

CXXSTD ?= c++14
CXXDEBUGFLAGS ?= -g

ifdef DEBUG
CXXOPTFLAGS ?= -Og
else
CXXOPTFLAGS ?= -O3
endif

BARECXXFLAGS=-std=$(CXXSTD) $(CXXDEBUGFLAGS) $(CXXOPTFLAGS) $(CXXWARNFLAGS) -D_REENTRANT  
CXXFLAGS+=$(BARECXXFLAGS) $(EXTRA_CXXFLAGS)
CXXINC=-Iinclude # used in all targets
CXXLINK=-Llib -lAria $(ARIA_CXXLINK) $(EXTRA_LFLAGS) # used for examples, tests etc.
CXXSTATICLINK=-Llib -Xlinker -Bstatic -lAria -Xlinker -Bdynamic $(ARIA_CXXSTATICLINK)
# ARIA_CXXLINK and ARIA_CXXSTATICLINK are defined in next section

sonamelinkflag=-Wl,-soname,libAria.$(sosuffix).$(majorlibver)

host:=$(shell uname | cut -d _ -f 1)
ifeq ($(host),MINGW32)
  $(info Building on MinGW)
  #CXXFLAGS+=-mwindows -mms-bitfields -D__MINGW__ -DMINGW
  BARECXXFLAGS+=-mms-bitfields -D__MINGW__ -DMINGW
#  ARIA_CXXLINK=-lpthreadGC2 -lwinmm -lws2_32 -lstdc++
  ARIA_CXXLINK=-lpthreadGC-3 -lwinmm -lws2_32 -lstdc++
  ARIA_CXXSTATICLINK=-Wl,-Bstatic -lpthread -Wl,-Bdynamic -lwinmm -lws2_32 -lstdc++
  binsuffix:=.exe
  sosuffix:=so
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
    $(info Building on Linux)
    ARIA_CXXLINK+=-lrt
    ARIA_CXXSTATICLINK+=-Xlinker -Bdynamic -lpthread -ldl -lrt -Xlinker -Bstatic -lstdc++ -Xlinker -Bdynamic
    sosuffix:=so
    ARIA_LINKFLAGS=-Wl,-soname,libAria.so.$(majorlibver)
  endif
endif


