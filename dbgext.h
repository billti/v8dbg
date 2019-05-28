#pragma once

#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>

#include <winrt/base.h>
#include <string>

// Globals for use throughout the extension. (Populated on load).
extern winrt::com_ptr<IDataModelManager> spDataModelManager;
extern winrt::com_ptr<IDebugHost> spDebugHost;

// To be implemented by the custom extension code. (Called on load).
bool CreateExtension();
void DestroyExtension();

// Utility functions

inline const wchar_t* U16ToWChar(const char16_t *pU16) {
  return reinterpret_cast<const wchar_t*>(pU16);
}

HRESULT inline CreateProperty(
    IDataModelManager *pManager,
    IModelPropertyAccessor *pProperty,
    IModelObject **ppPropertyObject)
{
    *ppPropertyObject = nullptr;

    VARIANT vtVal;
    vtVal.vt = VT_UNKNOWN;
    vtVal.punkVal = pProperty;

    HRESULT hr = pManager->CreateIntrinsicObject(ObjectPropertyAccessor, &vtVal, ppPropertyObject);
    return hr;
}

HRESULT inline CreateULong64(
    ULONG64 value,
    IModelObject **ppInt)
{
    HRESULT hr = S_OK;
    *ppInt = nullptr;

    VARIANT vtVal;
    vtVal.vt = VT_UI8;
    vtVal.ullVal = value;

    hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppInt);
    return hr;
}

HRESULT inline CreateInt32(
    int value,
    IModelObject **ppInt)
{
    HRESULT hr = S_OK;
    *ppInt = nullptr;

    VARIANT vtVal;
    vtVal.vt = VT_I4;
    vtVal.intVal = value;

    hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppInt);
    return hr;
}

HRESULT inline CreateBool(
    bool value,
    IModelObject **ppVal)
{
    HRESULT hr = S_OK;
    *ppVal = nullptr;

    VARIANT vtVal;
    vtVal.vt = VT_BOOL;
    vtVal.boolVal = value;

    hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
    return hr;
}

HRESULT inline CreateNumber(
    double value,
    IModelObject **ppVal)
{
    HRESULT hr = S_OK;
    *ppVal = nullptr;

    VARIANT vtVal;
    vtVal.vt = VT_R8;
    vtVal.dblVal = value;

    hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
    return hr;
}

HRESULT inline CreateString(
  std::u16string value,
  IModelObject **ppVal)
{
  HRESULT hr = S_OK;
  ppVal = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_BSTR;
  vtVal.bstrVal = ::SysAllocString(reinterpret_cast<const OLECHAR*>(value.c_str()));

  hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
  return hr;
}
