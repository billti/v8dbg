#pragma once

#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>

#include <winrt/base.h>

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
