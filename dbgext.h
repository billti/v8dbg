#pragma once

#if !defined(UNICODE) || !defined(_UNICODE)
#error Unicode not defined
#endif

#include <crtdbg.h>
#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>

#include <winrt/base.h>
#include <string>

// Globals for use throughout the extension. (Populated on load).
extern winrt::com_ptr<IDataModelManager> spDataModelManager;
extern winrt::com_ptr<IDebugHost> spDebugHost;
extern winrt::com_ptr<IDebugControl5> spDebugControl;

// To be implemented by the custom extension code. (Called on load).
bool CreateExtension();
void DestroyExtension();
