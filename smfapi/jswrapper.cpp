#include <jswrapper.h>
#include <ChakraCore.h>

static JsRuntimeHandle globalRuntime;
static JsContextRef globalContext;

void JSWrapper::InitializeRuntime() {
    JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &globalRuntime);
    JsCreateContext(globalRuntime, &globalContext);
    JsSetCurrentContext(globalContext);
}

void JSWrapper::DestroyRuntime() {
    JsSetCurrentContext(JS_INVALID_REFERENCE);
    JsDisposeRuntime(globalRuntime);
}