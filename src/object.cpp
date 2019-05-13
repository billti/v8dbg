#include "object.h"

/*
When queried for a v8::internal::Object, the steps are:
 - Is it a smi? If so, Just attach an intrinsic property of uint32 using
     IDataModelManager::CreateIntrinsicObject
 - If not, untag the pointer and attach a native type of HeapObject using
     IDataModelManager::CreateTypedObject (and let the HeapObject viewer do the rest).
*/
HRESULT V8ObjectContentsProperty::GetValue(
        PCWSTR pwszKey,
        IModelObject *pV8ObjectInstance,
        IModelObject **ppValue)
{
    Location loc;
    HRESULT hr = pV8ObjectInstance->GetLocation(&loc);
    VARIANT vtVal;
    vtVal.vt = VT_UI8;
    vtVal.ullVal = loc.Offset;
    hr = spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppValue);
    return hr;
}
