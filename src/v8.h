#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

using MemReader = std::function<bool(uint64_t address, size_t size, uint8_t* buffer)>;

inline uint64_t UnTagPtr(uint64_t ptr) { return ptr &= ~0x03ull; }

inline std::u16string ConvertToU16String(std::string utf8String);

enum class PropertyType {
  Smi,
  Number,
  String,
  Bool,
  Address,
  JSObject,
  NativeObject
};

struct Property {
  Property(std::u16string propertyName, int value)
      : name(propertyName), smiValue(value), type(PropertyType::Smi) {}
  Property(std::u16string propertyName, std::u16string value)
      : name(propertyName), strValue(value), type(PropertyType::String) {}

  std::u16string name;
  PropertyType type;

  // strValue is used if the type is String or Object
  // An NativeObject will have its type in strValue, and pointer in addrValue
  std::u16string strValue;

  union {
    int smiValue;
    double numValue;
    bool boolValue;
    uint64_t addrValue;
  };
};

struct V8MapObject {
  uint64_t HeapAddress;
  uint16_t InstanceType;
  std::u16string TypeName;
  // TODO Other fields, including slots etc.
};

struct V8HeapObject {
  uint64_t HeapAddress;
  bool IsSmi = false;
  std::u16string FriendlyName;  // e.g. string: "Hello, world"
  V8MapObject Map;
  std::vector<Property> Properties;
};

template <typename T>
bool ReadTypeFromMemory(MemReader memReader, uint64_t address, T* buffer) {
  size_t byteCount = sizeof *buffer;
  memReader(address, byteCount, reinterpret_cast<uint8_t*>(buffer));
  return true;
}

V8HeapObject GetHeapObject(MemReader memReader, uint64_t address);
