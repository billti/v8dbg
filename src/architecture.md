# Architecture

This extension is layed out as follows:

- The v8-layout.{cpp,h} files in this directory are the files that would
  ultimately be generated to describe the layout of the V8 objects. These are
  "leaf" files that depend only on the standard template library headers for
  easy inclusion into any project.
- The v8.{cpp,h} files in this directory represent the hand-coded logic to
  understand the interpretation of V8 objects (e.g. that the utf8 sequence of
  bytes for a SeqOneByteString comes after the header fields). This is again
  written to only depend on the standard template library.
- The object.{cpp,h} files in this directory provide the integration
  between the WinDbg specific APIs and the generic V8 source files. This code
  can read raw bytes in memory and return WinDbg representations of objects.
- The extension.{cpp,h} files in this directory provide implementations for the
  CreateExtension and DestroyExtension methods the generic extension files in
  the root directory require, and provide the integration with the above source.

When the extension is initialized, it can register a model with a type, for
example `v8::internal::HeapObject`. When the debugger needs to display that type
it will call a property getter. That getter can return a "synthetic object"
which may contain other properties and types. These properties are typically
"boxed" VARIANTs, such as BSTR, UI4, etc.

The model/property getting should be implemented in extension.cpp, and should
ask object.cpp to get for example "v8::internal::HeapObject" at address 0x0100.

Object will then ask V8 for a representation, which will look-up the type in
the v8-layout files, and construct dynamic properties from the header based on
this. It will then do any custom additions (e.g. the string contents, the
bytecode disassembly, the properties on a JSObject, etc.) and construct the
'synthetic' object to return from the property getter.

The Object created should be recursive, in that it should contain a synthetic
object for its base type. It should also create a brief display string that
provides a useful summary of each type (e.g. value of a smi or string, name of
a function or script, constructor name or first few props of a JSObject).

```text
So how would it handle v8::Local<v8::String>?

1. RegisterModelForType is called for v8::Local<*>
2. This will take the underlying ptr_ value and cast to T1.

1. RegisterModelForType was called on v8::String
2. This had the IPreferredRuntimeTypeConcept which says it is a v8::internal::String

1. RegisterModelForType was called on v8::internal::String
2. The registered model has the IDynamicKeyProviderConcept registered (and IDynamicConceptProvider)
3. When EnumerateKeys is called, all header fields, and any 'computed' fields,
   are returned. (And calculated/cached if not already).
4. As the GetKey methods is called for each, the value is returned.
```

Could there just be one model with RegisterModelForType called repeatedly with
that model for each type (be it internal or not)?

Would this work if for some types PreferredRuntimeType casts it, and then the
cast calls the same model for the properties?

Could the v8.cpp code maintain the mapping of type -> type for the PreferredRuntimeType?

Maybe just put both types in the map of type -> object layout?

How to determine an object type:
 - Is it a tagged pointer?
 - If yes, then cast to a HeapObject pointer.
   - For a HeapObject pointer, the first value is a Map tagged pointer.
   - Untag that pointer, and cast to a v8::internal::Map
   - Get the InstanceType value from the map.
   - Look up the object descriptor in the instancetype->desc mapping.
   - Use the name as the type name, and populate the keys based on the descriptor.
   - Use the type name to index the "calculated" props, and add keys from this.

  Each object is represented by a vector of v8Props. v8Props are either intrinsic
  types (int, double, string, bool, etc.) or 'obj', which is another vector of
  v8Props.

  struct v8Prop {
    Type     type;
    uint64_t address;
    union value {};
  }