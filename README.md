
> *IMPORTANT NOTE: The original ARIA 2.9.4 README follows. The code has since been modified
>  by removing some obsolete or little-used classes and other changes.
>  ArNetworking is also not currently included.  Therefore this README is now
>  out of date. *

ARIA
====

Adept MobileRobots Advanced Robotics Interface for Applications
---------------------------------------------------------------

Version 2.9.4
  
Linux 

Copyright 2002, 2003, 2004, 2005 ActivMedia Robotics, LLC. All rights reserved.
Copyright 2006, 2007, 2008, 2009 MobileRobots Inc. All rights reserved.
Copyright 2010-2015 Adept Technology. All rights reserved.
Copyright 2016-2018 Omron Adept Technologies, Inc. All rights reserved.

See LICENSE.txt for full license information about ARIA.

Please read this document for important details about using ARIA.

Contents
========

*  Introduction
*  Documentation
*  Licenses and Sharing
*  ARIA Package
*  Files of Note
*  Compiling programs that use ARIA
   *  Windows
   *  Linux
      *  Using ARIA's Makefiles
      *  Using your own Makefile or other build system
      *  Setting variables for ARIA Makefiles
      *  Learning more about using Linux
   * Mac OSX
   *  Learning more about C++
*  Using ARIA from Python and Java
*  Using ARIA from Matlab or Simulink
*  Simulator

Introduction
============

Welcome to the Adept MobileRobots Advanced Robotics Interface for Applications (ARIA).
ARIA is an object-oriented, application programming interface (API) for the
Adept MobileRobots (and ActivMedia) line of intelligent mobile robots, including
Pioneer 2/3 DX and AT, PeopleBot, PowerBot, AmigoBot, PatrolBot/Guiabot,
Seekur, SeekurJr and Pioneer LX mobile robots.

Written in the C++ language, ARIA  provides 
access to and management of the robot controller, as well
to many accessory robot sensors and effectors, as well as useful utilities
and infrastructure for cross-platform robot application development.
   
Several levels are available for application to use in ARIA,
from simple command-control of the robot server for direct-drive
navigation, to development of higher-level intelligent actions (aka
behaviors). 

ARIA is provided as open source software under the GNU General Public
License. This allows you to view, modify and rebuild the ARIA library
as desired, provided that software developed with ARIA comply with the
requirements of the license.  Please see LICENSE.txt for the full 
license under which ARIA has been provided.

The ARIA package includes both source code and pre-built libraries
and example programs. These libraries and programs were build with
GCC 4.x if on Linux, and Visual C++ 2012 (11.0), Visual C++ 2015 (14.0),
and Visual C++ 2017 (15.x) for Windows Desktop if on Windows.   

Using the above compilers for development is recommended.  See below 
for specific instructons on using these compilers to build your  
programs linked to ARIA, or to rebuild ARIA.

NOTE: If you use a different compiler or compiler version, you must 
rebuild the ARIA libraries to ensure link compatability.

To get started immediately with ARIA, refer to the examples in the
"examples" directory (start with "simpleConnect.cpp"), read the information
below, and in the ARIA Reference manual.

See below for more information about building programs with ARIA
on Windows and Linux and using the Windows and Linux development tools.


Documentation and Help
======================

Follow the INSTALL text instructions to install ARIA on your Linux or 
Windows workstation or robot computer. System requirements are included 
in the INSTALL.txt file.

ARIA includes a full API Reference Manual in HTML format. This manual,
Aria-Reference.html (and in the docs/ directory), includes documentation 
about each of the classes and methods in ARIA, as well as a comprehensive 
overview describing how to get stated understanding and using ARIA.  In 
addition, ARIA includes several example programs in the examples/ 
(start with simpleConnect.cpp, then explore the others) and 
advanced/ directories, and the header files and source code are included
in include/ and src/ as well.  

The ArNetworking library has its own reference manual,
ArNetworking-Reference.html in the ArNetworking subdirectory, and examples
in ArNetworking/examples.

If you plan on using the Java, Python or Matlab wrapper libraries, see the 
javaExamples, pythonExamples, ArNetworking/javaExamples,
ArNetworking/pythonExamples, and matlab directories for important information in
README files, and example programs. You should also read the ARIA Reference
manual for general information about ARIA -- the API in the wrapper libraries
are almost identical to the C++ API.

If you have any problems or questions using ARIA or your robot, the 
MobileRobots support site provides:
 
  * A FAQ (Frequently Asked Questions) list, at <http://robots.mobilerobots.com/FAQ.html>
  * A knowlege base of information on robot hardware and software, at <http://robots.mobilerobots.com>
  * All robot and device manuals
  * The Aria-Users mailing list, where you can discuss ARIA with other users and 
    MobileRobots software developers:
      * Search the archives at <http://robots.mobilerobots.com/archives/aria-users/threads.html>
      * Join the list at <http://robots.mobilerobots.com/archives/aria-info.html>
  * Information on contacting MobileRobots technical support.

Visit the MobileRobots support site at <http://robots.mobilerobots.com>

License and Sharing
===================

ARIA is released under the GNU Public License (GPL) v.2, which means 
that if you distribute any work which uses ARIA, you must distribute 
the entire source code to that work under the GPL as well.  Read the 
included LICENSE text for details.  We open-sourced ARIA under GPL with 
full source code not only for your convenience, but also so that you can 
share your work and enhancements to the software.  If you wish your 
ARIA changes to make it into our future ARIA versions, you will need to 
assign the copyright on those changes to Adept. Contact support@mobilerobots.com 
with these changes or with questions about this.

Accordingly, please do share your work, and please sign up for the 
exclusive aria-users@mobilerobots.com newslist so that you can benefit 
from others' work, too.

ARIA may instead be relicensed for proprietary, closed source applications. 
Contact sales@mobilerobots.com for details.

For answers to frequently asked questions about what the GPL allows
and requires, see <http://www.gnu.org/licenses/gpl-faq.html>


The ARIA Package
================

Aria/:
  docs      The API Reference Manual: Extensive documentation of all of ARIA
  examples  ARIA examples -- a good place to start; see examples/README.txt
  include   ARIA header (*.h) files
  src       ARIA source code (*.cpp) files
  lib or lib64 - Libraries (.lib files for Windows, .so files for Linux) 
  bin or bin64 - Contains Windows binaries and DLLs.
  params    Robot parameter files ("p3dx.p", for example) containing defaults
            for various robot types.  You can also add a custom parameter
			file here named for your robot's NAME (see ARIA reference manual).
  tests     Test files, somewhat esoteric but useful during ARIA development
  utils     Utility programs: file conversion and manipulation, and some tools
            used internally by MobileRobots for ARIA development and releases.
            See utils/README.txt for description of conversion utilities.
  python    Python wrapper modules and libraries
  pythonExamples - Example Python scripts and README file with details on
            how to use ARIA with Python.
  java      Java wrapper library
  javaExamples Example Java programs and README file with details on
            how to use ARIA with Java.
  maps      Some example map files that may be used with the simulator.
  matlab    Scripts and instructions that may be used to compile a simplified
            Matlab interface to ARIA.
  VSTemplates - Contains project templates for Visual Studio. Run the 
            included script to copy these templates to your home
			directory (Windows only)
  Command Prompt in ARIA Directory (Windows only) - Runs a Windows command
            prompt session that may be used to explore and run example
			programs from the ARIA directory.
  CommandLineOptions.txt - Summary of typical command-line options that 
            may be given to most ARIA programs
  LICENSE.txt        GPL license; agree to this to use ARIA
  Aria-*.sln    	   MS Visual C++ solutions for building ARIA and 'demo'.
  examples/All_examples-*.sln 
                  MS Visual C++ solutions for building ARIA examples.
  Makefile          (Linux only) GNU Make makefile (for building ARIA and examples
  Makefile_example  Example GNU Makefile (Linux only) that you can copy and
                    modify for your own project.
  INSTALL.txt       More information about installing ARIA
  README.txt        This file; also see READMEs in advanced/, examples/, and tests/
  Changes.txt       Summary of changes in each version of ARIA

Aria/ArNetworking/:  (A library used to facilitate network communication)
  docs      API Reference Manual for ArNetworking 
  examples  ArNetworking examples
  include   ArNetworking header (*.h) files
  src       ArNetworking source (*.cpp) files
  python    Python wrapper package
  pythonExamples - Python examples and instructions
  java      Java wrapper package
  javaExamples - Java examples and instructions
  
If you have installed any additional ARIA/Pioneer SDK libraries
then they will also be available as subdirectories of the Aria directory.

If you have installed ARNL, SONARNL or MOGS, then it will be in 
a separate installation (Arnl directory).



Building programs that use ARIA
===============================

Windows
-------

On Windows, use Microsoft Visual C++ 2012 (VC11), Visual C++ 2015 (VC14) 
for Windows Desktop, or Visual C++ 2017 (VC15) for Windows Desktop.
Free "Express" or "Community" versions of  Visual C++ can be 
downloaded from Microsoft (see microsoft.com or visualstudio.com), or 
full versions purchased.  Only Visual C++ is required. Other Visual 
Studio options such as "Universal" builds (for Windows 10, Phone and Tablet), 
C#, web development tools, .NET, SQL Server or database tools, Enterprise
tools, Silverlight, etc. are not required by ARIA or other 
Pioneer SDK libraries, and do not need to be installed.

Note: If you use a different version of Visual C++, you must rebuild 
the ARIA libraries. We recommand, test, and support only the above 
versions; other versions or other compilers may or may not work. If you
need to make any changes to use other compilers or compiler versions,
please share your experience with the aria-users mailing list.

You can start a new project from a template, or from scratch.

New Visual Studio Project from Template
---------------------------------------

To start a project from a template, first install copies of the
ARIA templates by running the "Install Visual C++ Templates.bat" script
from the Start menu -> All Programs -> MobileRobots -> Aria ->
Install Visual C++ Templates.  Exit Visual Studio if running.
Run Visual Studio.  Click on "New Project..." from the startup
page or the File menu.  Expand the "Installed", "Templates", "Visual C++", "ARIA" categories on the left, and select
"Program using ARIA Library".  Fill in your desired project names
and directory locations and click OK. An example program is included in the project which initializes ARIA, parses command-line arguments, attempts to connect to a robot, displays some data, then exits after sleeping for 3 seconds.  Modify this 
as desired.

The ARIA DLLs are in the ARIA "bin" directory (C:\Program Files\MobileRobots\Aria\bin).  
This directory can be added to the system PATH, or the appropriate
ARIA DLL copied to the same directory as your program executable, 
or you can place your program executable in the ARIA bin directory.

ARIA does not require the use of precompiled headers, and precompiled
ARIA headers are not provided.  

You do not need to include "stdafx.h" and "stdafx.cpp" in your project.

To make your program more portable, you can use a standard C++ definition
of your program's main function rather than WinMain or other 
Windows-specific conventions:

  int main(int argc, char **argv)
  {
      Aria::init();
	  
      ...
	  
	  Aria::exit(0);
	  returrn 0;
  }

All code that uses the ARIA API should include the Aria.h header file before
any other ARIA header files or header files from other Pioneer SDK libraries.
Aria.h will set up some preprocessor and type definitions needed by the rest
of the ARIA API:

  #include "Aria.h"

See the ARIA API reference documentation and example programs to 
get started writing your programs.

New Visual Studio Project from Scratch 
--------------------------------------

Begin by creating a new solution and C++ project for your 
application.   You may also add the appropriate ARIA project(s)
to your solution as well, as dependencies of your program.  This
will let your solution automatically rebuild the ARIA libary if 
neccesary.   Make sure to use the project files(s) corresponding
to the version of Visual C++ you are using.

Now you must configure your project to find ARIA. These instructions
assume a default installation; if you installed ARIA elsewhere you
must use different paths. If you keep your Visual Studo project within
ARIA's directory, you can also use relative paths (e.g. "..\lib" for the
library path).

ARIA DLL libraries are provided in two variants, "Debug" libraries that use 
the "Multithreaded Debug DLL" runtime library, and non-Debug or "Release" libraries
that use the plain "Multithreaded DLL" runtime library.  The ARIA solutions
use a "Debug" and a "Release" configuration to select these variants; you
can also use these configuration names as well if you wish.

It is also possible to build static ARIA libraries. These libraries use 
the "Multithreaded Debug" runtime library for Debug mode, or "Multithreaded"
for Release mode.  These libraries will include the word "Static" in their names.

You must set up the Debug configuration of your project to match the
ARIA Debug configuration, and also set up the Release configuration of
your project as well, to match the ARIA Release configuration.

The steps required to configure a new program project settings for use with 
the ARIA DLL libraries are as follows:

To edit your new project's configuration properties, right click on the 
project in the solution explorer pane and choose Properties.

1. Change the "Configuration" (upper left of the dialog) menu to "All
  Configurations".

2. Open the "General" section

  * Either set "Output Files" to "C:\Program Files\MobileRobots\Aria\bin" 
    (ARIA's bin directory, where Aria.dll etc. are), or put ARIA's bin directory
    in your system PATH environment variable, or copy the DLLs to your own
    project's output directory.
 
  * You may want to change Intermediate Files. ARIA projects use
    "obj\$(ConfigurationName)-<VC version>" to keep object files 
	separate and out of the way, but Visual C++'s default value is fine
	also.

  * Set "Common Language Runtime Support" to "No Common Language Runtime Support"

3. Open  the "Link" or "Linker" section.

 * To "Additional Library Path" add "C:\Program Files\MobileRobots\Aria\lib"
   (or other correct path to ARIA lib directory)
 
 * Open the "Input" subsection.

    * Change the "Configuration" menu (upper left of the window) to "Debug"

	* (Visual C++ 2017) To "Additional Dependencies", add:
        AriaDebugVC15.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingDebugVC15.lib
		
	* (Visual C++ 2015) To "Additional Dependencies", add:
        AriaDebugVC14.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingDebugVC14.lib 
		
	* (Visual C++ 2012) To "Additional Dependencies", add:
        AriaDebugVC11.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingDebugVC11.lib
		
	* ARIA does not require any of the Windows system libraries included
	  by Visual C++ as "Inherited values" in the Linker Additional Dependencies,
	  so you can uncheck "Inherit from parent or project defaults" after choosing
	  "Edit" from the drop-down menu next to the Additional Dependencies
	  field if you do not require any of those libraries in your program.

    * Change the "Configuration" menu to "Release"
		
    * (Visual C++ 2017) To "Additional Dependencies", add:
        AriaVC15.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingVC15.lib

	* (Visual C++ 2015) To "Additional Dependencies", add:
        AriaVC14.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingVC14.lib

	* (Visual C++ 2012) To "Additional Dependencies", add:
        AriaVC11.lib; winmm.lib; advapi32.lib; ws2_32.lib
      If you are using the ArNetworking library, also add:
        ArNetworkingVC11.lib
		
    * ARIA does not require any of the Windows system libraries included
	  by Visual C++ as "Inherited values" in the Linker Additional Dependencies,
	  so you can uncheck "Inherit from parent or project defaults" after choosing
	  "Edit" from the drop-down menu next to the Additional Dependencies
	  field if you do not require any of those libraries in your program.
		
    * Change the "Configuration" menu back to "All Configurations"

4. Click on the "C++" section.

  * Click on the "General" subsection

    * To "Additional Include Directories" add 
      "C:\Program Files\MobileRobots\Aria\include".
	 (or other correct path to ARIA's include directory)

   * Click on the "Code Generation" subsection

     * Change the "Configuration" menu (upper left of the window) to "Debug".

       * Under the "Use run-time library" pulldown select "Debug
         Multithreaded DLL".

     * Change the "Configuration" menu (upper left of the screen) to "Release".

       * Under the "Use run-time library" pulldown select "Multithreaded DLL".

     * Change the "Configuration" menu back to "All Configurations"
	 
To use the static libraries instead of DLLs, set the C++ runtime library to "Multithreaded" 
(Release configuration) and "Multithreaded Debug" (Debug configuration), 
list the static libraries for Additional Dependencies in the Linker Input section,
and also append ARIA_STATIC to the preprocessor definitions in the C++ -> Preprocessor
section (this prevents DLL "export" attributes from being added to method
declarations in the ARIA header files.)

The ARIA DLLs are in the ARIA "bin" directory (C:\Program Files\MobileRobots\Aria\bin
if installed).  This directory can be added to the system PATH, or the appropriate
ARIA DLL copied to the same directory as your program executable, or you can place 
your program executable in the ARIA bin directory.

ARIA does not require the use of precompiled headers, and precompiled
ARIA headers are not provided.  To disable the use of precompiled headers
in your project, open the "Precompiled Headers" subsection in the "C++"
section of your project's properties.  Change "Use Precompiled Headers"
to "no".  You can then remove "stdafx.h" and "stdafx.cpp" from your
project and remove the #include "stdafx.h" directive from your main 
source file.   

To make your program more portable, you can also change the definition
of your program's main function to standard C++:

  int main(int argc, char **argv)
  {
      ...
  }

All code that uses the ARIA API should include the Aria.h header file before
any othehr ARIA header files or header files from other Pioneer SDK libraries.
Aria.h will set up some preprocessor and type definitions needed by the rest
of the ARIA API.

  #include "Aria.h"

See the ARIA API reference documentation and example programs to 
get started writing your programs.


Linux
-----

On GNU/Linux two tools are used: the compiler (GCC), which compiles
and/or links a single file, and Make, which is used to manage 
building multiple files and their dependencies.  If you need to
deal with multiple files, you can copy a Makefile and modify it, 
or create a new one.

Note: In packages for Debian and Ubuntu, the shared libraries were built with the 
standard G++ version provided with the system. You must use 
the same version of G++ or a compatible version to build programs that 
link against it, or rebuild ARIA using your preferrend G++ version. (On
Debian and Ubuntu systems, the default g++ and gcc compiler versions may be changed
using the 'galternatives' program or the 'update-alternatives' command). 
In general, the 4.x series of GCC compilers are compatible with each other,
but not with 3.4 or other 3.x versions, and vice versa. We recommend using
the standard version of GCC on your Linux system, unless using an old 
version of Linux, (e.g. RedHat 7, Debian 3), in which case, we recommend
specifically using GCC 3.4.

When compiling ARIA or its examples, you may also temporarily override the 
C++ compiler command ('g++' by default) by setting the CXX environment 
variable before or while running make. E.g.: "make CXX=g++-3.4" or 
"export CXX=g++-3.4; make".

More information and documentation about GCC is available at 
<http://gcc.gnu.org>, and in the gcc 'man' page.  More information 
about Make is at <http://www.gnu.org/software/make/>. Several graphical 
IDE applications are available for use with GCC, though none are 
currently supported by MobileRobots.



### Using ARIA's Makefiles on Linux: ###

The ARIA Makefile is able to build any program consisting 
of one source code file in the 'examples', 'tests', or 'advanced' 
directory.  For example, if your source code file is 
'examples/newProgram.cpp', then you can run 'make examples/newProgram' 
from the top-level Aria directory, or 'make newProgram' from within 
the 'examples' directory, to build it.   This makes it easy to copy
one of the example programs to a new name and modify it -- a good way
to learn how to use ARIA.



### Using ARIA from your own Makefile or other build system: ###

If you want to keep your program in a different place than the ARIA 
examples directory, and use your own Makefile or other build tool,
you need to use the following g++ options to compile the program:

 -fPIC -I/usr/local/Aria/include 

If you wish, you may also use -g and -Wall for debugging information
and useful warnings.   ARIA does not use exceptions, so you may also
use -fno-exceptions if you wish; this will cause any use of exceptions
in your program to trigger a compile error.

For linking with libAria use these options:

  -L/usr/local/Aria/lib -lAria -lpthread -ldl -lrt

If you are also using ArNetworking, use the following compile option
in addition to the ARIA options above:

  -I/usr/local/Aria/ArNetworking/include
  
And for linking to libArNetworking:

  -lArNetworking


A Makefile for a program called "program" with source file "program.cpp"
might look something like this:


  all: program

  CFLAGS=-fPIC -g -Wall
  ARIA_INCLUDE=-I/usr/local/Aria/include
  ARIA_LINK=-L/usr/local/Aria/lib -lAria -lpthread -ldl -lrt

  %: %.cpp
	  $(CXX) $(CFLAGS) $(ARIA_INCLUDE) $< -o $@ $(ARIA_LINK)


If you run the `make` command in the same directory as this Makefile,
then GNU Make will check `program.cpp` for changes, and if newer than
`program`, will attempt to build the target program named "program"
using this compiler command, based on the expansions of the variables and
the target and dependency patterns which match "program" and "program.cpp":

  c++ -fPIC -g -Wall -I/usr/local/Aria/include program.cpp -o program -L/usr/local/Aria/lib -lAria -lpthread -ldl -lrt

Refer to the GNU Make manual <http://www.gnu.org/software/make> or 
other books or documentation about Make to learn more.

See the ARIA API reference documentation and example programs to 
get started writing your programs.


### Setting variables for ARIA Makefiles ###

You can add compile options to the following variables used in the ARIA
Makefiles, or replace default values. These can be set on the command
line when running `make`, for example:

  make CXX="g++-4.6"

These will be used when building or reduilding the ARIA library as well
as example and test programs from examples/ and tests/. ArNetworking also
has similar variables.


EXTRA_CXXFLAGS    Additional compile flags passed to the C++ compiler. You
            can add GCC options to enable profiling, optimizations, etc.  
            For example, to rebuild ARIA optimized for the Atom CPU
            found in the LX/MTX embedded computer, add -mtune=atom and -O2:
              make clean; make EXTRA_CXXFLAGS="-mtune=atom -O2"
            Or, to include all symbols to see more information in the
            output of ArLog::logBacktrace():
              make EXTRA_CXXFLAGS="-O0 -rdynamic"
            Or to enable profiling:
              make EXTRA_CXXFLAGS="-pg -fprofile-arcs"
            These compiler flags will be used in addition to ARIA's 
            default flags.

CXX         Specify the C++ compiler command to use. The default is g++ or c++.
            You can use this to use a different version of g++, or to give the
            full path to g++, or to try a different compiler other than G++.

JAVAC       If rebuilding the Java wrapper library, this specifies the Java 
            compiler command to use. Default is "javac".

JAR         If rebuilding the Java wrapper library, this specifies the JAR
            archiver command to use. Default is "jar".

JAVA_BIN    If rebuilding the Java wrapper library, this specifies a directory
            containing JDK commands such as javac and jar. 

SWIG        If rebuilding the Java and/or Python wrapper libraries, this
            specifies the swig command used.  Default is swig.

PYTHON_INCLUDE  
            If rebuilding the Python wrapper library, this specifies the Python
            include directory. Default is /usr/include/python2.5. You usually
            need to specify this if building on a Linux other than Debian 5.

INSTALL_DIR
SYSTEM_ETC_DIR
            Directories used when installing ARIA from source with the 
            `make install` command.

INSTALL     How to run the `install` tool. Default is "install --preserve-timestamps"



### Learning more about using Linux:###

If you are new to using GNU/Linux, some guides on getting started can be 
found at the following sites:

 * If your robot is using Debian, start with 
   <http://www.us.debian.org/doc/manuals/debian-tutorial>. More is at
   <http://www.debian.org/doc/>.

 * If your robot is using Ubuntu, refer to <http://help.ubuntu.com/> 
   for documentation and help.

 * <http://www.tldp.org> is a repository of many different guides, FAQ and
   HOWTO documents about Linux in general, including various programming
   and system administration tasks. Note however that many details can 
   differ between Linux distributions and not all HOWTO documents will
   be relevant to your distribution.

 * Ask Ubuntu and other question-answer sites can be searched to find answers to 
   problems using Linux, or to ask questions: 
    * <http://www.askubuntu.com>
    * More Stack Exchange sites are at <https://stackexchange.com/sites>

For more depth, there are many books about using Linux, consult your
local computer bookseller. The ideal way to learn about Linux is to work 
with an experienced colleague who can demonstrate things and answer 
questions as they arise.


Mac OSX
-------
 
It is possible to compile ARIA from source code on Mac OSX 10.04 (Tiger)
or later. 

First install XCode, then install the command line development tools:

  1. Run XCode
  2. Open "Preferences..." from the "XCode" application menu
  3. Select the "Downloads" tab
  4. Select "Command Line Tools"
  5. Click "Install" button for Command Line Tools.

Now you can build ARIA by running the "make" command inside the unpacked
ARIA source code directory from Terminal.

When running programs using the ARIA shared library, the "lib" directory
containing the shared library (.so file) must be in your DYLD_LIBRARY_PATH.
For example, if you unpacked ARIA-2.9.4.tgz in you home directory 
and compiled ARIA, then use this command:
   export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:~/Aria-2.9.4/lib



Learning C++
------------

If you are new to C++ programming, the definitive guide is Bjarne
Stroustrup's book "The C++ Programming Language".   The book
"C++ In a Nutshell", published by O'Reilly & Associates, is a
good quick guide and reference. There are also several websites
and many other books about C++.   You can search Stack Overflow
<http://www.stackoverflow.com> and other question-answer sites, 
or ask new questions if your question has not previously been
asked and answered.


Using ARIA from Java or Python
==============================

Even though ARIA was written in C++, it is possible to use ARIA from
the Java and Python programming languages as well. The directories
'python' and 'java' contain Python and Java packages automatically generated
by the 'swig' tool (http://www.swig.org) which mirror the ARIA API
closely, and are "wrappers" around the native ARIA library: When you call
an ARIA function in a Python or Java program, the wrapper re-marshalls the
function arguments and makes the equivalent call into the C++ ARIA library.
See the 'pythonExamples' and 'javaExamples' directories for more information
and example programs.  See the ARIA API Reference Manual in 'docs' for 
some more information.

More about the Python programming language is at <http://www.python.org>.
More about the Java programming language is at <http://java.sun.com>.


Using ARIA from Matlab or Simulink
==================================

As of ARIA 2.8, it is possible to use a small but essential subset of 
ARIA features from Matlab and Simulink.  This is done via MEX functions 
and a pure-C wrapper library called ariac.   Matlab functions and Simulink
blocks are provided for essential robot operation such as velocity control,
receiving position estimate and other basic information.  New functions
can be added to this interface using the already-implemented functions
as examples.

See matlab/README.md for details.

See ArNetworking/matlab/README.md for details on sending commands
to an ArNetworking server from Matlab.


MobileSim Simulator
===================

SRIsim is no longer included with ARIA.  There is now a seperately
downloadable MobileSim simulator at our support webpage 
<http://robots.mobilerobots.com>.  MobileSim is compatible with SRISim,
but adds many new features.




-------------------------
Adept MobileRobots
10 Columbia Drive
Amherst, NH, 03031. USA

support@mobilerobots.com
http://robots.mobilerobots.com
http://www.mobilerobots.com



ARIA Installation Instructions
===============================
Version 2.9.4

Don't forget to sign up for the Aria-Users mailing list by sending a message 
with the subject 'subscribe' to aria-users-request@mobilerobots.com.

ARIA software development is supported on Windows using MS Visual
C++, and on GNU/Linux using GNU development tools (make, GCC).  It is
possible to build ARIA on other Posix/Unix-like systems but this is
not officially supported or as frequently tested as Linux and Windows. 
You can discuss use of ARIA on other operating systems or build environments
on the aria-users group mailing list.

See README.txt for more information.


Windows:
--------

Run Aria-2.9.4.exe to invoke the setup program, then simply
follow the prompts.  All ARIA software will be installed in 
C:\Program Files\MobileRobots\Aria.  Shortcuts will be installed
in the Start menu to documentation, examples, and the Aria directory.

The kit includes the Aria library and two pre-built examples
that you can run from the bin directory. More examples are available in the 
examples directory. Visual C++ projects are provided to build (most) example
programs, as well as for ARIA itself.

In order to compile ARIA and the examples, you should use Microsoft 
Visual C++.  Free "Express" versions are available from Microsoft.
For more information see also http://robots.mobilerobots.com/wiki/Windows_C%2B%2B_Compilers

See README.txt and the ARIA API Reference documentation for important details
on how to build code on Windows vith Visual C++ and get started using
ARIA.


GNU/Linux:
----------

To use the precompiled Aria library under Linux, you will need GCC
(with the C++ compiler, g++) and Make.  The ARIA Ubuntu packages
come built with the standard GCC version for their distribution platform
(Indicated in the package file name).  If you would like to rebuild them with a 
different compiler or version, run "make clean; make" from the top 
level /usr/local/Aria directory, or use the source package.  

You can set environment variables to affect compilation with make, such as 
CXX and CXXFLAGS (see README.txt). For example, if you want to use the 
command "g++-3.4" to compile, set CXX to g++-3.4 and export the variable. 
See README.txt for more discussion of this.

The ARIA and ArNetworking libraries and some pre-built examples are included
in the Ubuntu packages.  You can run these examples and find more examples 
in the examples directory.  All ARIA software is placed in /usr/local/Aria.  
In addition, the file /etc/Aria will be created with the ARIA installation 
location, and /etc/ld.so.conf may be updated to include /usr/local/Aria/lib. 



To install on Debian or Ubuntu:
-------------------------------

Binary (pre-built) packages are available for Ubuntu 12 and later.

The libaria package provides libArNetworking as well 
as libAria, and all documentation, source code, and example programs.
libaria-python and libaria-java provide the Python and Java wrapper 
libraries respectively.

To install a .deb package file downloaded from the website, use dpkg.
For example:
   dpkg -i libaria_2.9.4+ubuntu12_i386.deb

Both 32-bit and 64-bit packages are provided. If you are on a 32-bit
operating system, use the package with "_i386" suffix. If you are on
a 64-bit system, use the package with "_amd64" suffix. To check
the architecture of your Debian or Ubuntu system, run the following command:
 dpkg-architecture  -qDEB_HOST_ARCH
Or open the System Settings GUI, and open Details.  The architecture
will be listed as "OS type".  

Aria will be installed in /usr/local/Aria.

You can install any required dependencies using apt-get, aptitude, 
synaptic, or the Ubuntu Software Center.

To uninstall the ARIA libraries, use 'apt-get remove libaria'. Any
packages that depend on libaria will also be removed.



To compile from source code and install on other platforms:
-----------------------------------------------------------

You may download the source code as a compressed 'tar'  or Zip 
archive and compile Aria yourself (See README.txt for notes 
about compiling the source).

Unpack the .tar.gz archive:
   tar xzvf ARIA-src-2.9.4.tgz

Enter the new directory, and use make to build ARIA:
   cd Aria-src-2.9.4
   make

Or if you are on a multicore or multiprocessor computer, build
with multiple processes in parallel:
   make -j4

To also make the ArNetworking librarie:
   cd ArNetworking
   make

To install Aria, become the root user and use make install:
   sudo make install

Aria will be installed in /usr/local/Aria.   The dynamic (shared) 
libraries are in /usr/local/Aria/lib and the header files are in
/usr/local/Aria/include and /usr/local/Aria/ArNetworking/include.

It is possible to build static libraries instead. From the Aria directory,
run:
  make lib/libAria.a
  make lib/libArNetworking.a
  cd examples
  make demoStatic

Next, you will then need to tell the dynamic linker where to find the ARIA 
libraries: For example, on Linux, either add "/usr/local/Aria/lib" to 
/etc/ld.so.conf for permanent effect, or add it to the LD_LIBRARY_PATH 
environment variable for temporary effect.  On Mac OSX, add the lib 
directory to DYLD_LIBRARY_PATH environment variable. Other platforms
will need other similar settings either in the OS or your application
project to locate the ARIA libraries.

The ARIA libraries depend require only the following standard Linux 
system libraries on Linux-based systems: libpthread, libdl 2, librt 1, 
libm 6, libc 6, libstdc++ 6.  

To compile ARIA, you will need GCC, G++ and GNU Make.

These are all standard system libraries and development tools, so on 
most current systems you should not need to install anything extra to 
compile and use ARIA (other than g++ and make if not yet installed.)

There is some support in ARIA for compiling on Mac OSX and MinGW, but
this is not regularly tested and verified. Contact the aria-users forum
for work with other users on supporting these and any other platforms. 


More Information
----------------

For documentation and information about using ARIA and ArNetwtorking,
refer to README.txt, the ARIA reference manual at doc/index.html, and
the ArNetworking reference manual at ArNetworking/doc/index.html. Information 
is also available on the support website http://robots.mobilerobots.com .


------------ 
Adept MobileRobots 
support@mobilerobots.com
http://www.mobilerobots.com
http://robots.mobilerobots.com

