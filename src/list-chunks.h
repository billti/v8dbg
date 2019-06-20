#pragma once

#include <crtdbg.h>
#include <string>
#include <vector>
#include "../utilities.h"
#include "extension.h"
#include "v8.h"

struct ListChunksAlias : winrt::implements<ListChunksAlias, IModelMethod> {
  HRESULT __stdcall Call(IModelObject* pContextObject, ULONG64 argCount,
                         _In_reads_(argCount) IModelObject** ppArguments,
                         IModelObject** ppResult,
                         IKeyStore** ppMetadata) noexcept override;
};

struct MemoryChunkIterator: winrt::implements<MemoryChunkIterator, IModelIterator> {
  HRESULT __stdcall Reset() noexcept override {
    _RPT0(_CRT_WARN, "Reset called on MemoryChunkIterator\n");
    position = 0;
    return S_OK;
  }

  // bp v8dbg!MemoryChunkIterator::GetNext
  HRESULT __stdcall GetNext(IModelObject** object, ULONG64 dimensions,
                            IModelObject** indexers,
                            IKeyStore** metadata) noexcept override {
    _RPT1(_CRT_WARN, "In GetNext. Dimensions = %d\n", dimensions);
    if (dimensions > 1) return E_INVALIDARG;
    if (position >= 8) return E_BOUNDS;

    HRESULT hr = S_OK;
    if (metadata != nullptr) *metadata = nullptr;

    winrt::com_ptr<IModelObject> spIndex, spValue;
    hr = CreateInt32(position + 42, spValue.put());
    if (FAILED(hr)) return hr;
    *object = spValue.detach();

    if (dimensions == 1) {
      hr = CreateULong64(position, spIndex.put());
      if (FAILED(hr)) return hr;
      *indexers = spIndex.detach();
    }
    position++;
    return hr;
  }

  ULONG position = 0;
};

struct MemoryChunks : winrt::implements<MemoryChunks, IIndexableConcept,
                                        IIterableConcept> {
  // IIndexableConcept members
  HRESULT __stdcall GetDimensionality(
      IModelObject* contextObject, ULONG64* dimensionality) noexcept override {
    *dimensionality = 1;
    return S_OK;
  }

  HRESULT __stdcall GetAt(IModelObject* contextObject, ULONG64 indexerCount,
                          IModelObject** indexers, IModelObject** object,
                          IKeyStore** metadata) noexcept override {
    _RPT0(_CRT_ERROR, "IndexableConcept::GetAt not implemented\n");
    return E_NOTIMPL;
  }

  HRESULT __stdcall SetAt(IModelObject* contextObject, ULONG64 indexerCount,
                          IModelObject** indexers,
                          IModelObject* value) noexcept override {
    _RPT0(_CRT_ERROR, "IndexableConcept::SetAt not implemented\n");
    return E_NOTIMPL;
  }

  // IIterableConcept
  HRESULT __stdcall GetDefaultIndexDimensionality(
      IModelObject* contextObject, ULONG64* dimensionality) noexcept override {
    *dimensionality = 1;
    return S_OK;
  }

  HRESULT __stdcall GetIterator(IModelObject* contextObject,
                                IModelIterator** iterator) noexcept override {
    _RPT0(_CRT_WARN, "In MemoryChunks::GetIterator\n");
    auto spMemoryIterator{winrt::make<MemoryChunkIterator>()};
    *iterator = spMemoryIterator.as<IModelIterator>().detach();
    return S_OK;
  }
};
