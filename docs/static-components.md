> [!NOTE]  
> These specs are not part of `vs.templ`.  
> They are just temporarily hosted here to decide what to do about that.

## Tags and attributes

### `ns:component`
Root for components
Any attribute is considered as default value for when it is later injected.

### `ns:section`
- [ ] `type`

### `ns:import`
- [ ] `src`
- [ ] `as` (optional)

Body is the alternative if loading fails.

### `ns:use`
- [ ] `s:src` either path or local resolution with prefix `#`

### `ns:slot`
- [ ] `s:name` identifier for the slot. If empty it is the default one. Special name `error` in case of component failure.

### `ns:inject` as element
- [ ] `src`
Body is default in case the injection fails

### `ns:inject.xxx` and `ns:inject-default.xxx` as props

### `ns:scope`
It just forces a new scope in which imports can hide what came before.
Totally optional, each component imposes its own scope, and the parent one is not being inherited.

### `ns:throw`

### `ns:debug`
Like for `vs.templ`.

### `ns-x:*` namespace
It references any imported component which appeared before.

## Examples

Check in `examples/advanced/components`.