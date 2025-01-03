---
title: Full Syntax reference
---

`vs.templ` uses special elements and attributes to define which actions the preprocessor should perform.  
These XML entities are scoped under the namespace `s` by default, but the user can set up a custom one as well.  
Please, notice that pugixml on which vs.templ is based does not have a full understanding of XML and namespaces are not covered.  
As such, they only operate as fancy prefixes at this scale.

## Expressions

Several of the attributes introduced by `vs.templ` accept expressions and not just simple literal values.
Expression can are used to access elements and attributes of the data XML, in addition to represent native types like integers or strings.  
Their definition and usage is purposefully restricted to prevent arbitrary code to be run.  
A full list of feasible expression types:

- Empty expression, generally representing errors
- String, automatically assigned from expressions starting with `#` (the prefix `#` is skipped)
- Integers (base 10), automatically assigned from expressions starting with a digit, `+`, `-` or `.`
- Paths, of which three forms exists:
  - those starting with `$`. This special symbol is used to mark the nearest scope in the data XML being visited, or root if none.
  - Paths with arbitrary prefix `{var-name}` where `var-name`is searched for and resolved from the symbols' stack.
  - Absolute paths starting from the root, with prefix `/`.

Path expressions will continue with one or more tokens `/`-terminated representing the tag name being visited.  
If terminated with `~prop-name` the relevant attribute is selected.
There are also two special meta-properties which can be accessed this way:

- Special access to the property`~!txt` to get the node text.
- Special access to the element's name via `~!tag`

Finally, there are stack meta-expressions, starting with `:`.  
Those are run by a stack VM and can be sparingly used to perform more complex tasks.  
They return a single expression in one of the other real types.  
The full specifications of these meta-expressions can be found [here](repl-vm.md).  
For most scenarios, the only ones really needed are casts, integer math operations and `cat` to merge strings.

No further combination or format is allowed in expression. Else, those might lead to undefined behaviour.  
However, the preprocessor should not generally throw exceptions, only emit error or warning logs.

### Examples

Using the following XML file as reference:

```xml
<hello>
    <world attribute-a="value-0">
        text-0
    </world>
    <!-- The second one will never be visited with paths like /hello/world/-->
    <world attribute-a="value-1">
        text-1
    </world>
</hello>
```

- `/hello/world/` is the list of children for the first `world` element in `hello`
- `/hello/world~attribute-a` is evaluated as `value-0`
- `/hello/world~!txt` is evaluated as `text-0`
- Assuming a for cycle in `/hello/`, its children will be navigated and `$~attribute-a` will be resolved in `value-0` and `value-1`.

## Operators for elements

Operators acting over elements will use information from the current static data sub-path to further generate a parametrized version of what shown in their children on the template tree.

### `for-range`

- `tag` is the name of the symbol where the current value will be stored. If empty the default `$` is used.
- `from` starting value.
- `to` final value.
- `step` step of increment. It can be negative. If so `to<from` must hold true.

Infinite cycles are detected before execution, in which case no cycle will run. Unlike other `for` variants, there is no header, footer or empty child. Anything inside a `for-range` is interpreted as `item`.

### `for` & `for-props`

To iterate over children and props of an element respectively.  
Aside from that, they mostly share the same interface.

- `tag`: the name of the symbol hosting the current XML node pointer. If empty, its default is `$`
- `in`: must be specified and is a path expression
- `filter`: as an expression in the internal [custom language](./calc.md).
- `sort-by`: (only available for `for`) list of `|` separated path expressions. Elements will be sorted giving priority from left to right
- `order-by`: order preference for each field in the `sort-by` or the only one implicit for `for-props`.  
  Each entry is a pair `type:comparator` with type either `ASC`, `DESC` or `RANDOM`.  
  If not provided, comparator is assumed to be the default one.  
  As an alternative comparator we could have a one using `.` to separate values in tokens, and order them token by token.
- `limit`: maximum number of entries to be iterated. If 0 all of them will be considered, if positive that or the maximum number, if negative all but that number if possible o no content.
- `offset`: offset from start (of the filtered and ordered list of children)

Both `for` & `for-props` support the following list of children. You can use as many instances of the same type as you want, they will be applied to the final document in the order they appear.

- `header`: shown at the top of a non-empty container
- `footer`: shown at the bottom of a non-empty container
- `empty`: shown if a container is empty
- `item`: the main body
- `error`: shown if it was not possible to retrieve items (because of an error in the path for example; in case of empty lists `empty` is used)

The symbol `$$` gets loaded with the entry number while iterating, so that it is possible to count which one we are at.

### `value`

To introduce the result of a (meta) expression in the tree.  
Numbers are serialized and added, strings are introduced as they are, attributes as strings and nodes are just appended as children.  
It accepts a an expression `src` as argument, by default set to `$`.

### `element`

To generate a new element whose type is determined by an expression `s:type` (this attribute is specifically namespaced).  
Any other property and child will be preserved.

### `check` & `case`

To perform conditional cut and paste in the final tree based on simple matches between a reference expression and some values.  
Inside the body of `test` we have one or more `case`.  
Attributes for `case`:

- `continue` default is `false`. If `true` it continues checking and executing even after a match. Else it will break.
- `when` an expression to compare against.

The order of `case` elements is important and determines the overall flow.

### `log`

Optional argument `type` set to:

- `notify` when something should be logged, but there is no fault or suspicious behaviour ongoing.
- `ok` for tasks completed successfully
- `error` when a condition prevents this activity from succeeding, but the situation is recoverable
- `warning` when something can complete, but it is suspicious.
- `panic` when an error triggered, and the situation cannot be recovered

Panic does not mean exceptions. For example, the evaluation of an expression might panic, but this problem is not propagated.  
It just means that there is no way to recover it, and downstream code will have to live with its failure.  
Error means that there is a capability recognized when it was raised for the system to rectify this issue.  
For example the inclusion of a file which does not exist can adopt the content of the `include` tag as fallback.

## Operators for properties

`xxx` are used as tags to identify groups under which multiple attributes should be used.

### `for.SUB-ATTR.[prop/value].xxx` & `for-props.SUB-ATTR.[prop/value].xxx`

As prop, attribute variants of `for` and `for-props`. They add attributes/values to the node they are defined within.

### `value.xxx`

As for the element version, to introduce the value of an expression as value of a prop `xxx`.

### `prop.xxx`

To generate new property whose name is determined by an expression and assign whatever values was assigned to `prop.xxx`.

### `when`

To test if the current element should be shown, if and only if the expression is `true`.
