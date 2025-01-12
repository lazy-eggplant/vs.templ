> [!WARNING]  
> Documentation is an ongoing effort.

`vs-templ` is a simple XML preprocessor.  
It can be used to statically generate new files from a _template_ definition and a _data source_.  
Static templates can be seen as extremely simple programs which are themselves serialized as XML, and interpreted by this preprocessor.  
While running, they consume input data (also XML, but could be anything else in principle) to generated output XML.

Details about the syntax & supported features are covered in a [dedicated page](https://lazy-eggplant.github.io/vs.templ/next/docs).  
You might want to check some [examples](https://lazy-eggplant.github.io/vs.templ/wasm-demo/) for a less verbose introduction to the syntax.

### Typical template definition

Given an XML data source:

```xml
<items>
    <item prop-a="Value 1">Text A</item>
    <item prop-a="Value 2">Text B</item>
    <item prop-a="Value 3">Text C</item>
</items>
```

and a template:

```xml
<ul>
<s:for in="/items/" sort-by="~prop-a" order-by="desc">
    <s:item>
        <li><s:value src="~prop-a"/>: <s:value src="~!txt"/></li>
    </s:item>
</s:for>
</ul>
```

once processed, the final result will be:

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

If you don't want to install `vs.templ`, it can be used standalone if you have a compatible WASM runtime on your system.  
WASM builds are attached to releases.

```bash
bun run ./vs.templ.js [...]
```

## Installation

```bash
meson setup build
meson install -C build
```

This works on most systems.  
Still, to ensure expected behaviour, I highly suggest a dry run by setting `DESTDIR` somewhere safe.

## Projects using `vs.templ`

- [vs](https://github.com/KaruroChori/vs-fltk) the parent project from which `vs.templ` derived.
- [vs.http](https://github.com/lazy-eggplant/vs.http) an HTTP server built around `vs.templ`.

## Why?

`vs-templ` was first developed in the context of [vs](https://github.com/karurochori/vs-fltk) to express static, yet parametric, components.  
While the XML ecosystem is often reliant on XSLT as a preprocessor, this option was quickly dismissed in the parent project for several reasons:

- The rest of the `vs` is based on `pugixml`. The only lightweight XSLT 1.0 implementation which is decently licensed is [libxslt](https://gitlab.gnome.org/GNOME/libxslt) based on [libxml2](https://gitlab.gnome.org/GNOME/libxml2).  
  Because of that, a trivial integration would have been quite wasteful: an additional serialization and parsing stage is needed to move the tree structure between libraries.
- The scope of XSLT, even in its very first version was excessive & its syntax just verbose. This is mostly a result of `xpath` being too powerful of a tool.
- At some point `vs` will come with dynamic native components, and the idea is for them to match in syntax and behaviour the extended tags introduced by this preprocessor. A custom implementation is the only way to ensure a seamless integration between the two.

Hence, `vs` vendors its own XSLT-ish preprocessor.  
Still, nothing about its semantics or syntax is directly tied to `vs`, so I am distributing it as a separate package, hoping it can reach a wider adoption.

If you have more questions, I collected some in a [FAQ](./docs/faq.md) document.
