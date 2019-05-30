#pragma once

#include "../dbgext.h"
#include "v8-layout.h"

class Extension {
 public:
  bool Initialize();
  ~Extension();
  static Extension* currentExtension;

  winrt::com_ptr<IDebugHostMemory2> spDebugMemory;
  winrt::com_ptr<IDebugHostTypeSignature> spTypeSignature;
  winrt::com_ptr<IModelObject> spSignatureModel;
  V8::Layout::V8Layout v8Layout;
};
