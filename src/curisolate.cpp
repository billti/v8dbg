#include "curisolate.h"

int GetIsolateKey(IDebugHostContext* pCtx) {
  auto spSym = Extension::currentExtension->spDebugHostSymbols;

  // Loop through the modules looking for the one that holds the "isolate_key_"
  winrt::com_ptr<IDebugHostSymbolEnumerator> spEnum;
  if (SUCCEEDED(spSym->EnumerateModules(pCtx, spEnum.put()))) {
    HRESULT hr = S_OK;
    while (true) {
      winrt::com_ptr<IDebugHostSymbol> spModSym;
      hr = spEnum->GetNext(spModSym.put());
      // hr == E_BOUNDS : hit the end of the enumerator
      // hr == E_ABORT  : a user interrupt was requested
      if (FAILED(hr)) break;
      winrt::com_ptr<IDebugHostModule> spModule;
      if (spModSym.try_as(spModule)) /* should always succeed */
      {
        winrt::com_ptr<IDebugHostSymbol> spIsolateSym;
        hr = spModule->FindSymbolByName(L"isolate_key_", spIsolateSym.put());
        if (SUCCEEDED(hr)) {
          Extension::currentExtension->spV8Module = spModule;
          SymbolKind kind;
          hr = spIsolateSym->GetSymbolKind(&kind);
          if (SUCCEEDED(hr)) {
            if (kind == SymbolData) {
              winrt::com_ptr<IDebugHostData> spIsolateKeyData;
              spIsolateSym.as(spIsolateKeyData);
              Location loc;
              hr = spIsolateKeyData->GetLocation(&loc);
              if (SUCCEEDED(hr)) {
                int isolate_key;
                ULONG64 bytesRead;
                hr = Extension::currentExtension->spDebugHostMemory->ReadBytes(pCtx, loc, &isolate_key, 4, &bytesRead);
                return isolate_key;
              }
            }
          }
        }
      }
    }
  }
  return -1;
}

  HRESULT __stdcall CurrIsolateAlias::Call(IModelObject* pContextObject, ULONG64 argCount,
                         _In_reads_(argCount) IModelObject** ppArguments,
                         IModelObject** ppResult,
                         IKeyStore** ppMetadata) noexcept {
    HRESULT hr = S_OK;
    *ppResult = nullptr;

    // TODO: Get @$curthread.Environment.EnvironmentBlock.TlsSlots (which should
    // be a void*[64]) Indexed via the static member:
    // v8!v8::internal::Isolate::isolate_key_ [Type: int]

    // Get the current context
    winrt::com_ptr<IDebugHostContext> spHostContext;
    winrt::com_ptr<IModelObject> spRootNamespace;
    hr = spDebugHost->GetCurrentContext(spHostContext.put());
    if (FAILED(hr)) return E_FAIL;

    winrt::com_ptr<IModelObject> spCurrThread;
    if(!GetCurrentThread(spHostContext, spCurrThread.put())) {
      return E_FAIL;
    }

    winrt::com_ptr<IModelObject> spEnvironment, spEnvironmentBlock;
    winrt::com_ptr<IModelObject> spTlsSlots, spSlotIndex, spIsolatePtr;
    hr =
        spCurrThread->GetKeyValue(L"Environment", spEnvironment.put(), nullptr);
    if (FAILED(hr)) return E_FAIL;

    hr = spEnvironment->GetKeyValue(L"EnvironmentBlock",
                                    spEnvironmentBlock.put(), nullptr);
    if (FAILED(hr)) return E_FAIL;

    // Unlike prior object, which are "synthetic", EnvironmentBlock and TlsSlots
    // are native types (TypeUDT) and thus GetRawValue rather than GetKeyValue
    // should be used to get field (member) values.
    ModelObjectKind kind;
    hr = spEnvironmentBlock->GetKind(&kind);
    if (kind != ModelObjectKind::ObjectTargetObject) return E_FAIL;

    hr = spEnvironmentBlock->GetRawValue(SymbolField, L"TlsSlots", 0,
                                          spTlsSlots.put());
    if (FAILED(hr)) return E_FAIL;

    int isolate_key = GetIsolateKey(spHostContext.get());
    hr = CreateInt32(isolate_key, spSlotIndex.put());
    if (isolate_key == -1 || FAILED(hr)) return E_FAIL;

    hr = GetModelAtIndex(spTlsSlots, spSlotIndex, spIsolatePtr.put());
    if (FAILED(hr)) return E_FAIL;

    // Need to dereference the slot and then get the address held in it
    winrt::com_ptr<IModelObject> spDereferencedSlot;
    hr = spIsolatePtr->Dereference(spDereferencedSlot.put());
    if (FAILED(hr)) return E_FAIL;

    VARIANT vtIsolatePtr;
    hr = spDereferencedSlot->GetIntrinsicValue(&vtIsolatePtr);
    if (FAILED(hr) || vtIsolatePtr.vt != VT_UI8) {
      return E_FAIL;
    }
    Location isolate_addr{vtIsolatePtr.ullVal};

    // If we got the isolate_key OK, then must have the V8 module loaded
    // Get the internal Isolate type from it
    winrt::com_ptr<IDebugHostType> spIsolateType, spIsolatePtrType;
    hr = Extension::currentExtension->spV8Module->FindTypeByName(L"v8::internal::Isolate", spIsolateType.put());
    hr = spIsolateType->CreatePointerTo(PointerStandard, spIsolatePtrType.put());

    winrt::com_ptr<IModelObject> spResult;
    hr = spDataModelManager->CreateTypedObject(spHostContext.get(), isolate_addr, spIsolateType.get(), spResult.put());

    if (SUCCEEDED(hr)) *ppResult = spResult.detach();
    return hr;
  }
