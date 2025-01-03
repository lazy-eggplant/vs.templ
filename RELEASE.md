## v0.3.7

This release brings many new features which further expand the capabilities of `vs.templ`:

- [x] Introduced a more robust logging infrastructure at all levels.
  - [x] Introduced `log` tag
- [x] Introduced [meta-expressions](../repl-vm.md) with their RPN virtual machine to support more complex evaluations.  
       A good number of operators have been implemented so far, with most of the architectural bits ironed out.
- [ ] Introduced `filter` for loops now that we have meta-expressions.
- [x] A significantly improved documentation.
- [x] Initial work to support prop-based commands.

No breaking change is expected from v0.3.5 for templates, but the public interface itself has.  
You might need to slightly refactor current code, and provide an explicit logging function.  
Next release is expected to be the very last for v0.3.x.
