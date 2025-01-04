---
title: FAQ
---

## Why not XSLT?

`vs-templ` was first developed in the context of [vs](https://github.com/karurochori/vs-fltk) to express static, yet parametric, components.  
While the XML ecosystem is often reliant on XSLT as a preprocessor, this option was quickly dismissed in the parent project for several reasons:

- The rest of the `vs` is based on `pugixml`. The only lightweight XSLT 1.0 implementation which is decently licensed is [libxslt](https://gitlab.gnome.org/GNOME/libxslt) based on [libxml2](https://gitlab.gnome.org/GNOME/libxml2).  
  Because of that, a trivial integration would have been quite wasteful: an additional serialization and parsing stage is needed to move the tree structure between libraries.
- The scope of XSLT, even in its very first version was excessive & its syntax just verbose. This is mostly a result of `xpath` being too powerful of a tool.
- At some point `vs` will come with dynamic native components, and the idea is for them to match in syntax and behaviour the extended tags introduced by this preprocessor. A custom implementation is the only way to ensure a seamless integration between the two.

Hence, `vs` vendors `vs.templ` its own XSLT-ish preprocessor.  
Still, nothing about its semantics or syntax is directly tied to `vs`, so I am distributing it as a separate package, hoping it can reach a wider adoption.

## Why not [handlebars](https://handlebarsjs.com/) or [mustache](https://mustache.github.io/)

That class of templating solutions cannot understand XML. As such, the resulting generation can be unsound.

## Is there a SAX implementation?

No, there is not.
Some features don't strictly require a DOM, but they don't really translate well in terms of a pull parser.  
The memory peak usage would not be much better either for many reasonable templates, so there is no real incentive to implement this preprocessor based on a different backend for now.  
Still, if you want to do so you are very welcome!

## Can we support other input formats for the dataset?

In theory, it would be possible for data to be expressed in other formats (eg. JSON) as well, but at the moment this is not a supported feature and is not likely going to be in scope for quite a while.  
However, a native integration of SQLite to use it as data-source is almost surely going to happen at some point.
