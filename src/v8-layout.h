#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace V8::Layout {

enum class FieldType: uint16_t {
  kTaggedSize,
  kUInt8Size,
  kUInt16Size,
  kInt32Size,
  kUInt32Size
};

struct FieldInfo {
  char16_t* name;
  FieldType type;
  uint16_t  offset;
};

using FieldVector = std::vector<FieldInfo>;

struct ObjectDesc {
  char16_t *name;
  ObjectDesc *baseType;
  FieldVector *fields;
  uint16_t headerSize;
};

extern std::map<std::u16string, ObjectDesc*> TypeNameToDescriptor;
extern std::map<uint16_t, ObjectDesc*> InstanceTypeToDescriptor;

} // namespace V8::Layout
