#include "../utilities.h"
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
  // TODO: Rather than treat everything as v8::internal::Object, just treat as
  // the generic type if derived from v8::internal::Object.
  if (FAILED(hr)) return hr;

  winrt::com_ptr<IDebugHostModule> spModule;
  winrt::com_ptr<IDebugHostType> spV8ObjectType;

  // TODO: It may be that the initial type and the V8 DLL are in different
  // modules, e.g. v8::Local could be in d8.exe, whereas most V8 types are in
  // v8.dll. Maybe just do a general search (not specific to a module)?
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
  hr = ext->spDebugHostMemory->ReadPointers(spContext.get(), loc, 1, &objAddress);
  if (FAILED(hr)) return hr;

  // If the val_ is a nullptr, then there is no value in the Local.
  if(objAddress == 0) {
    hr = CreateString(std::u16string{u"<empty>"}, ppValue);
  } else {
    // Should be a v8::internal::Object at the address
    hr = spDataModelManager->CreateTypedObject(spContext.get(), objAddress, spV8ObjectType.get(), ppValue);
  }

  return hr;
}
