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
    // TODO
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
    winrt::com_ptr<IDebugHostContext> spCtx;
    HRESULT hr = contextObject->GetContext(spCtx.put());
    if (FAILED(hr)) return hr;
    auto spMemoryIterator{winrt::make<MemoryChunkIterator>(spCtx)};
    *iterator = spMemoryIterator.as<IModelIterator>().detach();
    return S_OK;
  }
};
