#include <jswrapper.h>
#include <logger.h>
#include <filesystem>
#include <fstream>
#include <ChakraCore.h>
#include <thread>
#include <deque>
#include <functional>

std::thread jsWorkThread;
std::deque<std::function<void()>> jsWork;

static JsRuntimeHandle globalRuntime;
static JsContextRef globalContext;
static JsValueRef globalObj;

#include "jsFuncs.h"

void JSWrapper::DoWork(std::function<void()> work) {
    jsWork.push_back(work);
}

void JSWrapper::AwaitWork()
{
    while (!jsWork.empty()) {
        Sleep(100);
        continue;
    }
}

void JSWrapper::InitializeRuntime() {
    jsWorkThread = std::thread([]() {
        while (true) {
            if (jsWork.empty()) {
                Sleep(100);
                continue;
            }
            std::function<void()> toDo = jsWork.back();
            jsWork.pop_back();
            toDo();
        }
    });
    std::function<void()> initialization = []() {
        JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &globalRuntime);
        JsCreateContext(globalRuntime, &globalContext);
        JsSetCurrentContext(globalContext);
        JsGetGlobalObject(&globalObj);
    };
    jsWork.push_back(initialization);
    AwaitWork();
}

JsValueRef JSWrapper::GetGlobalObject() {
    return globalObj;
}

JsContextRef JSWrapper::GetGlobalContext() {
    return globalContext;
}

JsValueRef JSWrapper::CreateObject(const char* objectName, JsValueRef parent) {
    if (std::this_thread::get_id() != jsWorkThread.get_id()) {
        Logger::Print<Logger::WARNING>("{} was called from another thread!", __FUNCTION__);
    }
    JsValueRef object;
    JsPropertyIdRef objectPropId;
    JsCreateObject(&object);
    JsCreatePropertyId(objectName, strlen(objectName), &objectPropId);
    JsSetProperty(parent, objectPropId, object, true);
    return object;
}

JsValueRef JSWrapper::CreateFunction(const char* funcName, JsNativeFunction function, JsValueRef parent) {
    if (std::this_thread::get_id() != jsWorkThread.get_id()) {
        Logger::Print<Logger::WARNING>("{} was called from another thread!", __FUNCTION__);
    }
    JsValueRef func;
    JsPropertyIdRef funcPropId;
    JsCreateFunction(function, nullptr, &func);
    JsCreatePropertyId(funcName, strlen(funcName), &funcPropId);
    JsSetProperty(parent, funcPropId, func, true);
    return func;
}

JsValueRef JSWrapper::ToString(JsValueRef value) {
    JsValueRef stringValue;
    JsConvertValueToString(value, &stringValue);
    return stringValue;
}

std::string JSWrapper::ToCppString(JsValueRef value) {
    JsValueRef stringValue = ToString(value);
    size_t len = 0;
    JsCopyString(stringValue, nullptr, 0, &len);
    char* pString = (char*)_malloca(len + 1);
    if (!pString) {
        Logger::Print<Logger::FAILURE>("Failed to allocate buffer for js to string conversion");
        return "";
    }
    JsCopyString(stringValue, pString, len + 1, nullptr);
    return std::string(pString, len);
}

JsValueRef JSWrapper::FromCppString(std::string value)
{
    if (std::this_thread::get_id() != jsWorkThread.get_id()) {
        Logger::Print<Logger::WARNING>("{} was called from another thread!", __FUNCTION__);
    }
    JsValueRef jsString;
    JsCreateString(value.c_str(), value.size(), &jsString);
    return jsString;
}

JsValueRef JSWrapper::ReadProperty(JsValueRef obj, std::wstring prop)
{
    JsPropertyIdRef propId;
    JsGetPropertyIdFromName(prop.c_str(), &propId);
    JsValueRef jsValue;
    JsGetProperty(obj, propId, &jsValue);
    return jsValue;
}

void JSWrapper::HandleException() {
    JsValueRef exception;
    JsGetAndClearException(&exception);
    if (exception == JS_INVALID_REFERENCE) {
        Logger::Print<Logger::WARNING>("An exception is being handled, but its undefined?");
        return;
    }
    JsValueRef exMsg = ReadProperty(exception, L"message");
    std::string cppExMsg = ToCppString(exMsg);
    Logger::Print<Logger::FAILURE>("Javascript Error: {}, len: {}", cppExMsg, cppExMsg.size());
}

JsValueRef JSWrapper::Run(std::wstring code)
{
    if (std::this_thread::get_id() != jsWorkThread.get_id()) {
        Logger::Print<Logger::WARNING>("{} was called from another thread!", __FUNCTION__);
    }
    JsValueRef result;
    JsSetCurrentContext(globalContext);
    JsErrorCode err = JsRunScript(code.c_str(), JS_SOURCE_CONTEXT_NONE, L"", &result);
    if (err != JsNoError) {
        HandleException();
    }
    return result;
}

void JSWrapper::InitializeAPI() {
    using namespace JsNative;
    std::function<void()> initApi = []() {
        JsValueRef console = JSWrapper::CreateObject("console");
        JSWrapper::CreateFunction("error", console::error, console);
        JSWrapper::CreateFunction("warn", console::warn, console);
        JSWrapper::CreateFunction("print", console::print, console);

        JsValueRef patchers = JSWrapper::CreateObject("patchers");
        JSWrapper::CreateFunction("registerPatcher", patchers::registerPatchers, patchers);

        //Run the souped.js file
        std::filesystem::path soupedJsPath = "./js/souped.js";
        std::wifstream soupedJsStream(soupedJsPath);
        std::wstring soupedJsSource(std::istreambuf_iterator<wchar_t>(soupedJsStream), {});

        JsValueRef result = JSWrapper::Run(soupedJsSource);
    };
    jsWork.push_back(initApi);
    AwaitWork();
}

void JSWrapper::DestroyRuntime() {
    if (std::this_thread::get_id() != jsWorkThread.get_id()) {
        Logger::Print<Logger::WARNING>("{} was called from another thread!", __FUNCTION__);
    }
    jsWork.clear();
    JsSetCurrentContext(JS_INVALID_REFERENCE);
    JsDisposeRuntime(globalRuntime);
}