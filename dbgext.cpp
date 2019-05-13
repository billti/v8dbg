#include "dbgext.h"
#include "src/object.h"

using namespace std;

winrt::com_ptr<IDebugClient> spDebugClient;
winrt::com_ptr<IHostDataModelAccess> spDataModelAccess;
winrt::com_ptr<IDataModelManager> spDataModelManager;
winrt::com_ptr<IDebugHost> spDebugHost;
winrt::com_ptr<IDebugHostMemory2> spDebugHostMemory;

extern "C" {

	__declspec(dllexport) HRESULT CALLBACK DebugExtensionInitialize(PULONG /*pVersion*/, PULONG /*pFlags*/)
	{
		HRESULT hr = DebugCreate(__uuidof(IDebugClient), spDebugClient.put_void());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugClient.try_as(spDataModelAccess)) return E_FAIL;

        hr = spDataModelAccess->GetDataModel(spDataModelManager.put(), spDebugHost.put());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugHost.try_as(spDebugHostMemory)) return E_FAIL;

        winrt::com_ptr<IDebugHostSymbols> spHostSymbols;
        winrt::com_ptr<IDebugHostTypeSignature> spTypeSignature;
        winrt::com_ptr<IModelObject> spSignatureModel;

        auto objectDataModel{ winrt::make<V8ObjectDataModel>()};
        hr = spDataModelManager->CreateDataModelObject(objectDataModel.get(), spSignatureModel.put());
        if(FAILED(hr)) return E_FAIL;
        hr = spSignatureModel->SetConcept(__uuidof(IStringDisplayableConcept), objectDataModel.get(), nullptr);
        if(FAILED(hr)) return E_FAIL;

        // Add the 'contents' property on the object
        auto contentsProperty{ winrt::make<V8ObjectContentsProperty>()};
        winrt::com_ptr<IModelObject> spContentsPropertyModel;
        hr = CreateProperty(spDataModelManager.get(), contentsProperty.get(), spContentsPropertyModel.put());
        hr = spSignatureModel->SetKey(L"Contents", spContentsPropertyModel.get(), nullptr);

        if(!spDebugHost.try_as(spHostSymbols)) return E_FAIL;
        hr = spHostSymbols->CreateTypeSignature(L"v8::internal::Object", nullptr, spTypeSignature.put());
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
