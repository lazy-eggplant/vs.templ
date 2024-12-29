---
title: User Manual
---

`vs-templ` is a simple preprocessor for XML files.  
It can be used statically generate new files from a template definition.  
Static templates can be seen as extremely simple programs which are interpreted by this preprocessor.  
They consume input data also formatted as XML, and generate some output XML file.  
In theory, it is possible for data to be expressed in other formats (eg. JSON) as well, but at the moment this is not a supported feature.

## CLI

```
vs.tmpl <template-file> <data-file> [namespace=`s:`]
```

Unlike its usage in vs.fltk, template must be specified on its own, as it cannot be inferred.

There is also an alternative format:

```
vs.tmpl [namespace=`s:`]
```

with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)`

## Syntax quick reference

`vs.templ` uses special elements and attributes to determine the actions to be performed by the preprocessor.  
They are scoped under the namespace `s`, or any custom defined one.  
This man page only covers a quick syntax reference.  
If you are looking for the full specs, please check the [official repository](https://github.com/KaruroChori/vs-templ).

### Path expressions

Expression are used to access elements and attributes of the data XML from the template.  
Their definition and usage is purposefully restricted to prevent arbitrary code to be run.  
A full list of feasible expression types:

- String, automatically assigned from expressions starting with `#` (the prefix is skipped)
- Integers (base 10), automatically assigned from expressions starting with a digit, `+`, `-` or `.`
- Paths starting with `$`. This special symbol is used to mark the nearest scope being visited or root if none.
- Paths with arbitrary prefix `{var-name}` where `var-name`is searched for and resolved from the symbols' stack.
- Absolute paths starting from the root, with prefix `/`.

The rest of a path expression has one or more tokens `/`-terminated representing the tag name being visited.  
If terminated with `~prop-name` the relevant attribute is selected.

There are also two special properties:

- Special access to the property`~!txt` to get the node text.
- Special access to the element's name via `~!tag`

No further combination or format is allowed, and if used they might lead to undefined behaviour.  
However, the preprocessor should not result in exceptions.
