#pragma once

#if !defined(UNICODE) || !defined(_UNICODE)
#error Unicode not defined
#endif

#include <crtdbg.h>
#include <Windows.h>
#include <DbgEng.h>
#include <DbgModel.h>
#include <winrt/base.h>
