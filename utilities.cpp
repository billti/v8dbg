#include "utilities.h"

HRESULT CreateProperty(IDataModelManager* pManager,
                       IModelPropertyAccessor* pProperty,
                       IModelObject** ppPropertyObject) {
  *ppPropertyObject = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_UNKNOWN;
  vtVal.punkVal = pProperty;

  HRESULT hr = pManager->CreateIntrinsicObject(ObjectPropertyAccessor, &vtVal,
                                               ppPropertyObject);
  return hr;
}

HRESULT CreateULong64(ULONG64 value, IModelObject** ppInt) {
  HRESULT hr = S_OK;
  *ppInt = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_UI8;
  vtVal.ullVal = value;

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppInt);
  return hr;
}

HRESULT CreateInt32(int value, IModelObject** ppInt) {
  HRESULT hr = S_OK;
  *ppInt = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_I4;
  vtVal.intVal = value;

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppInt);
  return hr;
}

HRESULT CreateUInt32(uint32_t value, IModelObject** ppInt) {
  HRESULT hr = S_OK;
  *ppInt = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_UI4;
  vtVal.uintVal = value;

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppInt);
  return hr;
}

HRESULT CreateBool(bool value, IModelObject** ppVal) {
  HRESULT hr = S_OK;
  *ppVal = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_BOOL;
  vtVal.boolVal = value;

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
  return hr;
}

HRESULT CreateNumber(double value, IModelObject** ppVal) {
  HRESULT hr = S_OK;
  *ppVal = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_R8;
  vtVal.dblVal = value;

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
  return hr;
}

HRESULT CreateString(std::u16string value, IModelObject** ppVal) {
  HRESULT hr = S_OK;
  *ppVal = nullptr;

  VARIANT vtVal;
  vtVal.vt = VT_BSTR;
  vtVal.bstrVal =
      ::SysAllocString(reinterpret_cast<const OLECHAR*>(value.c_str()));

  hr =
      spDataModelManager->CreateIntrinsicObject(ObjectIntrinsic, &vtVal, ppVal);
  return hr;
}

bool GetModelAtIndex(winrt::com_ptr<IModelObject>& spParent,
                     winrt::com_ptr<IModelObject>& spIndex,
                     IModelObject** pResult) {
  winrt::com_ptr<IIndexableConcept> spIndexableConcept;
  HRESULT hr = spParent->GetConcept(
      __uuidof(IIndexableConcept),
      reinterpret_cast<IUnknown**>(spIndexableConcept.put()), nullptr);
  if (FAILED(hr)) return false;

  std::vector<IModelObject*> pIndexers{spIndex.get()};
  hr = spIndexableConcept->GetAt(spParent.get(), 1, pIndexers.data(), pResult,
                                 nullptr);
  return SUCCEEDED(hr);
}

bool GetCurrentThread(winrt::com_ptr<IDebugHostContext>& spHostContext,
                      IModelObject** pCurrentThread) {
  HRESULT hr = S_OK;
  winrt::com_ptr<IModelObject> spBoxedContext, spRootNamespace;
  winrt::com_ptr<IModelObject> spDebugger, spSessions, spProcesses, spThreads;
  winrt::com_ptr<IModelObject> spCurrSession, spCurrProcess, spCurrThread;

  // Get the current context boxed as an IModelObject
  VARIANT vtContext;
  vtContext.vt = VT_UNKNOWN;
  vtContext.punkVal = spHostContext.get();
  hr = spDataModelManager->CreateIntrinsicObject(ObjectContext, &vtContext,
                                                 spBoxedContext.put());
  if (FAILED(hr)) return false;

  hr = spDataModelManager->GetRootNamespace(spRootNamespace.put());
  if (FAILED(hr)) return false;

  hr = spRootNamespace->GetKeyValue(L"Debugger", spDebugger.put(), nullptr);
  if (FAILED(hr)) return false;

  hr = spDebugger->GetKeyValue(L"Sessions", spSessions.put(), nullptr);
  if (!GetModelAtIndex(spSessions, spBoxedContext, spCurrSession.put())) {
    return false;
  }

  hr = spCurrSession->GetKeyValue(L"Processes", spProcesses.put(), nullptr);
  if (!GetModelAtIndex(spProcesses, spBoxedContext, spCurrProcess.put())) {
    return false;
  }

  hr = spCurrProcess->GetKeyValue(L"Threads", spThreads.put(), nullptr);
  if (!GetModelAtIndex(spThreads, spBoxedContext, spCurrThread.put())) {
    return false;
  }
  *pCurrentThread = spCurrThread.detach();
  return true;
}
