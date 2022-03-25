
AriaCoda Test Build Containers:
-------------------------------

These containers build AriaCoda (just taken from the working directory) in different
Ubuntu or Debian environments.  libAria is built but no other builds or tests are done (yet).

To build a container image (xenial, for example):
  docker build .. -f Dockerfile-build-xenial
To run the new container image:
  docker run -a STDOUT -a STDERR <image ID>

Some images available on docker hub are:
  ubuntu:xenial     Ubuntu 16.04   NOTE: you will also neet to set ARIA_OMIT_DEPRECATED_MATH_FUNCS=1 when running make
  ubuntu:bionic     Ubuntu 18.04
  ubuntu:focal      Ubuntu 20.04
  ubuntu:impish     Ubuntu 21.10
  ubuntu:rolling    Latest non-LTS Ubuntu
  debian:latest-slim      Latest stable Debian ("slim" variant)
  debian:testing-slim     Testing Debian
  debian:unstable-slim    Unstable Debian
  debian:11-slim    Debian 11
  debian:10-slim    Debian 10
  alpine:latest     Latest Alpine (a very lightweight Linux distribution with only busybox instead of the usual utilities)
  archlinux:latest  Latest Arch

Use "docker pull" to download an image from Docker Hub if needed.


TODO I think we could build multiple test build containers by specifying multiple FROM commands? (In which case we would need to test for Xenial when setting EXTRA_CXXFLAGS in the build)
