
ARIA Utility Programs
=====================

You can build any program in this directory from the ARIA directory, or from
this directory.  E.g.:

     make utils/convertArMapToDXF

Some utilities have external dependencies, see below.

See the utility source code or run with -help command line option for more
information about each.

convertArMapToDXF
-----------------

Very basic conversion from the ARIA map file format to DXF.  Sufficient to get
any LINES present in the ArMap into a minimal DXF file that may be imported into
other graphics or mapping tools.

convertBitmapToArMap
--------------------

Convert pixels in a PNM, PPM, PBM or PGM bitmap file into points in an ARIA
map that can then be used with MobileSim, ARNL, MOGS or ARIA.  Requires the
"libnetpbm" library to be installed on the system to build.  Once built,
run with --help option for more information.


convertSfWorldToArMap
---------------------

Convert old Saphira "world" file to ARIA ArMap format.

recenterArMap
-------------

Translates positions of all LINES, POINTS and objects in an ArMap by given
amounts.



Internal Utilities
------------------
These utilities are used internally by MobileRobots for ARIA development and
release packaging:

* genCommandLineOptionDocs
* makeParams


