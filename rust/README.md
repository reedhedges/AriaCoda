
This rust crate uses bindgen to wrap the pure-C `ariac` wrapper
taken from the ARIA matlab support in `../matlab`.  `ariac` is
used since it is much simpler than the full ARIA C++ library, and
in general C bindings are easier to use directly from Rust rather than C++.
The ARIA C++ library uses many C++ standard library types (including
template types) as well as its own template types, and many other
C++ features not easily supported in Rust bindings.  But `ariac` 
provides the essentials you need to control the robot and get back
position and other state information, and it's very easy to use.
See `../matlab/ariac.h` for notes on its use.  

Use `cargo build` to build (after importing cargo shell environment
from ~/.cargo/env into your shell, if neccesary in your installation.)

Use `cargo test` to run a simple test that attempts to connect to
the robot.  Run the MobileSim simulator first, or run on a computer
connected to a robot via serial connection (e.g. robot onboard computer).  
The rust code for this test can be viewed in src/lib.rs.


