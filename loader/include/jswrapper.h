#pragma once

#include <string>

namespace JSWrapper {
	typedef void* JsValueRef;
	typedef JsValueRef(*JsNativeFunction)(JsValueRef, bool, JsValueRef*, unsigned short, void*);

	void InitializeRuntime();
	JsValueRef GetGlobalObject();
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