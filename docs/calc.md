Tentative specs for the expressions used in:
- `calc`
- its prop variant
- in filters within `for`
- maybe in `for-range`, replacing simple numbers.  

My plan is to add a stack-based language in polish notation, constrained to have only one root.  
For example:
```
add:2 int:2 [{var-a}/path~attribute] error add:2 [2] [3]
```

Compressed it would be something like:
```
+   int[{var-a}/path~attribute]!
    +[2][3]
```

Inside `[...]` there are path expressions. Type casting should be explicit. The delimiters are needed since expressions can contain spaces and other escaping characters.  
The reason for going with something like this are:
- to avoid a complex parser for what will end up being a quite marginal feature
- to avoid the arbitrary precedence rules usually associated to algebraic operator
- to be fast and easy to compile into native code via tcc

I will probably allow braces, just for the sake of readability. At some point I might validate them if present just to highlight potential bugs.   

Options for implementation:
- Add them to this repo directly.
- Build a schema based generator and use it. This way it is easier to build different backends and integrate fast dispatching via perfect minimal hashing functions.
- Find a library already implementing this design and use that.

Assumption: these programs are very short. As such their source is not read in streaming, but it is all resident in memory while compiling. This allows the usage of string_views and the buffer itself in place of allocating too many temporary objects in memory. Hence, processing these expressions should be very memory efficient.
