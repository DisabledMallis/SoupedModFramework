#include <jsutils.h>
#include <deque>
#include <stdexcept>
#include <shared_thread.h>
#include <fstream>
#include <logger.h>
using namespace JSUtils;

static shared_thread jsThread(10);
static std::thread conListener;
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
	conListener = std::thread([&]() {
		while (true) {
			std::string code;
			std::getline(std::cin, code);
			JsValue result = JSUtils::RunCode("console", code);
			if (result.IsValid()) {
				Logger::Print(result.cpp_str());
			}
		}
	});
	conListener.detach();
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
				Logger::Debug("Error whilst retrieving global object: {}", (void*)jsErr);
				JsValue exception;
				JsErrorCode exErr = JsGetAndClearException(&exception.internalRef);
				if (exErr == JsNoError) {
					std::string message = exception.GetProperty("message").cpp_str();
					Logger::Debug("JS Error: {}", message);
				}
			}
		}
	});
	jsThread.AwaitCompletion();
	return global;
}
JsValue JSUtils::RunCode(std::string name, std::string code) {
	JsValue result = JS_INVALID_REFERENCE;
	jsThread.DoWork([name, code, &result]() {
		std::wstring wName(name.begin(), name.end());
		std::wstring wCode(code.begin(), code.end());
		JsErrorCode jsErr = JsRunScript(wCode.c_str(), nextSourceContext++, wName.c_str(), &result.internalRef);
		CATCHERROR(jsErr);
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue JSUtils::RunFile(std::filesystem::path file) {
	std::ifstream stream(file);
	std::string sourceStr((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	return RunCode(file.string(), sourceStr);
}
JsErrorCode JSUtils::JsCallSafely(JsValueRef func, JsValueRef* argv, unsigned short argc, JsValueRef* result) {
	JsErrorCode jsErr;
	for (int i = 0; i < argc; i++) {
		JsAddRef(argv[i], nullptr);
	}
	jsThread.DoWork([&]() {
		JSUtils::SetCurrentContext(JSUtils::GetDefaultContext());
		jsErr = JsCallFunction(func, argv, argc, result);
		if (jsErr != JsNoError) {
			CATCHERROR(jsErr);
		}
	});
	jsThread.AwaitCompletion();
	for (int i = 0; i < argc; i++) {
		JsRelease(argv[i], nullptr);
	}
	return jsErr;
}
/*
* Returns: If an exception was handled/needed to be handled
* jsErr: The JS error code
*/
bool JSUtils::HandleException(JsErrorCode jsErr, std::string func, std::string file, int line) {
	if (jsErr == JsNoError) {
		return false;
	}
	bool result = false;
	jsThread.DoWork([&]() {
		JsValue exception;
		JsErrorCode exErr = JsGetAndClearException(&exception.internalRef);
		if (exErr == JsNoError) {
			std::string name = "<name>";
			if (exception.HasProperty("name")) {
				name = exception.GetProperty("name").cpp_str();
			}
			std::string message = "<message>";
			if (exception.HasProperty("message")) {
				message = exception.GetProperty("message").cpp_str();
			}
			Logger::Print<Logger::FAILURE>("{}: {}", name, message);
			if (exception.HasProperty("stack")) {
				std::string stack = exception.GetProperty("stack").cpp_str();
				Logger::Print<Logger::FAILURE>("{}", stack);
			}
			result = true;
			return;
		}
		Logger::Print<Logger::FAILURE>("Host error: {}, Function: {}, File: {}, Line: {}", exErr, func, file, line);
	});
	jsThread.AwaitCompletion();
	return result;
}
void JSUtils::ThrowException(std::string message) {
	jsThread.DoWork([&]() {
		JsValue jsMsg = message;
		JsValue exception;
		JsCreateError(jsMsg, &exception.internalRef);
		JsSetException(exception.internalRef);
	});
	jsThread.AwaitCompletion();
	CATCHERROR(JsErrorInExceptionState);
}

JsValue::JsValue() {
	this->internalRef = JS_INVALID_REFERENCE;
}
JsValue::JsValue(int value) {
	this->internalRef = JS_INVALID_REFERENCE;
	jsThread.DoWork([&]() {
		JsErrorCode err = JsIntToNumber(value, &this->internalRef);
		JsAddRef(this->internalRef, nullptr);
		if (err != JsNoError) {
			CATCHERROR(err);
		}
	});
	jsThread.AwaitCompletion();
}
JsValue::JsValue(JsValueRef valRef) {
	this->internalRef = valRef;
}
JsValue::JsValue(std::string text) {
	this->internalRef = JS_INVALID_REFERENCE;
	jsThread.DoWork([&]() {
		JsErrorCode err = JsCreateString(text.c_str(), text.length(), &this->internalRef);
		JsAddRef(this->internalRef, nullptr);
		if (err != JsNoError) {
			CATCHERROR(err);
		}
	});
	jsThread.AwaitCompletion();
}
JsValue::JsValue(std::string objName, bool isObject) {
	this->internalRef = JS_INVALID_REFERENCE;
	jsThread.DoWork([&]() {
		if (isObject) {
			JsCreateObject(&this->internalRef);
			JsAddRef(this->internalRef, nullptr);
		}
		else {
			JsValue(objName);
		}
	});
	jsThread.AwaitCompletion();
};
JsValue::JsValue(JsNativeFunction func) {
	this->internalRef = JS_INVALID_REFERENCE;
	jsThread.DoWork([&]() {
		JsErrorCode err = JsCreateFunction(func, nullptr, &this->internalRef);
		JsAddRef(this->internalRef, nullptr);
		if (err != JsNoError) {
			CATCHERROR(err);
		}
	});
	jsThread.AwaitCompletion();
}
JsValue::~JsValue() {
	//jsThread.DoWork([&]() {
	//	JsRelease(this->internalRef, nullptr); 
	//});
	//jsThread.AwaitCompletion();
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
			CATCHERROR(jsErr);
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
		JsErrorCode err = JsObjectHasProperty(this->internalRef, jsPropName.internalRef, &result);
		if(err != JsNoError)
			CATCHERROR(err);
	});
	jsThread.AwaitCompletion();
	return result;
}
JsValue& JsValue::GetProperty(std::string propName) {
	JsValue* result = new JsValue;
	jsThread.DoWork([&]() {
		JsValueRef propId;
		JsErrorCode createErr = JsCreatePropertyId(propName.c_str(), propName.length(), &propId);
		if (createErr != JsNoError)
			CATCHERROR(createErr);
		if (this->HasProperty(propName)) {
			JsErrorCode getErr = JsGetProperty(this->internalRef, propId, &result->internalRef);
			if (getErr != JsNoError)
				CATCHERROR(getErr);
			return;
		}
		ThrowException("Object does not have specified property");
	});
	jsThread.AwaitCompletion();
	return *result;
}
void JsValue::SetProperty(std::string propName, JsValue& value) {
	jsThread.DoWork([&]() {
		JsValueRef propId;
		JsErrorCode createErr = JsCreatePropertyId(propName.c_str(), propName.length(), &propId);
		if (createErr != JsNoError) {
			CATCHERROR(createErr);
		}
		JsErrorCode setErr = JsSetProperty(this->internalRef, propId, value.internalRef, true);
		if (setErr != JsNoError) {
			CATCHERROR(setErr);
		}
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
			CATCHERROR(typeErr);
		}
		if (type == JsString) {
			stringValue = this->internalRef;
		}
		else {
			JsErrorCode convertError = JsConvertValueToString(this->internalRef, &stringValue);
			if (convertError != JsNoError) {
				CATCHERROR(convertError);
			}
		}
		JsErrorCode lenErr = JsCopyString(stringValue, nullptr, 0, &length);
		if (lenErr != JsNoError) {
			CATCHERROR(lenErr);
		}
		string = (char*)malloc(length + 1);
		if (!string) {
			Logger::Debug("Failed to allocate string buffer");
		}
		JsErrorCode copyErr = JsCopyString(stringValue, string, length, nullptr);
		if (copyErr != JsNoError) {
			CATCHERROR(copyErr);
		}
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
				return;
			}
			ThrowException("Value isn't a boolean");
		}
		else {
			ThrowException("Value isn't valid");
		}
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
			Logger::Debug("Value isn't a number");
		}
		Logger::Debug("Value isn't valid");
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
