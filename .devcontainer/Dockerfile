
# AriaCoda Development Container:
# A container that contains the bare minimum for building AriaCoda, which can be used as a "dev container"
# with the Visual Studio Code "Remote Containers" feature. See .devcontainer/devcontainer.json for VS Code config.
#
# Accepts an argument IMAGE with the name of the base docker image to derive this container from,
# e.g. a base OS image from Docker Hub, or image file, or URL.
# The default is "ubuntu:latest" for latest stable. You can set IMAGE to choose another version of Ubuntu, or another version of Linux.
# Set IMAGE on the "docker build" command line like this:
#   docker build --build-arg IMAGE=ubuntu:xenial .
#
# e.g.
#   ubuntu:xenial     Ubuntu 16.04   NOTE: you will also neet to set ARIA_OMIT_DEPRECATED_MATH_FUNCS=1 when running make
#   ubuntu:bionic     Ubuntu 18.04
#   ubuntu:focal      Ubuntu 20.04
#   ubuntu:impish     Ubuntu 21.10
#   ubuntu:rolling    Latest non-LTS Ubuntu
#   debian:latest-slim      Latest stable Debian ("slim" variant)
#   debian:testing-slim     Testing Debian
#   debian:unstable-slim    Unstable Debian
#   debian:11-slim    Debian 11
#   debian:10-slim    Debian 10
#   alpine:latest     Latest Alpine (a very lightweight Linux distribution with only busybox instead of the usual utilities)
#   archlinux:latest  Latest Arch
#
# Use "docker pull" to download an image from Docker Hub if needed.


ARG IMAGE=ubuntu:latest
FROM ${IMAGE}

# TODO if image is ubuntu:xenial, set ARIA_OMIT_DEPRECATED_MATH_FUNCS in environment
# TODO use appropriate package manager if using another version of Linux (e.g. alpine needs to use apk instead of apt)
# TODO maybe we just need different Dockerfiles for different linuxes?

RUN apt-get update && apt-get --yes install g++ make gdb git 


# TODO include a CMD or ENTRYPOINT to build AriaCoda if NOT run as devcontainer from Visual Studio Code or github or wherever.

