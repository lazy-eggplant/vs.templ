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
  - [x] replace numbers in for-range with these kinds of expressions.
  - [ ] Evaluate their usage in when/is
- [ ] Introduce escaping via "%" of 7bit ascii of strings within path expressions (to avoid issues with special symbols later on)
- [ ] Introduce the hashing functions needed to perform RAND ordering in `order-by`.

# Milestones

## `v0.3.7`

- [ ] Add support for props-based commands.
- [ ] Improve debugging and error resolution during the static construction.

## `v0.3.9`

- [ ] Add environment data when used as a lib.
- [ ] Basic repl done.

## `v0.4.0`

- [ ] Add `random` as a proper `order-by` supported value (I must be able to specify the seed when building the document too).

## `v1.0.0`

- [ ] Some good testing coverage is highly desirable.
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)
