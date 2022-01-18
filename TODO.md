
TODO
====

If you would like to contribute, consider items below labelled "HELP WANTED", or
any improvement or change that would make this library more useful for you or
easier to use.  Contact me or discuss on the GitHub page.

* Build as libariacoda.so instead of libAria, with Debian packages named
  libariacoda3. (Avoid conflict with existing libaria in Ubuntu). Make a
  libAria.so symlink to help work with existing builds.
* Add example of `ExternalProject` for CMake to README.
* In packet receivers e.g. ArRobotPacketReceiver, return ArRobotPacket by value
  as data, with all callers using std::move to move it to temporary or to store it?
* In ArRangeDevice, store myRawReadings and myAdjustedRawReadings as
  `std::list<ArSensorReading>` rather than `std::list<ArSensorReading*>` and don't allocate
  each one. Use `emplace_back` when creating a new ArSensorReading objects for the
  list. [It looks like all uses of myRawReadings etc. are allocating new
  ArSensorReading objects with "new", not borrowing/sharing pointers to existing
  ArSensorReading objects. (DONE?)
* Fix virtual inheritance.  (Remove unnecessary virtual inheritance, make sure
  done correctly, etc.) (IN PROGRESS)
  * Virtual inheritance must be used for
  interface implementations, i.e. subclasses of:
    * ArRangeDevice
    * ArPTZ
    * ArDeviceConnection
    * ArGPS
    * ArFunctor
    * ArAction
    * ArRobotTypes?
    * ArBasePacket since there are places where an interface or more general class
      obtains an ArBasePacket from an implementation's reader/parser and passes it to a handler generically.
      (E.g. `ArPTZ::sensorInterpTask()`).  This design would have to be modified to avoid this.
  * But not extensions of base classes that just provide common functionality,
  such as:
    * ArASyncTask
    * ArPose and other utility classes with extensions (eg.. ArPoseWithTime)
    * ArRobotTypes?
  * Some base classes of virtual subclasses may need things like copy
    constructors deleted.  Review.  Make sure copying is disabled (deleted) or
    implemented correctly.
* There are no exceptions in ARIA.  Mark functions where it might matter as
  noexcept (eg for optimization).
* Bug: does not close robot connection when TCP connection write fails because
  remote side (ie sim) closed. (HELP WANTED)
* Change size parameter to read/write methods in ArDeviceConnection (and
  subclasses) to `size_t`.
* Use new laser simulator commands in ArSimulatedLaser instead of old SRISim
  commands (HELP WANTED)
* Edit (review and improve) README
* Remove exceptions from ArPacketUtil and re-add -fno-exceptions to build (and
  disable exceptions on Windows?).  Merge ArPacketUtil functionality into
  ArBasePacket (without impacting performance of packets)
* Add `[[deprecated]]` attribute (or `DEPRECATED` macro) to deprecated methods (DONE?)
* Update tests to remove use of ArSimpleConnector and fix C++ errors/warnings.  (HELP WANTED)
* Provide refactoring tips and instructions to users to transition existing
  code due to all changes below.
* Use std::optional in ArGPS and other device classes that may or may not
  have data available (and/or use expected)
* Introduce unit types (mm, m, deg, rad, mm/s, deg/s, etc. with custom literals
  and conversions) (and use std::chrono types) ? Especially useful for time
  (since we sometimes use seconds, sometimes miliseconds). Include formatted
  output functions (operator<<).  See
  <https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/>
  and <https://www.fluentcpp.com/2017/05/26/strong-types-conversions/>
* In often-used container/utility classes that would be frequently
  created/destroyed/copied, can we remove use of std::string members or
  other members that make them non-trivial? Can we then use member default
  initialization in declaration rather than a constructor?  Can we replace use of raw pointers
  with `shared_ptr`? Can we replace use of allocated memory that is freed from a
  raw pointer in destructor with `unique_ptr` for automatic deallocation?
* In all classes with destructors (e.g. virtual classes), set copy/move
  constructors and assignment operators to delete, default, or implement. (IN PROGRESS)
* Replace classes that only contain enums with "enum class".  Fix any non-scoped
  enums as well (I don't think there are any.)
* Switch to build2, CMake, or Meson (or other?) (HELP WANTED)
  * use `CMAKE_EXPORT_COMPILE_COMMANDS` to export `compile_commands.json` that analysis tools etc. might use (eg clang-tidy)
* Move examples and tests Makefile rules into their own Makefiles (with their
  own dependencies)?  (Makes CI builds slightly faster if we skip examples and
  non-unit tests)
* Use namespace?, remove class prefixes?
* Change installation locations to match current OS standards (Linux and
  Windows) (IN PROGRESS)
* Change robot parameter file loading to also check user home directory
* Remove `ArStringInfoGroup` (merge with data logger, let classes
  supply data accessors for data logger or other higher level uses
  such as UI, middleware, etc.) Remove "Holder" classes.
* Split robot config value storage (public ArRobotConfig) from
  ArRobotConfigPacketReader (becomes internally used only). (Note there was an
  old "ArRobotConfig" class but nothing actually ever used it.)
* Remove barriers to having simpler wrapper libraries (swig and non-swig)
* Separate into multiple libraries? (with optional full aria build also for
  compatibility)?  -- aria-core, aria-devices, aria-actionlib?
* Continue to reduce number of classes needed for typical applications.
  Reduce boilerplate needed on new programs. simpleConnect example should be <10
  lines of code.  ArRobot should be able to do default connection behavior
  with one call, same with laser.  ArRobot.connect()? Aria::connectRobot(robot)?
  Aria::init() should take argc and argv to create an ArArgumentParser.
* Remove `ArDrawingData` (move to ArNetworking registry of what sensor
  visualizations should look like, maybe configurable in server config file)
* [IN PRG] Remove various features in ARIA that are needed for ArNetworking/ARAM only.
* Remove some more classes:
  * ArActionKeyDrive? ArRatioInputKeydrive? ArKeyHandler? (Move into demo.cpp like ArModes was)
  * Move rest of classes from ArActionGroups.h/ArActionGroups.cpp into  
     inlined classes in examples/ActionGroups.h used by relevant examples.
* Add explicit "override" specifier to virtual methods that override [IN PRG]
* overhaul other documentation.  
* Add more data logger features.
* Move `matlab/ariac` to `ariac/`. Update matlab and ariac-rust builds.
* Remove weird stuff in ArSystemStatus
* Use namespaces instead of classes with only static members.
* Analyze important areas for performance and improve.
  * Serial and network IO
  * ArSyncTask (simplify "task tree" concept?) main loop.  This is really only
    used by ArRobot, no external uses use ArSyncTask AFAIK.  
  * ArRobot command and SIP processing
  * ArAction handling. (e.g. remove unused customizability of action resolver
    implementation.)
  * Reduce ArAction footprint.  Remove little-used features (arguments,
    getDesired(),  passing currentDesired() in to file() (or make it a const
    reference.) 
  * Sensor data processing (Some work already done in ArRangeBuffer)
* Start adding some real unit testing? (HELP WANTED)
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

* Use more modern portable C++/stdlib features (including tests that new standard library usage is equivalent to older usage)
  * Replace (deprecate) ArTime and ArUtil::getTime(), ArUtil::putCurrentXXXXInString(), ArUtil::parseTime(), ArUtil::localtime(),  with std::chrono etc.
  * Replace/deprecate some ariaTypedefs.h, ArUtil and ArMath functions: (DONE)
    * `ArTypes::Byte` -> `int8_t`
    * `ArTypes::Byte2` -> `int16_t`
    * `ArTypes::Byte4` -> `int32_t`
    * `ArTypes::Byte8` -> `int64_t`
    * `ArTypes::UByte` -> `uint8_t`
    * `ArTypes::UByte2` -> `uint16_t`
    * `ArTypes::Ubyte4` -> `uint32_t`
    * `ArTypes::Ubyte8` -> `uint64_t`
    * move `ArListPos` from `ariaTypedefs.h` into `ariaUtil.h`, remove `ariaTypedefs.h`. (NOT DONE)
  * Use `std::byte` for ArUtil::BITS types and bit manipulation? (C++17)
  * smart pointers (`std::shared_ptr`)
    * Start with various classes that take and store a pointer to another object in
      constructor, stored for the life of the object. Usually this is an
      `ArRobot*`, `ArRobotConnector*`, `ArLaserConnector*`, etc. Replace with
      unique_ptr for ownership, `shared_ptr` for shared reference.
    * If a class allocates and stores an object internally, replace with value
      rather than pointer, or use `unique_ptr`.  If it provides external access to
      that, return a const or non const reference (or `unique_ptr`?)
    * Fix accessors that take pointers as arguments but do not store the
      pointer. Most of these could be references? Or `const unique_ptr &`?
      If we were consistently following recommended guidelines, all `T*`
      parameters would be indirect reference for "observation" only, with no storage,
      but ARIA has lots of `T*` arguments for various purposes.
    * If a class is going to replace (reseat) an object, pass `unique_ptr<T>&`
      and document clearly. (I don't think this is ever done in ARIA?)
    * Fix accessors that return pointers to internally stored objects.  
      Use std::optional/nullopt if they only do that so they can return NULL
      for missing/uninitialized/error.
    * Fix accessors that return pointers to allocated objects. Some of these
      must be owned by the object that created them, others could be managed
      globally/allowed to live beyond creating object.
      Use std::optional/nullopt (C++17) if they only do that so they can return NULL
      for missing/uninitialized/error.
  * Use std::optional (C++17) for methods that return boolean status flag and return
    a value via a pointer argument.
  * Use `std::array`, `std::span` (C++20), or `std::string_view` (c++17) for buffers, arrays, pointer/length pairs.
  * Use `std::string_view` (C++17) in many places where `char*` are used and
    passed. Accept `std::string_view` in general as parameter in places where
    `char*` and `std::string&` are used as function paramemters.
  * Use standard `<thread>` library for threads rather than our own
  * ArRangeBuffer and other collections should perhaps implement iterator or STL container
    interface or provide more access to underlying standard containers/iterators. This makes them directly usable with standard algorithms and C++20 range/view.
  * Find more opportunities to use improved STL algorithms including parallel.
  * Verify that frequently used storage types like ArPose, RangeBuffer, etc. are compatible with move semantics
  * Use C++17 filesystem library. Remove file/directory functions from ArUtil.  
  * Replace use of scanf, atof, atoi etc. (and ArUtil wrappers) with
    `std::stod`, `std::strtod`, `std::from_chars`, etc.  Replace use of sprintf with
    `std::to_chars` or `std::to_string` or sstream with format manipulators, or `std::format` (coming after
    C++20, but there is also <https://github.com/fmtlib/fmt>in the mean time)
  * Use `std::string` and `string_view` more frequently rather than `char*`.
  * Use `std::array` instead of C arrays (or known-size vectors, but these seem to
    not occur or be very rare in ARIA) (PARTLY DONE)
* Simplify and improve ArFunctor
  * Replace ArFunctor usage with std::function?  
    * Users can provide any bare function, which is converted to std::function
    * Users can provide a lambda (note that it must specify a return type if it
      returns something in order to be safely assigned to a std::function.)
    * Users can provide a callable object with an operator().
    * Users can use `std::bind` with placeholder arguments, or `std::bind_front` (C++20) to provide an object instance
      bound to a member function (bind the method to a class instance),
    * Can we use a lighter weight callable type rather than std::function? Can
      we use a more constrained type consisting of just an object pointer and
      function signature (type)? Proposed `function_ref`?
    * Need to make sure it is possible and easy to use a `std::shared_ptr` to
      store object references (or other smart pointer).
    * std::function examples:

        ```C++
        class ArRobot {
          ...
          void addUserTask(std::function<void(void)> &callback);
          ...
        };
        ...
        robot.addUserTask( []() { cout << "Hello world\n"; } );

        // Note: must ensure `myinstance` is not destroyed before lambda and/or
        // removed as user task from `robot`.  
        auto l = [&myinstance])() { cout << myinstance.mymethod(); } );
        robot.addUserTask(l);

        robot.addUserTask(std::bind(&MyClass::mymethod, myinstance, std::placeholders::_1);

        // Note: you can either copy myinstance, or pass a pointer, when passing
        // to bind_front. Maybe you could std::move it also?
        robot.addUserTask(std::bind_front(&MyClass::mymethod, &myinstance));
        ...
        ```

        ```C++
        class MyClass {
          std::string myName;
        public:
          MyClass(const std::string& name) : myName(name) {};
          void mymethod() {
            std::cout << "MyClass::mymethod() called on " << myName << '\n';
          }
        };

        void test(ArRobot& robot)
        {
          // create a new instance of MyClass in a shared_ptr.
          std::shared_ptr<MyClass> myptr {std::make_shared<MyClass>("some MyClass instance")};
          robot.addUserTask( [myptr]() { myptr->mymethod(); } 
          // when we return, the lambda stores a shared_ptr for the MyClass
          // instance, so it will not be destroyed. If the user task is removed
          // from the ArRobot object, will the lambda and smart pointer be
          // deleted, thus the MyClass instance deleted?
          return;
        }
        ```

    * See <https://godbolt.org/z/Md7WKbzrP> for experimenting with
      `std::function` and `std::bind_front`.
    * Conversion functions can be written to convert deprecated ArFunctor types
       to `std::function`s in C++20 like this:

        ```C++
        template <class CT, typename ArgT>
        std::function<void(ArgT)> toStdFunction(const ArFunctor1C<CT, ArgT>& functor) {
          return std::bind_front(functor.myFunc, functor.myObj);
        }
        ```

      Or, pre-C++20 like this:

        ```C++
        template <class CT, typename ArgT>
        std::function<void(ArgT)> toStdFunction(const ArFunctor1C<CT, ArgT>& functor) {
          return std::bind(functor.myFunc, functor.myObj, std::placeholders::_1);
        }
        ```

      And ArGlobalFunctor classes can be typedef'd directly to std::functions
      like this:

        ```C++
        template <typename T>
        using ArGlobalFunctor1 = std::function<void(T)>;

        template <typename Ret, typename Arg>
        using ArGlobalRetFunctor1 = std::function<Ret(Arg)>;
        ```

      Maybe ArFunctorC can also be supported via type alias rather than
      conversion functions?

    * Note, should we be using `std::ref` or the proposed `function_ref` for the above? (Or only
      needed if target is an object with operator(), rather than function
      pointer or lambda?)
    * One feature ArFunctor has, which std::function does not, is a name that can be used for debugging.
      But maybe we can derive a class from std::function or wrap
      std::function it in a small struct with name or other additional
      features, and transparently expose the std::function API.
      Or just keep a separate map of std::type_index to names with functions to 
      look them up (std::map<std::type_index, std::string>) -- you can get a std::function's target type with
      target_type().  E.g.   
      `names_map.insert(std::type_index(f.target_type()), "name"); 
      auto it = names_map.find(std::type_index(f.target_type()));`
  * Or, just simplify ArFunctor classes with variadic template types for
    function arguments, and/or function signatures as a type, and template
    deduction to distinguish between function object class for free/global
    functions, instance member functions, const instance member functions, etc.
    See <tests/newFunctionObjectTemplates.cpp> for an example of this.
    It should be renamed to less misleading name, but we can add type aliases
    for compatibility with old ArFunctor classes.
* Mark some very frequently used inline methods noexcept? (because library might (or might not) have been compiled with -fno-exceptions but user applications probably won't be)
* Javascript (NodeJS) wrapper via SWIG (HELP WANTED)
* Automated Rust wrapper via ritual or SWIG or other tool (HELP WANTED)
* Keep moving internal stuff into .cpp files, or if headers need to be shared
  into an internal include directory inside src.  Only public API should be in
  include/
* Keep finding 'protected' members to make 'private'.  
* Keep removing AREXPORT from inline and private members. (IN PRG)
* Add modern packaging to python wrapper (HELP WANTED)
* Add better rust packaging to rust wrapper (HELP WANTED)
* Fix all the sprintf errors in demo.cpp and ARIA (e.g. it's trying to use sprintf to include
  a copy of the previous string value in a new value, or append new formatted
  values to the previous string value, which is a potential bug
  and newer compilers warn about it. Instead a new string should be formatted
  then appended std::string, IO streams, or
  std::format (C++20 feature, not yet implemented in gcc, clang or msvc as of
  2/21) to build the string.  ("fmt::format" is a 3rd party implementation 
  generally compatible with future std::format that could be easily included 
  internally only by demo.)
* Change ArLog to use c++ io instead of c io.  Use `clog` as output stream
  instead of stderr and fflush.  
* ArDeviceConnection implementation for session recording/playback with three
  modes:
    1. read packets from file to send to ARIA, write packets sent.
    2. recording mode: write session to file, pass through to another device
connection.
    3. verification mode: send packets from file to ARIA, recieve packets sent and check
against what was recorded.
* In several places there are sets of methods that operate with different data
  types. Replace with template functions. ArArgumentParser, ArArgumentBuilder?,
  ArRobotParams, ArConfig
* Add C++20 contract declarations?  (I.e. [[expects...]], [[ensures...]],
  [[assert...]])
    * Would need to wrap in macros with checking for whether the feature is
      supported by compiler, allows building with C++ standard < 20 or not yet
      implemented, can also allow manual (selective?) disabling of contract
      declarations and assertions at compile time or by user code setting flag
      etc., or replace with clang __builtin_assume/msvc __assume optionally at build, and can 
      also add additional attributes like [[unlikely]].

Maybe TODO eventually
----------

* Add "talker" prefix filter to ArNMEAParser and ArGPS. I.e. list of talker IDs
  to accept, ignoring all others.  This would be used on a complex vehicle
  instrument system in which different "talkers" (sources, devices, instruments)
  may emit the same sentence types, or to support a GPS receiver that
  doesn't use "GP" prefix for some reason.   Talker IDs are two characters.
  (is check of `std::vector<uint16_t>` or `std::vector<char[2]>`, with vector
  size probably 1, 2 or 3, a low performance impact? How about a fixed size
  array of size 6 or something?)
* Replace ArNMEAParser with the guts of gpsd? (device support and nmea packet
  parsing layers)
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
