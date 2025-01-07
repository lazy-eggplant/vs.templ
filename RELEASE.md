## v0.3.13



### New features

- `data` command has been added to introduce external data souces.
- Examples! A LOT OF EXAMPLEEES!
- We even have the [most blazingly fast web server](https://github.com/lazy-eggplant/vs.http) based on `vs.templ` (according to source myself)!

### Bug fixes

- `test/case` fixed.
- Fixed behaviour of `for-prop` tag command to ensure good symbols are provided in the loop.

### Breaking changes

- Reverted and fixed `src` in for cycles to `in` as it initially was.
- Changed the preprocessor interface to introduce the new `loadfn` (the name has been recycled from before, the older `loadfn` is now `includefn`)
