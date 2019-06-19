#pragma once

#include <crtdbg.h>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>


namespace V8::Layout {

using byte = uint8_t;

// TODO: Should this code call the debugger to use symbols for these offsets?
const int kOff_isolate_to_isolate_data = 0;
const int kOff_isolate_to_heap_ = 0x90e8;
const int kOff_isolate_data_to_roots_ = 0x38;

enum class FieldType: uint16_t {
  kTaggedSize,
  kUInt8Size,
  kUInt16Size,
  kInt32Size,
  kUInt32Size
};

struct FieldInfo {
  std::u16string name;
  FieldType type;
  uint16_t  offset;
};

using FieldVector = std::vector<FieldInfo>;

struct ObjectDesc {
  std::u16string name;
  std::u16string baseType;
  FieldVector fields;
  uint16_t headerSize;
};

class V8Layout {
 public:
  std::map<std::u16string, ObjectDesc> TypeNameToDescriptor;
  std::map<uint16_t, std::u16string> InstanceTypeToTypeName;
  std::map<int, std::u16string> OddballKindToName;

  V8Layout::V8Layout() {
    // Generated list of object layouts
    FieldVector No_Fields = {};

    FieldVector HeapObject_Fields = {
      {u"Map", FieldType::kTaggedSize, 0},
    };
    ObjectDesc HeapObject = {u"v8::internal::HeapObject", /* no base */ u"", HeapObject_Fields, 8};

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
      {u"InstanceDescriptors", FieldType::kTaggedSize, 40},
      {u"LayoutDescriptor", FieldType::kTaggedSize, 48},
      {u"DependentCode", FieldType::kTaggedSize, 56},
      {u"PrototypeValidityCell", FieldType::kTaggedSize, 64},
      {u"TransitionsOrPrototypeInfo", FieldType::kTaggedSize, 72},
    };
    ObjectDesc Map = {u"v8::internal::Map", HeapObject.name, Map_Fields, 80};

    FieldVector Name_Fields = {
      {u"HashField", FieldType::kInt32Size, 8}
    };
    ObjectDesc Name = {u"v8::internal::Name", HeapObject.name, Name_Fields, 12};

    FieldVector String_Fields = {
      {u"Length", FieldType::kInt32Size, 12}
    };
    ObjectDesc String = {u"v8::internal::String", Name.name, String_Fields, 16};

    // Below 3 types don't add any fields on top of the base type
    ObjectDesc SeqString = {u"v8::internal::SeqString", String.name, No_Fields, 16};
    ObjectDesc SeqOneByteString = {u"v8::internal::SeqOneByteString", SeqString.name, No_Fields, 16};
    ObjectDesc SeqTwoByteString = {u"v8::internal::SeqTwoByteString", SeqString.name, No_Fields, 16};

    FieldVector ConsString_Fields = {
      {u"First", FieldType::kTaggedSize, 16},
      {u"Second", FieldType::kTaggedSize, 24},
    };
    ObjectDesc ConsString = {u"v8::internal::ConsString", String.name, ConsString_Fields, 32};

    FieldVector Script_Fields = {
      {u"Source", FieldType::kTaggedSize, 8},
      {u"Name", FieldType::kTaggedSize, 16},
      {u"LineOffset", FieldType::kTaggedSize, 24},
      {u"ColumnOffset", FieldType::kTaggedSize, 32},
      {u"Context", FieldType::kTaggedSize, 40},
      {u"ScriptType", FieldType::kTaggedSize, 48},
      {u"LineEnds", FieldType::kTaggedSize, 56},
      {u"Id", FieldType::kTaggedSize, 64},
      {u"EvalFromSharedOrWrappedArguments", FieldType::kTaggedSize, 72},
      {u"EvalFromPosition", FieldType::kTaggedSize, 80},
      {u"SharedFunctionInfos", FieldType::kTaggedSize, 88},
      {u"Flags", FieldType::kTaggedSize, 96},
      {u"SourceUrl", FieldType::kTaggedSize, 104},
      {u"SourceMappingUrl", FieldType::kTaggedSize, 112},
      {u"HostDefinedOptions", FieldType::kTaggedSize, 120},
    };
    ObjectDesc Script = {u"v8::internal::Script", HeapObject.name, Script_Fields, 128};

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
    ObjectDesc SharedFunctionInfo = {u"v8::internal::SharedFunctionInfo", HeapObject.name, SharedFunctionInfo_Fields, 56};

    FieldVector JSReceiver_Fields = {
      {u"PropertiesOrHash", FieldType::kTaggedSize, 8},
    };
    ObjectDesc JSReceiver = {u"v8::internal::JSReceiver", HeapObject.name, JSReceiver_Fields, 16};

    FieldVector JSObject_Fields = {
      {u"Elements", FieldType::kTaggedSize, 16},
    };
    ObjectDesc JSObject = {u"v8::internal::JSObject", JSReceiver.name, JSObject_Fields, 24};

    FieldVector JSFunction_Fields = {
      {u"SharedFunctionInfo", FieldType::kTaggedSize, 24},
      {u"Context", FieldType::kTaggedSize, 32},
      {u"FeedbackCell", FieldType::kTaggedSize, 40},
      {u"Code", FieldType::kTaggedSize, 48},
      {u"PrototypeOrInitialMap", FieldType::kTaggedSize, 56},
    };
    ObjectDesc JSFunction = {u"v8::internal::JSFunction", JSObject.name, JSFunction_Fields, 64};

    FieldVector FixedArrayBase_Fields = {
      {u"Length", FieldType::kTaggedSize, 8},
    };
    ObjectDesc FixedArrayBase = {u"v8::internal::FixedArrayBase", HeapObject.name, FixedArrayBase_Fields, 16};
    ObjectDesc FixedArray = {u"v8::internal::FixedArray", FixedArrayBase.name, No_Fields, 16};

    FieldVector DescriptorArray_Fields = {
      {u"NumberOfAllDescriptors", FieldType::kUInt16Size, 8},
      {u"NumberOfDescriptors", FieldType::kUInt16Size, 10},
      {u"RawNumberOfMarkedDescriptors", FieldType::kUInt16Size, 12},
      {u"Filler16Bits", FieldType::kUInt16Size, 14},
      {u"EnumCache", FieldType::kTaggedSize, 16},
    };
    ObjectDesc DescriptorArray = {u"v8::internal::DescriptorArray", HeapObject.name, DescriptorArray_Fields, 24};

    // Taken from class-defintions-tq.h for TorqueGeneratedOddball
    FieldVector Oddball_Fields = {
      {u"ToNumberRaw", FieldType::kTaggedSize, 8},
      {u"ToString", FieldType::kTaggedSize, 16},
      {u"ToNumber", FieldType::kTaggedSize, 24},
      {u"TypeOf", FieldType::kTaggedSize, 32},
      {u"Kind", FieldType::kTaggedSize, 40},
    };
    ObjectDesc Oddball = {u"v8::internal::Oddball", HeapObject.name, Oddball_Fields, 48};



    // Maps from instance type and class name to object descriptor
    TypeNameToDescriptor = {
      {HeapObject.name, HeapObject},
      {Map.name, Map},
      {Name.name, Name},
      {String.name, String},
      {SeqString.name, SeqString},
      {SeqOneByteString.name, SeqOneByteString},
      {SeqTwoByteString.name, SeqTwoByteString},
      {ConsString.name, ConsString},
      {SharedFunctionInfo.name, SharedFunctionInfo},
      {Script.name, Script},
      {JSReceiver.name, JSReceiver},
      {JSObject.name, JSObject},
      {JSFunction.name, JSFunction},
      {FixedArrayBase.name, FixedArrayBase},
      {FixedArray.name, FixedArray},
      {Oddball.name, Oddball},
      {DescriptorArray.name, DescriptorArray},
      // Register internal types for public types
      {u"v8::String", String},
      // TODO...
    };

// Note: You can get these from the PDB also ("dt v8!v8::internal::InstanceType")
    InstanceTypeToTypeName = {
      {0x21, ConsString.name},
      {0x28, SeqOneByteString.name},
      {0x43, Oddball.name},
      {0x44, Map.name},
      {0x60, Script.name},
      {0x76, FixedArray.name},
      {0x94, DescriptorArray.name},
      {0x9b, SharedFunctionInfo.name},
      {0x421, JSObject.name},
      {0x451, JSFunction.name},
      // TODO...
    };

// Based on constants in oddball.h
    OddballKindToName = {
      {0, u"False"},
      {1, u"True"},
      {2, u"TheHole"},
      {3, u"Null"},
      {4, u"ArgumentsMarker"},
      {5, u"Undefined"},
      {6, u"Uninitialized"},
      {7, u"Other"},
      {8, u"Exception"},
      {9, u"OptimizedOut"},
      {10, u"StaleRegister"},
    };
  }
};

} // namespace V8::Layout
