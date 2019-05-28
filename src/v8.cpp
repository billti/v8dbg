#include <crtdbg.h>
#include "v8.h"
#include "v8-layout.h"
#include <Windows.h>

#if defined(WIN32)
inline std::u16string ConvertToU16String(std::string utf8String) {
  int lenChars = ::MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);

  char16_t *pBuff = static_cast<char16_t*>(malloc(lenChars * sizeof(char16_t)));

  // On Windows wchar_t is the same a 16char_t
  static_assert(sizeof(wchar_t) == sizeof(char16_t));
  lenChars = ::MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, 
      reinterpret_cast<wchar_t*>(pBuff), lenChars);
  std::u16string result{pBuff};
  free(pBuff);

  return result;
}
#else
  #error String encoding conversion must be provided for the target platform.
#endif

V8HeapObject GetHeapObject(MemReader memReader, uint64_t address) {
  V8HeapObject obj;
  obj.HeapAddress = address;

  // The first value in the Object's memory space is the Map pointer.
  // This is also tagged, so clear the low 2 bits.
  ReadTypeFromMemory(memReader, address, &obj.Map.HeapAddress);
  obj.Map.HeapAddress = UnTagPtr(obj.Map.HeapAddress);

  uint64_t instanceTypeAddr = obj.Map.HeapAddress + 12;
  ReadTypeFromMemory(memReader, instanceTypeAddr, &obj.Map.InstanceType);

  auto typeName = V8::Layout::InstanceTypeToDescriptor.find(obj.Map.InstanceType);

  if (typeName == V8::Layout::InstanceTypeToDescriptor.end()) {
    // Couldn't find the type of object from the map. Not much else can be done.
    return obj;
  }

  obj.Map.TypeName = typeName->second->name;

  // TODO: Loop through the properties reading/populating the vector
  if (std::u16string{typeName->second->name}.compare(u"v8::internal::SeqOneByteString") == 0) {
    int strLength;
    ReadTypeFromMemory(memReader, obj.HeapAddress + 12, &strLength);
    uint64_t strAddress = obj.HeapAddress + 16;

    // Need to get the single-byte representation and convert
    // Create a null initialized string one longer than the length
    std::string strValueSingleByte(static_cast<size_t>(strLength + 1), '\0');
    memReader(strAddress, strLength, reinterpret_cast<uint8_t*>(strValueSingleByte.data()));
    std::u16string strValue = ConvertToU16String(strValueSingleByte);

    obj.Properties.push_back(Property{u"Length", strLength});
    obj.Properties.push_back(Property{u"Value", strValue});
    // TODO: obj.FriendlyName
  }

  return obj;
}
