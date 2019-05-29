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
  winrt::com_ptr<IDebugHostSymbols> spHostSymbols;

  if (!spDebugHost.try_as(this->spDebugMemory)) return false;

  // Create an instance of the DataModel 'parent' for v8::internal::Object types
  auto objectDataModel{winrt::make<V8ObjectDataModel>()};
  HRESULT hr = spDataModelManager->CreateDataModelObject(
      objectDataModel.get(), spSignatureModel.put());
  if (FAILED(hr)) return false;
  hr = spSignatureModel->SetConcept(__uuidof(IStringDisplayableConcept),
                                    objectDataModel.get(), nullptr);
  if (FAILED(hr)) return false;
  auto iDynamic = objectDataModel.as<IDynamicKeyProviderConcept>();
  hr = spSignatureModel->SetConcept(__uuidof(IDynamicKeyProviderConcept),
                                    iDynamic.get(), nullptr);
  if (FAILED(hr)) return false;

  // Add the 'Contents' properties to add to the parent model.
  // auto contentsProperty{ winrt::make<V8ObjectContentsProperty>()};
  // winrt::com_ptr<IModelObject> spContentsPropertyModel;
  // hr = CreateProperty(spDataModelManager.get(), contentsProperty.get(),
  // spContentsPropertyModel.put()); hr = spSignatureModel->SetKey(L"Contents",
  // spContentsPropertyModel.get(), nullptr);

  // Parent the model for the type
  if (!spDebugHost.try_as(spHostSymbols)) return false;
  hr = spHostSymbols->CreateTypeSignature(L"v8::internal::Object", nullptr,
                                          spTypeSignature.put());
  if (FAILED(hr)) return false;
  hr = spDataModelManager->RegisterModelForTypeSignature(
      spTypeSignature.get(), spSignatureModel.get());

  return !FAILED(hr);
}

Extension::~Extension() {
  spDataModelManager->UnregisterModelForTypeSignature(spSignatureModel.get(),
                                                      spTypeSignature.get());
}
