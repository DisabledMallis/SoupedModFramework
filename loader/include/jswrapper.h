#pragma once

#include <string>
#include <functional>

typedef void* JsRef;
typedef JsRef JsValueRef;
typedef JsRef JsContextRef;
typedef unsigned __int64 JsSourceContext;
typedef JsValueRef(*JsNativeFunction)(JsValueRef, bool, JsValueRef*, unsigned short, void*);

namespace JSWrapper {
	void DoWork(std::function<void()> jsWork);
	void AwaitWork();
	void InitializeRuntime();
	JsValueRef GetGlobalObject();
	JsContextRef GetGlobalContext();
	JsValueRef CreateObject(const char* objectName, JsValueRef parent = JSWrapper::GetGlobalObject());
	JsValueRef CreateFunction(const char* funcName, JsNativeFunction function, JsValueRef parent = JSWrapper::GetGlobalObject());
	JsValueRef ToString(JsValueRef value);
	std::string ToCppString(JsValueRef value);
	JsValueRef FromCppString(std::string value);
	JsValueRef ReadProperty(JsValueRef obj, std::wstring prop);
	void HandleException();
	JsValueRef Run(std::wstring code);
	void InitializeAPI();
	void DestroyRuntime();
};