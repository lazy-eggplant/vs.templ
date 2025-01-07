# Supported commands

This is a list of commands which are currently supported:

## Block based:

- [x] `for-range`
- [x] `for`
  - [x] base structure
  - [x] filter
  - [x] sort & order
  - [x] limit & offset
- [x] `for-prop` (mostly a copy & paste from `for`)
- [x] `value`
- [x] `element`
- [x] `when`/`is`
- [x] `log` to add entries to the logging data
- [x] `include` to append template files

## Props based

- [ ] for
- [ ] for-prop
- [x] value
- [x] prop (dual of element)
- [x] when

## Missing features

- [x] Stack based language.
  - [x] Stack based evaluations for `eval` element
  - [x] and for `filter` prop on for like cycles.
  - [x] replace numbers in for-range with these kinds of expressions.
  - [x] Evaluate their usage in when/is
- [x] Introduce the hashing functions needed to perform RAND ordering in `order-by`.
- [x] Floating point numbers (same serialization as for integers, but ending in `f`).  
       Cannot be mixed with integers in operations, explicit casts are needed.

# Milestones"

## `v0.3.15`

- [ ] Add support for all props-based which were left commands.

## `v0.4.x`

- [ ] Much more robust test-suite with higher coverage.
- [ ] Optimization for lower-end systems by reducing explicit memory allocations if possible and adopting string views.
- [ ] Support for string natural order (111 > 22).

## `v0.8.x`

- [ ] ~~Add sqlite backend in place of input XML data.~~ no longer needed, we can load data.

## `v1.0.0`

- [ ] Some good testing coverage is highly desirable.
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)
