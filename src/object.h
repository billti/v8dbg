#pragma once

#include "../dbgext.h"
#include "extension.h"
#include "v8.h"
#include <vector>
#include <string>

// The representation of the underlying V8 object that will be cached on the
// DataModel representation. (Needs to implement IUnknown).
struct __declspec(uuid("6392E072-37BB-4220-A5FF-114098923A02")) IV8CachedObject: IUnknown {
  virtual HRESULT __stdcall GetCachedV8HeapObject(V8HeapObject** ppHeapObject) = 0;
};

struct V8CachedObject: winrt::implements<V8CachedObject, IV8CachedObject> {
  V8CachedObject(IModelObject* pV8ObjectInstance) {
    Location loc;
    HRESULT hr = pV8ObjectInstance->GetLocation(&loc);
    if(FAILED(hr)) return; // TODO error handling

    auto memReader = [](uint64_t address, size_t size, uint8_t *pBuffer) {
      ULONG64 bytes_read;
      Location loc{address};
      HRESULT hr = Extension::currentExtension->spDebugMemory->ReadBytes(USE_CURRENT_HOST_CONTEXT, loc, pBuffer, size, &bytes_read);
      return SUCCEEDED(hr);
    };

    heapObject = ::GetHeapObject(memReader, loc.GetOffset());
  }

  V8HeapObject heapObject;

  HRESULT __stdcall GetCachedV8HeapObject(V8HeapObject** ppHeapObject) noexcept override {
    *ppHeapObject = &this->heapObject;
    return S_OK;
  }
};

struct V8ObjectKeyEnumerator: winrt::implements<V8ObjectKeyEnumerator, IKeyEnumerator>
{
  V8ObjectKeyEnumerator(winrt::com_ptr<IV8CachedObject> &v8CachedObject)
      :spV8CachedObject{v8CachedObject} {}

  int index = 0;
  winrt::com_ptr<IV8CachedObject> spV8CachedObject;

  HRESULT __stdcall Reset() noexcept override
  {
    index = 0;
    return S_OK;
  }

  // This method will be called with a nullptr 'value' for each key if returned
  // from an IDynamicKeyProviderConcept. It will call GetKey on the
  // IDynamicKeyProviderConcept interface after each key returned.
  HRESULT __stdcall GetNext(
      BSTR* key,
      IModelObject** value,
      IKeyStore** metadata
  ) noexcept override
  {
    V8HeapObject *pV8HeapObject;
    HRESULT hr = spV8CachedObject->GetCachedV8HeapObject(&pV8HeapObject);

    if (index >= pV8HeapObject->Properties.size()) return E_BOUNDS;

    auto namePtr = pV8HeapObject->Properties[index].name.c_str();
    *key = ::SysAllocString(U16ToWChar(namePtr));
    ++index;
    return S_OK;
  }
};

struct V8ObjectDataModel: winrt::implements<V8ObjectDataModel, IDataModelConcept, IStringDisplayableConcept, IDynamicKeyProviderConcept>
{
    winrt::com_ptr<IV8CachedObject> GetCachedObject(IModelObject* contextObject) {
      // Get the IModelObject for this parent object. As it is a dynamic provider,
      // there is only one parent directly on the object.
      winrt::com_ptr<IModelObject> spParentModel, spContextAdjuster;
      HRESULT hr = contextObject->GetParentModel(0, spParentModel.put(), spContextAdjuster.put());

      // See if the cached object is already present
      winrt::com_ptr<IUnknown> spContext;
      hr = contextObject->GetContextForDataModel(spParentModel.get(), spContext.put());

      winrt::com_ptr<IV8CachedObject> spV8CachedObject;

      if(SUCCEEDED(hr)) {
        spV8CachedObject = spContext.as<IV8CachedObject>();
      } else {
        spV8CachedObject = winrt::make<V8CachedObject>(contextObject);
        spV8CachedObject.as(spContext);
        contextObject->SetContextForDataModel(spParentModel.get(), spContext.get());
      }

      return spV8CachedObject;
    }

    HRESULT __stdcall InitializeObject(
        IModelObject* modelObject,
        IDebugHostTypeSignature* matchingTypeSignature,
        IDebugHostSymbolEnumerator* wildcardMatches
    ) noexcept override
    {
        return S_OK;
    }

    HRESULT __stdcall GetName( BSTR* modelName) noexcept override
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall ToDisplayString(
        IModelObject* contextObject,
        IKeyStore* metadata,
        BSTR* displayString
    ) noexcept override
    {
        *displayString = ::SysAllocString(L"V8 Object");
        return S_OK;
    }

    // IDynamicKeyProviderConcept
    HRESULT __stdcall GetKey(
        IModelObject *contextObject,
        PCWSTR key,
        IModelObject** keyValue,
        IKeyStore** metadata,
        bool *hasKey
    ) noexcept override
    {
      winrt::com_ptr<IV8CachedObject> spV8CachedObject = GetCachedObject(contextObject);
      V8HeapObject* pV8HeapObject;
      HRESULT hr = spV8CachedObject->GetCachedV8HeapObject(&pV8HeapObject);

      *hasKey = false;
      for(auto &k: pV8HeapObject->Properties) {
        const char16_t *pKey = reinterpret_cast<const char16_t*>(key);
        if (k.name.compare(pKey) == 0) {
          *hasKey = true;
          if(keyValue != nullptr) {
            winrt::com_ptr<IModelObject> spValue;
            switch (k.type) {
              case PropertyType::Smi:
                CreateInt32(k.smiValue, spValue.put());
                *keyValue = spValue.detach();
                break;
              case PropertyType::Address:
                CreateULong64(k.addrValue, spValue.put());
                *keyValue = spValue.detach();
                break;
              case PropertyType::String:
                CreateString(k.strValue, spValue.put());
                *keyValue = spValue.detach();
                break;
              case PropertyType::Bool:
                CreateBool(k.boolValue, spValue.put());
                *keyValue = spValue.detach();
                break;
              case PropertyType::Number:
                CreateNumber(k.numValue, spValue.put());
                *keyValue = spValue.detach();
                break;
              // TODO: Other types (e.g. nested objects)
              default:
                assert(false);
            }
          }
          return S_OK;
        }
      }

      // TODO: Should this be E_* if not found?
      return S_OK;
    }

    HRESULT __stdcall SetKey(
        IModelObject *contextObject,
        PCWSTR key,
        IModelObject *keyValue,
        IKeyStore *metadata
    ) noexcept override
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall EnumerateKeys(
        IModelObject *contextObject,
        IKeyEnumerator **ppEnumerator
    ) noexcept override
    {
      auto spV8CachedObject = GetCachedObject(contextObject);

      auto enumerator{ winrt::make<V8ObjectKeyEnumerator>(spV8CachedObject)};
      *ppEnumerator = enumerator.detach();
      return S_OK;
    }
};

// TODO: Below is not currently used.
struct V8ObjectContentsProperty: winrt::implements<V8ObjectContentsProperty, IModelPropertyAccessor>
{
    HRESULT __stdcall GetValue(
        PCWSTR pwszKey,
        IModelObject *pV8ObjectInstance,
        IModelObject **ppValue);

    HRESULT __stdcall SetValue(
        PCWSTR /*pwszKey*/,
        IModelObject * /*pProcessInstance*/,
        IModelObject * /*pValue*/)
    {
        return E_NOTIMPL;
    }
};

HRESULT CreateHeapSyntheticObject(IDebugHostContext* pContext, ULONG64 heapAddress, IModelObject** ppResult);
