# Supported commands

This is a list of commands which are currently supported:

- [x] Add support for `element.text()` & `element.name()` in query.
- [x] Replace the current split implementation with something not requiring these many allocations

## Block based:

- [x] for-range
- [ ] for
  - [x] base structure
  - [ ] filter
  - [x] sort & order
  - [x] limit & offset
- [x] for-prop (mostly a copy & paste from `for`)
- [x] value
- [x] element
- [x] when/is

## Props based

- [ ] for
  - [ ] base structure
  - [ ] filter
  - [ ] sort & order
  - [ ] limit & offset
- [ ] for-prop (mostly a copy & paste from `for`)
- [ ] value
- [ ] prop (dual of element)

# Milestones

## `v0.3.0`
- [x] At least feature parity with what it was before moving it out of `vs-fltk`.
- [x] Some degree of documentation.

## `v0.4.0`
- [ ] Add support for props-based commands.
- [ ] Initial setup for the test-suite.

## `v1.0.0`
- [ ] Support for all commands, except `filter` in all `for` variants. Syntax for it has not been determined yet.  
- [x] Functional CLI needed.  
- [ ] Install scripts working.
- [ ] Some minimal testing facility is highly desirable.