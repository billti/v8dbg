#pragma once

#include "dbgext.h"

inline const wchar_t* U16ToWChar(const char16_t *pU16) {
  return reinterpret_cast<const wchar_t*>(pU16);
}

inline const wchar_t* U16ToWChar(std::u16string& str) {
  return U16ToWChar(str.data());
}

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

HRESULT CreateProperty(
    IDataModelManager *pManager,
    IModelPropertyAccessor *pProperty,
    IModelObject **ppPropertyObject);

HRESULT CreateULong64(ULONG64 value, IModelObject **ppInt);

HRESULT CreateInt32(int value, IModelObject **ppInt);

HRESULT CreateBool(bool value, IModelObject **ppVal);

HRESULT CreateNumber(double value, IModelObject **ppVal);

HRESULT CreateString(std::u16string value, IModelObject **ppVal);

bool GetModelAtIndex(winrt::com_ptr<IModelObject>& spParent,
                     winrt::com_ptr<IModelObject>& spIndex,
                     IModelObject **pResult);

bool GetCurrentThread(winrt::com_ptr<IDebugHostContext>& spHostContext,
                      IModelObject** pCurrentThread);
