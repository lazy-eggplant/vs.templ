---
title: Information for developers
---

## Building

- A modern C++ toolchain supporting C++20
- `meson` as the build system of choice
- `pandoc` if you want to generate the documentation/man pages

To build, test and install it you can just use normal meson commands.

## Using it

Right now this project is only available as a meson package.  
I might consider adding `cmake` later on to gain a wider compatibility.

## Versioning

At this time, this repository is only available as a [meson](https://mesonbuild.com/) package.  
Releases are tagged on the repo, with `master` as the main development branch.  
Semantic versioning will be followed after `v1.0.0`.  
Before that, any release can and will have breaking changes.

Versions ending with an odd revision number are meant for proper for releases, while even numbers are only for in-dev stages.
