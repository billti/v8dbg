#include "../utilities.h"
#include "extension.h"
#include "curisolate.h"
#include "object.h"

Extension* Extension::currentExtension = nullptr;
const wchar_t *pcurIsolate = L"curisolate";

bool CreateExtension() {
  _RPTF0(_CRT_WARN, "Entered CreateExtension\n");
  if (Extension::currentExtension != nullptr || spDataModelManager == nullptr ||
      spDebugHost == nullptr) {
    return false;
  } else {
    Extension* newExtension = new (std::nothrow) Extension();
    if (newExtension && newExtension->Initialize()) {
      Extension::currentExtension = newExtension;
      return true;
    } else {
      delete newExtension;
      return false;
    }
  }
}

void DestroyExtension() {
  _RPTF0(_CRT_WARN, "Entered DestroyExtension\n");
  if (Extension::currentExtension != nullptr) {
    delete Extension::currentExtension;
    Extension::currentExtension = nullptr;
  }
  return;
}

winrt::com_ptr<IDebugHostType> Extension::GetV8ObjectType(winrt::com_ptr<IDebugHostContext>& spCtx) {
  if (spV8ObjectType != nullptr) {
    bool isEqual;
    if (SUCCEEDED(spV8ModuleCtx->IsEqualTo(spCtx.get(), &isEqual)) && isEqual) {
      return spV8ObjectType;
    } else {
      spV8ObjectType = nullptr;
    }
  }

  GetV8Module(spCtx); // Will force the correct module to load
  if (spV8Module == nullptr) return spV8ObjectType; // Will also be null here

  HRESULT hr = spV8Module->FindTypeByName(L"v8::internal::Object", spV8ObjectType.put());
  return spV8ObjectType;
}

winrt::com_ptr<IDebugHostModule> Extension::GetV8Module(winrt::com_ptr<IDebugHostContext>& spCtx) {
  // Return the cached version if it exists and the context is the same
  if (spV8Module != nullptr) {
    bool isEqual;
    if (SUCCEEDED(spV8ModuleCtx->IsEqualTo(spCtx.get(), &isEqual)) && isEqual) {
      return spV8Module;
    } else {
      spV8Module = nullptr;
      spV8ModuleCtx = nullptr;
    }
  }

  // Loop through the modules looking for the one that holds the "isolate_key_"
  winrt::com_ptr<IDebugHostSymbolEnumerator> spEnum;
  if (SUCCEEDED(spDebugHostSymbols->EnumerateModules(spCtx.get(), spEnum.put()))) {
    HRESULT hr = S_OK;
    while (true) {
      winrt::com_ptr<IDebugHostSymbol> spModSym;
      hr = spEnum->GetNext(spModSym.put());
      // hr == E_BOUNDS : hit the end of the enumerator
      // hr == E_ABORT  : a user interrupt was requested
      if (FAILED(hr)) break;
      winrt::com_ptr<IDebugHostModule> spModule;
      if (spModSym.try_as(spModule)) /* should always succeed */
      {
        winrt::com_ptr<IDebugHostSymbol> spIsolateSym;
        // The below symbol is specific to the main V8 module
        hr = spModule->FindSymbolByName(L"isolate_key_", spIsolateSym.put());
        if (SUCCEEDED(hr)) {
          spV8Module = spModule;
          spV8ModuleCtx = spCtx;
          break;
        }
      }
    }
  }
  // This will be the located module, or still nullptr if above fails
  return spV8Module;
}

bool Extension::Initialize() {
  _RPTF0(_CRT_WARN, "Entered ExtensionInitialize\n");

  if (!spDebugHost.try_as(spDebugHostMemory)) return false;
  if (!spDebugHost.try_as(spDebugHostSymbols)) return false;
  if (!spDebugHost.try_as(spDebugHostExtensibility)) return false;

  // Create an instance of the DataModel 'parent' for v8::internal::Object types
  auto objectDataModel{winrt::make<V8ObjectDataModel>()};
  HRESULT hr = spDataModelManager->CreateDataModelObject(
      objectDataModel.get(), spObjectDataModel.put());
  if (FAILED(hr)) return false;
  hr = spObjectDataModel->SetConcept(__uuidof(IStringDisplayableConcept),
                                     objectDataModel.get(), nullptr);
  if (FAILED(hr)) return false;
  auto iDynamic = objectDataModel.as<IDynamicKeyProviderConcept>();
  hr = spObjectDataModel->SetConcept(__uuidof(IDynamicKeyProviderConcept),
                                     iDynamic.get(), nullptr);
  if (FAILED(hr)) return false;

  // Parent the model for the type
  hr = spDebugHostSymbols->CreateTypeSignature(L"v8::internal::Object", nullptr,
                                          spObjectTypeSignature.put());
  if (FAILED(hr)) return false;
  hr = spDataModelManager->RegisterModelForTypeSignature(
      spObjectTypeSignature.get(), spObjectDataModel.get());

  // Create an instance of the DataModel 'parent' class for v8::Local<*> types
  auto localDataModel{winrt::make<V8LocalDataModel>()};
  // Create an IModelObject out of it
  hr = spDataModelManager->CreateDataModelObject(localDataModel.get(),
                                                 spLocalDataModel.put());
  if (FAILED(hr)) return false;

  // Create a type signature for the v8::Local symbol
  hr = spDebugHostSymbols->CreateTypeSignature(L"v8::Local<*>", nullptr,
                                          spLocalTypeSignature.put());
  if (FAILED(hr)) return false;
  hr = spDebugHostSymbols->CreateTypeSignature(L"v8::MaybeLocal<*>", nullptr,
                                          spMaybeLocalTypeSignature.put());
  if (FAILED(hr)) return false;

  // Add the 'Value' property to the parent model.
  auto localValueProperty{winrt::make<V8LocalValueProperty>()};
  winrt::com_ptr<IModelObject> spLocalValuePropertyModel;
  hr = CreateProperty(spDataModelManager.get(), localValueProperty.get(),
                      spLocalValuePropertyModel.put());
  hr = spLocalDataModel->SetKey(L"Value", spLocalValuePropertyModel.get(),
                                nullptr);
  // Register the DataModel as the viewer for the type signature
  hr = spDataModelManager->RegisterModelForTypeSignature(
      spLocalTypeSignature.get(), spLocalDataModel.get());
  hr = spDataModelManager->RegisterModelForTypeSignature(
      spMaybeLocalTypeSignature.get(), spLocalDataModel.get());

  // Register the @$currisolate function alias.
  auto currIsolateFunction{winrt::make<CurrIsolateAlias>()};

  VARIANT vtCurrIsolateFunction;
  vtCurrIsolateFunction.vt = VT_UNKNOWN;
  vtCurrIsolateFunction.punkVal =
      static_cast<IModelMethod*>(currIsolateFunction.get());

  hr = spDataModelManager->CreateIntrinsicObject(
      ObjectMethod, &vtCurrIsolateFunction, spCurrIsolateModel.put());
  hr = spDebugHostExtensibility->CreateFunctionAlias(pcurIsolate,
                                                     spCurrIsolateModel.get());

  return !FAILED(hr);
}

Extension::~Extension() {
  _RPTF0(_CRT_WARN, "Entered Extension::~Extension\n");
  spDebugHostExtensibility->DestroyFunctionAlias(pcurIsolate);

  spDataModelManager->UnregisterModelForTypeSignature(
      spObjectDataModel.get(), spObjectTypeSignature.get());
  spDataModelManager->UnregisterModelForTypeSignature(
      spLocalDataModel.get(), spLocalTypeSignature.get());
  spDataModelManager->UnregisterModelForTypeSignature(
      spLocalDataModel.get(), spMaybeLocalTypeSignature.get());
}
