## v0.4.7

Introduced tree-rewrite capabilities with `s:for` via the additional attributes `src-children` and `dst-children`.  
Basically the same for cycle is applied recursively inside the relative path described by `src-children` and applied inside `dst-children`.  

We also added support for more VM functions:
- `timestamp` as the name implies
- `rid` to generate a 256bit random ID serialized as hex string.
- `sid` to generate an `int`-sized sequential ID, autoincrementing and with an optional family ID to have independent sequences.

Finally, logs now have a proper context which provided the path of the several locations which are affected, as well as the offset which can be used to locate the issue.  
For more details check changes to `log_t::ctx`.