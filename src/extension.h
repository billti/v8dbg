#pragma once

#include "../dbgext.h"

class Extension {
public:
    bool Initialize();
    ~Extension();
    static Extension* currentExtension;

    winrt::com_ptr<IDebugHostMemory2> spDebugMemory;
};
