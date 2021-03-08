
TODO
====

* Add [[deprecated]] attribute to deprecated methods
* Update tests to remove use of ArSimpleConnector and fix C++ errors/warnings.  (HELP WANTED)
* Provide refactoring tips and insttructions to users to transition existing
  code due to all changes below.
* Switch to CMake?
  * use CMAKE_EXPORT_COMPILE_COMMANDS to export compile_commands.json that analysis tools etc. might use (eg clang-tidy)
* Use namespace?, remove class prefixes? 
* Change installation locations to match current OS standards (Linux and
  Windows)
* Change robot parameter file loading to check user home directory 
* Remove `ArStringInfoGroup` (merge with data logger, let classes
  supply data accessors for data logger or other higher level uses 
  such as UI, middleware, etc.)
* Remove barriers to having simpler wrapper libraries (swig and non-swig)
* Separate into multiple libraries? (with optional full aria build also for
  compatibilty)?  -- aria-core, aria-devices, aria-actionlib? 
* Continue to reduce number of classes needed for typical applications.
  Reduce boilerplate needed on new programs. simpleConnect example should be <10
  lines of code.
* Reduce header dependencies. Fewer files should include Aria.h, ideally none -- 
  Aria.h should only be for users who need a single file that includes
  everything for convenience.
* Remove `ArDrawingData` (move to ArNetworking registry of what sensor
  visualizations should look like, maybe configurable in server config file)
* [IN PRG] Remove various features in ARIA that are needed for ArNetworking/ARAM only.
* Remove some more classes:
  * ArActionKeyDrive? ArRatioInputKeydrive? ArKeyHandler? (Move into demo.cpp like ArModes was)
  * Move rest of classes from ArActionGroups.h/ArActionGroups.cpp into  
     inlined classes in examples/ActionGroups.h used by relevant examples.
* overhaul other documentation.  
* Add more data logger features.
* Consolidate `src/*.cpp` and `include/*.h` in `aria/` subdirectories? should
  match header installation directory.  
* Move `matlab/ariac` to `ariac/`. Update matlab and ariac-rust builds.
* Remove weird stuff in ArSystemStatus
* Set up Travis or Jenkins to test build on checkin (later also run tests).
* Start adding some real unit testing?
  * use doctest in python examples and tests, or in an examples/tests file?
  * Use unit tests to check against regressions as we do the refactoring in this
    list.
* Replace some of the fixed size numeric typedefs with C/C++ standard types
  (e.g. `UByte` to `uint8_t`, etc.
* Use more modern portable C++/stdlib features (including tests that new standard library usage is equivalent to older usage)
  * Replace (deprecate) ArTime and ArUtil::getTime(), ArUtil::putCurrentXXXXInString(), ArUtil::parseTime(), ArUtil::localtime(),  with std::chrono etc.
  * Replace/deprecate some ariaTypedefs.h, ArUtil and ArMath functions:
    * ArTypes::Byte -> int8_t
    * ArTypes::Byte2 -> int16_t
    * ArTypes::Byte4 -> int32_t
    * ArTypes::Byte8 -> int64_t
    * ArTypes::UByte -> uint8_t
    * ArTypes::UByte2 -> uint16_t
    * ArTypes::Ubyte4 -> uint32_t
    * ArTypes::Ubyte8 -> uint64_t
    * move ArListPos from ariaTypedefs.h into ariaUtil.h, remove ariaTypedefs.h.
  * smart pointers
  * Use standard `<thread>` library for threads rather than our own
  * Replace ArFunctor usage with std::function, lamdas and other standard features
  * ArRangeBuffer and other collections should perhaps implement iterator or STL container
    interface or provide more access to underlying standard containers/iterators. This makes them directly usable with standard algorithms and C++20 range/view.
  * Find more opportunities to use improved STL algorithms including parallel.
  * Verify that frequently used storage types like ArPose, RangeBuffer, etc. are compatible with move semantics
  * Use C++17 filesystem library. Remove file/directory functions from ArUtil.  
  * Replace use of atof, atoi etc. (and ArUtil wrappers) with std::stod etc. 
  * Use modern smart pointers
* Mark some very frequently used inline methods noexcept? (because library might (or might not) have been compiled with -fno-exceptions but user applications probably won't be)
* Javascript (NodeJS) wrapper via SWIG (HELP WANTED)
* Automated Rust wrapper via ritual or SWIG or other tool (HELP WANTED)
* Keep moving internal stuff into .cpp files, or if headers need to be shared
  into an internal include directory inside src.  Only public API should be in
  include/
* Keep finding 'protected' members to make 'private'.  
* Keep removing AREXPORT from inline and private members.
* Add modern packaging to python wrapper (HELP WANTED)
* Add better rust packanging to rust wrapper (HELP WANTED)
 

Maybe TODO eventually
----------

* Simplify ArMap.  (Restore from older version of ARIA without "Components"?)
* Possibly refactor ArRobot to be a smaller class. Separate robot communications
  and control/data API from other higher level features
* Replace old ArModuleLoader with similar with more useful/desirable features
* Remove ArActions or move to another library.
* Move device interfaces to separate libraries?
* Add proper unit tests.
* Incorporate some ArNetworking replacement.  Generic mechanism for ARIA classes
  to expose methods (actions/callbacks) to UI or middleware layers.  (With multiple
  implementations possible in other libraries or code units, e.g. user custom, ROS, HTTP REST, etc.)
* Develop logging (ArLog) further.
* enums in config items


