#pragma once

#include "../dbgext.h"

struct V8ObjectDataModel: winrt::implements<V8ObjectDataModel, IDataModelConcept, IStringDisplayableConcept>
{
    HRESULT __stdcall InitializeObject(
        IModelObject* modelObject,
        IDebugHostTypeSignature* matchingTypeSignature,
        IDebugHostSymbolEnumerator* wildcardMatches
    ) noexcept override
    {
        return S_OK;
    }

    HRESULT __stdcall GetName( BSTR* modelName) noexcept override
    {
        return E_NOTIMPL;
    }
    HRESULT __stdcall ToDisplayString(
        IModelObject* contextObject,
        IKeyStore* metadata,
        BSTR* displayString
    ) noexcept override
    {
        *displayString = ::SysAllocString(L"V8 Object");
        return S_OK;
    }
};

struct V8ObjectContentsProperty: winrt::implements<V8ObjectContentsProperty, IModelPropertyAccessor>
{
    HRESULT __stdcall GetValue(
        PCWSTR pwszKey,
        IModelObject *pV8ObjectInstance,
        IModelObject **ppValue);

    HRESULT __stdcall SetValue(
        PCWSTR /*pwszKey*/,
        IModelObject * /*pProcessInstance*/,
        IModelObject * /*pValue*/)
    {
        return E_NOTIMPL;
    }
};

HRESULT CreateHeapSyntheticObject(IDebugHostContext* pContext, ULONG64 heapAddress, IModelObject** ppResult);
