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

  winrt::com_ptr<IDebugHostContext> spCtx;
  hr = pV8LocalInstance->GetContext(spCtx.put());
  if (FAILED(hr)) return hr;
  winrt::com_ptr<IDebugHostType> spV8ObjectType = Extension::currentExtension->GetV8ObjectType(spCtx);

  Location loc;
  hr = pV8LocalInstance->GetLocation(&loc);
  if (FAILED(hr)) return hr;

  // Read the pointer at the Object location
  ULONG64 objAddress;
  hr = ext->spDebugHostMemory->ReadPointers(spCtx.get(), loc, 1, &objAddress);
  if (FAILED(hr)) return hr;

  // If the val_ is a nullptr, then there is no value in the Local.
  if(objAddress == 0) {
    hr = CreateString(std::u16string{u"<empty>"}, ppValue);
  } else {
    // Should be a v8::internal::Object at the address
    hr = spDataModelManager->CreateTypedObject(spCtx.get(), objAddress, spV8ObjectType.get(), ppValue);
  }

  return hr;
}
