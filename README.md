AriaCoda
========

[![Build Status](https://travis-ci.com/reedhedges/AriaCoda.svg?branch=master)](https://travis-ci.com/reedhedges/AriaCoda)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://img.shields.io/badge/License-GPL%20v2-blue.svg)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-green.svg)](https://img.shields.io/badge/Platform-Linux-green.svg)
[![Platform: Windows (MSVC 2019)](https://img.shields.io/badge/Platform-Windows%20%28MSVC%202019%29-green.svg)](https://img.shields.io/badge/Platform-Windows%20%28MSVC%202019%29-green.svg)
[![Platform: Windows (MinGW)](https://img.shields.io/badge/Platform-Windows%20%28MinGW%29-green.svg)](https://img.shields.io/badge/Platform-Windows%20%28MinGW%29-green.svg)
[![Platform: MacOSX](https://img.shields.io/badge/Platform-MacOSX-green.svg)](https://img.shields.io/badge/Platform-MacOSX-green.svg)

AriaCoda is a C++ library used to communicate with Pioneer and
Pioneer-compatible mobile robot controllers, many sensors and other accessory
devices, and includes various useful tools for mobile robotics applications.

Much of the library is mostly C++03 and C with C++11 and C++14 features
sometimes used as well.  (In the future it may be further modernized.)

AriaCoda is based on the prior open-source ARIA library (Copyright 2002, 2003, 2004, 2005 
ActivMedia Robotics, LLC.  Copyright 2006, 2007, 2008, 2009 MobileRobots Inc., 
Copyright 2010-2015 Adept Technology, Copyright 2016-2018 Omron Adept Technologies, Inc.)

AriaCoda is distributed under the terms of the GNU General Public License
(GPL) version 2.

See [LICENSE.txt](LICENSE.txt) for full license information about AriaCoda.


Requirements
------------

AriaCoda is intended to be used on any recent standard Linux, MacOSX, Windows 7, 
or Windows 10 system with standard system libraries, common developer tools, and
a recent C++ compiler only.  
There are no required library dependencies other than the C++ standard library.

On Linux, the GNU C++ compiler (g++/gcc), GNU Make, and standard shell
tools are required.  On Ubuntu or Debian Linux, run 
`sudo apt install make g++` to install.   (Clang may also work but is not
tested.)   The C++ compiler must support at least C++14.  Ubuntu Bionic 
or later is recommended. (See below for older systems.)

On Windows, Visual C++ 2019 is required to use the provides project and
solution files.  The free "Community" edition can be downloaded
from <http://visualstudio.microsoft.com/downloads>.  (Only the C++ compiler
and Windows SDK are required, no additional Visual Studio components
are needed.)   It may be possible to use older versions of Visual C++ 
if neccesary, but you will need to create new project files for AriaCoda
using that version of Visual C++.  (Generally, Visual C++ project files
are specific to versions of Visual C++, though Visual Studio can often 
upgrade older project files.)

Note: As new versions of Visual C++ are released in the future, 
the Visual C++ project and solution files may be updated, requiring that 
you update your Visual C++ version.

In addition, some optional feaures have additional requirements. 
* Doxygen, for API reference documentation generation. See  <http://www.doxygen.org>
* SWIG, for interface generation for Python and Java interfaces. See <http://www.swig.org>.
* Java and Python developmen libraries for Python and Java interfaces
* Matlab MEX compiler for Matlab interface
* Rust bindgen tool for Rust interface (and cargo)
* Debian/Ubuntu packaging tools to create Debian/Ubuntu packages

Major Changes from previous Aria releases
-----------------------------------------

Several major changes have been made for AriaCoda since the last release of Aria
by Omron/Adept MobileRobots:

* All header files have been moved into an `Aria` subdirectory. All header file `#include` directives must be updated. For example, use `#include "Aria/ArRobot.h"` instead of `#include "ArRobot.h"`.
* ArNetworking is no longer included or installed with Aria
* Some classes have been removed (if obsolete, or only neccesary for use with ArNetworking or used in non-open source products).  Code for these classes has been moved into the `attic` subdirectory of the git repository.  Removed classes include the following:
  * ArSimpleConnector (use ArRobotConnector, ArLaserConnector, and other connector classes instead.)
  * ACTS related classes
  * ArAMPTU, ArP2Arm, ArIrrfDevice, ArTCM2, ArSonyPTZ and ArVersalogicIO have been removed.
  * ArMode and subclasses, and all keyboard handling code, generally only used by examples/demo.cpp, has just been moved into examples/demo.cpp.
  * ArNetServer
  * Support classes for ArNetworking (ArDrawingData etc.)
  * some other classes have been removed as well.
* Many small fixes and changes to improve optimization and conformance/correctness with newer C++ standards
* The list of sensor readings in ArRangeBuffer has been changed from a list of
  pointers to ArPoseWithTime objects (`std::list<ArPoseWithTime*>`) to simply a list of ArPoseWithTime
  objects (`std::list<ArPoseWithTime>`). ArRangeDevice:getCurrentBuffer() and
  ArRangeDevice::getCumulativeBuffer() have been replaced by getCurrentReadings()
  and getCumulativeReading() which return a const reference to the list intead of a pointer.   
  This sould make it a bit easier to work with range device data but will require changing 
  any code that accesses it through these interfaces.   (The old methods
  returning pointers have been retained as "Ptr" versions, but will be removed in
  the future.)
* ArLog default output type is now stderr instead of stdout. 
* Many unneccesary uses of "virtual" method declaration (including
  destructors) have been removed. If you derive from any ARIA class and intend
  to override a method, the use of the "override" specifier is encouranged;
  the C++ compiler should then warn if the base class method is not virtual.
* Many small fixes and changes to improve optimization and
  conformance/correctness. This may have introduced bugs or changes in behavior,
  please report any problems on github issues.

Several other changes are planned that will not be compatible with prior Aria
releases, see [TODO.md](TODO.md).

Over time, the library may be more consistently modernized to C++17 or later
(unless significant rewriting of generally working and stable code would be
required).  This may require further deprecating, removing or changing some 
interfaces. The biggest changes will likely be to replace Functors, ArTime, and 
threading and other OS portability wrappers with newer C++ standards, and possibly 
requiring use of smart pointers especially when objects or references are passed
into or out of the ARIA API, as well as more consistent use of e.g. `std::string`
rather than `char *`, use of standard types rather than types defined by ARIA,
etc.  See [TODO.md](TODO.md) for details.


Support and Contributing
------------------------

AriaCoda is not supported by Omron Corporation or any other commercial organization.
There are many ways to help continued improvement and maintainence of AriaCoda:

* Report bugs or request improvements.  Did you encounter documentation that
is incorrect or difficult to understand?  Would you like to see example code
for a particular feature?  Is there a bug or problem with AriaCoda? 
[Report the issue here.](https://github.com/reedhedges/AriaCoda/issues)
* Contribute code.  See [TODO.md](TODO.md) for ideas.  Changes
can be discussed on [GitHub Issues](https://github.com/reedhedges/AriaCoda/issues). 
* Financial sponsorship of the primary developer through
[GitHub Sponsorship](https://github.com/sponsors/reedhedges) or through
other means. This will let me spend more time working on AriaCoda. 
Contact me to discuss further.

Building AriaCoda
-----------------

On Linux, enter the AriaCoda source directory in a terminal,
and run `make`.    

Use `make -j4` to build targets in parallel with a maximum of 4 jobs.
(Use `make -j2` for 2 jobs, etc.)

Run `make help` for information on more make rules and parameters.

On Windows, open `AriaCoda.sln` with Visual C++ 2019 and build the
solution.  

Note: As new versions of Visual C++ are released in the future, 
the Visual C++ project and solution files may be updated, requiring that 
you update your Visual C++ version.

Note: To allow building on older Linux/GCC versions (e.g. Ubuntu Xenial), some compilation flags can
be added to `EXTRA_CXXFLAGS` when building:

    make EXTRA_CXXFLAGS=-DARIA_OMIT_DEPRECATED_MATH_FUNCS 

Installation
------------

Not yet implemented.

TODO


Documentation
-------------

Documentation, including some annotated examples and 
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
to your program PATH in the System control panel, if desired, to allow
you to directry run any program in `C:\doxygen\bin` directry from the command
prompt.)  


Example Programs
----------------

Example programs can be found in the `examples/` directory.

Simulator 
---------

AriaCoda may be used with the [AMRISim](http://github.com/reedhedges/AMRISim) simulator, or MobileSim.

