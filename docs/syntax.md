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

- `tag` the name of the symbol hosting the current XML node pointer. If empty, its default is `$`
- `in` must be specified and is a path expression
- `filter` as an expression in the internal [custom language](./calc.md).
- `sort-by` (only available for `for`) list of comma separated path expressions. Elements will be sorted giving priority from left to right
- `order-by` order preference for each field in the `sort-by` or the only one implicit for `for-props`. Each entry is a pair `type:comparator` with type either ASC, DESC or RANDOM. If not provided, comparator is assumed to be the default one. As an alternative comparator we could have a one using `.` to separate values in tokens, and order them token by token.
- `limit` maximum number of entries to be iterated. If 0 all of them will be considered, if positive that or the maximum number, if negative all but that number if possible o no content.
- `offset` offset from start (of the filtered and ordered list of children)

Both `for` & `for-props` support the following list of children. You can use as many instances of them as you want, in any order.

- `header` shown at the top of a nonempty container
- `footer` shown at the bottom of a nonempty container
- `empty` shown if a container is empty
- `item` the main body
- `error` shown if it was not possible to retrieve items (because of an error in the path)

### `value`

To introduce the value of an expression as text content of an element. It accepts a path expression `src` as argument. By default, it is assumed to be `$`.  
It also supports an additional `format` argument, but at this stage it has no implementation.

### `element`

To generate a new element whose type is determined by a tag expression `ns:type`. Any other property and child will be preserved.

### `when` & `is`

To perform conditional cut and paste in the final tree based on simple matches between a reference expression and some values.  
`when` accepts a single `subject` property as a path expression.  
Inside the body of `when` we have one or more `is`.  
Attributes for `is`:

- `continue` default is `false`. If `true` it continues checking and executing even after a match. Else it will break.
- `value` a path expression to compare against.

The order of `is` elements is important and determines the overall flow.

### `debug`

## Operators for properties

`xxx` are used as tags to identify groups under which multiple attributes should be used.

### `for.SUB-ATTR.[prop/value].xxx` & `for-props.SUB-ATTR.[prop/value].xxx`

As prop, attribute variants of `for` and `for-props`. They add attributes/values to the node they are defined within.

### `value.SUB-ATTR.xxx`

As prop, to introduce the value of an expression as value of a prop `xxx`.

### `prop.xxx`

To generate new props whose name is determined by an expression.

### `debug.XXX`