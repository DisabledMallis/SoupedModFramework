#include "../jsfuncs.h"
#include <patchers.h>
#include <functional>
#include <logger.h>
#include <jswrapper.h>

using namespace JsNative;

jsfunction(patchers::registerPatchers) {
	if (jsargc > 0) {
		unsigned int cbRefC = 0;
		JsValueRef jsCallback = jsargv[1];
		JsAddRef(jsCallback, &cbRefC);
		std::function<bool(std::string, std::string&)> patchInvoker = [jsCallback, &cbRefC](std::string fileName, std::string& fileContent)->bool {
			JsAddRef(jsCallback, &cbRefC);
			bool success;
			std::function<void()> doPatchwork = [&success, jsCallback, &cbRefC, fileName, &fileContent]() {
				JsAddRef(jsCallback, &cbRefC);
				JsValueRef jsFileName = JSWrapper::FromCppString(fileName);
				JsValueRef jsFileContent = JSWrapper::FromCppString(fileContent);

				JsValueRef undefined;
				JsGetUndefinedValue(&undefined);

				JsValueRef jsArgs[] = {
					undefined,
					jsFileName,
					jsFileContent
				};
				JsErrorCode ctxErr = JsSetCurrentContext(JSWrapper::GetGlobalContext());
				if (ctxErr != JsNoError) {
					Logger::Print<Logger::FAILURE>("Context Error code (Did we get GC'd?): {}", ctxErr);
					JsRelease(jsCallback, &cbRefC);
					return false;
				}
				JsValueRef result;
				JsErrorCode jsErr = JsCallFunction(jsCallback, jsArgs, 3, &result);
				if (jsErr != JsNoError) {
					Logger::Print<Logger::WARNING>("JS Error code: {}", jsErr);
					JSWrapper::HandleException();
					JsRelease(jsCallback, &cbRefC);
					return false;
				}

				JsValueRef successful = JSWrapper::ReadProperty(result, L"successful");
				JsBooleanToBool(successful, &success);
				if (success) {
					JsValueRef jsPatchedContent = JSWrapper::ReadProperty(result, L"data");
					fileContent = JSWrapper::ToCppString(jsPatchedContent);
				}

				JsRelease(jsCallback, &cbRefC);
				return success;
			};
			JSWrapper::DoWork(doPatchwork);
			JSWrapper::AwaitWork();
			JsRelease(jsCallback, &cbRefC);
			return success;
		};
		Patchers::RegisterPatcher(patchInvoker);
		JsRelease(jsCallback, &cbRefC);
	}
	return JS_INVALID_REFERENCE;
}