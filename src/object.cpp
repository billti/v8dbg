#include "object.h"
#include "extension.h"

/*
When queried for a v8::internal::Object, the steps are:
 - Is it a smi? If so, Just attach an intrinsic property of uint32 using
     IDataModelManager::CreateIntrinsicObject
 - If not, untag the pointer and attach a synthetic type with the properties of interest
*/
HRESULT V8ObjectContentsProperty::GetValue(
        PCWSTR pwszKey,
        IModelObject *pV8ObjectInstance,
        IModelObject **ppValue)
{
    auto ext = Extension::currentExtension;
    if (ext == nullptr) return E_FAIL;

    Location loc;
    HRESULT hr = pV8ObjectInstance->GetLocation(&loc);
    if(FAILED(hr)) return hr;

    // Read the pointer at the Object location
    winrt::com_ptr<IDebugHostContext> spContext;
    ULONG64 heapAddress;
    hr = pV8ObjectInstance->GetContext(spContext.put());
    if(FAILED(hr)) return hr;
    hr = ext->spDebugMemory->ReadPointers(spContext.get(), loc, 1, &heapAddress);
    if(FAILED(hr)) return hr;

    if(heapAddress & 0x01) {
        // Tagged pointer. Clear the bottom 3 bits
        heapAddress &= ~0x03ui64;
        hr = CreateHeapSyntheticObject(spContext.get(), heapAddress, ppValue);
        return hr;
    } else {
        // Smi
        VARIANT vtVal;
        vtVal.vt = VT_UI8;
        vtVal.ullVal = heapAddress;
        // TODO: The bit shifting for x64 smi
        hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppValue);
        return hr;
    }
}

HRESULT CreateHeapSyntheticObject(IDebugHostContext* pContext, ULONG64 heapAddress, IModelObject** ppResult) {
    HRESULT hr = spDataModelManager->CreateSyntheticObject(pContext, ppResult);
    winrt::com_ptr<IModelObject> spHeapAddress;
    winrt::com_ptr<IModelObject> spMapAddress;

    ULONG64 mapAddress;
    hr = Extension::currentExtension->spDebugMemory->ReadPointers(pContext, Location(heapAddress), 1, &mapAddress);
    // TODO: Assert mapAddress & 0x01
    // Untag the pointer
    mapAddress &= ~0x03ui64;

    hr = CreateULong64(heapAddress, spHeapAddress.put());
    hr = CreateULong64(mapAddress, spMapAddress.put());
    hr = (*ppResult)->SetKey(L"HeapAddress", spHeapAddress.get(), nullptr);
    hr = (*ppResult)->SetKey(L"MapAddress", spMapAddress.get(), nullptr);
    return S_OK;
}
