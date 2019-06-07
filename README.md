AriaCoda
=========

AriaCoda is based on ARIA (Copyright 2002, 2003, 2004, 2005 ActivMedia Robotics, LLC.
Copyright 2006, 2007, 2008, 2009 MobileRobots Inc., Copyright 2010-2015 Adept Technology,
Copyright 2016-2018 Omron Adept Technologies, Inc.)

AriaCoda is distributed under the terms of the GNU General Public License
(GPL) version 2.

See LICENSE.txt for full license information about AriaCoda.


Requirements
------------

AriaCoda is intended to be used on any standard Linux or Windows 7 or 10 
system with standard system libraries and common developer tools only.  

On Linux, the GNU C++ compiler (g++/gcc), GNU Make, and standard shell
tools are required.  On Ubuntu or Debian Linux, run 
`sudo apt install make g++` to install. 

On Windows, Visual C++ 2019 will be required (project files are still being
updated and created).   The free "Community" edition can be downloaded
from <http://visualstudio.microsoft.com/downloads>.  (Only the C++ compiler
and Windows SDK are required, no additional Visual Studio components
are needed.) 

In addition, some optional feaures have additional requirements. 
* Doxygen, for API reference documentation generation
* SWIG, for interface generation for Python and Java interfaces
* Java and Python developmen libraries for Python and Java interfaces
* Matlab MEX compiler for Matlab interface
* Rust bindgen tool for Rust interface (and cargo)
* Debian packaging tools to create Debian packages


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

