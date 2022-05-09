#pragma once

#include <ChakraCore.h>

#define jsfunction(funcName) JsValueRef CALLBACK funcName(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
#define jsargc (argumentCount-1)
#define jsargv arguments

namespace JsNative {
    namespace console {
        jsfunction(error);
        jsfunction(warn);
        jsfunction(print);
    }
    namespace patchers {
        jsfunction(registerPatcher);
    }
};