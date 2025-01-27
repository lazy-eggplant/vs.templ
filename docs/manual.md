---
title: User Manual
---

`vs.templ` is a simple preprocessor for XML files.  
It can be used statically generate new files from a template definition.  
Static templates can be seen as extremely simple programs which are interpreted by this preprocessor.  
They consume input data also formatted as XML, and generate some output XML file.  
In theory, it is possible for data to be expressed in other formats (e.g. JSON) as well, but at the moment this is not a supported feature.

## CLI

```bash
vs.tmpl <template-file> <data-file> [namespace=`s:`]
```

Unlike its usage in `vs.fltk`, template must be specified on its own, as it cannot be inferred form a request in the data file.

There is also an alternative format:

```bash
vs.tmpl [namespace=`s:`]
```

with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)`

## Syntax quick reference

`vs.templ` uses special elements and attributes to determine the actions to be performed by the preprocessor.  
They are scoped under the namespace `s`, or any custom defined one.  
This man page only covers a quick syntax reference.  
If you are looking for the full specs, please check the [official repository](https://github.com/KaruroChori/vs-templ).

TODO: write cheat sheet of syntax here.
