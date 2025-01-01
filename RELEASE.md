## v0.3.7

This release brings many new features which further expand what this templating engine can do:

- [ ] Introduced a more robust logging infrastructure at all levels.
- [x] Introduced [meta-expressions](../repl-vm.md) with their VM to support more complex evaluations.  
       Only few operators have been implemented so far, but all the architectural bits have been ironed out.
- [ ] Introduced `filter` for loops now that we have meta-expressions.
- [x] A significantly improved documentation.
- [x] Initial work to support prop-based commands.

No breaking change is expected from v0.3.5 for templates, but the public interface itself has. You might need to slightly refactor current code, and provide an explicit logging function.  
Next release is expected to be the very last for v0.3.x.
