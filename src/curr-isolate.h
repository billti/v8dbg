#pragma once

#include <crtdbg.h>
#include <string>
#include <vector>
#include "../dbgext.h"
#include "extension.h"
#include "v8.h"

struct CurrIsolateAlias : winrt::implements<CurrIsolateAlias, IModelMethod> {
  HRESULT __stdcall Call(IModelObject* pContextObject, ULONG64 argCount,
                         _In_reads_(argCount) IModelObject** ppArguments,
                         IModelObject** ppResult,
                         IKeyStore** ppMetadata) noexcept override {
    HRESULT hr = S_OK;
    *ppResult = nullptr;

    // TODO: Get @$curthread.Environment.EnvironmentBlock.TlsSlots (which should
    // be a void*[64]) Indexed via the static member:
    // v8!v8::internal::Isolate::isolate_key_ [Type: int]

    winrt::com_ptr<IModelObject> spResult;
    hr = CreateString(u"This is the isolate", spResult.put());
    if (SUCCEEDED(hr)) *ppResult = spResult.detach();
    return hr;
  }
};
