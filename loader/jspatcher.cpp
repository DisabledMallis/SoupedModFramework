#include "jsfuncs.h"
#include <patchers.h>
#include <functional>
#include <logger.h>
#include <jswrapper.h>

using namespace JsNative;

jsfunction(patchers::registerPatchers) {
	if (jsargc > 0) {
		JsValueRef jsCallback = jsargv[1];
		std::function<bool(std::string, std::string&)> patchInvoker = [jsCallback](std::string fileName, std::string& fileContent)->bool {
			JsValueRef jsFileName = JSWrapper::FromCppString(fileName);
			JsValueRef jsFileContent = JSWrapper::FromCppString(fileContent);

			JsValueRef undefined;
			JsGetUndefinedValue(&undefined);

			JsValueRef jsArgs[] = {
				undefined,
				jsFileName,
				jsFileContent
			};
			JsValueRef result;
			JsErrorCode jsErr = JsCallFunction(jsCallback, jsArgs, 3, &result);
			if (jsErr != JsNoError) {
				JSWrapper::HandleException();
				return false;
			}

			JsValueRef successful = JSWrapper::ReadProperty(result, L"successful");
			JsValueRef jsPatchedContent = JSWrapper::ReadProperty(result, L"data");
			fileContent = JSWrapper::ToCppString(jsPatchedContent);

			bool success = false;
			JsBooleanToBool(successful, &success);
			return success;
		};
		Patchers::RegisterPatcher(patchInvoker);
	}
	return JS_INVALID_REFERENCE;
}