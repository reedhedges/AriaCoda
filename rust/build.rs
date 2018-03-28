extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main()
{
  println!("cargo:rustc-link-lib=ariac");
  println!("cargo:rustc-link-search=native=../lib");
  let bindings = bindgen::Builder::default()
    .header("../matlab/ariac.h")
    .generate()
    .expect("Unable to generate bindings");
  let path = PathBuf::from(env::var("OUT_DIR").unwrap());
  bindings.write_to_file(path.join("bindings.rs")).expect("Couldn't write bindings!");
}

