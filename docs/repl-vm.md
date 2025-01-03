---
title: Computing expressions
---

Tentative specs for higher order expressions to add some degree of freedom in calculations.  
They are still interpreted as expression, starting with `:`. The content of the expression represent the serialization of a program in reverse polish notation (RPN).  
The VM is going to run it, and the only element left on stack is taken as the final expression value.

For example, assuming the environment had `i` set to `1`:

```
: `#name-` `{i}` cat:*`
```

will return an expression of type string embedding the value `name-1`.

Type casting should be explicit, but some operations like `concat` might cast automatically.  
The delimiters are needed, since some types of expressions like strings can contain spaces and other escaping characters.

## Design

### Why RPN?

The reason for going with something like this are:

- to avoid a complex parser for what will end up being a quite marginal feature
- to avoid the arbitrary precedence rules usually associated to algebraic operator
- to be fast and easy to compile into native code via tcc if so desired (but for now it is just going to be interpreted)

### Possible extensions

I will probably allow braces, just for the sake of readability, and to avoid being explicit about the number of arguments a function should take.  
At some point I might validate them if present just to highlight potential bugs.

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

### Array operators

- [ ] `count` `[expr] #`, count the dimensionality of expression returning a 1-dimensional scalar number
- [ ] `reduce` `[initial] [reducer] [container] V`, reduces to a 1-dimensional object each element of the container starting from the initial expression (sum, join etc)
- [ ] `filter` `[container] F`, filters elements out of a container
- [ ] `map` `[expr] [container] M`, maps container based on a lambda

### Generic

- [ ] `dup` duplicate the last element on stack
- [x] `nop` no operation
- [ ] `erase` remove last n elements from stack
- [ ] `swap` swap the two top elements
- [ ] `eval` (probably I will not expose this one) expose a nested repl vm
- [ ] `load` load variable on stack from the environment map
- [ ] `store` (probably I will not expose this one) store variable form stack to the environment map
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

And all the typical math operations as usual

- [x] `+` `-` `*` `/` `mod` (and mnemonic versions too)
- [ ] `pow` `log`
- [x] `count.0` & `count.1`

### Cast

- [ ] `as.string`
- [ ] `as.integer`
- [ ] `as.float`

### Special

- [x] `PIPE` and `APOS` to escape `|` and `\`` respectively
- [x] `(` & `)` as nop just to enable formatting in expression (not enforced, but the LSP might check)
- [x] `rem` to tag the prior tag as comment (remove the last element from stack)
