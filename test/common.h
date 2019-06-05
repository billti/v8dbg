#pragma once

#if !defined(UNICODE) || !defined(_UNICODE)
#error Unicode not defined
#endif

#include <DbgEng.h>
#include <DbgModel.h>
#include <Windows.h>
#include <crtdbg.h>
#include <winrt/base.h>

#include <string>

struct MyOutput : IDebugOutputCallbacks {
  // Inherited via IDebugOutputCallbacks
  virtual HRESULT __stdcall QueryInterface(REFIID InterfaceId,
                                           PVOID* Interface) override {
    return E_NOTIMPL;
  }
  virtual ULONG __stdcall AddRef(void) override { return 0; }
  virtual ULONG __stdcall Release(void) override { return 0; }
  virtual HRESULT __stdcall Output(ULONG Mask, PCSTR Text) override {
    if (Mask & DEBUG_OUTPUT_NORMAL) {
      log += Text;
    }
    return S_OK;
  }

  std::string log;
};

// For return values, see:
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-status-xxx
struct MyCallback : IDebugEventCallbacks {
  // Inherited via IDebugEventCallbacks
  virtual HRESULT __stdcall QueryInterface(REFIID InterfaceId,
                                           PVOID* Interface) override {
    return E_NOTIMPL;
  }
  virtual ULONG __stdcall AddRef(void) override { return S_OK; }
  virtual ULONG __stdcall Release(void) override { return S_OK; }
  virtual HRESULT __stdcall GetInterestMask(PULONG Mask) override {
    *Mask = DEBUG_EVENT_BREAKPOINT | DEBUG_EVENT_CREATE_PROCESS;
    return S_OK;
  }
  virtual HRESULT __stdcall Breakpoint(PDEBUG_BREAKPOINT Bp) override {
    ULONG64 bpOffset;
    winrt::check_hresult(Bp->GetOffset(&bpOffset));
    // printf("Breakpoint hit at %llx\n", bpOffset);

    // Break on breakpoints? Seems reasonable.
    return DEBUG_STATUS_BREAK;
  }
  virtual HRESULT __stdcall Exception(PEXCEPTION_RECORD64 Exception,
                                      ULONG FirstChance) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall CreateThread(ULONG64 Handle, ULONG64 DataOffset,
                                         ULONG64 StartOffset) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall ExitThread(ULONG ExitCode) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall ExitProcess(ULONG ExitCode) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall LoadModule(ULONG64 ImageFileHandle,
                                       ULONG64 BaseOffset, ULONG ModuleSize,
                                       PCSTR ModuleName, PCSTR ImageName,
                                       ULONG CheckSum,
                                       ULONG TimeDateStamp) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall UnloadModule(PCSTR ImageBaseName,
                                         ULONG64 BaseOffset) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall SystemError(ULONG Error, ULONG Level) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall SessionStatus(ULONG Status) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall ChangeDebuggeeState(ULONG Flags,
                                                ULONG64 Argument) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall ChangeEngineState(ULONG Flags,
                                              ULONG64 Argument) override {
    return E_NOTIMPL;
  }
  virtual HRESULT __stdcall ChangeSymbolState(ULONG Flags,
                                              ULONG64 Argument) override {
    return E_NOTIMPL;
  }
  virtual HRESULT CreateProcessW(ULONG64 ImageFileHandle, ULONG64 Handle,
                                 ULONG64 BaseOffset, ULONG ModuleSize,
                                 PCSTR ModuleName, PCSTR ImageName,
                                 ULONG CheckSum, ULONG TimeDateStamp,
                                 ULONG64 InitialThreadHandle,
                                 ULONG64 ThreadDataOffset,
                                 ULONG64 StartOffset) {
    printf("Image created: %s\n", ImageName);

    // Should fire once the target process is launched. Break to create
    // breakpoints, etc.
    return DEBUG_STATUS_BREAK;
  }
};
