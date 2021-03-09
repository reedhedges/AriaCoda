include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

mod tests {
  use super::*;
  #[test]
  fn connect_test() {
    unsafe {
      println!("Initializing ARIA and connecting to robot... (Note: You must run the simulator or have a robot connection available for this to work.)");
      aria_init();
      let cr = arrobot_connect();
      match cr {
        1 => {},
        0 => panic!("Error connecting to robot"),
        _ => panic!("Error connecting to robot")
      }
      println!("connected to robot ok");
      aria_exit(0);
    }
  }
}

