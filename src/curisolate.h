#pragma once

#include <crtdbg.h>
#include <string>
#include <vector>
#include "../utilities.h"
#include "extension.h"
#include "v8.h"

int GetIsolateKey(winrt::com_ptr<IDebugHostContext>& spCtx);
HRESULT GetCurrentIsolate(winrt::com_ptr<IModelObject>& spResult);

struct CurrIsolateAlias : winrt::implements<CurrIsolateAlias, IModelMethod> {
  HRESULT __stdcall Call(IModelObject* pContextObject, ULONG64 argCount,
                         _In_reads_(argCount) IModelObject** ppArguments,
                         IModelObject** ppResult,
                         IKeyStore** ppMetadata) noexcept override;
};
