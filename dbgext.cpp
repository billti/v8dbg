#include "dbgext.h"

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

		return S_OK;
	}

	HRESULT CALLBACK DebugExtensionCanUnload(void)
	{
		return S_OK;
	}

	void CALLBACK DebugExtensionUninitialize()
	{
		return;
	}

	void CALLBACK DebugExtensionUnload()
	{
		return;
	}
}
