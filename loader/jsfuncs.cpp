#include "jsfuncs.h"
#include <logger.h>
#include <jswrapper.h>

using namespace JsNative;

jsfunction(console::error) {
    if (jsargc >= 1) {
        Logger::Print<Logger::FAILURE>("{}", JSWrapper::ToCppString(jsargv[1]));
    }
    return JS_INVALID_REFERENCE;
}

jsfunction(console::warn) {
    if (jsargc >= 1) {
        Logger::Print<Logger::WARNING>("{}", JSWrapper::ToCppString(jsargv[1]));
    }
    return JS_INVALID_REFERENCE;
}

jsfunction(console::print) {
    if (jsargc >= 1) {
        Logger::Print("{}", JSWrapper::ToCppString(jsargv[1]));
    }
    return JS_INVALID_REFERENCE;
}