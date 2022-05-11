#include "jsutils.h"
#include <logger.h>

static JSContextRef currentContext = 0;
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
JSValueRef JSUtils::CallFunction(JSObjectRef func, JSObjectRef thisPtr, const JSValueRef* argv, int argc) {
	if (!currentContext) {
		Logger::Print<Logger::FAILURE>("A function was called from native, but there is no JS context");
		return JSUtils::GetUndefined();
	}
	if (!func) {
		Logger::Print<Logger::FAILURE>("Tried to call a null function?");
		return JSUtils::GetUndefined();
	}
	if (JSValueIsUndefined(JSUtils::GetContext(), func)) {
		Logger::Print<Logger::FAILURE>("Tried to call an undefined function?");
		return JSUtils::GetUndefined();
	}
	if (!JSObjectIsFunction(currentContext, func)) {
		Logger::Print<Logger::FAILURE>("Attempted to call a function from native, but the object provided isn't a function!");
		return JSUtils::GetUndefined();
	}
	JSValueRef exception = 0;
	JSValueRef result = JSObjectCallAsFunction(currentContext, func, thisPtr, argc, argv, &exception);
	if (exception) {
		//Handle exception
		JSStringRef jsExMsg = 0;
		if (JSValueIsString(JSUtils::GetContext(), exception)) {
			jsExMsg = (JSStringRef)exception;
		}
		else if (JSValueIsObject(JSUtils::GetContext(), exception)) {
			JSObjectRef exObj = (JSObjectRef)exception;
			JSValueRef msgProp = JSUtils::ReadProperty(exObj, "message");
			if (JSValueIsString(JSUtils::GetContext(), msgProp)) {
				jsExMsg = (JSStringRef)msgProp;
			}
			else {
				Logger::Print<Logger::WARNING>("A JS exception was thrown but the message couldn't be retrieved");
			}
		}
		else {
			Logger::Print<Logger::WARNING>("A JS exception was thrown but exception type is unknown");
		}
		std::string ex = GetString(jsExMsg);
		Logger::Print<Logger::FAILURE>("Function call failed with exception: {}", ex);
		return GetUndefined();
	}
	return result;
}
JSValueRef JSUtils::ReadProperty(JSObjectRef obj, std::string prop) {
	JSStringRef jsPropStr = CreateString(prop);
	if (JSObjectHasProperty(currentContext, obj, jsPropStr)) {
		JSValueRef jsProp = JSObjectGetProperty(currentContext, obj, jsPropStr, 0);
		return jsProp;
	}
	Logger::Print<Logger::FAILURE>("Object does not have property '{}'", prop);
	return JSUtils::GetUndefined();
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