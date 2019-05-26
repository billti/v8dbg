// This file SHOULD BE generated to describe the object layouts in V8

#include <crtdbg.h>
#include "v8-layout.h"

namespace V8::Layout {

FieldVector No_Fields = {};

// Generated list of object layouts
FieldVector HeapObject_Fields = {
  {u"Map", FieldType::kTaggedSize, 0},
};
ObjectDesc HeapObject = {u"v8::internal::HeapObject", /* no base */ nullptr, &HeapObject_Fields, 8};

FieldVector Map_Fields = {
  {u"InstanceSizeInWords", FieldType::kUInt8Size, 8},
  {u"InObjectPropertiesStartOrConstructorFunctionIndex", FieldType::kUInt8Size, 9},
  {u"UsedOrUnusedInstanceSizeInWords", FieldType::kUInt8Size, 10},
  {u"VisitorId", FieldType::kUInt8Size, 11},
  {u"InstanceType", FieldType::kUInt16Size, 12},
  {u"BitField", FieldType::kUInt8Size, 14},
  {u"BitField2", FieldType::kUInt8Size, 15},
  {u"BitField3", FieldType::kUInt32Size, 16},
  {u"OptionalPadding", FieldType::kUInt32Size, 20},
  {u"Prototype", FieldType::kTaggedSize, 24},
  {u"ConstructorOrBackPointer", FieldType::kTaggedSize, 32},
  {u"TransitionsOrPrototypeInfo", FieldType::kTaggedSize, 40},
  {u"Descriptors", FieldType::kTaggedSize, 48},
  {u"LayoutDescriptor", FieldType::kTaggedSize, 56},
  {u"DependentCode", FieldType::kTaggedSize, 64},
  {u"PrototypeValidityCell", FieldType::kTaggedSize, 72}
};
ObjectDesc Map = {u"v8::internal::Map", &HeapObject, &Map_Fields, 80};

FieldVector Name_Fields = {
  {u"HashField", FieldType::kInt32Size, 8}
};
ObjectDesc Name = {u"v8::internal::Name", &HeapObject, &Name_Fields, 12};

FieldVector String_Fields = {
  {u"Length", FieldType::kInt32Size, 12}
};
ObjectDesc String = {u"v8::internal::String", &Name, &String_Fields, 16};

// Below 3 types don't add any fields on top of the base type
ObjectDesc SeqString = {u"v8::internal::SeqString", &String, &No_Fields, 16};
ObjectDesc SeqOneByteString = {u"v8::internal::SeqOneByteString", &SeqString, &No_Fields, 16};
ObjectDesc SeqTwoByteString = {u"v8::internal::SeqTwoByteString", &SeqString, &No_Fields, 16};

FieldVector ConsString_Fields = {
  {u"First", FieldType::kTaggedSize, 16},
  {u"Second", FieldType::kTaggedSize, 24},
};
ObjectDesc ConsString = {u"v8::internal::ConsString", &String, &ConsString_Fields, 32};

FieldVector SharedFunctionInfo_Fields = {
  {u"FunctionData", FieldType::kTaggedSize, 8},
  {u"NameOrScopeInfo", FieldType::kTaggedSize, 16},
  {u"OuterScopeInfoOrFeedbackMetaData", FieldType::kTaggedSize, 24},
  {u"ScriptOrDebugInfo", FieldType::kTaggedSize, 32},
  {u"Length", FieldType::kUInt16Size, 40},
  {u"FormalParameterCount", FieldType::kUInt16Size, 42},
  {u"ExpectedNofProperties", FieldType::kUInt16Size, 44},
  {u"FunctionTokenOffset", FieldType::kUInt16Size, 46},
  {u"Flags", FieldType::kInt32Size, 48},
  {u"UniqueId", FieldType::kInt32Size, 52},
};
ObjectDesc SharedFunctionInfo = {u"v8::internal::SharedFunctionInfo", &HeapObject, &SharedFunctionInfo_Fields, 56};

// Maps from instance type and class name to object descriptor
std::map<uint16_t, ObjectDesc*> InstanceTypeToDescriptor = {
  {0x28, &SeqOneByteString},
  {0x55, &Map},
  // TODO...
};

std::map<std::u16string, ObjectDesc*> TypeNameToDescriptor = {
  {HeapObject.name, &HeapObject},
  {Map.name, &Map},
  {Name.name, &Name},
  {String.name, &String},
  {SeqString.name, &SeqString},
  {SeqOneByteString.name, &SeqOneByteString},
  {SeqTwoByteString.name, &SeqTwoByteString},
  {ConsString.name, &ConsString},
  {SharedFunctionInfo.name, &SharedFunctionInfo},
  // Register internal types for public types
  {u"v8::String", &String},
  // TODO...
};

} // namespace V8::Layout
