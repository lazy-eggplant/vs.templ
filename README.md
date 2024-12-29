> [!WARNING]  
> Some features are still missing, they are tracked in the [TODO](./TODO.md) file.
> Documentation is an ongoing effort.

`vs-templ` is a simple preprocessor for XML files. It can be used statically generate new files from a template definition.  
Static templates can be seen as extremely simple programs which are interpreted by this preprocessor.  
They consume input data also formatted as XML, and generate some output XML file.  
In theory, it is possible for data to be expressed in other formats (eg. JSON) as well, but at the moment this is not a supported feature.

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
<s:for in="$/items/" sort-by="$~prop-a" order-by="desc">
    <s:item>
        <li><s:value src="$~prop-a"/>: <s:value src="$~!txt"/></li>
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

## Usage

The functionality of this template builder is exposed as a library which can be used for static or dynamic linking.  
A self-contained CLI utility is also provided as a frontend.

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

## Syntax

Details about the supported syntax are covered in a [dedicated page](./docs/syntax.md)

## Why?

`vs-templ` was first developed in the context of [vs](https://github.com/karurochori/vs-fltk) to express static, yet parametric, components.  
While the XML ecosystem is often reliant on XSLT as a preprocessor, this option was quickly dismissed for several reasons:

- The rest of the `vs` project is based on `pugixml`. The only lightweight XSLT 1.0 implementation which is decently licensed is [libxslt](https://gitlab.gnome.org/GNOME/libxslt) based on [libxml2](https://gitlab.gnome.org/GNOME/libxml2).  
  Because of that, a trivial integration would have been quite wasteful: an additional serialization and parsing stage is needed to move the tree structure between libraries.
- The scope of XSLT, even in its very first version was excessive & its syntax just verbose. This is mostly a result of `xpath` being too powerful of a tool.
- At some point `vs` will come with dynamic native components, and the idea is for them to match in syntax and behaviour the extended tags introduced by this preprocessor. A custom implementation is the only way to ensure a seamless integration between the two.

Hence, `vs` vendors its own XSLT-ish preprocessor. Still, nothing about its semantics or syntax is directly tied to `vs`. For this reason it is distributed as a separate package, hoping it can reach a wider adoption.
