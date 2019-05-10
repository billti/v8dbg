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

extern "C" {

	__declspec(dllexport) HRESULT CALLBACK DebugExtensionInitialize(PULONG /*pVersion*/, PULONG /*pFlags*/)
	{
		HRESULT hr = DebugCreate(__uuidof(IDebugClient), spDebugClient.put_void());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugClient.try_as(spDataModelAccess)) return E_FAIL;

        hr = spDataModelAccess->GetDataModel(spDataModelManager.put(), spDebugHost.put());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugHost.try_as(spDebugHostMemory)) return E_FAIL;

        winrt::com_ptr<IModelObject> spProcess, spModel;
        hr = spDataModelManager->AcquireNamedModel(L"Debugger.Models.Process", spProcess.put());
        if(FAILED(hr)) return E_FAIL;

        auto MyDataModel{ winrt::make<Foo>()};

        hr = spDataModelManager->CreateDataModelObject(MyDataModel.get(), spModel.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spModel->SetConcept(__uuidof(IStringDisplayableConcept), MyDataModel.get(), nullptr);
        if(FAILED(hr)) return E_FAIL;

        hr = spProcess->AddParentModel(spModel.get(), nullptr, false);
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
