
TODO
====

If you would like to contribute, consider items below labelled "HELP WANTED", or
any improvement or change that would make this library more useful for you or
easier to use.  Contact me or discuss on the GitHub page.

* Change .github/worklows/builds.yml to only do a test build if source code or build files (Makefiles etc.) change.
* Change uses of std::to_string() to std::to_chars(), but that requires C++17. Change remaining instances of sprintf/snprintf to use std::to_string() (C++11) or std::to_chars() (C++17), or std::format() (C++20).  Change use of strncpy, strncat, etc. to use std::string or version of std::string that has an internall char buffer on the stack. (In Progress)
* Replace use of strncpy() with strncpy_s() and memcpy() with memcpy_s().
* Use rsize_t and RSIZE_MAX in utility functions or other functions that do array bounds checking.
* Use more standard math functions instead of doing our own checked arithmetic e.g. fdim(), isless()/isgreater()/etc., fma(), etc., with error checking. 
* ? Build as libariacoda.so instead of libAria.so, with Debian packages named
  libariacoda3. (Avoid conflict with existing libaria in Ubuntu). Make a
  libAria.so symlink to help work with existing builds.
* Add example of `ExternalProject` for CMake to README.
* In certain specific places (maybe ArFunctor parameters?) use an (rvalue) reference so that data can optionally (explicitly or implicitly or not) be moved from caller through to function callback without a copy.
* In packet receivers e.g. ArRobotPacketReceiver, return ArRobotPacket by value
  as data, with all callers using std::move to move it to temporary or to store it?
* (DONE?) Remove any "commercial" code (note, code is  still GPL, but only relevant to "commercial" use), remove any "default (noncommercial) behavior" conditions. 
* Add build option to specify custom output dir/prefixes (for obj, lib, etc.)
* In many places (ArInterpolation, ArRangeBuffer, laser implementations...) a std::list is used to store objects as a buffer, because we want to remove old items from the beginning of the list and add new items to the end.  This could be replaced with std::deque, or ArRingBuffer or other contiguous ring buffer that doesn't need to allocate small objects for list items, but can keep things in contiguous memory space. In the case of ArRangeBuffer, we sometimes want to remove items from the middle. (This could be done in an array or vector if we have an "invalid" flag, and take the number of invalid or bad items into account when checking capacity, and the underlying capacity is not limited to the desired capacity of valid items. We want to keep the list/vector in order by time so that removing old elements, which is done more often, is fast.  In places other than ArRangeBuffer if the list/vector does not need to be in order, just move the last vector element on top of the element to be deleted,then `pop_back()`.) 
  * Laser implementations (ArLMS1xx, ArLMS2xx, ArSZSeriel, ArS3Series, etc.) still use a std::list for a queue of packets (myPackets), but have a hack that throws away old packets and only parses the most recent packet. So just store one Packet object in the class.  Is there a reason not to do this? Does the ArLaser or ArRangeDevice interface require it?  If we do need a queue, use std::deque or a circular/ring buffer.
  * In lots of places std::list is used as a container, when another container might be better. They are often small lists storing 5-20 items, usually only ever appended to or built at startup only, or only modified infrequently. Could be replaced with std::vector or std::deque.  Or a few places may benefit from still storing a std::list, or in other cases std::forward_list, or std::set, unordered set, multiset, etc.  Some of these are actually lists of pointers to manually allocated objects (with new), which makes even less sense, we should just store objects in the list. (Note that forward_list has no pointer or reference to its back, so can only push/pop on the front, but it does have insert_after and splice_after, or can be a stack by using push_front and pop_front.)  (IN PROGRESS for places where performance might matter a bit, like range sensor reading buffers)
  * In some places a std::map is used to map numeric indices or IDs to objects. This could probably be a vector instead.  std::map is used elsewhere to map things like string names or other identifiers to objects; most of these are probably relatively small and work well with a std::map, but if there is a case where an std::unordered_map could be better, replace it.  Maybe also std::flat_map?
* Possible bug: ArRobot is not setting the time component of myEncoderPose or myRawEncoderPose? Ned to investigate.
* Possible bug: ArInterpolation forward interpolation (future prediction) may not be working correctly.  May just be returning last pose in history, and never returning -1 (too far in future).
* Many classes provide setters for parameters that only need to be set when the object is constructed, these could be removed and the member variable made const.
* Classes like ArUtil and ArMath that act like namespaces (they only contain static functions) should just be changed to namespaces.
* (DONE) In ArRangeDevice, store myRawReadings and myAdjustedRawReadings as
  `std::list<ArSensorReading>` rather than `std::list<ArSensorReading*>` and don't allocate
  each one. Use `emplace_back` when creating a new ArSensorReading objects for the
  list. [It looks like all uses of myRawReadings etc. are allocating new
  ArSensorReading objects with "new", not borrowing/sharing pointers to existing
  ArSensorReading objects.
* (MOSTLY DONE) Fix virtual inheritance.  (Remove unnecessary virtual inheritance, make sure
  done correctly, etc.)
  * Some classes may still have unneccesary virtual methods, even if not participating in any virtual inheritance. Should clearly document any classes that are intended for virtual inheritance.   (Uneccesary destructors are commented out in some classes, remove that.) 
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
* Apply `final` in some classes with virtual inheritance that users won't inherit (though we have had cases in ARIA where we did additional inheritance to add specific features.)
* Add noexcept to methods whoose only standard library calls are themselves noexcept, or which calls have try/catch clause. 
* Find any const methods that are private or only used internally. consider replacing them with static functions in the .cpp file, and add `__attribute__((const))` if compiling with gcc (or clang?). (Is there an equivalent in MSVC?)
* There are no exceptions in ARIA.  Mark functions where it might matter as
  noexcept (eg for optimization).
* Bug: does not close robot connection when TCP connection write fails because
  remote side (ie sim) closed. (HELP WANTED)
* Change size parameter to read/write methods in ArDeviceConnection (and
  subclasses) to `size_t`.
* Use new laser simulator commands in ArSimulatedLaser instead of old SRISim
  commands (HELP WANTED)
* Edit (review and improve) README
* Remove exceptions from ArPacketUtil? (Use -fno-exceptions to build?)
  Merge ArPacketUtil functionality into
  ArBasePacket (without impacting performance of packets)
* Add `[[deprecated]]` attribute (or `DEPRECATED` macro) to deprecated methods (DONE?)
* Audit raw array indexing, pointer arithmetic, unneccesary new/pointers (need to make sure all classes are correctly copy/move constructable/assignable first; may need to refactor header files if a class just forward declares a class and uses pointers in header file), memset/memcpy/strcpy instead of assignment or construction, and other potential memory errors.  Use gsl::span (or just verify correctness. add tests.)  Make sure container iteration is always within bounds. (HELP WANTED)
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
* Analyze important areas for optimization/performance and improve; not high priority unless we want to use ARIA on embedded systems or at a much higher performance level (e.g. much higher cycle rate) than currently.
  * Reduce size of ArPose, ArSensorReading, anything else stored in larger amounts.  Can we use floats instead of doubles in ArPose, and other places? Will it help?  Use integers in some places? On 64-bit int is the same size as float, but math will be (ever so) slightly faster.
    * sizeof(ArPose with doubles) = 24 [64/24=2 rem 16]
    * sizeof(ArPose with floats) = 12 [64/12=5, rem 4]. 
    * sizeof(ArSensorReading with doubles) = 192
    * sizeof(ArSensorReading with floats) = 112
    * sizeof(ArTime) = 16
    * Alignment of int = 4, alignment of float = , alignment of double = , alignment of bool = 1.
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
  * Change some std::string members to std::string_view, if they are always (or could be always) set from constant string literals.  (Eg debugging names.) 
  * Change `const std::string&` and `const char*` function arguments to `std::string_view`.  (And change calls that convert from `std::string` using `c_str()` to just pass the `std::string` directly to the `std::string_view` argument). 
  * Manage packet reading (e.g. myPacketList in ArRobot) a bit more efficiently (can maybe use value semantics in packet reader/writer rather than allocating and deleting all the time; e.g. move ArRobotPacket objects into a myPacketList as `std::list<ArRobotPacket>`, splicing the list as needed, or just always as a vector or array buffer, perhaps ArRingBuffer) 
  * Many devices (e.g. ArLMS1xx) write data to packets by sprintf'ing to a string, then memcpy'ing that string to the packet buffer.  We should just format the numbers and text into the packet buffer serially without printf/sprintf. (Partially improved)
  * We can reduce mutex contention by using std::shared_mutex, or finding most frequently used data and (carefully) using std::atomic.
  * Some classe have arbitrary member order, some have the reordered to reduce padding.  But we could also try to co-locate data that will be used together in frequently run (hot) code.
  * Use algorithm functions from the standard library with parallel/unsequenced execution policies in some places? (requires c++17)

* Start adding some real unit testing? (HELP WANTED) (in progress)
  * Begin with existing tests/, separate non-automatable tests (maybe to separate directory?). Currently automated tests (that do checks) are identified in Make rule
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
  * Use std::optional (C++17) instead of pointers-that-might-be-null. 
  * Use `std::array`, `std::span` (C++20), or `std::string_view` (c++17) for buffers, arrays, pointer/length pairs.
  * Use `std::string_view` (C++17) in many places where `char*` are used and
    passed. Accept `std::string_view` in general as parameter in places where
    `char*` and `std::string&` are used as function paramemters.
  * Use standard `<thread>` library for threads rather than our own. (We genreally don't ever cancel threads in Aria, so that API could be removed from ArThread in the mean time, and use later replaced with std::thread; if we do need to cancel a thread we could use C++20 request_stop or std::jthread instead, but many asynchronous classes in Aria already have their own stop request mechanism.)   Use std::mutex or std::shared_mutex (since in many places class member accessors could do shared_lock() on a shared_mutex instead of exclusive mutex, especially ArRobot and ArRangeDevice) (in some cases maybe a recursive mutex instead.)  Note: pthreads has a rwlock with pthread_rwlock_rdlock() and pthread_rwlock_wrlock() that is similar to a shared_mutex. Or implement our own reader and writer locks on top of mutexes and counters and condition variables or semaphores or similar.   Maybe do some measurement to see if there is reader lock contention in a larger application.  One danger is thta many readers keep locking the shared_mutex, and the writer blocks for a long time, which could slow down e.g. the ArRobot packet processing task, which might be bad. 
  * Could we make lots of shared data std::atomic (e.g. in ArRobot, ArRangeDevices (most of them are threaded), etc.)?  Is there a cost to having a lot of atomic variables? Are there classes with just a few shared variables?
  * ArRangeBuffer and other collections should perhaps implement iterator or STL container
    interface or provide more access to underlying standard containers/iterators. This makes them directly usable with standard algorithms and C++20 range/view. (Possibly with parallel executors.)
  * Find more opportunities to use improved STL algorithms including parallel execution. (Parallel execution most useful with larger data sets, and in time cretical sections like robot and sensor data processing, not so much in startup config, etc.)
  * Verify that frequently used storage types like ArPose, RangeBuffer, etc. are compatible with move semantics. Most of these should just contain trivial members but some might contain mutex objects, or pointers to self-allocated memory, or other members that need special handling.   Implement correct move for ArMutex perhaps.
  * Use C++17 filesystem library. Remove file/directory functions from ArUtil.  
  * Replace use of scanf, atof, atoi etc. (and ArUtil wrappers) with
    `std::stod`, `std::strtod`, `std::from_chars`, etc.  Replace use of sprintf with
    `std::to_chars` or `std::to_string` or sstream with format manipulators, or `std::format` (coming after
    C++20, but there is also <https://github.com/fmtlib/fmt>in the mean time), just appending strings, etc.   (Note that sprintf can have performance impacts, even taking a mutex lock!)
  * Use `std::string` and `string_view` more frequently rather than `char*`.
  * Use `std::array` instead of C arrays (or known-size vectors, but these seem to
    not occur or be very rare in ARIA) (PARTLY DONE)
* Simplify and improve ArFunctor
  * Replace ArFunctor usage with functional objects from C++ standard library, or something else:
    * Could use std::function or std::move_only_function, or methods accepting function callbacks can be templates with `T&&` (convert to std::function to store?) and `requires std::invocable<...>` or `requires std::invokable_r<...>` (Note "requires" is from C++20).
    * Users should be able to use instead:
      * any bare function pointer, which is converted to std::function (or similar)
      * a lambda (note that it must specify a return type if it
      returns something in order to be safely assigned to a std::function.)
      * an object with an operator().
    * Or, users can use `std::bind` with placeholder arguments, or `std::bind_front` (C++20) to provide an object instance
      bound to a member function (bind the method to a class instance),
    * Can we use a lighter weight callable type rather than std::function? Can
      we use a more constrained type consisting of just an object pointer and
      function signature (type)? Proposed `function_ref`?
    * When accepting a user's callback function, use a template type that `requires` std::invocable ? 
    * Maybe just store a simple function pointer, a user could use a lambda that captures an object to call a member function.
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

    * See <https://godbolt.org/z/Md7WKbzrP> and <https://godbolt.org/z/6T9r8jWx6> for experimenting with
      `std::function`, `requires std::invocable` and `std::bind_front`.
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

    * Or, replace ArFunctor throughout Aria with std::function, but also make converting between 
      std::function and ArFunctor possible, e.g. add operator() to ArFunctor,
      and constructor that takes std::function to store and optionally use instead of virtual 
      derived class invoke() overload.  Note that most important for compatibility is users providing
      their own ArFunctor objects to ARIA classes, but there are also a few places in the ARIA API that return
      ArFunctor objects or pointers that users can store and call.

    * Or, just break the API and require users to switch to std::function or lambda closures. 

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
    * Alternative to only improve ArFunctor:, use variadic template types for
      function arguments, and/or function signatures as a type, and template
      deduction to distinguish between function object class for free/global
      functions, instance member functions, const instance member functions, etc.
      See <tests/newFunctionObjectTemplates.cpp> for an example of this.
      It should be renamed to less misleading name, but we can add type aliases
      for compatibility with old ArFunctor classes.
* Use std::size() and std::ssize() (that's C++20 though) to get correct size signedness for comparisons.
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
* ArRobot's private member variables could be reordered to reduce padding, but there is usually only ever one ArRobot instance.  Can some of these be removed or moved into objects that are optionally allocated if they are only very occasionally used?
* Add methods to ArThread to set affinity and priority (Linux: pthread_set_affinity, pthread_setschedprio; Windows: SetThreadAffinityMask, SetThreadPriority). Simplify for common use case.
* Test with different locales. Fix file name handling, ArDataLogger, etc. when given non-ASCII compatible strings.  (ArDataLogger should also have consistent number formatting regardless of locale.)   (Note that on Windows, printf()/fprintf() may do weird conversions of strings depending on the locale setting or some other character set setting in Windows or whatever terminal emulator is displaying text.) 

Maybe TODO eventually
----------

* Virtual inheritance is used in Aria to implement an interface for different typyes of hardware (e.g. laser rangefinders).  Usually there is a small handful of these and they will only be instantiated once in the program.  So this could also be done with a template interface specialized for the implementations implementing (non-virtually) similar APIs.  
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
* ArRobot's Task tree infrastructure and the ArActionResolver suffer from YAGNI fallacy a bit, it is general and customizable but in practice only pretty fixed task structure is used (with SensorInterpTasks and UserTasks as the only branches since they contain user-supplied tasks) and the default ArPriorityResolver implementation have ever been used. Therefore instead of allocating virtual ArACtionResolver subclass or accepting pointer to user's implementation, just collapse them with the parent "interface" classes and store them directly in ArRobot, and 
* Use general state machine library for the little state machines than are used in ARIA. (e.g. boost::sml) Should be pretty simple and lightweight.
* Many instances of `std::map` could be replaced by `std::flat_map`, especially if they are relatively static and/or small. (C++23 feature?)
* Could ASIO (boost) or other popular library replace some of the lower level portability wrappers/utilities used for network and serial IO?  Perhaps, but this would be a fairly invasive change unless the existing classes became wrapper APIs around the third party library (possibly throwing away some benefits of it.) 

* In code that parses sensor data by comparing received strings against string literals (keywords etc.), measure and consider more optimized string comparisons that take advantage of the fact that it's always against literal string constants. (E.g. with known size, etc.)   Most of these probably use strncmp which is pretty fast already, but using a string_view for the string literal might provide an even faster string comparison.  (See <https://quick-bench.com/q/ZVb2eoRKaKaFCFi-p4orPPk1yR4>)
