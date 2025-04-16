---
title: Computing expressions
---

> [!WARNING]  
> This document was originally just a draft to keep as reference while implementing the RPN VM.  
> As such, its content and language do not fully represent features as they have been implemented.  
> Also, some functions reported as _not implemented_ will never be, and few more are yet to report.

Tentative specs for higher order expressions to add some degree of freedom in calculations.  
They are still interpreted as expression, starting with `:`. The content of the expression represent the serialization of a program in reverse polish notation (RPN).  
The VM is going to run it, and the only element left on stack is taken as the final expression value.

For example, assuming the environment had `i` set to `1`:

```
: `{i}` `#name-` cat:*`
```

will return an expression of type string embedding the value `name-1`.

Type casting must generally be explicit, but some operators might have automatic casting to simplify code.  
The delimiters are needed, since some types of expressions like strings can contain spaces and other escaping characters.  
The character `|` should never be used as it interfere with the serialization of tuples used by `vs.templ`.  
Escaping sequences are defined for \` and `|` as well.

## Design

### Why RPN?

Several reasons:

- to avoid a complex parser for what will end up being a quite marginal feature
- it avoids the arbitrary precedence rules usually associated with algebraic operator
- to be fast and easy to compile into native code if so desired (but for now a vm is all we need)

### Possible extensions

I will probably allow braces as NOP operators, just as syntax sugar to improve readability.  
At some point they could get validated in LSP, so that potential bugs are highlit.

### Options for implementation

Assumption: these programs are very short.  
As such their source is not read in streaming, but it is all resident in memory while compiling.  
This allows the usage of `string_views` and the buffer itself in place of allocating too many temporary objects in memory.  
Hence, processing these expressions should be very memory efficient.

- Add the vm to this repo directly.
- Build a generic vm library for which specific instances can be generated downstream of a schema.  
  This way it is easier to build different backends and integrate fast dispatching via perfect minimal hashing functions.
- ~~Find a library already implementing this design and use that.~~ no success in that

## Operators

### Generic

- [x] `dup` duplicate the last element on stack
- [x] `nop` no operation
- [x] `rem` remove last n elements from stack
- [ ] `swap` swap the two top elements
- [ ] `log` to log errors/warning etc somewhere

### String operators

- [x] `cat` `[container] cat`, simplified version of `V` where based on the type, the 0 is used as initial and the reducer is the natural `+` operation
- [x] `join` to join together strings on the stack with the first string.
- [ ] ~~`rcat` like cat but in reverse order~~
- [ ] ~~`rjoin` like join but in reverse order~~
- [ ] `esc` escape number to string literal

### Boolean operators

Boolean types are not directly supported. They are just integers with extra semantic.

- [x] `if` `[false] [true] [condition] ?`
- [x] `and` `or` `not` as expected (they are bitwise)
- [ ] `xor` `nor` `nand` as expected (they are bitwise).
- [x] `true` `false` to load those values in stack.

### Comparison operators

- [x] `eq` `neq` `bg` `bge` `lt` `lte` for integers & floats
- [ ] For strings we have at least three criteria, which is making these operations harder to handle
  - [ ] Typical comparison based on lexicographic comparison
  - [ ] Dot comparison, introduced by `vs` to handle nesting
  - [ ] Natural comparison, where for example $20<100$

### Common algebraic operators

And all the typical maths operations as usual

- [x] `+` `-` `*` `/` `mod` (and mnemonic versions too)
- [ ] `pow` `log`
- [x] `count.0` & `count.1`

### IDs
- [ ] 64bit sequential id (with optional family, 0 by default)
- [ ] 256bit random uniform id
- [ ] Timestamp (fixed once for all, it will be the same at any point of the generation)

### Cast

- [x] `as.str`
- [x] `as.int`
- [x] `as.float`
- [x] `as.bool`

### Special

- [x] `PIPE` and `APOS` to escape `|` and `\`` respectively
- [x] `(` & `)` as nop just to enable formatting in expression (not enforced, but the LSP might check)
- [x] `rem` to tag the prior tag as comment (remove the last element from stack)

