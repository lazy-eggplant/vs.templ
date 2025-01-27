Sorting criteria are used in two situations:

- in the `order-by` attributes of cycles, supported by all of them in some way except for `for-range`
- as basic boolean binary operator in the RPN virtual machine

The two implementations are not unified, so feature parity is desirable but not intrinsic.

## Ordering method

### `ASC` & `DESC`

Whatever ordinal is assigned to the entries, sorting is just based on an ascending or descending order.

### `RANDOM`

Data of any type is first hashed, and `asc` is applied to those hashes.
For integers, floats and booleans this just involves their immediate data.  
For strings the characters buffer is used as the hash source. If the dot notation is needed, each token separated by `.` will be hashed and compared on its own.
Nodes should not be directly compared. Doing so is not illegal, just nonsensical.

## Comparison operators for supported types.

### Nodes

Via suffix `:n` or `:node`.  
Nodes should not be directly compared.  
Doing so is not illegal, just nonsensical and UB.  
A more precise semantic for this case might be introduced later on.

### ~~Attributes~~

They are always cast to string before evaluation. So they will not be compared as XML nodes.

### Integers

Via suffix `:i`, `:int` or `:integer`.  
Comparison as expected from any basic math class.

### Floats

Via suffix `:f`, `:float``.  
Comparison as expected from any basic math class.

### Booleans

Via suffix `:b`, `:bool` or `:boolean`.  
`true` is smaller than `false`.

### Strings

Strings are by far the most complex to handle as multiple criteria are possible.

#### Encoding ordering

Via suffix `:s`, `:str` or `:string`.
Based on the binary representation of UTF-8 encoding.

#### Natural ordering

Via suffix `:ns`, `:natstr` or `:natural-string`.
`hello10` bigger than `hello2`.

#### Lexicographic ordering

Via suffix `:ls`, `:lexistr` or `:lexi-string`.
For symbols in the Latin alphabet this is more or less equivalent to the encoding ordering.  
However, different languages have totally different customary approaches, and are not often based on alphabets.  
Basically any criteria rooted in linguistic arguments goes in here.
