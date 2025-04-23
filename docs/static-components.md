> [!NOTE]  
> These specs are not part of `vs.templ`.  
> They are just temporarily hosted here to decide what to do about that.

## What is this
These are specifications for an agnostic component model based on XML.  
It allows packaging code as single file components, and reuse it inside others.

## Tags and attributes

### `ns:component`
Root for any component.
Further attributes will be assumed as default values for when they are later injected.

### `ns:section`
A section inside the component, to logically split its content and let the downstream application handle it on its own.
- [ ] `type` a marker, usually in the form `logic-function/format`. Used as metadata downstream.
Any other argument will also be handled downstream.

### `ns:import`
- [ ] `src` a path-like to the object to import, handled externally
- [ ] `as` (optional) a name to use the resource, if not provided downstream will assign one based on path
Any other attribute is accepted, and will be package as part of the import request.

Body content is used as alternative if loading fails.

### `ns:include`
- [ ] `src` a path-like to the object to import, handled externally
In-place substitution of content. Used to introduce global things in the component scope.

Body content is used as alternative if loading fails.

### `ns:use`
- [ ] `s:src` either path or local resolution with prefix `#`
Any other attribute is passed on.
Body contains slots, some of which have special features, allowing for error handling for example.

### `ns-x:*` namespace
Elements in this namespace are shorthands for the `use` command.

### `ns:slot`
- [ ] `s:name` identifier for the slot. If empty it is the default one. Special name `error` in case of component failure.
Any further argument is applied to the injection location.

Its body is also for injection.

### `ns:inject` as element
- [ ] `src`
Body is default for cases in which the injection fails.

### `ns:inject.src.xxx` and `ns:inject.defval.xxx` as props
Like their element counterpart, with the restriction of being simple strings as they operate on attributes.

### `ns:scope`
It just forces a new scope in which imports and mixins can hide what came before.
Totally optional, each component imposes its own scope, and the parent one is not being inherited.

### `ns:throw`
To close the process early. At times an error is so bad that it is not possible to continue.

### `ns:debug`
Like for `vs.fltk`.
- [ ] `type` enum with values from `warning`, `info`, `panic`, `ok`, `error`, `log` with `log` being default.
- [ ] `msg` as a string

### `ns:mixin`
-  [ ] `ns:rule`
All the other attributes are part of the mixin definition.

### `ns:mixins` attribute
To apply a list of mixins to the current element, beyond those which are implicitly applied.
Mixins are **always** evaluated and replaced in place. They don't pass barriers between components.

## Examples

Check in `examples/advanced/components`.