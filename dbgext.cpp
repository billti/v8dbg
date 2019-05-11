#include "dbgext.h"

using namespace std;

winrt::com_ptr<IDebugClient> spDebugClient;
winrt::com_ptr<IHostDataModelAccess> spDataModelAccess;
winrt::com_ptr<IDataModelManager> spDataModelManager;
winrt::com_ptr<IDebugHost> spDebugHost;
winrt::com_ptr<IDebugHostMemory2> spDebugHostMemory;

struct Foo : winrt::implements<Foo, IDataModelConcept, IStringDisplayableConcept> {
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
        *displayString = ::SysAllocString(L"Sausages");
        return S_OK;
    }
};


struct MyHeapObject : winrt::implements<Foo, IDataModelConcept, IStringDisplayableConcept> {
    HRESULT __stdcall InitializeObject(
        IModelObject* modelObject,
        IDebugHostTypeSignature* matchingTypeSignature,
        IDebugHostSymbolEnumerator* wildcardMatches
    ) noexcept override
    {
        // Should be able to get the ptr_ property.
        winrt::com_ptr<IKeyEnumerator> spKeyEnumerator;
        HRESULT hr = modelObject->EnumerateKeyValues(spKeyEnumerator.put());

        BSTR bstrKey = nullptr;
        winrt::com_ptr<IModelObject> spKeyValue;
        winrt::com_ptr<IKeyStore> spKeyStore;
        ModelObjectKind objKind;

      keyloop:
        hr = spKeyEnumerator->GetNext(&bstrKey, spKeyValue.put(), spKeyStore.put());
        if(SUCCEEDED(hr))
        {
            hr = spKeyValue->GetKind(&objKind);
            // TODO: Get the value
            SysFreeString(bstrKey);
            spKeyValue = nullptr;
            spKeyStore = nullptr;
            goto keyloop;
        }
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
        *displayString = ::SysAllocString(L"MyHeapObject");
        return S_OK;
    }
};

extern "C" {

	__declspec(dllexport) HRESULT CALLBACK DebugExtensionInitialize(PULONG /*pVersion*/, PULONG /*pFlags*/)
	{
		HRESULT hr = DebugCreate(__uuidof(IDebugClient), spDebugClient.put_void());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugClient.try_as(spDataModelAccess)) return E_FAIL;

        hr = spDataModelAccess->GetDataModel(spDataModelManager.put(), spDebugHost.put());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugHost.try_as(spDebugHostMemory)) return E_FAIL;

        // ***** TODO: Temp impl to try and add a property to the process *****
        winrt::com_ptr<IModelObject> spProcess, spModel;
        hr = spDataModelManager->AcquireNamedModel(L"Debugger.Models.Thread", spProcess.put());
        if(FAILED(hr)) return E_FAIL;

        auto MyDataModel{ winrt::make<Foo>()};

        hr = spDataModelManager->CreateDataModelObject(MyDataModel.get(), spModel.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spModel->SetConcept(__uuidof(IStringDisplayableConcept), MyDataModel.get(), nullptr);
        if(FAILED(hr)) return E_FAIL;

        // Create a property on the object
        winrt::com_ptr<IModelObject> spKey;
        VARIANT vt;
        vt.vt = VT_UI8;
        vt.ullVal = 0x5151515151515151;
        hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vt, spKey.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spModel->SetKey(L"Isolate", spKey.get(), nullptr);
        if(FAILED(hr)) return E_FAIL;

        hr = spProcess->AddParentModel(spModel.get(), nullptr, false);
        if(FAILED(hr)) return E_FAIL;

        // ***** Register model for v8::internal::HeapObject
        winrt::com_ptr<IDebugHostSymbols> spHostSymbols;
        winrt::com_ptr<IDebugHostTypeSignature> spTypeSignature;
        winrt::com_ptr<IModelObject> spSignatureModel;

        auto heapObjectModel{ winrt::make<MyHeapObject>()};
        hr = spDataModelManager->CreateDataModelObject(heapObjectModel.get(), spSignatureModel.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spSignatureModel->SetConcept(__uuidof(IStringDisplayableConcept), heapObjectModel.get(), nullptr);
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugHost.try_as(spHostSymbols)) return E_FAIL;
        hr = spHostSymbols->CreateTypeSignature(L"v8::internal::HeapObject", nullptr, spTypeSignature.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spDataModelManager->RegisterModelForTypeSignature(spTypeSignature.get(), spSignatureModel.get());
        if(FAILED(hr)) return E_FAIL;

		return S_OK;
	}

	HRESULT CALLBACK DebugExtensionCanUnload(void)
	{
        if(winrt::get_module_lock())  return S_FALSE;

        return S_OK;
	}

	void CALLBACK DebugExtensionUninitialize()
	{
        // TODO: Free all the objects
		return;
	}

	void CALLBACK DebugExtensionUnload()
	{
		return;
	}
}
