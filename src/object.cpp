#include <crtdbg.h>
#include "object.h"
#include "extension.h"
#include "v8-layout.h"
#include "v8.h"

HRESULT V8LocalValueProperty::GetValue(PCWSTR pwszKey,
                                       IModelObject* pV8LocalInstance,
                                       IModelObject** ppValue) {
  // Get the parametric type within v8::Local<*>
  // Set value to a pointer to an instance of this type.

  auto ext = Extension::currentExtension;
  if (ext == nullptr) return E_FAIL;

  winrt::com_ptr<IDebugHostType> spType;
  HRESULT hr = pV8LocalInstance->GetTypeInfo(spType.put());
  if (FAILED(hr)) return hr;

  bool isGeneric;
  hr = spType->IsGeneric(&isGeneric);
  if (FAILED(hr) || !isGeneric) return E_FAIL;

  winrt::com_ptr<IDebugHostSymbol> spGenericArg;
  hr = spType->GetGenericArgumentAt(0, spGenericArg.put());
  if (FAILED(hr)) return hr;

  winrt::com_ptr<IDebugHostModule> spModule;
  winrt::com_ptr<IDebugHostType> spV8ObjectType;
  hr = spType->GetContainingModule(spModule.put());
  hr = spModule->FindTypeByName(L"v8::internal::Object", spV8ObjectType.put());

  Location loc;
  // Location is an internal representation, but can be read from OK.
  hr = pV8LocalInstance->GetLocation(&loc);
  if (FAILED(hr)) return hr;

  // Read the pointer at the Object location
  winrt::com_ptr<IDebugHostContext> spContext;
  ULONG64 objAddress;
  hr = pV8LocalInstance->GetContext(spContext.put());
  if (FAILED(hr)) return hr;
  hr = ext->spDebugMemory->ReadPointers(spContext.get(), loc, 1, &objAddress);
  if (FAILED(hr)) return hr;

  // Should be a v8::internal::Object at the address
  hr = spDataModelManager->CreateTypedObject(spContext.get(), objAddress, spV8ObjectType.get(), ppValue);
  return hr;
}

// Note: Below is not used. Here for reference purposes.
HRESULT CreateHeapSyntheticObject(IDebugHostContext* pContext,
                                  ULONG64 heapAddress,
                                  IModelObject** ppResult) {
  HRESULT hr = spDataModelManager->CreateSyntheticObject(pContext, ppResult);
  winrt::com_ptr<IModelObject> spHeapAddress;
  winrt::com_ptr<IModelObject> spMapAddress;

  uint64_t mapAddress;
  hr = Extension::currentExtension->spDebugMemory->ReadPointers(
      pContext, Location(heapAddress), 1, &mapAddress);

  // Untag the pointer
  mapAddress = UnTagPtr(mapAddress);

  hr = CreateULong64(heapAddress, spHeapAddress.put());
  hr = CreateULong64(mapAddress, spMapAddress.put());
  hr = (*ppResult)->SetKey(L"HeapAddress", spHeapAddress.get(), nullptr);
  hr = (*ppResult)->SetKey(L"MapAddress", spMapAddress.get(), nullptr);
  return S_OK;
}
