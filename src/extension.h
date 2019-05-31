#pragma once

#include "../dbgext.h"
#include "v8-layout.h"

class Extension {
 public:
  bool Initialize();
  ~Extension();
  static Extension* currentExtension;

  winrt::com_ptr<IDebugHostMemory2> spDebugMemory;
  winrt::com_ptr<IDebugHostSymbols> spHostSymbols;

  winrt::com_ptr<IDebugHostTypeSignature> spObjectTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spLocalTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spMaybeLocalTypeSignature;
  winrt::com_ptr<IModelObject> spObjectDataModel;
  winrt::com_ptr<IModelObject> spLocalDataModel;
  V8::Layout::V8Layout v8Layout;
};
