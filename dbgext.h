#pragma once

#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>

#include <winrt/base.h>

// Globals for use throughout the extension. (Populated on load).
extern winrt::com_ptr<IDataModelManager> spDataModelManager;
extern winrt::com_ptr<IDebugHost> spDebugHost;

// To be implemented by the custom extension code. (Called on load).
bool CreateExtension();
void DestroyExtension();

// Utility functions
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
