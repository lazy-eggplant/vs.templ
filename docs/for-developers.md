---
title: Information for developers
---

## Building

- A modern C++ toolchain supporting C++20
- `meson` as the build system of choice
- `pandoc` (optional) if you want to generate the documentation/man pages
- `gperf` (optional) if you want to regenerate the command dispatchers
- `valgrind` (optional) to run some tests

To build, test and install it you can just use normal meson commands like.

```bash
meson setup build
meson compile -C build
meson install -C build
```

## Embedding it

At this time, this repository is only available as a [meson](https://mesonbuild.com/) package.  
I might consider adding `cmake` later on to gain a wider compatibility.

If `pugixml` is already provided as a dependency in the main project, that will be used.

The CLI in `src/app` shows everything that is needed to use `vs.templ` as a library.

### C bindings

At this time, no C bindings are provided since all my downstream project don't need them, and pugixml is C++ only.  
Still, the public interface is quite thin, so they can be easily introduced if so desired.

## Versioning

Releases are tagged on the repo, with `master` as the main development branch.  
Semantic versioning will be followed after `v1.0.0`.  
Before that, any release can and will have breaking changes.

Versions ending with an odd revision number are meant for proper for releases, while even numbers are only for in-dev stages.
