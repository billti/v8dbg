#pragma once

#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>

#include <winrt/base.h>

struct MyPropertyAccessor : winrt::implements<MyPropertyAccessor, IModelPropertyAccessor>
{

    HRESULT __stdcall GetValue(
        PCWSTR key,
        IModelObject* contextObject,
        IModelObject** value
    ) noexcept override;

    HRESULT __stdcall SetValue(
        PCWSTR key,
        IModelObject* contextObject,
        IModelObject* value
    ) noexcept override
    {
        return E_NOTIMPL;
    }
};
