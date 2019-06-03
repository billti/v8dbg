#pragma once

#include <crtdbg.h>
#include <string>
#include <vector>
#include "../dbgext.h"
#include "extension.h"
#include "v8.h"

winrt::com_ptr<IModelObject> GetAtIndex(
    winrt::com_ptr<IModelObject>& spParent,
    winrt::com_ptr<IModelObject>& spIndexer) {
  winrt::com_ptr<IModelObject> spResult;
  winrt::com_ptr<IIndexableConcept> spIndexableConcept;
  HRESULT hr = spParent->GetConcept(
      __uuidof(IIndexableConcept),
      reinterpret_cast<IUnknown**>(spIndexableConcept.put()), nullptr);
  if (FAILED(hr)) throw L"Couldn't get indexable concept";
  std::vector<IModelObject*> pIndexers{spIndexer.get()};
  hr = spIndexableConcept->GetAt(spParent.get(), 1, pIndexers.data(),
                                 spResult.put(), nullptr);
  if (FAILED(hr)) throw L"Couldn't locate index";
  return spResult;
}

int GetIsolateKey(IDebugHostContext* pCtx) {
  // Enumerate all modules in the current UI context (process) of the debug
  // host:
  auto spSym = Extension::currentExtension->spHostSymbols;

  winrt::com_ptr<IDebugHostSymbolEnumerator> spEnum;
  if (SUCCEEDED(spSym->EnumerateModules(pCtx, spEnum.put()))) {
    HRESULT hr = S_OK;
    while (true) {
      winrt::com_ptr<IDebugHostSymbol> spModSym;
      hr = spEnum->GetNext(spModSym.put());
      if (FAILED(hr)) break;
      winrt::com_ptr<IDebugHostModule> spModule;
      if (spModSym.try_as(spModule)) /* should always succeed */
      {
        // spModule is one of the modules in the context
        BSTR moduleName;
        hr = spModule->GetName(&moduleName);
        ::SysFreeString(moduleName);

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
                hr = Extension::currentExtension->spDebugMemory->ReadBytes(pCtx, loc, &isolate_key, 4, &bytesRead);
                return isolate_key;
              }
            }
          }
        }
      }
    }
    // hr == E_BOUNDS : hit the end of the enumerator
    // hr == E_ABORT  : a user interrupt was requested /
    //                  propagate upwards immediately
  }
  return -1;
}

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

    // Get the current context
    winrt::com_ptr<IDebugHostContext> spHostContext;
    winrt::com_ptr<IModelObject> spRootNamespace, spBoxedContext;
    hr = spDebugHost->GetCurrentContext(spHostContext.put());

    // TODO Test
    int isolate_key = GetIsolateKey(spHostContext.get());
    if (isolate_key == -1) return E_FAIL;

    VARIANT vtContext;
    vtContext.vt = VT_UNKNOWN;
    vtContext.punkVal = spHostContext.get();
    spDataModelManager->CreateIntrinsicObject(ObjectContext, &vtContext,
                                              spBoxedContext.put());

    hr = spDataModelManager->GetRootNamespace(spRootNamespace.put());

    winrt::com_ptr<IModelObject> spDebugger, spSessions, spProcesses, spThreads;
    hr = spRootNamespace->GetKeyValue(L"Debugger", spDebugger.put(), nullptr);

    hr = spDebugger->GetKeyValue(L"Sessions", spSessions.put(), nullptr);
    winrt::com_ptr<IModelObject> spCurrSession =
        GetAtIndex(spSessions, spBoxedContext);

    hr = spCurrSession->GetKeyValue(L"Processes", spProcesses.put(), nullptr);
    winrt::com_ptr<IModelObject> spCurrProcess =
        GetAtIndex(spProcesses, spBoxedContext);

    hr = spCurrProcess->GetKeyValue(L"Threads", spThreads.put(), nullptr);
    winrt::com_ptr<IModelObject> spCurrThread =
        GetAtIndex(spThreads, spBoxedContext);

    winrt::com_ptr<IModelObject> spEnvironment, spEnvironmentBlock, spTlsSlots,
        spSlotIndex;
    hr =
        spCurrThread->GetKeyValue(L"Environment", spEnvironment.put(), nullptr);

    // Below returns a HRESULT of 0x01. EnvironmentBlock is of type
    // ObjectTargetObject (Others above are of type ObjectSynthetic)
    hr = spEnvironment->GetKeyValue(L"EnvironmentBlock",
                                    spEnvironmentBlock.put(), nullptr);

    // This first attempt returns an HRESULT of 0x80070057 (The parameter is
    // incorrect.) hr = spEnvironmentBlock->GetKeyValue(L"TlsSlots",
    // spTlsSlots.put(), nullptr); The below seems to work.
    ModelObjectKind kind;
    hr = spEnvironmentBlock->GetKind(&kind);
    if (kind == ModelObjectKind::ObjectTargetObject) {
      // TODO What does this mean exactly that it's a TargetObject? Let's check
      // the type...
      winrt::com_ptr<IDebugHostType> spDebugHostType;
      hr = spEnvironmentBlock->GetTypeInfo(spDebugHostType.put());
      TypeKind typeKind;
      hr = spDebugHostType->GetTypeKind(&typeKind);
      // It's a TypeUDT. Maybe GetRawValue?
      hr = spEnvironmentBlock->GetRawValue(SymbolField, L"TlsSlots", 0,
                                           spTlsSlots.put());
      // Seems to work? :shrug:
    }

    hr = CreateInt32(isolate_key, spSlotIndex.put());

    auto isolatePtr = GetAtIndex(spTlsSlots, spSlotIndex);
    // Above also return 0x01 and is ObjectTargetReference. Maybe dereference?
    winrt::com_ptr<IModelObject> spDereferencedSlot;
    hr = isolatePtr->Dereference(spDereferencedSlot.put());
    VARIANT vtIsolatePtr;
    hr = spDereferencedSlot->GetIntrinsicValue(&vtIsolatePtr);
    if (FAILED(hr) || vtIsolatePtr.vt != VT_UI8) {
      return E_FAIL;
    }
    Location isolate_addr{vtIsolatePtr.ullVal};

    // If we got the isolate_key OK, then must have the V8 module
    winrt::com_ptr<IDebugHostType> spIsolateType, spIsolatePtr;
    hr = Extension::currentExtension->spV8Module->FindTypeByName(L"v8::internal::Isolate", spIsolateType.put());
    hr = spIsolateType->CreatePointerTo(PointerStandard, spIsolatePtr.put());

    winrt::com_ptr<IModelObject> spResult;
    hr = spDataModelManager->CreateTypedObject(spHostContext.get(), isolate_addr, spIsolateType.get(), spResult.put());

    if (SUCCEEDED(hr)) *ppResult = spResult.detach();
    return hr;
  }
};
