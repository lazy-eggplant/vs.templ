# Milestones

## `v0.4.5`

- [x] Code cleanup.
- [x] Speed up `O(nm)` children detection in cycles.
- [ ] Stringview version of `strnatcmp` to optimize the dot comparisons
- [ ] Rework the mechanisms used in `order-by` and fill missing comparisons in the RPN

## `v0.4.7`

- [ ] Properly show ctx information while logging
- [ ] In the UI for the demo, add a loader as the first boot is quite slow
- [ ] Fill in the docs at least of the public interface

## `v0.4.x`

- [ ] Much more robust test-suite with higher coverage.
- [ ] Optimization for lower-end systems by reducing explicit memory allocations if possible and adopting string views.
- [ ] Expose a mechanism to allow externally defined expressions? Not sure about this one, but it might be good when embedding this library.

## `v1.0.0`

- [ ] Some good testing coverage is highly desirable.
- [ ] Add a proper xml schema for this language (rng in place of xsd as they are more flexible for unordered files that I want to support)
