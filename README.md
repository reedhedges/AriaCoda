AriaCoda
=========

AriaCoda is a C++03 library used to communicate with Pioneer and
Pioneer-compatible mobile robot controllers, many sensors and other accessory
devices, and includes various useful tools for mobile robotics applications.

AriaCoda is based on ARIA (Copyright 2002, 2003, 2004, 2005 ActivMedia Robotics, LLC.
Copyright 2006, 2007, 2008, 2009 MobileRobots Inc., Copyright 2010-2015 Adept Technology,
Copyright 2016-2018 Omron Adept Technologies, Inc.)

AriaCoda is distributed under the terms of the GNU General Public License
(GPL) version 2.

See [LICENSE.txt](LICENSE.txt) for full license information about AriaCoda.


Requirements
------------

AriaCoda is intended to be used on any standard Linux or Windows 7 or 10 
system with standard system libraries and common developer tools only.  

On Linux, the GNU C++ compiler (g++/gcc), GNU Make, and standard shell
tools are required.  On Ubuntu or Debian Linux, run 
`sudo apt install make g++` to install. 

On Windows, Visual C++ 2019 is required to use the provides project and
solution files.  The free "Community" edition can be downloaded
from <http://visualstudio.microsoft.com/downloads>.  (Only the C++ compiler
and Windows SDK are required, no additional Visual Studio components
are needed.)   It may be possible to use older versions of Visual C++ 
if neccesary, but you will need to create new project files for AriaCoda
using that version of Visual C++.  (Generally, Visual C++ project files
are specific to versions of Visual C++, though Visual Studio can often 
upgrade older project files.)

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
* Some classes have been removed (if obsolete, or only neccesary for use with ArNetworking or used in non-open source products).  Including the following:
  * ArSimpleConnector (use ArRobotConnector, ArLaserConnector, and other connector classes instead.)
  * ACTS related classes
  * ArAMPTU, ArP2Arm, ArIrrfDevice, ArTCM2, ArSonyPTZ and ArVersalogicIO have been removed.
  * ArMode and subclasses, and all keyboard handling code, generally only used by examples/demo.cpp, has just been moved into examples/demo.cpp.
  * ArNetServer
  * Support classes for ArNetworking (ArDrawingData etc.)
Code for these classes has been moved into the `attic` subdirectory of the git
repository.

Several other changes are planned that will not be compatible with prior Aria
releases, see [TODO.md](TODO.md).


Building AriaCoda
-----------------

On Linux, enter the AriaCoda source directory in a terminal,
and run `make`.   

On Windows, open `AriaCoda.sln` with Visual C++ and build the
solution.


Installation
------------

TODO


Reference Documentation
-----------------------

API Reference Documentation, including some annotated examples and 
an in-depth introduction/overview, can be generated using the Doxygen
tool.

To install Doxygen on Ubuntu or Debian Linux, run `sudo apt install doxygen`.

To generate the AriaCoda documentation on Linux, run `make doc`.

To install Doxygen on Windows, go to <http://www.doxygen.nl/download.html>, and 
use the download link under "A binary distribution for Windows...".

To generate the AriaCoda documentation on Windows, open a command prompt
terminal (choose "Run..." from Start menu ond enter `cmd`), enter the 
AriaCoda source directory (e.g. `cd %HOMEPAHT%\AriaCoda` or similar), and run 
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

