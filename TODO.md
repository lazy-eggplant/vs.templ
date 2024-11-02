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

## `v1.0.0`
- [ ] Support for all commands, except `filter` in all `for` variants. Syntax for it has not been determined yet.  
- [ ] Functional CLI needed.  
- [ ] Some minimal testing facility is highly desirable.