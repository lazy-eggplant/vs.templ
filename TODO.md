# Milestones

## `v0.4.1`

- [ ] In the UI for the demo, lock button to generate while fetching examples
- [ ] In the UI for the demo, add a loader as the first boot is quite slow
- [ ] Fill in the docs at least of the public interface

## `v0.4.2`

- [ ] Rework the mechanisms used in `order-by` and fill missing comparisons in the RPN
  - [ ] Support for string natural order (111 > 22).
  - [ ] Strings in dot notation when ordering by random should apply hashing and comparison to each block sequentially.
  - [ ] String support in RPN

## `v0.4.3`

- [ ] Properly show ctx information while logging

## `v0.4.x`

- [ ] Much more robust test-suite with higher coverage.
- [ ] Optimization for lower-end systems by reducing explicit memory allocations if possible and adopting string views.
- [ ] Expose a mechanism to allow externally defined expressions? Not sure about this one, but it might be good when embedding this library.

## `v1.0.0`

- [ ] Some good testing coverage is highly desirable.
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)
