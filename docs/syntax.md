---
title: Full Syntax reference
---

> [!WARNING]  
> While all core features have been implemented, but documentation efforts are still ongoing.

`vs.templ` uses special elements and attributes to define actions for the preprocessor to perform.  
These XML entities are scoped with the prefix determined by the URI `vs.templ`. Conventionally, the prefix used is `s`, but users can set up their own.  
Please, notice that _pugixml_, on which `vs.templ` is based, does not have a full understanding of XML, and namespaces are in the list of features not covered.  
As such, this preprocessor will not really perform any actual validation, if not for its own prefix.

## Expressions

Most of the attributes introduced by `vs.templ` accept expressions, and not just simple literal values.
Expression can are used to access elements and attributes from the data source, or they can represent native types like _integers_, _floats_ and _strings_.  
Their definition and usage is purposefully restricted, to prevent the execution of arbitrary code.  
This is a full list of all expression types:

- **Empty** expression, a pseudo-value often used to represent errors
- **String**, automatically assigned from expressions starting with `#` (the prefix `#` is skipped and not part of the final string)
- **Integers** (base 10), automatically assigned from expressions starting with a digit, `+`, `-` or `.`
- **Floating point numbers**, like integers but always ending with `f`
- **Paths**, of which few forms exists:
  - Paths with arbitrary prefix `{var-name}` where `var-name`is searched for and resolved from the symbols' stack. They can only appear as the first token.
  - Those starting with `$`. This special symbol is used to mark the nearest recorded node from the data XML being visited, or root if none. It is a shorthand for `{$}`.
  - Absolute paths starting from the root, with prefix `/`.
  - Paths starting with `~`, an attribute rooted on `{$}`.

Path expressions will continue with one or more tokens `/`-terminated representing the tag name being visited.  
If terminated with `~prop-name` the relevant attribute is selected.
There are also two special meta-properties which can be accessed this way:

- Special access to the property`~!txt` to get the node text.
- Special access to the element's name via `~!tag`

Finally, there are stack meta-expressions, starting with `:`.  
Those are run by a stack VM and can be sparingly used to perform more complex tasks.  
They return a single expression in one of the other real types.  
The full specifications of these meta-expressions can be found [here](repl-vm.md).  
For most scenarios, only a minor subset is going to be useful, like some basic integer maths, comparisons and `cat` to merge strings.

No further combination or format is allowed in expression. Else, their parsing will fail.  
However, the preprocessor will not generally throw exceptions, the preferred mechanism is to emit log messages and continue.

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
- `/hello/world~!txt` is evaluated as `text-0` (excluding white-spaces)
- Assuming a for cycle in `/hello/`, its children will be navigated and `$~attribute-a` will be resolved in `value-0` and `value-1`.

## Element-based operators

Element-based operators are special elements in the `vs.templ` namespace; they are used to control flow or add content in the final document.

### `value`

`value` is used to introduce the result of a (meta) expression in the final XML tree:

- **Integers** and **floats** are automatically serialized before adding them to the page.
- **strings** are just appended.
- **attributes** are (like always) interpreted as strings.
- **nodes** are added as subtrees.

`value` accepts such expression via its `src` argument; by default, it is set to `$`.  
If the computation for the expression fails for any reason, the body of `value` will be used in the final tree.

#### Examples

```xml
<s:value src=":...">This fails, so this text will be shown</s:value>
<s:value src=": `1` `2` +">Since the operation is ok and returns `3` as integer, this text will not show up</s:value>
```

### element

This command is used to generate a new element, whose type is determined by the expression resolved in the attribute `s:type` (this one is specifically scoped in `vs.templ` namespace).  
Any other property and child will be preserved.

In case of failure, the entire element and its children are dropped from the output document.

#### Examples

```xml
<s:element s:type="#tag-name" prop-1="Hello">Content</s:element>
<s:element s:type=":" prop-1="Hello">This will not be shown as the expression is not valid</s:element>
```

### for-range

The simplest of the for cycles, not based on the input XML source. The following attributes are defined:

- `tag` is the name of the symbol where the current value will be stored. If empty the default `$` is used.
- `from` starting value to count from.
- `to` final value to reach.
- `step` step of increment. It can be negative. If so `to<from` must hold true.

Infinite cycles are detected before execution, in which case no step is run.  
Unlike other `for` variants, there is no header, footer or empty child.  
Everything inside a `for-range` body is interpreted as `item` to append in the generated document.

#### Example

```xml
<s:for-range tag="i" from="0" to="10" step="1">
    <s:for-range tag="j" from="0" to=": `{i}`" step="1">
        <s:value src=": `{j}` `{i}` mul" />
    </s:for-range>
</s:for-range>
```

### for & for-props

This pair of commands is used to respectively iterate over children or props of a base element.  
They basically share the same interface with minimal differences:

- `tag`: the name of the symbol hosting the current XML node pointer. If left empty, its default is assumed to be `$`
- `in`: must be specified as a path expression (or a meta-expression resolving in one)
- `filter`: as a meta-expression expressed in the internal [custom language](./calc.md).
- `sort-by`: (only available for `for`) list of `|`-separated path expressions (or meta-expressions resolving as such)l elements will be sorted giving priority from left to right to the different criteria
- `order-by`: order preference for each field in the `sort-by` or the only one implicit for `for-props`. Also `|`-separated. More details are described in the [sorting](./sorting.md) page
- `limit`: maximum number of entries to be iterated. If 0 all of them will be considered, if positive up to that number, if negative all except the last n-th (o no content if too big).
- `offset`: offset from start (of the filtered and ordered list of children)

Both `for` & `for-props` support the following types of children.

- `header`: shown at the top of a non-empty container
- `footer`: shown at the bottom of a non-empty container
- `empty`: shown if a container is empty
- `item`: the main body
- `error`: shown if it was not possible to retrieve items (because of an error in the path for example; in case of empty lists `empty` is used)

You can use as many instances of the same type as you want, they will be applied to the final document in the same order as they appear after grouping.

The symbol `[tag].c` gets loaded with the entry number we are iterating over, so that it is possible to count which one we are at.  
For `for-props` there are also `[tag].k` and `[tag].v` for key and value of the prop. `[tag]` stands for the value determined by the `tag` property, and by default is `$`.

#### Nested for

Since v0.4.7 it is possible to use the `for` loop base on elements to perform operations on trees.  
Assumptions:
- The structure of the node in the source data has always one single location where children can be found.
- Filters, sorting and ordering clauses will be applied the same for all cells.
- The placement for children in the destination tree is unique for each of the parent entries.

If these conditions are not fulfilled, `vs.templ` behaviour is undetermined, but as usual, exceptions or hard failures are avoided.  

To use nesting one has to:
- Provide an expression via attribute `src-children` to resolve the location of children in each cell based on the current parent. "$" is valid and means "check my direct children".
- Provide an optional expression via attribute `dst-children` to resolve the location of the generated XML where new cells should be expanded; if empty, its default value is `item`.

If `src-children` is missing, no recursive operation will take place, regardless of `dst-children` value.

### test & case

`test` is used to perform conditional cuts in the final tree, based on a cascade of expressions.  
Inside each `test` we have one or more `case`.  
Attributes for `case`:

- `continue` default is `false`. If `true` it continues checking and executing even after a match. Else it will break.
- `when` an expression to evaluate.

The order of `case` elements is important and determines the overall flow.

### log

This one is used to send messages to the current logging interface.  
It has an optional argument `type` which can be set to:

- `info` when something should be logged, but there is no fault or suspicious behaviour ongoing (default value).
- `ok` for tasks completed successfully
- `error` when a condition prevents this activity from succeeding, but the situation is recoverable
- `warning` when something can complete, but it is suspicious.
- `panic` when an error triggered, and the situation cannot be recovered

Panic does not mean exceptions. For example, the evaluation of an expression might panic, but this problem is not propagated any further.  
Panic just means that there is no way to recover the situation, and downstream code will have to live with such failure.  
Error on the other hand means that the system is expected to be able to rectify this issue.  
For example, the inclusion of a file which does not exist can adopt the content of the `include` tag as fallback, as such it is not considered panic.

### include

> [!IMPORTANT]  
> The full behaviour of this function is determined by downstream integration.

`include` can be used to insert in place the file defined by `src`. If not found, it uses the content inside its body.  
External files will always have their root element removed when included.

File loading for a single `include` instance appearing in code is only done once, even if in a cycle. Later requests will show the same content as before.  
Because of this, the source is expected to be static, at least throughout the entire session.  
Unlike most of the other instances, `src` is just a static string, not an expression nor a meta-expression.  
This is because the evaluation of the file must be **ALWAYS** statically resolved.

The functionality of `include` is not fully provided by `vs.templ` and requires downstream integration.  
As such, no caching is provided by `vs.templ`; if you need that, you will have to implement it as part of the load function passed to the preprocessor constructor.  
Similarly, circular dependencies are not tested. It is up to you to use a load function which ensures they will not occur.

The CLI shipping with this library has a very limited implementation which will load files as XML with normal filesystem paths.

#### Examples

```xml
<s:include src="component.xml">
    <h1>Placeholder title</h1>
    <p>This will be used if the import fails. Leave empty if not needed</p>
</s:include>
```

### data

> [!IMPORTANT]  
> The full behaviour of this function is determined by downstream integration.

This command loads a data source in memory, and exposes it via a tag name.  
Any property which is not `src` and `tag` will be exposed to the supplier function for parametrization.  
Data retrieval is not part of `vs.templ`, so it is up to the downstream implementation to handle it as desired.

The CLI shipping with the library will not handle complex features like filtering, sorting etc.  
The `src` passed is just being used as a regular fs path.

## Property-based operators

Many of these are equivalent to some element-base version, but their output in the generated tree is in the form of attributes and not elements.
`xxx` is used as marker to identify groups of attributes which should be considered collectively.  
Please, notice that the generation of namespaced attributes is [not allowed at this point](https://github.com/lazy-eggplant/vs.templ/issues/21).

### for.SUB-ATTR.xxx & for-props.SUB-ATTR.xxx

As prop, attribute variants of `for` and `for-props`. They add new attributes to the node they are defined within.  
`SUB-ATTR` is the marker for the command property, while `xxx` is the final attribute name.

### value.xxx

As for the element version, to introduce the result of an expression as value of prop `xxx`.

### prop.xxx

To generate new property whose name and value are determined by the pair passed as value (`|`-separated).

#### Examples

```xml
<tag s:prop.0="#hello|#world" s:prop.1="#hallo|#Welt"/>
```

### when

To test if the current element should be rendered, if and only if the expression is `true`.  
Basically a shorthand of a full element-based `when` to make some common cases less verbose.

#### Examples

```xml
<tag s:when=": false">Content will not be shown!</tag>
```

### when.xxx

If and only if the condition is true, the prop `xxx` is added as empty string. Else it is skipped.
