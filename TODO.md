# Supported commands

This is a list of commands which are currently supported:

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
- [ ] eval
- [x] when/is
- [ ] debug / error / log to add entries to the logging data
- [ ] throw / catch to trigger errors and provide regions to have them resolved

## Props based

- [ ] for
  - [ ] base structure
  - [ ] filter
  - [ ] sort & order
  - [ ] limit & offset
- [ ] for-prop (mostly a copy & paste from `for`)
- [ ] value
- [ ] prop (dual of element)
- [ ] calc

## Missing features

- [ ] Stack based language.
  - [ ] Stack based evaluations for `eval` element
  - [ ] and for `filter` prop on for like cycles.
  - [ ] BREAKING replace numbers in for-range with these kinds of expressions.
  - [ ] Evaluate their usage in when/is
- [ ] Introduce escaping via "%" of 7bit ascii of strings within path expressions (to avoid issues with `[` `]` later on)
- [ ] Introduce the hashing functions needed to perform RAND ordering in `order-by`.

# Milestones

## `v0.3.5`

- [x] At least feature parity with what it was before moving it out of `vs-fltk`.
- [x] Some degree of documentation.
- [x] Add support for `element.text()` & `element.name()` in query.
- [x] Replace the current split implementation with something requiring less allocations

## `v0.4.0`

- [ ] Add support for props-based commands.
- [x] Initial setup for the test-suite.
- [x] Add actual support of dot comparison in strings (to be tested).
- [ ] Add random as a proper `order-by` supported value (I must be able to specify the seed when building the document too).
- [x] man page generation via meson (not great, but I'll take it).
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)

## `v1.0.0`

- [ ] Support for all commands, except `filter` in all `for` variants & `eval`. Syntax for it has not been determined yet.
- [x] Functional CLI needed.
- [x] Install scripts working (testing needed).
- [ ] Some good testing coverage is highly desirable.
