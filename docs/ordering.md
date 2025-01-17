Ordering criteria are used in two situations:

- in the `order-by` attributes of cycles for each type but `for-range`
- as basic boolean binary operator in the RPN virtual machine

The two implementations are not unified, so feature parity is desired but not intrinsic.

## Order types

### `ASC` & `DESC`

Whatever ordinal is assigned to the entries, sorting is just based on an ascending or descending order.

### `RANDOM`

Data of any type is first hashed, and `ASC` is applied to those hashes.
For integers, floats and booleans this just involves their immediate data.  
For strings the characters buffer is used as the hash source. If the dot notation is needed, each token separated by `.` will be hashed and compared on its own.
Nodes should not be directly compared. Doing so is not illegal, just nonsensical.

## Comparison operators for supported types.

### Nodes

Nodes should not be directly compared. Doing so is not illegal, just nonsensical and UB. A more precise semantic for this case might be introduced later on.

### Attributes

By default, they are cast to string before evaluation.

### Integers

Comparison as expected from any basic math class.

### Floats

Comparison as expected from any basic math class.

### Booleans

`true` is smaller than `false`.

### Strings

Strings are by far the most complex to handle as multiple criteria are possible.  
Strings 

#### Encoding ordering

Based on the binary representation of UTF-8 encoding.

#### Natural ordering

`hello10` bigger than `hello2`.

#### Lexicographic ordering

For symbols in the Latin alphabet this is more or less equivalent to the encoding ordering.  
However, different languages have totally different customary approaches, and are not often based on alphabets.  
Basically any criteria rooted in linguistic arguments goes in here.