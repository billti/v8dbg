#include "curisolate.h"

int GetIsolateKey(winrt::com_ptr<IDebugHostContext>& spCtx) {
  auto spV8Module = Extension::currentExtension->GetV8Module(spCtx);
  if (spV8Module == nullptr) return -1;

  winrt::com_ptr<IDebugHostSymbol> spIsolateSym;
  HRESULT hr =
      spV8Module->FindSymbolByName(L"isolate_key_", spIsolateSym.put());
  if (SUCCEEDED(hr)) {
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
          hr = Extension::currentExtension->spDebugHostMemory->ReadBytes(
              spCtx.get(), loc, &isolate_key, 4, &bytesRead);
          return isolate_key;
        }
      }
    }
  }
  return -1;
}

HRESULT GetCurrentIsolate(winrt::com_ptr<IModelObject>& spResult) {
  HRESULT hr = S_OK;
  spResult = nullptr;

  // Get the current context
  winrt::com_ptr<IDebugHostContext> spHostContext;
  winrt::com_ptr<IModelObject> spRootNamespace;
  hr = spDebugHost->GetCurrentContext(spHostContext.put());
  if (FAILED(hr)) return hr;

  winrt::com_ptr<IModelObject> spCurrThread;
  if (!GetCurrentThread(spHostContext, spCurrThread.put())) {
    return E_FAIL;
  }

  winrt::com_ptr<IModelObject> spEnvironment, spEnvironmentBlock;
  winrt::com_ptr<IModelObject> spTlsSlots, spSlotIndex, spIsolatePtr;
  hr = spCurrThread->GetKeyValue(L"Environment", spEnvironment.put(), nullptr);
  if (FAILED(hr)) return E_FAIL;

  hr = spEnvironment->GetKeyValue(L"EnvironmentBlock", spEnvironmentBlock.put(),
                                  nullptr);
  if (FAILED(hr)) return E_FAIL;

  // EnvironmentBlock and TlsSlots are native types (TypeUDT) and thus GetRawValue
  // rather than GetKeyValue should be used to get field (member) values.
  ModelObjectKind kind;
  hr = spEnvironmentBlock->GetKind(&kind);
  if (kind != ModelObjectKind::ObjectTargetObject) return E_FAIL;

  hr = spEnvironmentBlock->GetRawValue(SymbolField, L"TlsSlots", 0,
                                       spTlsSlots.put());
  if (FAILED(hr)) return E_FAIL;

  int isolate_key = GetIsolateKey(spHostContext);
  hr = CreateInt32(isolate_key, spSlotIndex.put());
  if (isolate_key == -1 || FAILED(hr)) return E_FAIL;

  hr = GetModelAtIndex(spTlsSlots, spSlotIndex, spIsolatePtr.put());
  if (FAILED(hr)) return E_FAIL;

  // Need to dereference the slot and then get the address held in it
  winrt::com_ptr<IModelObject> spDereferencedSlot;
  hr = spIsolatePtr->Dereference(spDereferencedSlot.put());
  if (FAILED(hr)) return hr;

  VARIANT vtIsolatePtr;
  hr = spDereferencedSlot->GetIntrinsicValue(&vtIsolatePtr);
  if (FAILED(hr) || vtIsolatePtr.vt != VT_UI8) {
    return E_FAIL;
  }
  Location isolate_addr{vtIsolatePtr.ullVal};

  // If we got the isolate_key OK, then must have the V8 module loaded
  // Get the internal Isolate type from it
  winrt::com_ptr<IDebugHostType> spIsolateType, spIsolatePtrType;
  hr = Extension::currentExtension->GetV8Module(spHostContext)
           ->FindTypeByName(L"v8::internal::Isolate", spIsolateType.put());
  if (FAILED(hr)) return hr;
  hr = spIsolateType->CreatePointerTo(PointerStandard, spIsolatePtrType.put());
  if (FAILED(hr)) return hr;

  hr = spDataModelManager->CreateTypedObject(
      spHostContext.get(), isolate_addr, spIsolateType.get(), spResult.put());
  if (FAILED(hr)) return hr;

  return S_OK;
}

HRESULT __stdcall CurrIsolateAlias::Call(IModelObject* pContextObject,
                                         ULONG64 argCount,
                                         IModelObject** ppArguments,
                                         IModelObject** ppResult,
                                         IKeyStore** ppMetadata) noexcept {
  HRESULT hr = S_OK;
  *ppResult = nullptr;
  winrt::com_ptr<IModelObject> spResult;
  hr = GetCurrentIsolate(spResult);
  if (SUCCEEDED(hr)) *ppResult = spResult.detach();
  return hr;
}
