#include "HakoniwaProfilerImpl.h"
#include "Debugger.h"
#include "ComUtil.h"
#include "FunctionInfo.h"
#include "Tranpoline.h"

#include <string>
#include <wchar.h>
#include <corhlpr.h>
#include <memory>

#pragma comment (lib, "corguids.lib")

HakoniwaProfilerImpl::HakoniwaProfilerImpl() {
	mRefCount = 1;
}

HakoniwaProfilerImpl::~HakoniwaProfilerImpl() {
}

HRESULT HakoniwaProfilerImpl::SetProfilerEventMask() {
	// set the event mask 
	DWORD eventMask = 0;
	eventMask |= COR_PRF_MONITOR_NONE;
	eventMask |= COR_PRF_MONITOR_JIT_COMPILATION;
	eventMask |= COR_PRF_DISABLE_INLINING;
	eventMask |= COR_PRF_DISABLE_OPTIMIZATIONS;
	eventMask |= COR_PRF_USE_PROFILE_IMAGES;

	return mCorProfilerInfo2->SetEventMask(eventMask);
}

STDMETHODIMP HakoniwaProfilerImpl::QueryInterface(REFIID riid, void **ppObj) {
	*ppObj = nullptr;
	
	if (riid == IID_IUnknown) {
		*ppObj = static_cast<IUnknown *>(static_cast<HakoniwaProfiler *>(this));
		AddRef();
		return S_OK;
	}

	if (riid == IID_HakoniwaProfiler) {
		*ppObj = static_cast<HakoniwaProfiler *>(this);
		AddRef();
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback) {
		*ppObj = static_cast<ICorProfilerCallback*>(this);
		AddRef();
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback2) {
		*ppObj = static_cast<ICorProfilerCallback2*>(this);
		AddRef();
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback3) {
		*ppObj = dynamic_cast<ICorProfilerCallback3*>(this);
		AddRef();
		return S_OK;
	}

	if (riid == IID_ICorProfilerInfo || riid == IID_ICorProfilerInfo2) {
		mCorProfilerInfo2.CopyTo(riid, ppObj);
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE HakoniwaProfilerImpl::AddRef() {
	return InterlockedIncrement(&mRefCount);
}

ULONG STDMETHODCALLTYPE HakoniwaProfilerImpl::Release() {
	long nRefCount = InterlockedDecrement(&mRefCount);
	if (nRefCount == 0) {
		delete this;
	}
	return nRefCount;
}

STDMETHODIMP HakoniwaProfilerImpl::Initialize(IUnknown *pICorProfilerInfoUnk) {
	Debugger::printf(L"HakoniwaProfilerImpl::Initialize()\n");

	ICorProfilerInfo2* iInfo2;
	HRESULT hr = pICorProfilerInfoUnk->QueryInterface(__uuidof(iInfo2), (LPVOID*)&iInfo2);
	if (FAILED(hr)) {
		Debugger::printf(L"Error: Failed to get ICorProfiler2\n");
		exit(-1);
	} else {
		Debugger::printf(L"Got ICorProfilerInfo2\n");
	}
	mCorProfilerInfo2.Attach(iInfo2);

	hr = SetProfilerEventMask();
	if (FAILED(hr)) {
		Debugger::printf(L"Error: Failed to SetProfilerEventMask\n");
	} else {
		Debugger::printf(L"SetEventMask()\n");
	}

	Debugger::printf(L"Successfully initialized profiling\n");

	return S_OK;
}

const int MAX_LENGTH = 1024;

#include <memory>

STDMETHODIMP HakoniwaProfilerImpl::JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock) {
	std::shared_ptr<FunctionInfo> fi(FunctionInfo::CreateFunctionInfo(mCorProfilerInfo2.Get(), functionID));

	if (fi->get_ClassName() == L"System.DateTime" && fi->get_FunctionName() == L"get_Now") {
		Debugger::printf(L"%s", fi->get_SignatureText().c_str());
		Tranpoline tranpoline(mCorProfilerInfo2, fi);
		tranpoline.Update(L"HakoniwaProfiler.MethodHook.RegexReplacement", L"get_Now");
	}

	if (fi->get_ClassName() == L"ConsoleAppTest.Program" && fi->get_FunctionName() == L"getStr1") {
		Debugger::printf(L"%s", fi->get_SignatureText().c_str());
		Tranpoline tranpoline(mCorProfilerInfo2, fi);
		tranpoline.Update(L"HakoniwaProfiler.MethodHook.RegexReplacement", L"getStr1");
	}

	if (fi->get_ClassName() == L"ConsoleAppTest.Program" && fi->get_FunctionName() == L"haveArguments") {
		Debugger::printf(L"%s", fi->get_SignatureText().c_str());
		Tranpoline tranpoline(mCorProfilerInfo2, fi);
		tranpoline.Update(L"HakoniwaProfiler.MethodHook.RegexReplacement", L"haveArguments");
	}

	if(fi->get_ClassName() == L"ConsoleAppTest.Program" && fi->get_FunctionName() == L"haveManyArguments") {
		Debugger::printf(L"%s", fi->get_SignatureText().c_str());
		Tranpoline tranpoline(mCorProfilerInfo2, fi);
		tranpoline.Update(L"HakoniwaProfiler.MethodHook.RegexReplacement", L"haveManyArguments");
	}

	return S_OK;
}

