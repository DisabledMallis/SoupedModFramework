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
static JsValue global = JS_INVALID_REFERENCE;
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
	jsThread.DoWork([&]() {
		if (global.internalRef == JS_INVALID_REFERENCE) {
			JsErrorCode jsErr = JsGetGlobalObject(&global.internalRef);
			if (jsErr != JsNoError) {
				Logger::Print<Logger::WARNING>("Error whilst retrieving global object: {}", (void*)jsErr);
				JsValue exception;
				JsErrorCode exErr = JsGetAndClearException(&exception.internalRef);
				if (exErr == JsNoError) {
					std::string message = exception.GetProperty("message").cpp_str();
					Logger::Print<Logger::FAILURE>("JS Error: {}", message);
				}
			}
		}
	});
	jsThread.AwaitCompletion();
	return global;
}

JsValue::JsValue() {
	this->internalRef = JS_INVALID_REFERENCE;
}
JsValue::JsValue(int value) {
	jsThread.DoWork([&]() {
		JsIntToNumber(value, &this->internalRef);
	});
}
JsValue::JsValue(JsValueRef valRef) {
	this->internalRef = valRef;
}
JsValue::JsValue(std::string text) {
	jsThread.DoWork([&]() {
		JsCreateString(text.c_str(), text.length(), &this->internalRef);
	});
}
JsValue::JsValue(std::string objName, bool isObject) {
	jsThread.DoWork([&]() {
		if (isObject) {
			JsCreateObject(&this->internalRef);
		}
		else {
			JsValue(objName);
		}
	});
};
JsValue::JsValue(JsNativeFunction func) {
	jsThread.DoWork([&]() {
		JsCreateFunction(func, nullptr, &this->internalRef);
	});
	jsThread.AwaitCompletion();
}
bool JsValue::IsValid() {
	if (!this) {
		return false;
	}
	if (!this->internalRef) {
		return false;
	}
	bool result = false;
	jsThread.DoWork([&]() {
		JsValueType valType = JsUndefined;
		JsErrorCode jsErr = JsGetValueType(this->internalRef, &valType);
		if (jsErr != JsNoError) {
			result = false;
			return;
		}
		if (valType == JsUndefined) {
			result = false;
			return;
		}
		if (valType == JsNull) {
			result = false;
			return;
		}
		result = true;
		return;
	});
	jsThread.AwaitCompletion();
	return result;
}
bool JsValue::HasProperty(std::string propName) {
	if (!this->IsValid()) {
		return false;
	}
	bool result;
	jsThread.DoWork([&]() {
		JsValue jsPropName = propName;
		JsObjectHasProperty(this->internalRef, jsPropName.internalRef, &result);
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue& JsValue::GetProperty(std::string propName) {
	JsValue* result = new JsValue;
	jsThread.DoWork([&]() {
		JsValueRef propId;
		JsCreatePropertyId(propName.c_str(), propName.length(), &propId);
		if (this->HasProperty(propName)) {
			JsGetProperty(this->internalRef, propId, &result->internalRef);
			return;
		}
		throw std::runtime_error("Object does not have specified property");
	});
	jsThread.AwaitCompletion();
	return *result;
}
void JsValue::SetProperty(std::string propName, JsValue& value) {
	jsThread.DoWork([&]() {
		JsValueRef propId;
		JsCreatePropertyId(propName.c_str(), propName.length(), &propId);
		JsSetProperty(this->internalRef, propId, value.internalRef, true);
	});
	jsThread.AwaitCompletion();
}
std::string JsValue::cpp_str() {
	char* string = nullptr;
	size_t length;
	jsThread.DoWork([&]() {
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
		JsCopyString(stringValue, nullptr, 0, &length);
		string = (char*)malloc(length + 1);
		if (!string) {
			throw std::runtime_error("Failed to allocate string buffer");
		}
		JsCopyString(stringValue, string, length + 1, nullptr);
	});
	jsThread.AwaitCompletion();
	return std::string(string, length);
}
JsValue::operator bool() {
	bool result;
	jsThread.DoWork([&]() {
		if (this->IsValid()) {
			JsValueType valType;
			JsGetValueType(this->internalRef, &valType);
			if (valType == JsBoolean) {
				JsBooleanToBool(this->internalRef, &result);
			}
			throw std::exception("Value isn't a boolean");
		}
		throw std::exception("Value isn't valid");
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue::operator int() {
	int result;
	jsThread.DoWork([&]() {
		if (this->IsValid()) {
			JsValueType valType;
			JsGetValueType(this->internalRef, &valType);
			if (valType == JsNumber) {
				JsNumberToInt(this->internalRef, &result);
			}
			throw std::exception("Value isn't a number");
		}
		throw std::exception("Value isn't valid");
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue::operator double() {
	double result;
	jsThread.DoWork([&]() {
		if (this->IsValid()) {
			JsValueType valType;
			JsGetValueType(this->internalRef, &valType);
			if (valType == JsNumber) {
				JsNumberToDouble(this->internalRef, &result);
			}
			throw std::exception("Value isn't a number");
		}
		throw std::exception("Value isn't valid");
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue::operator JsValueRef() {
	return this->internalRef;
}
void JsValue::operator=(bool value) {
	jsThread.DoWork([&]() {
		JsBoolToBoolean(value, &this->internalRef);
	});
}
void JsValue::operator=(int value) {
	jsThread.DoWork([&]() {
		JsIntToNumber(value, &this->internalRef);
	});
}
void JsValue::operator=(double value) {
	jsThread.DoWork([&]() {
		JsDoubleToNumber(value, &this->internalRef);
	});
}
void JsValue::operator=(JsValueRef value) {
	this->internalRef = value;
}
void JsValue::operator=(JsNativeFunction value) {
	jsThread.DoWork([&]() {
		JsCreateFunction(value, nullptr, &this->internalRef);
	});
}
void JsValue::operator=(std::string value) {
	jsThread.DoWork([&]() {
		JsCreateString(value.c_str(), value.length(), &this->internalRef);
	});
}
JsValue::operator std::string() {
	return this->cpp_str();
}
JsValue JSUtils::RunCode(std::string name, std::string code) {
	JsValue result = JS_INVALID_REFERENCE;
	jsThread.DoWork([name, code, &result]() {
		std::wstring wName(name.begin(), name.end());
		std::wstring wCode(code.begin(), code.end());
		JsErrorCode jsErr = JsRunScript(wCode.c_str(), nextSourceContext++, wName.c_str(), &result.internalRef);
		if (jsErr == JsNoError) {
			return;
		}
		Logger::Print<Logger::FAILURE>("Script execution failed: {}", jsErr);
		JsValue exception;
		JsErrorCode exErr = JsGetAndClearException(&exception.internalRef);
		if (exErr == JsNoError) {
			std::string message = exception.GetProperty("message").cpp_str();
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
	std::string sourceStr((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	return RunCode(file.string(), sourceStr);
}