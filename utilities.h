#pragma once

#include "dbgext.h"

inline const wchar_t* U16ToWChar(const char16_t *pU16) {
  return reinterpret_cast<const wchar_t*>(pU16);
}

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
