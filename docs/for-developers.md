---
title: Information for developers
---

## Building

- A modern C++ toolchain supporting C++20
- `meson` as the build system of choice
- `pandoc` (optional) if you want to generate the documentation/man pages
- ~~`gperf` (optional) if you want to regenerate the command dispatchers~~ not in use right now, when frozen will be removed this is its replacement.
- `valgrind` (optional) to run some tests

To build, test and install it you can just use normal meson commands like.

```bash
meson setup build
meson compile -C build
meson install -C build
```

Building for release should be a bit more heavy on flags, for example:

```bash
meson setup -Db_lto=true -Db_thinlto_cache=true -Db_lto_mode=thin build --buildtype=release
```

## Embedding it

At this time, this repository is only available as a [meson](https://mesonbuild.com/) package.  
I might consider adding `cmake` later on to gain a wider compatibility.

If `pugixml` is already provided as a dependency in the main project, that one shall be used.

`vs.templ` requires you to implement some features downstream.  
If not provided, the library will still be usable, but the command tags `include` and `data` cannot operate as for specifications.  
The CLI in `src/app` shows everything that is needed to use `vs.templ` as a library in your own projects.

### C bindings

At this time, no C bindings are provided. My downstream projects have no need for them yet, and pugixml has no C++ interface regardless.  
Still, the public interface of this library is quite thin, so they can be easily introduced if so desired.

## Versioning

Releases are tagged on the repo. `master` is the main development branch.  
Semantic versioning will be followed after `v1.0.0`.  
Before that, any release can and will have breaking changes, but they are usually described in the release notes.

Versions ending with an odd revision number are proper public releases, while even numbers are reserved for in-dev stages.
