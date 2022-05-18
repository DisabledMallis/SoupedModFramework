#include <jsutils.h>
#include <deque>
#include <stdexcept>
using namespace JSUtils;

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
	JsDisposeRuntime(runtime);
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
	JsSetCurrentContext(currentContext);
}
JsContextRef JSUtils::GetCurrentContext() {
	return currentContext;
}

JsValue JSUtils::GetGlobalObject() {
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
JsValue::JsValue(std::string objName, JsValue parentObj) {
	JsPropertyIdRef propId;
	JsCreateObject(&this->internalRef);
	JsCreatePropertyId(objName.c_str(), objName.length(), &propId);
	JsSetProperty(parentObj, propId, this->internalRef, true);
};
JsValue::JsValue(std::string name, JsNativeFunction func, JsValue parentObj) {
	JsCreateFunction(func, nullptr, this->GetInternalRef());
	JsPropertyIdRef fPropId;
	JsCreatePropertyId(name.c_str(), name.length(), &fPropId);
	JsSetProperty(parentObj, fPropId, this->GetInternalRef(), true);
}
JsValueRef* JsValue::GetInternalRef() {
	return &this->internalRef;
}
bool JsValue::IsValid() {
	JsValueType valType;
	JsErrorCode jsErr = JsGetValueType(this->GetInternalRef(), &valType);
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
JsValue::operator bool() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->GetInternalRef(), &valType);
		if (valType == JsBoolean) {
			bool result;
			JsBooleanToBool(this->GetInternalRef(), &result);
			return result;
		}
		throw std::exception("Value isn't a boolean");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator int() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->GetInternalRef(), &valType);
		if (valType == JsNumber) {
			int result;
			JsNumberToInt(this->GetInternalRef(), &result);
			return result;
		}
		throw std::exception("Value isn't a number");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator double() {
	if (this->IsValid()) {
		JsValueType valType;
		JsGetValueType(this->GetInternalRef(), &valType);
		if (valType == JsNumber) {
			double result;
			JsNumberToDouble(this->GetInternalRef(), &result);
			return result;
		}
		throw std::exception("Value isn't a number");
	}
	throw std::exception("Value isn't valid");
}
JsValue::operator JsValueRef() {
	return this->GetInternalRef();
}
void JsValue::operator=(bool value) {
	JsBoolToBoolean(value, this->GetInternalRef());
}
void JsValue::operator=(int value) {
	JsIntToNumber(value, this->GetInternalRef());
}
void JsValue::operator=(double value) {
	JsDoubleToNumber(value, this->GetInternalRef());
}
void JsValue::operator=(JsValueRef value) {
	this->internalRef = value;
}
void JsValue::operator=(std::string value) {
	std::wstring wValue(value.begin(), value.end());
	JsPointerToString(wValue.c_str(), wValue.length(), this->GetInternalRef());
}
std::string JsValue::cpp_str() {
	JsValueRef stringValue;
	JsConvertValueToString(this->internalRef, &stringValue);
	char* string = nullptr;
	size_t length;
	JsCopyString(stringValue, nullptr, 0, &length);
	string = (char*)_malloca(length + 1);
	if (!string) {
		throw std::runtime_error("Failed to allocate string buffer");
	}
	JsCopyString(stringValue, string, length + 1, nullptr);
	return std::string(string, length);
}
JsValue::operator std::string() {
	return this->cpp_str();
}
JsValue JsValue::operator[](const char* prop) {
	JsValue jsProp(prop);
	JsValue result;
	JsObjectGetProperty(this->internalRef, jsProp.GetInternalRef(), result.GetInternalRef());
	return result;
}

JsValue JSUtils::RunScript(std::string name, std::string code) {
	JsValue result;
	std::wstring wName(name.begin(), name.end());
	std::wstring wCode(code.begin(), code.end());
	JsErrorCode jsErr = JsRunScript(wCode.c_str(), nextSourceContext++, wName.c_str(), result.GetInternalRef());
	if (jsErr == JsNoError) {
		return result;
	}
	throw std::runtime_error("Script execution failed: " + jsErr);
}