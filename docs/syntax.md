---
title: Full Syntax reference
---

> [!WARNING]  
> While all core features have been implemented, documentation efforts are still ongoing.

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
- String, automatically assigned from expressions starting with `#` (the prefix `#` is skipped and not part of the final string)
- Integers (base 10), automatically assigned from expressions starting with a digit, `+`, `-` or `.`
- Floating point numbers, like integers but always ending with `f`
- Paths, of which three forms exists:
  - those starting with `$`. This special symbol is used to mark the nearest recorded node from the data XML being visited, or root if none.
  - Paths with arbitrary prefix `{var-name}` where `var-name`is searched for and resolved from the symbols' stack. They can only appear as the first token.
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
For most scenarios, only a minor subset is going to be useful, like some basic integer maths, comparisons and `cat` to merge strings.

No further combination or format is allowed in expression. Else, their parsing will fail.  
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

Element operators or tag operators are special elements either acting on their children or they use them as default value in case of failure.  
There are several to control flow or add content to the final document.

### `value`

To introduce the result of a (meta) expression in the tree.  
**Integers** and **floats** are automatically serialized before adding them to the page.  
**strings** are just appended.  
**attributes** are interpreted as strings.  
**nodes** are added as subtree.

`value` accepts an expression `src` as argument, by default set to `$`.  
If the expression fails, its body will be used instead.

#### Examples

```xml
<s:value src=":...">This fails, so this text will be used</s:value>
<s:value src=": `1` `2` +">Since the operation is ok and returns `3` as integer, this text will not show up</s:value>
```

### element

This command is usedto generate a new element whose type is by an expression passed via the attribute `s:type` (this one is specifically namespaced).  
Any other property and child will be preserved.

In case of failure, the tag is dropped.

#### Examples

```xml
<s:element s:type="#tag-name" prop-1="Hello">Content</s:element>
<s:element s:type=":" prop-1="Hello">This will not be shown as the expression is not valid</s:element>
```

### for-range

- `tag` is the name of the symbol where the current value will be stored. If empty the default `$` is used.
- `from` starting value.
- `to` final value.
- `step` step of increment. It can be negative. If so `to<from` must hold true.

Infinite cycles are detected before execution, in which case no step will run. Unlike other `for` variants, there is no header, footer or empty child. Anything inside a `for-range` is interpreted as `item`.

#### Example

```xml
<s:for-range tag="i" from="0" to="10" step="1">
    <s:for-range tag="j" from="0" to=": `{i}`" step="1">
        <s:value src=": `{j}` `{i}` mul" />
    </s:for-range>
</s:for-range>
```

### for & for-props

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

The symbol `[tag].c` gets loaded with the entry number we are iterating over, so that it is possible to count which one we are at.  
For `for-props` there are also `[tag].k` and `[tag].v` for key and value of the prop. `[tag]` by default is `$`.

### test & case

To perform conditional cut and paste in the final tree based on simple matches between a reference expression and some values.  
Inside the body of `test` we have one or more `case`.  
Attributes for `case`:

- `continue` default is `false`. If `true` it continues checking and executing even after a match. Else it will break.
- `when` an expression to compare against.

The order of `case` elements is important and determines the overall flow.

### log

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

### include

> [!IMPORTANT]  
> The full behaviour of this function is determined by downstream integration.

Add in place the file defined in `src`. If not found, it uses the content inside `include`. External files will have their root removed when included.  
File loading for a single `include` instance appearing in code is only done once, even if in a cycle. Later requests will show the same content as before.  
`src` is just a static string, not an expression. This is because the evaluation of the file must be statically resolved.

The functionality of `include` is not provided by `vs.templ` and requires downstream integration.  
As such is no caching provided by `vs.templ`; if you need that, you will have to implement it as part of the load function passed to the preprocessor constructor.  
Similarly, circular dependencies are not tested. It is up to you to use a load function which ensures they will not occur.

The CLI shipping with this library has a very limited implementation which will load files as XML with normal fs paths.

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

## Operators for properties

`xxx` are used as tags to identify groups under which multiple attributes should be used.

### for.SUB-ATTR.xxx & for-props.SUB-ATTR.xxx

As prop, attribute variants of `for` and `for-props`. They add attributes/values to the node they are defined within.

### value.xxx

As for the element version, to introduce the value of an expression as value of a prop `xxx`.

### enable.xxx

If the condition is true, the prop `xxx` is added with no specific value.

### prop.xxx

To generate new property whose name and value are determined by the pair passed as value.

#### Examples

```xml
<tag s:prop.0="#hello|#world" s:prop.1="#hallo|#Welt"/>
```

### when

To test if the current element should be shown, if and only if the expression is `true`.
