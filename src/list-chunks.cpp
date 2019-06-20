#include "list-chunks.h"

HRESULT __stdcall ListChunksAlias::Call(IModelObject* pContextObject,
                                        ULONG64 argCount,
                                        _In_reads_(argCount)
                                            IModelObject** ppArguments,
                                        IModelObject** ppResult,
                                        IKeyStore** ppMetadata) noexcept {
  HRESULT hr = S_OK;
  // TODO
  // Get @$curisolate().heap_.space_ of type [Type: v8::internal::Space * [8]]

  // Iterate through each space*, and if not null get $->memory_chunk_list_ [Type: v8::base::List<v8::internal::MemoryChunk>]

  // Let $chunk = $.front_ [Type: v8::internal::MemoryChunk *]

  // loop: If $chunk = nullptr then break

  // Report $->area_start_ and $->area_end_ [Type: unsigned __int64]

  // Let $chunk = $->list_node_.next_ [Type: v8::internal::MemoryChunk *] and goto loop:

  // For each memory chunk list...
  // TODO: What representation? A custom/synthesized type with start/end, and pointer back to Space/MemoryChunk?

  hr = CreateInt32(42, ppResult);
  return hr;
}
