> [!WARNING]  
> Documentation is an ongoing effort.

`vs.templ` is a simple XML preprocessor. Its design objectives are:

- to be easily embeddable
- low memory & storage footprint (special builds can make it viable on embedded devices)

It can be used to statically generate XML documents from a _template_ definition and one or more _data sources_.  
Static templates can be seen as simple programs serialized in XML. `vs.templ` is a virtual machine which can run them.  
Input is provided from the data sources (usually XML documents as well), and a new one is returned as output.

You can check out our [playground](https://lazy-eggplant.github.io/vs.templ/wasm-demo/) to get familiar with `vs.templ` capabilities and syntax.
If you want a more comprehensive and verbose description, you can check the [dedicated documentation](https://lazy-eggplant.github.io/vs.templ/next/docs).  
We also [generate doxygen](https://lazy-eggplant.github.io/vs.templ/next/doxygen) documentation to support developers who want to integrate `vs.templ` in their own projects.

### Quick example

Given an XML data source

```xml
<items>
    <item prop-a="Value 1">Text A</item>
    <item prop-a="Value 2">Text B</item>
    <item prop-a="Value 3">Text C</item>
</items>
```

and a template

```xml
<ul>
<s:for in="/items/" sort-by="~prop-a" order-by="desc">
    <s:item>
        <li><s:value src="~prop-a"/>: <s:value src="~!txt"/></li>
    </s:item>
</s:for>
</ul>
```

after processing by `vs.templ`, the final result will be:

```xml
<ul>
    <li>Value 3: Text C</li>
    <li>Value 2: Text B</li>
    <li>Value 1: Text A</li>
</ul>
```

For more examples, please check the [online playground](https://lazy-eggplant.github.io/vs.templ/wasm-demo/).

## Usage

This preprocessor can be used in one of two ways:

### As a library

`vs.templ` is specifically intended to be embedded in other applications.  
You can use it either as a dynamic or a static library.  
If you want to integrate `vs.templ` in your own codebase, [this](./docs/for-developers.md) is where to start.

### Via its CLI

This repo provides a minimal standalone wrapper to access most features offered by this library from command line.

```bash
vs.templ <template-file> <data-file> [namespace=`s:`]
```

There is also an alternative format:

```bash
vs.templ [namespace=`s:`]
```

with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)`

#### vs.templ.js

If you don't want to install `vs.templ`, there is a standalone wasm/wasi build available in the release information.  
To use it just run:

```bash
bun run ./vs.templ.js [...]
```

## Installation

Check the [Makefile](./Makefile). It will work out of the box on most systems.  
There are some flags in there you can configure without having to touch Meson directly.

## Projects using `vs.templ`

- [vs](https://github.com/KaruroChori/vs-fltk) the parent project from which `vs.templ` derived.
- [enance-amamento](https://github.com/KaruroChori/enance-amamento) as templating engine for the imported XML files.
- [vs.http](https://github.com/lazy-eggplant/vs.http) an HTTP server built around `vs.templ`.

## Why?

`vs.templ` was first developed in the context of [vs](https://github.com/karurochori/vs-fltk) to express static, yet parametric, components.  
While the XML ecosystem is often reliant on XSLT as a preprocessor, this option was quickly dismissed in the parent project for several reasons:

- The rest of the `vs` is based on `pugixml`. The only lightweight XSLT 1.0 implementation which is decently licensed is [libxslt](https://gitlab.gnome.org/GNOME/libxslt) based on [libxml2](https://gitlab.gnome.org/GNOME/libxml2).  
  Because of that, a trivial integration would have been quite wasteful: an additional serialization and parsing stage is needed to move the tree structure between libraries.
- The scope of XSLT, even in its very first version was excessive & its syntax just verbose. This is mostly a result of `xpath` being too powerful of a tool.
- At some point `vs` will come with dynamic native components, and the idea is for them to match in syntax and behaviour the extended tags introduced by this preprocessor. A custom implementation is the only way to ensure a seamless integration between the two.

Hence, `vs` vendors its own XSLT-ish preprocessor.  
Still, nothing about its semantics or syntax is directly tied to `vs`, so I am distributing it as a separate package, hoping it can reach a wider adoption.

To cover further questions, I collected some in a [FAQ](./docs/faq.md) document.
