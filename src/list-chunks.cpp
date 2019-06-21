#include "list-chunks.h"
#include "curisolate.h"

// v8dbg!ListChunksAlias::Call
HRESULT __stdcall ListChunksAlias::Call(IModelObject* pContextObject,
                                        ULONG64 argCount,
                                        _In_reads_(argCount)
                                            IModelObject** ppArguments,
                                        IModelObject** ppResult,
                                        IKeyStore** ppMetadata) noexcept {
  HRESULT hr = S_OK;

  winrt::com_ptr<IDebugHostContext> spCtx;
  hr = spDebugHost->GetCurrentContext(spCtx.put());
  if (FAILED(hr)) return hr;

  hr = spDataModelManager->CreateSyntheticObject(spCtx.get(), ppResult);
  if (FAILED(hr)) return hr;

  auto spIterator{winrt::make<MemoryChunks>()};
  auto spIndexableConcept = spIterator.as<IIndexableConcept>();
  auto spIterableConcept = spIterator.as<IIterableConcept>();

  hr = (*ppResult)->SetConcept(__uuidof(IIndexableConcept), spIndexableConcept.get(), nullptr);
  if (FAILED(hr)) return hr;
  hr = (*ppResult)->SetConcept(__uuidof(IIterableConcept), spIterableConcept.get(), nullptr);
  if (FAILED(hr)) return hr;
  return hr;
}

HRESULT MemoryChunkIterator::PopulateChunkData() {
  winrt::com_ptr<IModelObject> spIsolate, spHeap, spSpace;
  chunks.clear();

  HRESULT hr = GetCurrentIsolate(spIsolate);
  if (FAILED(hr)) return hr;

  hr = spIsolate->GetRawValue(SymbolField, L"heap_", RawSearchNone, spHeap.put());
  hr = spHeap->GetRawValue(SymbolField, L"space_", RawSearchNone, spSpace.put());
  if (FAILED(hr)) return hr;

  winrt::com_ptr<IDebugHostType> spSpaceType;
  hr = spSpace->GetTypeInfo(spSpaceType.put());
  if (FAILED(hr)) return hr;

  // Iterate over the array of Space pointers
  winrt::com_ptr<IIterableConcept> spIterable;
  hr = spSpace->GetConcept(__uuidof(IIterableConcept),
                           reinterpret_cast<IUnknown**>(spIterable.put()),
                           nullptr);
  if (FAILED(hr)) return hr;

  winrt::com_ptr<IModelIterator> spSpaceIterator;
  hr = spIterable->GetIterator(spSpace.get(), spSpaceIterator.put());
  if (FAILED(hr)) return hr;

  // Loop through all the spaces in the array
  winrt::com_ptr<IModelObject> spSpacePtr;
  while (spSpaceIterator->GetNext(spSpacePtr.put(), 0, nullptr, nullptr) != E_BOUNDS) {
    // Should have gotten a "v8::internal::Space *". Dereference, then get field
    // "memory_chunk_list_" [Type: v8::base::List<v8::internal::MemoryChunk>]
    winrt::com_ptr<IModelObject> spSpace, spChunkList, spMemChunkPtr, spMemChunk;
    hr = spSpacePtr->Dereference(spSpace.put());
    if (FAILED(hr)) return hr;
    hr = spSpace->GetRawValue(SymbolField, L"memory_chunk_list_", RawSearchNone, spChunkList.put());
    if (FAILED(hr)) return hr;

    // Then get field "front_" [Type: v8::internal::MemoryChunk *]
    hr = spChunkList->GetRawValue(SymbolField, L"front_", RawSearchNone,
                                  spMemChunkPtr.put());
    if (FAILED(hr)) return hr;

    // Loop here on the list of MemoryChunks for the space
    while (true) {
      // See if it is a nullptr (i.e. no chunks in this space)
      VARIANT vtFrontVal;
      hr = spMemChunkPtr->GetIntrinsicValue(&vtFrontVal);
      if (FAILED(hr) || vtFrontVal.vt != VT_UI8) return E_FAIL;
      if (vtFrontVal.ullVal == 0) {
        break;
      }

      // Dereference and get fields "area_start_" and "area_end_" (both uint64)
      hr = spMemChunkPtr->Dereference(spMemChunk.put());
      if (FAILED(hr)) return hr;

      winrt::com_ptr<IModelObject> spStart, spEnd;
      hr = spMemChunk->GetRawValue(SymbolField, L"area_start_", RawSearchNone, spStart.put());
      if (FAILED(hr)) return hr;
      hr = spMemChunk->GetRawValue(SymbolField, L"area_end_", RawSearchNone, spEnd.put());
      if (FAILED(hr)) return hr;

      VARIANT vtStart, vtEnd;
      hr = spStart->GetIntrinsicValue(&vtStart);
      if (FAILED(hr) || vtStart.vt != VT_UI8) return E_FAIL;
      hr = spEnd->GetIntrinsicValue(&vtEnd);
      if (FAILED(hr) || vtEnd.vt != VT_UI8) return E_FAIL;

      ChunkData chunkEntry;
      chunkEntry.area_start = spStart;
      chunkEntry.area_end = spEnd;
      chunkEntry.space = spSpace;
      chunks.push_back(chunkEntry);

      // Follow the list_node_.next_ to the next memory chunk
      winrt::com_ptr<IModelObject> spListNode;
      hr = spMemChunk->GetRawValue(SymbolField, L"list_node_", RawSearchNone, spListNode.put());
      if (FAILED(hr)) return hr;

      spMemChunkPtr = nullptr;
      spMemChunk = nullptr;
      hr = spListNode->GetRawValue(SymbolField, L"next_", RawSearchNone, spMemChunkPtr.put());
      if (FAILED(hr)) return hr;
      // Top of the loop will check if this is a nullptr and exit if so
    }
    spSpacePtr = nullptr;
    spSpace = nullptr;
  }

  return S_OK;
}

HRESULT MemoryChunkIterator::GetNext(IModelObject** object, ULONG64 dimensions,
                                     IModelObject** indexers,
                                     IKeyStore** metadata) noexcept {
  _RPT1(_CRT_WARN, "In GetNext. Dimensions = %d\n", dimensions);
  HRESULT hr = S_OK;
  if (dimensions > 1) return E_INVALIDARG;

  if (position == 0) {
    hr = PopulateChunkData();
    if (FAILED(hr)) return hr;
  }
  if (position >= chunks.size()) return E_BOUNDS;

  if (metadata != nullptr) *metadata = nullptr;

  winrt::com_ptr<IModelObject> spIndex, spValue;

  if (dimensions == 1) {
    hr = CreateULong64(position, spIndex.put());
    if (FAILED(hr)) return hr;
    *indexers = spIndex.detach();
  }

  // Create the synthetic object representing the chunk here
  ChunkData& currChunk = chunks.at(position++);
  hr = spDataModelManager->CreateSyntheticObject(spCtx.get(), spValue.put());
  if (FAILED(hr)) return hr;
  hr = spValue->SetKey(L"area_start", currChunk.area_start.get(), nullptr);
  if (FAILED(hr)) return hr;
  hr = spValue->SetKey(L"area_end", currChunk.area_end.get(), nullptr);
  if (FAILED(hr)) return hr;
  hr = spValue->SetKey(L"space", currChunk.space.get(), nullptr);
  if (FAILED(hr)) return hr;

  *object = spValue.detach();
  return hr;
}
