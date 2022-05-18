#include <jsutils.h>
#include <deque>
#include <stdexcept>
#include <shared_thread.h>
#include <fstream>
#include <logger.h>
using namespace JSUtils;

static shared_thread jsThread(10);
static JsRuntimeHandle runtime;
static JsContextRef defaultContext;
static JsContextRef currentContext;
static JsValue global;
static std::deque<std::function<void()>> initTasks;
static unsigned int nextSourceContext;

void JSUtils::SetupRuntime() {
	JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);
	defaultContext = MakeContext();
	SetCurrentContext(defaultContext);
	for (auto task : initTasks) {
		task();
	}
}
void JSUtils::OnInitialize(std::function<void()> task) {
	initTasks.push_back(task);
}
void JSUtils::DestroyRuntime() {
	jsThread.DoWork([]() {
		JsDisposeRuntime(runtime);
	});
}

JsContextRef JSUtils::MakeContext() {
	JsContextRef newContext;
	JsCreateContext(runtime, &newContext);
	return newContext;
}
JsContextRef JSUtils::GetDefaultContext() {
	return defaultContext;
}
void JSUtils::SetCurrentContext(JsContextRef ctx) {
	currentContext = ctx;
	jsThread.DoWork([&]() {
		JsSetCurrentContext(currentContext);
	});
}
JsContextRef JSUtils::GetCurrentContext() {
	return currentContext;
}

JsValue& JSUtils::GetGlobalObject() {
	if (!global.IsValid()) {
		JsGetGlobalObject(global.GetInternalRef());
	}
	return global;
}

JsValue::JsValue() {
	this->internalRef = JS_INVALID_REFERENCE;
}
JsValue::JsValue(int value) {
	JsIntToNumber(value, &this->internalRef);
}
JsValue::JsValue(JsValueRef valRef) {
	this->internalRef = valRef;
}
JsValue::JsValue(std::string text) {
	JsCreateString(text.c_str(), text.length(), &this->internalRef);
}
JsValue::JsValue(std::string objName, bool isObject) {
	if (isObject) {
		this->internalRef = JS_INVALID_REFERENCE;
		JsPropertyIdRef propId;
		JsCreateObject(&this->internalRef);
	}
	else {
		JsValue(objName);
	}
};
JsValue::JsValue(JsNativeFunction func) {
	this->internalRef = JS_INVALID_REFERENCE;
	JsCreateFunction(func, nullptr, &this->internalRef);
}
JsValueRef* JsValue::GetInternalRef() {
	return &this->internalRef;
}
bool JsValue::IsValid() {
	if (!this) {
		return false;
	}
	if (!this->internalRef) {
		return false;
	}
	JsValueType valType = JsUndefined;
	JsErrorCode jsErr = JsGetValueType(this->internalRef, &valType);
	if (jsErr != JsNoError) {
		return false;
	}
	if (valType == JsUndefined) {
		return false;
	}
	if (valType == JsNull) {
		return false;
	}
	return true;
}
bool JsValue::HasProperty(std::string propName) {
	if (!this->IsValid()) {
		return false;
	}
	JsValue jsPropName = propName;
	bool result;
	JsObjectHasProperty(this->internalRef, *jsPropName.GetInternalRef(), &result);
	return result;
}
JsValue::operator bool() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->internalRef, &valType);
		if (valType == JsBoolean) {
			bool result;
			JsBooleanToBool(this->internalRef, &result);
			return result;
		}
		throw std::exception("Value isn't a boolean");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator int() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->internalRef, &valType);
		if (valType == JsNumber) {
			int result;
			JsNumberToInt(this->internalRef, &result);
			return result;
		}
		throw std::exception("Value isn't a number");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator double() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->internalRef, &valType);
		if (valType == JsNumber) {
			double result;
			JsNumberToDouble(this->internalRef, &result);
			return result;
		}
		throw std::exception("Value isn't a number");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator JsValueRef() {
	return this->internalRef;
}
void JsValue::operator=(bool value) {
	JsBoolToBoolean(value, &this->internalRef);
}
void JsValue::operator=(int value) {
	JsIntToNumber(value, &this->internalRef);
}
void JsValue::operator=(double value) {
	JsDoubleToNumber(value, &this->internalRef);
}
void JsValue::operator=(JsValueRef value) {
	this->internalRef = value;
}
void JsValue::operator=(std::string value) {
	JsCreateString(value.c_str(), value.length(), &this->internalRef);
}
std::string JsValue::cpp_str() {
	JsValueRef stringValue;
	JsValueType type;
	JsErrorCode typeErr = JsGetValueType(this->internalRef, &type);
	if (typeErr != JsNoError) {
		throw std::runtime_error("Failed to get value's type for string conversion");
	}
	if (type == JsString) {
		stringValue = this->internalRef;
	}
	else {
		JsConvertValueToString(this->internalRef, &stringValue);
	}
	char* string = nullptr;
	size_t length;
	JsCopyString(stringValue, nullptr, 0, &length);
	string = (char*)malloc(length + 1);
	if (!string) {
		throw std::runtime_error("Failed to allocate string buffer");
	}
	JsCopyString(stringValue, string, length + 1, nullptr);
	return std::string(string, length);
}
JsValue::operator std::string() {
	return this->cpp_str();
}
JsValue& JsValue::operator[](std::string prop) {
	return JsValue::operator[](prop.c_str());
};
JsValue& JsValue::operator[](const char* prop) {
	JsValue result;
	JsPropertyIdRef propId;
	JsCreatePropertyId(prop, strlen(prop), &propId);
	if (this->HasProperty(prop)) {
		JsGetProperty(this->internalRef, propId, result.GetInternalRef());
		return result;
	}
	JsSetProperty(this->internalRef, propId, result.GetInternalRef(), true);
	return result;
}
JsValue JSUtils::RunCode(std::string name, std::string code) {
	JsValue result = JS_INVALID_REFERENCE;
	jsThread.DoWork([&]() {
		std::wstring wName(name.begin(), name.end());
		std::wstring wCode(code.begin(), code.end());
		JsErrorCode jsErr = JsRunScript(wCode.c_str(), nextSourceContext++, wName.c_str(), result.GetInternalRef());
		if (jsErr == JsNoError) {
			return;
		}
		Logger::Print<Logger::FAILURE>("Script execution failed: {}", jsErr);
		JsValue exception;
		JsErrorCode exErr = JsGetAndClearException(exception.GetInternalRef());
		if (exErr == JsNoError) {
			std::string message = exception["message"].cpp_str();
			Logger::Print<Logger::FAILURE>("JS Error: {}", message);
			return;
		}
		Logger::Print<Logger::FAILURE>("FATAL: Failed clearing exception, expect a crash?");
		Logger::Print<Logger::FAILURE>("Error code: {}", exErr);
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue JSUtils::RunFile(std::filesystem::path file) {
	std::ifstream stream(file);
	std::string sourceStr(std::istreambuf_iterator<char>(stream), {});
	return RunCode(file.string(), sourceStr);
}