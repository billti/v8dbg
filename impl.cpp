#include "dbgext.h"

HRESULT MyPropertyAccessor::GetValue(
    PCWSTR key,
    IModelObject* contextObject,
    IModelObject** value) noexcept
{
    winrt::com_ptr<IKeyEnumerator> spKeyEnumerator;
    HRESULT hr = contextObject->EnumerateKeyValues(spKeyEnumerator.put());

    BSTR bstrKey = nullptr;
    winrt::com_ptr<IModelObject> spKeyValue;
    winrt::com_ptr<IKeyStore> spKeyStore;
    ModelObjectKind objKind;

    keyloop:
    hr = spKeyEnumerator->GetNext(&bstrKey, spKeyValue.put(), spKeyStore.put());
    if(SUCCEEDED(hr))
    {
        hr = spKeyValue->GetKind(&objKind);
        // TODO: Get the value
        SysFreeString(bstrKey);
        spKeyValue = nullptr;
        spKeyStore = nullptr;
        goto keyloop;
    }
    return S_OK;
}
