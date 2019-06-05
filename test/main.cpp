#include "common.h"

#include <cstdio>
#include <exception>

// See the docs at
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/using-the-debugger-engine-api

const char* DbgExt = "C:\\src\\github\\dbgext\\x64\\dbgext.dll";
const char* SymbolPath = "C:\\src\\github\\v8\\out\\x64.debug";
const char* CommandLine =
    "C:\\src\\github\\v8\\out\\x64.debug\\d8.exe -e \"console.log('hello, "
    "world');\"";

void RunTests() {
  // Get the Debug client
  winrt::com_ptr<IDebugClient5> pClient;
  HRESULT hr = DebugCreate(__uuidof(IDebugClient5), pClient.put_void());
  winrt::check_hresult(hr);

  // Set noisy symbol loading on
  winrt::com_ptr<IDebugSymbols> pSymbols;
  hr = pClient->QueryInterface(__uuidof(IDebugSymbols), pSymbols.put_void());
  winrt::check_hresult(hr);

  // Turn on noisy symbol loading
  // hr = pSymbols->AddSymbolOptions(0x80000000 /*SYMOPT_DEBUG*/);
  // winrt::check_hresult(hr);

  // Symbol loading fails if the pdb is in the same folder as the exe, but it's
  // not on the path.
  hr = pSymbols->SetSymbolPath(SymbolPath);
  winrt::check_hresult(hr);

  // Set the callbacks
  MyOutput output;
  MyCallback callback;
  hr = pClient->SetOutputCallbacks(&output);
  winrt::check_hresult(hr);
  hr = pClient->SetEventCallbacks(&callback);
  winrt::check_hresult(hr);

  // Launch the process with the debugger attached
  DEBUG_CREATE_PROCESS_OPTIONS procOptions;
  procOptions.CreateFlags = DEBUG_PROCESS;
  procOptions.EngCreateFlags = 0;
  procOptions.VerifierFlags = 0;
  procOptions.Reserved = 0;
  hr =
      pClient->CreateProcessW(0, const_cast<char*>(CommandLine), DEBUG_PROCESS);
  winrt::check_hresult(hr);

  // Wait for the attach event
  winrt::com_ptr<IDebugControl3> pDebugControl;
  hr = pClient->QueryInterface(__uuidof(IDebugControl3),
                               pDebugControl.put_void());
  winrt::check_hresult(hr);
  hr = pDebugControl->WaitForEvent(0, INFINITE);
  winrt::check_hresult(hr);

  // Set a breakpoint
  // PDEBUG_BREAKPOINT bp;
  winrt::com_ptr<IDebugBreakpoint> bp;
  hr = pDebugControl->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID,
                                    bp.put());
  winrt::check_hresult(hr);
  hr = bp->SetOffsetExpression("d8_exe!v8::Shell::ExecuteString");
  winrt::check_hresult(hr);
  hr = bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
  winrt::check_hresult(hr);

  hr = pDebugControl->SetExecutionStatus(DEBUG_STATUS_GO);
  winrt::check_hresult(hr);

  // Wait for the breakpoint. Fails here saying device is in invalid state, but
  // everything else was S_OK??
  hr = pDebugControl->WaitForEvent(0, INFINITE);
  winrt::check_hresult(hr);

  ULONG type, procId, threadId, descUsed;
  byte desc[1024];
  hr = pDebugControl->GetLastEventInformation(
      &type, &procId, &threadId, nullptr, 0, nullptr,
      reinterpret_cast<PSTR>(desc), 1024, &descUsed);
  winrt::check_hresult(hr);

  ULONG64 extHandle;
  hr = pDebugControl->AddExtension(DbgExt, 0, &extHandle);
  // HACK: Below fails, but is required for the extension to actually
  // initialize. Just the AddExtension call doesn't actually load and initialize
  // it.
  pDebugControl->CallExtension(extHandle, "Foo", "Bar");

  // Step one line to ensure locals are available
  hr = pDebugControl->SetCodeLevel(DEBUG_LEVEL_SOURCE);
  hr =
      pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "p", DEBUG_EXECUTE_ECHO);
  hr = pDebugControl->WaitForEvent(0, INFINITE);

  // Do some actual testing
  output.log.clear();
  hr = pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                              "dx @$curisolate().isolate_data_",
                              DEBUG_EXECUTE_ECHO);
  if (output.log.find("[Type: v8::internal::RootsTable]") ==
      std::string::npos) {
    printf(
        "***ERROR***: 'dx @$curisolate()' did not return the expected isolate "
        "types\n");
  } else {
    printf("SUCCESS: Function alias @$curisolate\n");
  }

  // Do some actual testing
  output.log.clear();
  hr = pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "dx name.Value",
                              DEBUG_EXECUTE_ECHO);
  if (output.log.find("<SeqOneByteString>") == std::string::npos) {
    printf(
        "***ERROR***: 'dx name.value' did not return the expected local "
        "representation");
  } else {
    printf("SUCCESS: v8::Local<v8::Value> decoding\n");
  }

  printf("=== Run completed! ===\n");
  // Detach before exiting
  hr = pClient->DetachProcesses();
  winrt::check_hresult(hr);
}

int main(int argv, char** pargv) {
  // Initialize COM... Though it doesn't seem to matter if you don't!
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  winrt::check_hresult(hr);
  RunTests();

  CoUninitialize();
  return 0;
}
