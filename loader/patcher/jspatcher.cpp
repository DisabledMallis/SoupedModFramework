#include "../jsfuncs.h"
#include <patchers.h>
#include <functional>
#include <logger.h>
#include <jswrapper.h>

using namespace JsNative;

jsfunction(patchers::registerPatcher) {
	if (jsargc == 2) {
		JsValueRef jsCallback = jsargv[1];
		std::string targetFile = JSWrapper::ToCppString(jsargv[2]);
		unsigned int cbRefC = 0;
		unsigned int tfRefC = 0;
		JsAddRef(jsCallback, &cbRefC);
		std::function<bool(std::string, std::string&)> patchInvoker = [jsCallback, targetFile, &cbRefC, &tfRefC](std::string fileName, std::string& fileContent)->bool {
			JsAddRef(jsCallback, &cbRefC);
			if (targetFile.size() == 0) {
				Logger::Print<Logger::WARNING>("A javascript patcher was invoked with no target?");
				return false;
			}
			if (fileName.find(targetFile) == std::string::npos) {
				//The patch doesn't target this file
				return false;
			}
			std::string patchResult;
			bool success;
			std::function<void()> doPatchwork = [&patchResult, &success, fileName, fileContent, &cbRefC, jsCallback]() {
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
					patchResult = JSWrapper::ToCppString(jsPatchedContent);
				}

				JsRelease(jsCallback, &cbRefC);
				return success;
			};
			JSWrapper::DoWork(doPatchwork);
			JSWrapper::AwaitWork();
			fileContent = patchResult;
			Logger::Print("fileContent: {}", fileContent);
			JsRelease(jsCallback, &cbRefC);
			return success;
		};
		Patchers::RegisterPatcher(patchInvoker);
		JsRelease(jsCallback, &cbRefC);
	}
	else {
		Logger::Print<Logger::FAILURE>("patchers.registerPatchers called with the incorrect number of arguments");
	}
	return JS_INVALID_REFERENCE;
}