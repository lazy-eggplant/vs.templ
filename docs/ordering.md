Ordering criteria are used in two situations:

- in `order-by` attributes of cycles of every type except `for-range`
- as basic boolean binary operator in the RPN vm

## Order types

### `ASC` & `DESC`

### `RANDOM`

Data of any type is first hashed.
For integers, floats and booleans this only involves their immediate data.  
For strings the characters buffer. Order will be based on the numerical comparison of the hash.  
For strings adopting the dot notation, each token is handled separately in hashing and comparing.

## Comparison operators for supported types.

### Nodes

### Integers

### Floats

### Booleans

### Strings

#### Natural ordering

#### Lexicographic ordering

#### Pseudo-random ordering

#### Dot modifier
