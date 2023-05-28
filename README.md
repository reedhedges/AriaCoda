AriaCoda
========

[![Build Status](https://github.com/reedhedges/AriaCoda/actions/workflows/builds.yml/badge.svg)](https://github.com/reedhedges/AriaCoda/actions/workflows/builds.yml)

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://img.shields.io/badge/License-GPL%20v2-blue.svg)

[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-green.svg)](https://img.shields.io/badge/Platform-Linux-green.svg)
[![Platform: Windows (MSVC 2019)](https://img.shields.io/badge/Platform-Windows%20%28MSVC%202019%29-green.svg)](https://img.shields.io/badge/Platform-Windows%20%28MSVC%202019%29-green.svg)
[![Platform: Windows (MinGW)](https://img.shields.io/badge/Platform-Windows%20%28MinGW%29-green.svg)](https://img.shields.io/badge/Platform-Windows%20%28MinGW%29-green.svg)
[![Platform: MacOSX](https://img.shields.io/badge/Platform-MacOSX-green.svg)](https://img.shields.io/badge/Platform-MacOSX-green.svg)

AriaCoda is a C++ library used to communicate with Pioneer and
Pioneer-compatible mobile robot controllers, many sensors and other accessory
devices, and includes various useful tools for mobile robotics applications.

The C++ code in AriaCoda is mostly C++03 and C with some C++11 and C++14 features
used as well.  It can be compiled with any C++ compiler supporting
at least C++14. (In the future it may be updated to C++17 or C++20.)

AriaCoda is based on the prior open-source ARIA library (Copyright 2002, 2003, 2004, 2005
ActivMedia Robotics, LLC.  Copyright 2006, 2007, 2008, 2009 MobileRobots Inc.,
Copyright 2010-2015 Adept Technology, Copyright 2016-2018 Omron Adept Technologies, Inc.)

AriaCoda is distributed under the terms of the GNU General Public License
(GPL) version 2.

See [LICENSE.txt](LICENSE.txt) for full license information about AriaCoda.

Requirements
------------

AriaCoda is intended to be used on any recent standard Linux, MacOSX, Windows 7,
or Windows 10 system. The only requirements are standard operating system libraries
(POSIX, as well as the Windows C Runtime and Platform SDK if on Windows and some
standard Linux libraries if on Linux). 

See the section "Building AriaCoda" below for requirements to build AriaCoda
from source code.

Major Changes from previous Aria releases
-----------------------------------------

Several major changes have been made for AriaCoda since the last release of Aria
2.9 by Omron/Adept MobileRobots:

* All header files have been moved into an `Aria` subdirectory. All header file `#include` directives must be updated. For example, use `#include "Aria/ArRobot.h"` instead of `#include "ArRobot.h"`.
* When installed on a Linux system, standard installation locations are used.
  See "Installation" below for details.
* ArNetworking is no longer included or installed with Aria
* Some classes have been removed (if obsolete, or only necessary for use with ArNetworking or used in non-open source products).  Code for these classes has been moved into the `attic` subdirectory of the git repository.  Removed classes include the following:
  * ArSimpleConnector (use ArRobotConnector, ArLaserConnector, and other connector classes instead.)
  * ACTS related classes
  * ArAMPTU, ArP2Arm, ArIrrfDevice, ArTCM2, ArSonyPTZ and ArVersalogicIO have been removed.
  * ArMode and subclasses, and all keyboard handling code, generally only used by examples/demo.cpp, has just been moved into examples/demo.cpp.
  * ArNetServer
  * Support classes for ArNetworking (ArDrawingData etc.)
  * some other classes have been removed as well.
* The list of sensor readings in ArRangeBuffer has been changed from a list of
  pointers to ArPoseWithTime objects (`std::list<ArPoseWithTime*>`) to simply a list of ArPoseWithTime
  objects (`std::list<ArPoseWithTime>`). ArRangeDevice:getCurrentBuffer() and
  ArRangeDevice::getCumulativeBuffer() have been replaced by getCurrentReadings()
  and getCumulativeReading() which return a const reference to the list instead of a pointer.
  This should make it a bit easier to work with range device data but will require changing
  any code that accesses it through these interfaces.   (The old methods
  returning pointers have been retained as "Ptr" versions, but will be removed in
  the future.)
* ArLog default output type is now stderr instead of stdout.
* Many unnecessary uses of "virtual" method declaration (including
  destructors) have been removed. If you derive from any ARIA class and intend
  to override a method, the use of the "override" specifier is encouraged;
  the C++ compiler should then warn if the base class method is not virtual.
  If you encounter problems using AriaCoda because of this change, please submit
  a bug report in Github issues.
* Some function argument and return types have been changed from `int` to
  `size_t` or other more specific or appropriate type.
* The feature of ArFunctor subclasses to store parameter values and the various
  invoke() overloads that used them if too few arguments provided, has been
  deprecated.  Future changes will probably remove this feature (or ArFunctor
  will be replaced entirely by standard C++ features.)
* Fixed width integer types aliased in the ArTypes (ArTypes::Byte, ArTypes::Byte2,
  ArTypes::Byte4, ArTypes::Byte8, ArTypes::UByte, ArTypes::UByte2, ArTypes::UByte4,
  ArTypes::Ubyte8) are deprecated.  Use standard C++ fixed width integer types instead
  (`std::int8_t`, `std::int16_t`, `std::int32_t`, `std::int64_t`, `std::uint8_t`, 
  `std::uint16_t`, `std::uint32_t`, `std::uint64_t`).
* ArMap::getMapObjects() has been changed to return a const reference (`const
  std::list<ArMapObject*>&`) rather than a pointer. Other accessors in
  ArMap have been made `const`.  (Note, This may be further changed in the future  
  to a list of ArMapObject objects or smart pointers rather than pointers.)
  ArMap::getMapObjectsPtr() has been added for internal use and as an easy 
  compatibility function, but use outside of the library is deprecated.
* Various fixes and changes to improve optimization and
  conformance/correctness.  Due to code removal, various optimizations in the 
  source code (especially in frequently used storage and utility classes such 
  as ArSensorReading, ArRangeBuffer, ArRangeDevice, ArPose, etc), and compilation 
  optimizations turned on by default, the AriaCoda
  shared library should be slightly more efficient than previous versions of ARIA. 
  Additional warning flags have been enabled during compilation, and some 
  of the potential problems indicated by those warnings have been fixed.
  If any changes have introduced undetected bugs or unintentional changes in behavior,
  please report any problems on github issues.

Several other changes are planned that will not be compatible with prior Aria
releases, see [TODO.md](TODO.md).

AriaCoda defines the preprocessor symbols `ARIACODA` and `ARIA_3` which may be
used to perform conditional compilation in an application depending on whether
ARIA 2.x or AriaCoda are being used.

Over time, the library may be more consistently modernized to C++17 or later
(unless significant rewriting of generally working and stable code would be
required).  This may require further deprecating, removing or changing some
interfaces. The biggest changes will likely be to replace ArFunctor, ArTime, and
threading and other OS portability wrappers with newer C++ standards, and possibly
requiring use of smart pointers especially when objects or references are passed
into or out of the ARIA API, as well as more consistent use of e.g. `std::string`
or `std::string_view` rather than `char *`, use of standard types rather than types defined by ARIA,
etc.  See [TODO.md](TODO.md) for details.

Support and Contributing
------------------------

AriaCoda is not supported by Omron Corporation or any other commercial organization.
There are many ways to help continued improvement and maintenance of AriaCoda:

* Report bugs or request improvements.  Did you encounter documentation that
is incorrect or difficult to understand?  Would you like to see example code
for a particular feature?  Is there a bug or problem with AriaCoda?
[Report the issue here.](https://github.com/reedhedges/AriaCoda/issues)
* Contribute improvements, bug fixes, tests, documentaton fixes, updated Visual C++ project files, etc.  
See [TODO.md](TODO.md) for ideas.  Changes
can be discussed on [GitHub Issues](https://github.com/reedhedges/AriaCoda/issues).
* Financial sponsorship of the primary developer through
[GitHub Sponsorship](https://github.com/sponsors/reedhedges) or through
other means. This will let me spend more time working on AriaCoda.
Contact me to discuss further.

Building AriaCoda
-----------------

The only requirements to build AriaCoda from source code are 
common developer tools, and a C++ compiler that
supports C++14.  There are no additional library dependencies.

On Linux, the GNU C++ compiler (g++/gcc), GNU Make, and standard shell
tools are required.  On Ubuntu or Debian Linux, run
`sudo apt install make g++` to install.   (Clang can be used as well.)
The C++ compiler must support at least C++14;  
GCC 9 or later is recommended. (Ubuntu Bionic or later is recommended 
as Linux operating system.) See below for notes on older GCC/OS versions.

On Windows, Visual C++ 2019 is required to use the provided project and
solution files.  The free "Community" edition can be downloaded
from <http://visualstudio.microsoft.com/downloads>.  The core C++ development
and Windows SDK components are required, no additional Visual Studio components
are needed.   See more build instructions below.

It is also possible to build AriaCoda with MinGW on Windows.

On Mac OSX, XCode and command-line development tools are required.  
(Run XCode, open "Preferences" from the application menu, select "Downloads",
select "Command Line Tools", and click "Install".)   Run `make` from a
Terminal shell.

In addition, some optional features have additional build requirements.

* Doxygen, for API reference documentation generation. See  <http://www.doxygen.org>
* SWIG, for interface generation for Python and Java interfaces. See <http://www.swig.org>.
* Java and Python development libraries for Python and Java interfaces
* Matlab MEX compiler for Matlab interface
* Rust bindgen tool for Rust interface (and cargo)
* Debian/Ubuntu packaging tools to create Debian/Ubuntu packages

The Makefile used to build AriaCoda on Linux and MacOSX uses certain variables
to contain compilation flags, compiler command name, and optional features, or to
set defaults for a "Debug" build vs. optimized "Release" build.

Run `make help` for brief descriptions of these variables as well as 
available target rules. 

Variables an be set in the shell environment or on the command line when running `make`, 
e.g., `DEBUG` disables some optimizations, and `CXX` overrides the default compiler command:

    make DEBUG=1 CXX=\"ccache c++\"

`EXTRA_CXXFLAGS` adds additional C++ compilation flags to ARIA's, e.g.
to optimize for a computer with an Intel Atom processor (i.e. PioneerLX), 
and math functions in general, you could do this:

    make EXTRA_CXXFLAGS="-mcpu=atom -ffast-math" -j6

Or to optimize for Intel CPUs, supporting Core2 and later:

    make EXTRA_CXXFLAGS="-march=core2 -mtune=intel" -j6

Or to get additional optimization based on the features of the CPU of the
build host:

    make EXTRA_CXXFLAGS="-march=native -mtune=native" -j6

To compile with clang and libc++ instead of gcc and libstdc++:

    make CXX=clang++ EXTRA_CXXFLAGS=-stdlib=libc++

To compile with clang and libc++ with some of the LLVM sanitizer options turned on:

    make DEBUG=1 CXX=clang++ EXTRA_CXXFLAGS="-stdlib=libc++ -fsanitize=address,undefined" -j6

However, be aware that files compiled with different C++ compilers, and sometimes
with different compilation flags (e.g. -stdlib) may not be compatible, resulting
in linker errors or other errors. (You cannot yet specify custom output directories
when building AriaCoda -- but contributions to enable this would be welcome; use `make clean` to start with a clean build.)

Other variables are available. Run `make help` for description of all variables.

### Linux

On Linux enter the AriaCoda source directory in a terminal,
and run `make`.

Use `make -j6` to build faster by building in parallel with a maximum of 6 jobs
(`-j4` for 4 jobs, `-j2` for 2 jobs, etc.), or `make -j` to run all jobs in parallel.

Run `make help` for information on more make rules and parameters.

Note: To build on certain older Linux versions (glibc versions) (e.g. Ubuntu Xenial),
add the `-DARIA_OMIT_DEPRECATED_MATH_FUNCS` compilation flag to `EXTRA_CXXFLAGS` when building:

    make EXTRA_CXXFLAGS=-DARIA_OMIT_DEPRECATED_MATH_FUNCS 

A few optional features (e.g. `constexpr` support) may be omitted from specific parts of the API
if building with older compilers or standard library versions.

### Windows

On Windows, open `AriaCoda.sln` with Visual Studio 2019 and build the
solution.   This will build AriaCoda as a static library (`lib/ARIA.lib`
in Release mode or `lib/AriaDebug.lib` in Debug mode.)

Note: As new versions of Visual C++ are released in the future,
the Visual C++ project and solution files may be updated, requiring that
you update your Visual C++ version.

If there is an error in Visual Studio about build tools not installed when loading
the AriaCoda project, open "Tools" -> "Get Tools and Features...", choose "Individual
components", and install the latest version of "MSVC C++ x64/x86 build tools",
"C++ core features", and the latest version of the Windows SDK (Or install the
complete C++ development environment option).

It may be possible to use older versions of Visual C++, but you will
need to create new project files for AriaCoda using that version of Visual C++.  
(Generally, Visual C++ project files are specific to versions of Visual C++, though
Visual Studio can often upgrade older project files.)

### MacOSX

On Mac OSX, XCode and command-line development tools are required.  
(Run XCode, open "Preferences" from the application menu, select "Downloads",
select "Command Line Tools", and click "Install".)   Run `make` from a
Terminal shell to build the Aria dynamic library.

### Configuring library path 

Note: After building AriaCoda, you can build and run example and test programs
without installing on the system by adding the library directory to your dynamic
library path, otherwise you will receive an error similar to the following:

    error while loading shared libraries: libAria.so: cannot open shared object file: No such file or directory

Linux:

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/AriaCoda/lib

Mac OSX:

    export DYLIB_LIBRARY_PATH=$DYLIB_LIBRARY_PATH:~/AriaCoda/lib

Replace `~/AriaCoda` with the correct path to your AriaCoda source directory.

Installation
------------

AriaCoda can be installed using Ubuntu/Debian packages or after building the source
code (using the `install` rules in the Makefile).

The available Ubuntu/Debian packages are:
* libaria3
* libaria3-dev
* libaria3-doc
* libaria3-utils
* libaria3-java
* python3-aria
* libaria3-examples

Use `apt` to install.

`libaria3` installs the shared library into the correct subdirectory
of `/usr/lib`.  `libaria3-dev` installs additional shared library aliases 
and the header files in `/usr/include/Aria`.  `libaria3-doc` installs
documentation in `/usr/share/doc/Aria`.  `libaria3-utils` installs
`ariaDemo`, `mtxPower`, and `seekurPower` into `/usr/bin`.  `libaria3-examples`
installs example program source code to `/usr/share/doc/Aria/examples`.

To install AriaCoda from source code, use `sudo make install`.  This installs the
`libAria.so` shared library (also with versioned files) into `/usr/local/lib`
and, the header files in `/usr/local/include/Aria`. Robot parameter files
are installed into `/usr/local/share/Aria/params`.
Use `make install-doc` to install API reference documentation into
`/usr/local/share/doc/Aria/doc`, `make install-utils`
to install `demo` (as `ariaDemo`), `mtxPower`, and `seekurPower` into
`/usr/local/bin`,
`make install-python` to install the Python wrapper files, and `make
install-java` to install the Java wrapper.   Run `make install-examples` to
install the example program source code into
`/usr/local/share/doc/Aria/examples`.  

To specify an alternate installation
location, set the `DESTDIR` variable in the environment or when running `make`.
If `DESTDIR` is set, then the library is installed in `$DESTDIR/usr/lib`,
header files are installed in `$DESTDIR/usr/include/Aria`, robot parameter files
are installed in `$DESTDIR/usr/share/Aria/params`, documentation is installed in
`$DESTDIR/usr/share/doc/Aria`, utility programs are installed 
into `$DESTDIR/usr/bin`, example program source code is installed in 
`$DESTDIR/share/doc/Aria/examples`.

Documentation
-------------

[Reference documentation](docs/index.html), including some annotated examples and
an in-depth introduction and overview to AriaCoda, can be generated using the Doxygen
tool.

To install Doxygen on Ubuntu or Debian Linux, run `sudo apt install doxygen`.

Then, to generate the documentation, run `make doc`.

To install Doxygen on Windows, go to <http://www.doxygen.nl/download.html>, and
use the download link under "A binary distribution for Windows...".

To generate the AriaCoda documentation on Windows, open a command prompt
terminal (choose "Run..." from Start menu ond enter `cmd`), enter the
AriaCoda source directory (e.g. `cd %HOMEPATH%\AriaCoda` or similar), and run
Doxygen using the `doxygen.conf` configuration file by entering
`C:\doxygen\bin\doxygen.exe doxygen.conf`.  (You can add `C:\doxygen\bin`
to your program PATH in the System control panel, if desired.)  

Example Programs
----------------

Example program  source code can be found in the `examples/` directory.  View the
[examples `README.txt` file](examples/README.txt) for descriptions.

### Configuring library path

On Linux you need to add the `lib` directory to your `LD_LIBRARY_PATH` environment
variable to run example programs without installing AriaCoda on the system, e.g.:

    export LD_LIBRARY_PATH=`pwd`/lib
    cd examples
    make simpleConnect
    ./simpleConnect

Otherwise you will receive an error similar to the following:

    error while loading shared libraries: libAria.so: cannot open shared object file: No such file or directory

On MacOSX you similarly need to add the `lib` directory to your `DYLD_LIBRARY_PATH`
environment variable, e.g.:

    export DYLD_LIBRARY_PATH=`pwd`/lib
    cd examples
    make simpleConnect
    ./simpleConnect

On Windows, the AriaCoda DLL and example program binaries are both stored in the
`bin` directory, which allows the programs to find the Aria DLL.   Or, an example
program may be built with static linkage to the AriaCoda library.

### Specifying robot connection port

You can specify the serial port device to use when running any example or test program
using the `-robotPort` command line argument. Other command-line arguments for configuring
the robot connection are also available, use the `-help` command-line argument to display
information.  (These arguments can be used with all programs using `ArArgumentParser`
and `ArRobotConnector`, including your own.) 

If using a USB-serial adapter with device name `/dev/ttyUSB0` on Linux, use a command such as the following:

    ./simpleConnect -robotPort /dev/ttyUSB0

On Windows, a USB-serial adapter may use a new "COM" port name, e.g. `COM3`, `COM4`, etc.:

    simpleConnect.exe -robotPort COM3

On Linux USB-serial device names are printed in the Linux kernel log when connected, use
`sudo dmesg` to view.  On Windows, USB-serial port names are shown in the system control panel.

More information on how ARIA connects to the robot and other devices can be found in the
[reference documentation](docs/index.html).

Simulator
---------

AriaCoda may be used with the [AMRISim](http://github.com/reedhedges/AMRISim) simulator, or MobileSim.

Simply run the simulator first (enable emulated Pioneer interface if necessary), then run
any program using AriaCoda on the same host. No special configuration changes or version needs to
be used to use AMRISim, AriaCoda will automatically detect AMRISim if it is running on the same
computer, or you can connect remotely to AMRISim using the `-remoteHost` command-line option,
or network broadcast service discovery (currently implemented in the Python
wrapper only.)
