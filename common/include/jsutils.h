#pragma once

#include <JavaScriptCore/JavaScript.h>

#define jsfunction(funcName) JSValueRef funcName(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
#define jsargc (argumentCount-1)
#define jsargv arguments

namespace JSUtils {
	void InitializeRuntime();
};