#pragma once

#include "../utilities.h"
#include "v8-layout.h"

class Extension {
 public:
  bool Initialize();
  ~Extension();
  winrt::com_ptr<IDebugHostModule> GetV8Module(winrt::com_ptr<IDebugHostContext>& spCtx);
  winrt::com_ptr<IDebugHostType> Extension::GetV8ObjectType(winrt::com_ptr<IDebugHostContext>& spCtx);
  static Extension* currentExtension;

  V8::Layout::V8Layout v8Layout;
  winrt::com_ptr<IDebugHostMemory2> spDebugHostMemory;
  winrt::com_ptr<IDebugHostSymbols> spDebugHostSymbols;
  winrt::com_ptr<IDebugHostExtensibility> spDebugHostExtensibility;

  winrt::com_ptr<IDebugHostTypeSignature> spObjectTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spLocalTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spMaybeLocalTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spHandleTypeSignature;
  winrt::com_ptr<IDebugHostTypeSignature> spMaybeHandleTypeSignature;
  winrt::com_ptr<IModelObject> spObjectDataModel;
  winrt::com_ptr<IModelObject> spLocalDataModel;
  winrt::com_ptr<IModelObject> spCurrIsolateModel;
  winrt::com_ptr<IModelObject> spListChunksModel;

 private:
  winrt::com_ptr<IDebugHostModule> spV8Module;
  winrt::com_ptr<IDebugHostType> spV8ObjectType;
  winrt::com_ptr<IDebugHostContext> spV8ModuleCtx;
  ULONG v8ModuleProcId;
};
