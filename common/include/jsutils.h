#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <string>

#define jsfunction(funcName) JSValueRef funcName(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
#define jsargc argumentCount
#define jsargv arguments
#define undefined JSUtils::GetUndefined()

namespace JSUtils {
	void SetContext(JSContextRef ctx);
	JSContextRef GetContext();
	JSObjectRef GetGlobalObject();
	JSStringRef CreateString(std::string text);
	std::string GetString(JSStringRef text);
	JSValueRef GetUndefined();
	JSValueRef CallFunction(JSObjectRef func, const JSValueRef* argv, int argc);
	JSValueRef ReadProperty(JSObjectRef obj, std::string prop);
	JSObjectRef CreateObject(std::string objectName, JSObjectRef parentObj = GetGlobalObject());
	JSObjectRef CreateFunction(std::string funcName, JSObjectCallAsFunctionCallback callback, JSObjectRef parentObj = GetGlobalObject());
};