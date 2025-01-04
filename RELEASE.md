## v0.3.11

### New features

- [x] Introduced `include` to load more template files. Please, notice that checks to avoid circular dependencies are to be performed externally.
- [ ] All the remaining prop-based commands.
- [x] Loading the environment of a preprocessor is now possible via `load_env`.

### Bug fixing

- Fixed several broken maths operations like `*`.

### Breaking changes

The interface of `preprocessor` was changed one again to support the loader function. Minimal changes are expected downstream.
