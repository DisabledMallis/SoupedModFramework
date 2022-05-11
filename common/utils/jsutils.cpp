#include "jsutils.h"
#include <logger.h>

static JSContextRef currentContext;
void JSUtils::SetContext(JSContextRef ctx) {
	currentContext = ctx;
}

JSContextRef JSUtils::GetContext() {
	return currentContext;
}

JSObjectRef JSUtils::GetGlobalObject()
{
	return JSContextGetGlobalObject(currentContext);
}

JSStringRef JSUtils::CreateString(std::string text) {
	return JSStringCreateWithUTF8CString(text.c_str());
}
std::string JSUtils::GetString(JSStringRef text) {
	size_t len = JSStringGetLength(text);
	char* buffer = (char*)_malloca(len + 1);
	if (!buffer) {
		return "";
	}
	JSStringGetUTF8CString(text, buffer, len);
	return std::string(buffer, len);
}
JSValueRef JSUtils::GetUndefined() {
	return JSValueMakeUndefined(currentContext);
}
JSValueRef JSUtils::CallFunction(JSObjectRef func, const JSValueRef* argv, int argc) {
	if (!currentContext) {
		Logger::Print<Logger::FAILURE>("A function was called from native, but there is no JS context");
		return 0;
	}
	return JSObjectCallAsFunction(currentContext, func, 0, argc, argv, 0);
}
JSValueRef JSUtils::ReadProperty(JSObjectRef obj, std::string prop) {
	JSStringRef jsPropStr = CreateString(prop);
	JSValueRef jsProp = JSObjectGetProperty(currentContext, obj, jsPropStr, 0);
	return jsProp;
}

JSObjectRef JSUtils::CreateObject(std::string objectName, JSObjectRef parentObj) {
	JSStringRef jsObjStr = CreateString(objectName);
	JSObjectRef obj = JSObjectMake(currentContext, 0, 0);
	JSObjectSetProperty(currentContext, parentObj, jsObjStr, obj, 0, 0);
	return obj;
}

JSObjectRef JSUtils::CreateFunction(std::string funcName, JSObjectCallAsFunctionCallback callback, JSObjectRef parentObj)
{
	JSStringRef jsFuncStr = JSStringCreateWithUTF8CString(funcName.c_str());
	JSObjectRef func = JSObjectMakeFunctionWithCallback(currentContext, jsFuncStr, callback);
	JSObjectSetProperty(currentContext, parentObj, jsFuncStr, func, 0, 0);
	return func;
}