#include "extension.h"
#include <crtdbg.h>
#include "object.h"

Extension* Extension::currentExtension = nullptr;

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

bool Extension::Initialize() {
  _RPTF0(_CRT_WARN, "Entered ExtensionInitialize\n");

  if (!spDebugHost.try_as(spDebugMemory)) return false;
  if (!spDebugHost.try_as(spHostSymbols)) return false;

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
  hr = spHostSymbols->CreateTypeSignature(L"v8::internal::Object", nullptr,
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
  hr = spHostSymbols->CreateTypeSignature(L"v8::Local<*>", nullptr,
                                          spLocalTypeSignature.put());
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

  return !FAILED(hr);
}

Extension::~Extension() {
  _RPTF0(_CRT_WARN, "Entered Extension::~Extension\n");
  spDataModelManager->UnregisterModelForTypeSignature(spObjectDataModel.get(),
                                                      spObjectTypeSignature.get());
  spDataModelManager->UnregisterModelForTypeSignature(
      spLocalDataModel.get(), spLocalTypeSignature.get());
}
