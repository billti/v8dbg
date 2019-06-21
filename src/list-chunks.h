#pragma once

#include <crtdbg.h>
#include <optional>
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

struct ChunkData {
  winrt::com_ptr<IModelObject> area_start;
  winrt::com_ptr<IModelObject> area_end;
  winrt::com_ptr<IModelObject> space;
};


struct MemoryChunkIterator: winrt::implements<MemoryChunkIterator, IModelIterator> {
  MemoryChunkIterator(winrt::com_ptr<IDebugHostContext>& hostContext): spCtx(hostContext){};

  HRESULT PopulateChunkData();

  HRESULT __stdcall Reset() noexcept override {
    _RPT0(_CRT_WARN, "Reset called on MemoryChunkIterator\n");
    position = 0;
    return S_OK;
  }

  HRESULT __stdcall GetNext(IModelObject** object, ULONG64 dimensions,
                            IModelObject** indexers,
                            IKeyStore** metadata) noexcept override;

  ULONG position = 0;
  std::vector<ChunkData> chunks;
  winrt::com_ptr<IDebugHostContext> spCtx;
};

struct MemoryChunks
    : winrt::implements<MemoryChunks, IIndexableConcept, IIterableConcept> {
  // IIndexableConcept members
  HRESULT __stdcall GetDimensionality(
      IModelObject* contextObject, ULONG64* dimensionality) noexcept override {
    *dimensionality = 1;
    return S_OK;
  }

  HRESULT __stdcall GetAt(IModelObject* contextObject, ULONG64 indexerCount,
                          IModelObject** indexers, IModelObject** object,
                          IKeyStore** metadata) noexcept override {
    _RPT0(_CRT_WARN, "In IIndexableConcept::GetAt\n");
    if (indexerCount != 1) return E_INVALIDARG;
    if (metadata != nullptr) *metadata = nullptr;
    HRESULT hr = S_OK;
    winrt::com_ptr<IDebugHostContext> spCtx;
    hr = contextObject->GetContext(spCtx.put());
    if (FAILED(hr)) return hr;

    // This should be instantiated once for each synthetic object returned,
    // so should be able to cache/reuse an iterator
    if (!optChunks.has_value()) {
      _RPT0(_CRT_WARN, "Caching memory chunks for the indexer\n");
      optChunks.emplace(spCtx);
      _ASSERT(optChunks.has_value());
      optChunks->PopulateChunkData();
    }

    VARIANT vtIndex;
    hr = indexers[0]->GetIntrinsicValueAs(VT_UI8, &vtIndex);
    if (FAILED(hr)) return hr;

    if (vtIndex.ullVal >= optChunks->chunks.size()) return E_BOUNDS;

    ChunkData currChunk = optChunks->chunks.at(vtIndex.ullVal);
    winrt::com_ptr<IModelObject> spValue;
    hr = spDataModelManager->CreateSyntheticObject(spCtx.get(), spValue.put());
    if (FAILED(hr)) return hr;
    hr = spValue->SetKey(L"area_start", currChunk.area_start.get(), nullptr);
    if (FAILED(hr)) return hr;
    hr = spValue->SetKey(L"area_end", currChunk.area_end.get(), nullptr);
    if (FAILED(hr)) return hr;
    hr = spValue->SetKey(L"space", currChunk.space.get(), nullptr);
    if (FAILED(hr)) return hr;

    *object = spValue.detach();
    return S_OK;
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
    winrt::com_ptr<IDebugHostContext> spCtx;
    HRESULT hr = contextObject->GetContext(spCtx.put());
    if (FAILED(hr)) return hr;
    auto spMemoryIterator{winrt::make<MemoryChunkIterator>(spCtx)};
    *iterator = spMemoryIterator.as<IModelIterator>().detach();
    return S_OK;
  }

  std::optional<MemoryChunkIterator> optChunks;
};
