---
title: FAQ
---

## Is there a SAX implementation?

No, there is not.
Some features don't strictly require a DOM, but they don't really translate well in terms of a pull parser.  
The memory peak usage would not be much better either for many reasonable templates, so there is no real incentive to implement this preprocessor based on a different backend for now.  
Still, if you want to do so you are very welcome!

## Can we support other input formats for the dataset?

In theory, it would be possible for data to be expressed in other formats (eg. JSON) as well, but at the moment this is not a supported feature and is not likely going to be in scope for quite a while.  
However, native integration of SQLite as a data-source will happen for sure at some point.
