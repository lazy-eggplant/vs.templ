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
- [ ] `include` to append template files

## Props based

- [ ] for
  - [ ] base structure
  - [ ] filter
  - [ ] sort & order
  - [ ] limit & offset
- [ ] for-prop (mostly a copy & paste from `for`)
- [ ] value
- [ ] prop (dual of element)
- [x] when

## Missing features

- [x] Stack based language.
  - [x] Stack based evaluations for `eval` element
  - [x] and for `filter` prop on for like cycles.
  - [x] replace numbers in for-range with these kinds of expressions.
  - [x] Evaluate their usage in when/is
- [ ] Introduce the hashing functions needed to perform RAND ordering in `order-by`.
- [x] Floating point numbers (same serialization as for integers, but ending in `f`).  
       Cannot be mixed with integers in operations, explicit casts are needed.

# Milestones

## `v0.3.9`

Just a corrective versions.

## `v0.3.11`

- [ ] Add support for all props-based commands.
- [x] Complete more of the repl vm operator to cover specs.

## `v0.3.13`

- [ ] Complete more of the repl vm operator to cover specs.
- [ ] Add environment data when used as a lib.

## `v0.4.x`

- [ ] `include` (only fs when used via CLI)

## `v0.5.x`

- [ ] Add `random` as a proper `order-by` supported value (I must be able to specify the seed when building the document too).

## `v0.8.x`

- [ ] Add sqlite backend in place of input XML data.

## `v1.0.0`

- [ ] Some good testing coverage is highly desirable.
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)
