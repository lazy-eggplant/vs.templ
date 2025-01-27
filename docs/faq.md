---
title: FAQ
---

## Why not using XSLT?

`vs.templ` was first developed in the context of [vs](https://github.com/karurochori/vs-fltk) to express static, yet parametric, components.  
While the XML ecosystem is often reliant on XSLT as a preprocessor, this option was quickly dismissed in the parent project for several reasons:

- The rest of the `vs` is based on `pugixml`. The only lightweight XSLT 1.0 implementation which is decently licensed is [libxslt](https://gitlab.gnome.org/GNOME/libxslt) based on [libxml2](https://gitlab.gnome.org/GNOME/libxml2).  
  Because of that, a trivial integration would have been quite wasteful: an additional serialization and parsing stage is needed to move the tree structure between libraries.
- The scope of XSLT, even in its very first version was excessive & its syntax just verbose. This is mostly a result of `xpath` being too powerful of a tool.
- At some point `vs` will come with dynamic native components, and the idea is for them to match in syntax and behaviour the extended tags introduced by this preprocessor. A custom implementation is the only way to ensure a seamless integration between the two.

Hence, `vs` vendors `vs.templ` its own XSLT-ish preprocessor.  
Still, nothing about its semantics or syntax is directly tied to `vs`, so I am distributing it as a separate package, hoping it can reach a wider adoption.

## Why not [handlebars](https://handlebarsjs.com/) or [mustache](https://mustache.github.io/)?

This class of templating solutions cannot understand XML.  
As such, the resulting generation will not ensure something syntactically correct is generated.

## Then why not `react` with an XML writer?

That is a fine option, and you will be able to leverage the flexibility of a full JavaScript runtime.  
Well, its bloat as well, you cannot really avoid that.  
On a modern system, the final release build of `vs.templ` as a shared `vs.templ` is less than 300kB.  
If you need an embeddable solution for your pipelines, and you are already using `pugixml`, the cost of integrating `vs.templ` is virtually none.  
Any other solution is going to be much more expensive.

Even when used standalone, `vs.templ` shines: a fully static wasm/wasi build (so `libc`, `libc++`, `pugixml` and any other dependency bundled) is less 7MB.

## Is there a SAX implementation?

No, there is not.  
Some features don't strictly require a DOM, but they don't really translate well in terms of a pull parser.  
The memory peak usage would not be much better either for most reasonable templates, so there is no real incentive to implement this preprocessor based on a different backend for now.  
If one needs to access data sources with thousands or millions of entries before processing and setting the window of interest, external sources are the right approach, and they are fully supported.  
That way, there is no need to load the entire data source in memory as it would happen with an XML file.

## Can we support other input formats for the dataset?

In theory, it would be possible for data to be expressed in other formats (eg. JSON) as well, but at the moment this is not a supported feature and is not likely going to be in scope for quite a while.  
For the time being, you can offer external data sources of whatever type by implementing the optional `loadfn` downstream.  
This allows `vs.templ` to indirectly work with any data source you desire.
