> [!WARNING]  
> Some features are still missing, they are tracked in the [TODO](./TODO.md) file.  
> Documentation is an ongoing effort.

`vs-templ` is a simple preprocessor for XML files.  
It can be used to statically generate new files from a template definition and a data source.  
Static templates can be seen as extremely simple programs serialized in XML which are interpreted by this preprocessor.
They consume input data (also XML) to generated output XML.

Details about the syntax & supported features are covered in a [dedicated page](https://lazy-eggplant.github.io/vs.templ/next/docs), or you might want to check some [examples](https://lazy-eggplant.github.io/vs.templ/wasm-demo/).

## Examples

### Simple `for` cycle

```xml
<items>
    <item prop-a="Value 1">Text A</item>
    <item prop-a="Value 2">Text B</item>
    <item prop-a="Value 3">Text C</item>
</items>
```

and

```xml
<ul>
<s:for in="/items/" sort-by="~prop-a" order-by="desc">
    <s:item>
        <li><s:value src="~prop-a"/>: <s:value src="~!txt"/></li>
    </s:item>
</s:for>
</ul>
```

results in

```xml
<ul>
    <li>Value 3: Text C</li>
    <li>Value 2: Text B</li>
    <li>Value 1: Text A</li>
</ul>
```

For more examples check the [online playground](https://lazy-eggplant.github.io/vs.templ/wasm-demo/)!

## Usage

The functionality of this template builder is exposed as a library.  
You can link it to your own application either statically or dynamically.
This repository is also providing a self-contained CLI utility as a system utility.

### As a library

If you want to integrate `vs-templ` in your own application or contribute to its development, [this](./docs/for-developers.md) is where to start.

### Via its CLI

```
vs.tmpl <template-file> <data-file> [namespace=`s:`]
```

There is also an alternative format:

```
vs.tmpl [namespace=`s:`]
```

with both files added via pipes, like `vs.tmpl <(cat template.xml) <(cat data.xml)`

## Installation

```bash
meson setup build
meson install -C build
```

should work on most systems. I highly suggest a dry run by setting `DESTDIR` to ensure expected behaviour.

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
