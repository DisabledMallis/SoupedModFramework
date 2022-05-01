#include "nativeLaunch.h"

#include <Windows.h>

JSValueRef NativeLaunch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
	MessageBoxA(nullptr, "Launching", "Launcher", MB_OK);
	return 0;
}