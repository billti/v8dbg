#include "dbgext.h"

winrt::com_ptr<IDataModelManager> spDataModelManager;
winrt::com_ptr<IDebugHost> spDebugHost;

extern "C" {
    __declspec(dllexport) HRESULT CALLBACK DebugExtensionInitialize(PULONG /*pVersion*/, PULONG /*pFlags*/)
    {
        winrt::com_ptr<IDebugClient> spDebugClient;
        winrt::com_ptr<IHostDataModelAccess> spDataModelAccess;

        HRESULT hr = DebugCreate(__uuidof(IDebugClient), spDebugClient.put_void());
        if(FAILED(hr)) return E_FAIL;

        if(!spDebugClient.try_as(spDataModelAccess)) return E_FAIL;

        hr = spDataModelAccess->GetDataModel(spDataModelManager.put(), spDebugHost.put());
        if(FAILED(hr)) return E_FAIL;

        return CreateExtension() ? S_OK : E_FAIL;
    }

    __declspec(dllexport) HRESULT CALLBACK DebugExtensionCanUnload(void)
    {
        return winrt::get_module_lock() ? S_FALSE : S_OK;
    }

    __declspec(dllexport) void CALLBACK DebugExtensionUninitialize()
    {
        DestroyExtension();
    }

    __declspec(dllexport) void CALLBACK DebugExtensionUnload() {}
}
