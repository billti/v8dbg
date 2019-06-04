#include "common.h"

#include <cstdio>
#include <exception>

// See the docs at https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/using-the-debugger-engine-api

#define CheckHr(hr) (winrt::check_hresult(hr))

struct MyOutput : IDebugOutputCallbacks {
	// Inherited via IDebugOutputCallbacks
	virtual HRESULT __stdcall QueryInterface(REFIID InterfaceId, PVOID* Interface) override
	{
		return E_NOTIMPL;
	}
	virtual ULONG __stdcall AddRef(void) override
	{
		return 0;
	}
	virtual ULONG __stdcall Release(void) override
	{
		return 0;
	}
	virtual HRESULT __stdcall Output(ULONG Mask, PCSTR Text) override
	{
		printf(Text);
		return S_OK;
	}
};

// For return values, see: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-status-xxx
struct MyCallback : IDebugEventCallbacks {
	// Inherited via IDebugEventCallbacks
	virtual HRESULT __stdcall QueryInterface(REFIID InterfaceId, PVOID* Interface) override
	{
		return E_NOTIMPL;
	}
	virtual ULONG __stdcall AddRef(void) override
	{
		return S_OK;
	}
	virtual ULONG __stdcall Release(void) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetInterestMask(PULONG Mask) override
	{
		*Mask = DEBUG_EVENT_BREAKPOINT | DEBUG_EVENT_CREATE_PROCESS;
		return S_OK;
	}
	virtual HRESULT __stdcall Breakpoint(PDEBUG_BREAKPOINT Bp) override
	{
		ULONG64 bpOffset;
		CheckHr(Bp->GetOffset(&bpOffset));
		printf("Breakpoint hit at %llx\n", bpOffset);

		// Break on breakpoints? Seems reasonable.
		return DEBUG_STATUS_BREAK;
	}
	virtual HRESULT __stdcall Exception(PEXCEPTION_RECORD64 Exception, ULONG FirstChance) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CreateThread(ULONG64 Handle, ULONG64 DataOffset, ULONG64 StartOffset) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExitThread(ULONG ExitCode) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExitProcess(ULONG ExitCode) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall LoadModule(ULONG64 ImageFileHandle, ULONG64 BaseOffset, ULONG ModuleSize, PCSTR ModuleName, PCSTR ImageName, ULONG CheckSum, ULONG TimeDateStamp) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall UnloadModule(PCSTR ImageBaseName, ULONG64 BaseOffset) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SystemError(ULONG Error, ULONG Level) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SessionStatus(ULONG Status) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ChangeDebuggeeState(ULONG Flags, ULONG64 Argument) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ChangeEngineState(ULONG Flags, ULONG64 Argument) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ChangeSymbolState(ULONG Flags, ULONG64 Argument) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CreateProcessW(ULONG64 ImageFileHandle, ULONG64 Handle, ULONG64 BaseOffset, ULONG ModuleSize, PCSTR ModuleName, PCSTR ImageName, ULONG CheckSum, ULONG TimeDateStamp, ULONG64 InitialThreadHandle, ULONG64 ThreadDataOffset, ULONG64 StartOffset)
	{
		printf("Image created: %s\n", ImageName);

		// Should fire once the target process is launched. Break to create breakpoints, etc.
		return DEBUG_STATUS_BREAK;
	}
};

void DoWork() {
	// Get the Debug client
	winrt::com_ptr<IDebugClient5> pClient;
	HRESULT hr = DebugCreate(__uuidof(IDebugClient5), pClient.put_void());
	CheckHr(hr);

	// Set noisy symbol loading on
	winrt::com_ptr<IDebugSymbols> pSymbols;
	hr = pClient->QueryInterface(__uuidof(IDebugSymbols), pSymbols.put_void());
	CheckHr(hr);
	hr = pSymbols->AddSymbolOptions(0x80000000 /*SYMOPT_DEBUG*/);
	CheckHr(hr);
	// Symbol loading fails if the pdb is in the same folder as the exe, but it's not on the path.
	hr = pSymbols->SetSymbolPath("C:\\src\\github\\v8\\out\\x64.debug");
	CheckHr(hr);

	// Set the callbacks
	MyOutput output;
	MyCallback callback;
	hr = pClient->SetOutputCallbacks(&output);
	CheckHr(hr);
	hr = pClient->SetEventCallbacks(&callback);
	CheckHr(hr);

	// Launch the process with the debugger attached
	DEBUG_CREATE_PROCESS_OPTIONS procOptions;
	procOptions.CreateFlags = DEBUG_PROCESS;
	procOptions.EngCreateFlags = 0;
	procOptions.VerifierFlags = 0;
	procOptions.Reserved = 0;
	hr = pClient->CreateProcessW(0, const_cast<char*>("C:\\src\\github\\v8\\out\\x64.debug\\d8.exe -e \"console.log('hello, world');\""), DEBUG_PROCESS);
	CheckHr(hr);

	// Wait for the attach event
	winrt::com_ptr <IDebugControl3> pDebugControl;
	hr = pClient->QueryInterface(__uuidof(IDebugControl3), pDebugControl.put_void());
	CheckHr(hr);
	hr = pDebugControl->WaitForEvent(0, INFINITE);
	CheckHr(hr);

	// Set a breakpoint
	//PDEBUG_BREAKPOINT bp;
	winrt::com_ptr <IDebugBreakpoint> bp;
	hr = pDebugControl->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID, bp.put());
	CheckHr(hr);
	hr = bp->SetOffsetExpression("d8_exe!v8::Shell::ExecuteString");
	CheckHr(hr);
	hr = bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
	CheckHr(hr);

	hr = pDebugControl->SetExecutionStatus(DEBUG_STATUS_GO);
	CheckHr(hr);

	// Wait for the breakpoint. Fails here saying device is in invalid state, but everything else was S_OK??
	hr = pDebugControl->WaitForEvent(0, INFINITE);
	CheckHr(hr);

	ULONG type, procId, threadId, descUsed;
	byte desc[1024];
	hr = pDebugControl->GetLastEventInformation(&type, &procId, &threadId, nullptr, 0, nullptr, reinterpret_cast<PSTR>(desc), 1024, &descUsed);
	CheckHr(hr);

  ULONG64 extHandle;
  hr = pDebugControl->AddExtension("C:\\src\\github\\dbgext\\x64\\dbgext.dll", 0, &extHandle);
  //hr = pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "dx @$curisolate()", DEBUG_EXECUTE_ECHO);
  hr = pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "k", DEBUG_EXECUTE_ECHO);

	// Detach before exiting
	hr = pClient->DetachProcesses();
	CheckHr(hr);
}

int main(int argv, char** pargv)
{
	// Initialize COM... Though it doesn't seem to matter if you don't!
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CheckHr(hr);
	DoWork();

	CoUninitialize();
	return 0;
}
