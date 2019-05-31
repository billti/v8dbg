#include <crtdbg.h>
#include "extension.h"
#include "v8.h"
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

V8HeapObject GetHeapObject(MemReader memReader, uint64_t taggedPtr) {
  // Read the value at the address, and see if it is a tagged pointer

  V8HeapObject obj;

  // Check for non-objects first
  if (!(taggedPtr & 0x01)) {
    if (taggedPtr == 0xCCCCCCCCCCCCCCCC)
    {
      obj.HeapAddress = taggedPtr;
      obj.FriendlyName = u"<uninitialized>";
    } else {
      // TODO: Only handles 64-bit platforms with uncompressed pointers currently
      obj.IsSmi = true;
      obj.HeapAddress = static_cast<int>(static_cast<intptr_t>(taggedPtr) >> 32);
      std::wstring smiValue = std::to_wstring(obj.HeapAddress);
      obj.FriendlyName = std::u16string{u"<Smi>: "} + reinterpret_cast<const char16_t*>(smiValue.c_str());
    }
    return obj;
  }

  obj.HeapAddress = UnTagPtr(taggedPtr);
  // The first value in the Object's memory space is the Map pointer.
  // This is also tagged, so clear the low 2 bits.
  ReadTypeFromMemory(memReader, obj.HeapAddress, &obj.Map.HeapAddress);
  obj.Map.HeapAddress = UnTagPtr(obj.Map.HeapAddress);

  uint64_t instanceTypeAddr = obj.Map.HeapAddress + 12;
  ReadTypeFromMemory(memReader, instanceTypeAddr, &obj.Map.InstanceType);

  V8::Layout::V8Layout& v8Layout = Extension::currentExtension->v8Layout;

  auto typeName = v8Layout.InstanceTypeToTypeName.find(obj.Map.InstanceType);

  if (typeName == v8Layout.InstanceTypeToTypeName.end()) {
    // Couldn't find the type of object from the map. Not much else can be done.
    return obj;
  }

  obj.Map.TypeName = typeName->second;

  // TODO: Loop through the properties reading/populating the vector
  if (typeName->second.compare(u"v8::internal::SeqOneByteString") == 0) {
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

    obj.FriendlyName = std::u16string{u"<SeqOneByteString>: "} + strValue;
  }

  return obj;
}
