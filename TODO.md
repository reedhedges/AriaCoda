
TODO
====

* Edit (review and improve) README
* Add [[deprecated]] attribute to deprecated methods
* Update tests to remove use of ArSimpleConnector and fix C++ errors/warnings.  (HELP WANTED)
* Provide refactoring tips and insttructions to users to transition existing
  code due to all changes below.
* In often-used container classes, can we remove use of std::string members or
  other members that make them non-trivial? Can we then use member default
  initialization in declaration rather than a constructor?  Can we replace use of raw pointers
  with `shared_ptr`? Can we replace use of allocated memory that is freed from a
  raw pointer in destructor with `unique_ptr` for automatic deallocation?
* In all classes with destructors (e.g. virtual classes), set copy/move
  constructors and assignment operators to delete, default, or implement.
* Replace classes that only contain enums with "enum class".  Fix any non-scoped
  enums as well (I don't think there are any.)
* Switch to CMake?
  * use CMAKE_EXPORT_COMPILE_COMMANDS to export compile_commands.json that analysis tools etc. might use (eg clang-tidy)
* Move examples and tests Makefile rules into their own Makefiles (with their
  own dependencies)?  (Makes CI builds slightly faster if we skip examples and
  non-unit tests)
* Use namespace?, remove class prefixes? 
* Change installation locations to match current OS standards (Linux and
  Windows)
* Change robot parameter file loading to also check user home directory 
* Remove `ArStringInfoGroup` (merge with data logger, let classes
  supply data accessors for data logger or other higher level uses 
  such as UI, middleware, etc.)
* Split robot config value storage (public ArRobotConfig) from
  ArRobotConfigPacketReader (becomes internally used only). (Note there was an 
  old "ArRobotConfig" class but nothing actually ever used it.) 
* Remove barriers to having simpler wrapper libraries (swig and non-swig)
* Separate into multiple libraries? (with optional full aria build also for
  compatibilty)?  -- aria-core, aria-devices, aria-actionlib? 
* Continue to reduce number of classes needed for typical applications.
  Reduce boilerplate needed on new programs. simpleConnect example should be <10
  lines of code.  ArRobot should be able to do default connection behavior
  with one call, same with laser.  ArRobot.connect()? Aria::connectRobot(robot)?
  Aria::init() should take argc and argv to create an ArArgumentParser.
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
* Add explicit "override" specifier 
* overhaul other documentation.  
* Add more data logger features.
* Consolidate `src/*.cpp` and `include/*.h` in `aria/` subdirectories? should
  match header installation directory.  
* Move `matlab/ariac` to `ariac/`. Update matlab and ariac-rust builds.
* Remove weird stuff in ArSystemStatus
* Start adding some real unit testing?
  * use doctest in python examples and tests, or in an examples/tests file?
  * Use unit tests to check against regressions as we do the refactoring in this
    list.
  * Use catch2, doctest, googletests?
  * Use ApprovalTestsCPP)?
    * Add text serialization for some or all objects so we can do operations on
      them and verify important data contents?  (Or should text used for
      approval testing be the return values of various calls?)
      (Does not need to be inside object, can be a separate operator<<
      specialized for it that only lives in the test code, but maybe it's a useful
      tool for live debugging to have in the library so it could be a method in
      the object or operator<< in a separate file.)
    * Can be used just for smaller unit tests of individual functions etc
      especially with multiple parameter combinations.
  * Send simulator commands to put the robot in certain states where we can
    check robot and sensor data received.
    * Use ApprovalTests CPP to compare laser and sonar data captures with known correct
      laser data set for that state/situation?
    * Needs standard maps (e.g. a box) in the simulator, a way to turn off
      odometry error remotely, etc.
    * Can we do some tests without the simulator? (May need to check that e.g.
      pose estimate or calculated velocity is within a tolerance vs. commanded
      velocity or expected true pose)
    * This is as much a test of the simulator as ARIA! 
  * Or resurrect ArFileConnector and read recorded Pioneer protocol sessions and
      check state at end or at certain points in the session.  (E.g. a special 
      "MARK" packet triggers a callback, or we verify at every robot loop).
      Should add easy way to record session to file, including from demo. 
      See more below.
  * Check ArConfig input/output
  * Check potentially tricky API calls for breaking changes/regressions
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
  * Replace ArFunctor usage with std::function.
    * Users can provide any bare function, which is converted to std::function
    * Users can provide a lambda
    * Users can provide a callable object with an operator().
    * Users can use `std::bind` with placeholder arguments, or `std::bind_front` (C++20) to provide an object instance
      bound to a member function (bind the method to a class instance), 
    * e.g.:
        class ArRobot { 
          ... 
          void addUserTask(std::function<void(void)> &callback);
          ...
        };
        ...
        robot.addUserTask( []() { cout << "Hello world\n"; } );

        // Note: must ensure `myinstance` is not destroyed before lambda andl/or
        // removed as user task from `robot`.
        auto l = [&myinstance])() { cout << myinstance.mymethod(); } );
        robot.addUserTask(l);

        robot.addUserTask(std::bind(&MyClass::mymethod, myinstance, std::placeholders::_1);

        robot.addUserTask(std::bind_front(&MyClass::mymethod, myinstance));
        ...
    * See <https://godbolt.org/z/Ts6nax> for experimenting with std::function and std::bind_front.
    * Can we bind argument values and store it  for later in the std::function
      object?  If so can we later change the argument values? These feature of ArFunctor isn't really used much.
      Classes that need to do this could store a tuple of arguments and use std::apply
    * Conversion functions can be written to convert deprecated ArFunctor types to std::functions
      in C++20 like this:
        template <class CT, typename ArgT>
        std::function<void(ArgT)> toStdFunction(const ArFunctor1C<CT, ArgT>& functor) {
          return std::bind_front(functor.myFunc, functor.myObj);
        }
      Or, pre-C++20 like this:
        template <class CT, typename ArgT>
        std::function<void(ArgT)> toStdFunction(const ArFunctor1C<CT, ArgT>& functor) {
          return std::bind(functor.myFunc, functor.myObj, std::placeholders::_1);
        }
      And ArGlobalFunctor classes can be typedef'd directly to std::functions
      like this:
        template <typename T>
        using ArGlobalFunctor1 = std::function<void(T)>;

        template <typename Ret, typename Arg>
        using ArGlobalRetFunctor1 = std::function<Ret(Arg)>;
    * Note, should we be using std::ref or function_ref for the above? (Or only
      needed if target is an object with operator(), rather than function
      pointer or lambda?)
    * The other feature ArFunctor has is a name that can be used for debugging.
      But maybe we can use some introspection functions to get string
      representations of the function type names and target type name. Or wrap
      it in a small container class with a subclass containing an optional name.
  * ArRangeBuffer and other collections should perhaps implement iterator or STL container
    interface or provide more access to underlying standard containers/iterators. This makes them directly usable with standard algorithms and C++20 range/view.
  * Find more opportunities to use improved STL algorithms including parallel.
  * Verify that frequently used storage types like ArPose, RangeBuffer, etc. are compatible with move semantics
  * Use C++17 filesystem library. Remove file/directory functions from ArUtil.  
  * Replace use of scanf, atof, atoi etc. (and ArUtil wrappers) with
    std::stod, std::strtod, std::from_chars, etc.  Replace use of sprintf with
    std::to_chars or sstream with format manipulators, or std::format (coming in
    C++20, but there is also https://github.com/fmtlib/fmt in the mean time)
  * Use `std::string` and `string_view` more frequently rather than `char*`.
  * Use std::array instead of C arrays (or known-size vectors, but these seem to
    not occur or be very rare in ARIA)

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
* Fix all the sprintf errors in demo.cpp and ARIA (e.g. it's trying to use sprintf to include
  a copy of the previous string value in a new value, or append new formatted
  values to the previous string value, which is a potential bug
  and newer compilers warn about it. Instead a new string should be formatted
  then appended with strcpy or similar.) Or use std::string/IO streams, or
  std::format (C++20 feature, not yet implemented in gcc, clang or msvc as of
  2/21) to build the string. 
* Change ArLog to use c++ io instead of c io.  Use `clog` as output stream
  instead of stderr and fflush.  
 

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
* Develop logging (ArLog) further. Or replace with spdlog and {fmt}. 
* enums in config items


