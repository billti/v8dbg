#include <crtdbg.h>
#include "dbgext.h"

winrt::com_ptr<IDataModelManager> spDataModelManager;
winrt::com_ptr<IDebugHost> spDebugHost;

extern "C" {
__declspec(dllexport) HRESULT __stdcall DebugExtensionInitialize(PULONG /*pVersion*/, PULONG /*pFlags*/) {
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

  winrt::com_ptr<IDebugClient> spDebugClient;
  winrt::com_ptr<IHostDataModelAccess> spDataModelAccess;

  HRESULT hr = DebugCreate(__uuidof(IDebugClient), spDebugClient.put_void());
  if (FAILED(hr)) return E_FAIL;

  if (!spDebugClient.try_as(spDataModelAccess)) return E_FAIL;

  hr = spDataModelAccess->GetDataModel(spDataModelManager.put(),
                                       spDebugHost.put());
  if (FAILED(hr)) return E_FAIL;

  return CreateExtension() ? S_OK : E_FAIL;
}

__declspec(dllexport) HRESULT __stdcall DebugExtensionCanUnload(void) {
  auto lock_count = winrt::get_module_lock().load();
  return lock_count == 0;
}

__declspec(dllexport) void __stdcall DebugExtensionUninitialize() {
  DestroyExtension();
  spDebugHost = nullptr;
  spDataModelManager = nullptr;
  _CrtDumpMemoryLeaks();
}

__declspec(dllexport) void __stdcall DebugExtensionUnload() {
  return;
}

} // extern "C"
