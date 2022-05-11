#include <patchers.h>
#include <logger.h>
#include "../ui/webui.h"

//success(filename, data)
std::vector<std::function<bool(std::string, std::string&)>> patcherList;

jsfunction(Patchers::registerPatcher) {
	JSValueRef patcherId = JSUtils::GetUndefined();
	if (jsargc == 2) {
		JSObjectRef jsCallback = (JSObjectRef)jsargv[0];
		
		if (!jsCallback) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher with no callback");
			return patcherId;
		}
		if (JSValueIsUndefined(JSUtils::GetContext(), jsCallback)) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher, but the callback is undefined. Did the callback get GC'd?");
			return patcherId;
		}
		if (!JSValueIsObject(JSUtils::GetContext(), jsCallback)) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
			return patcherId;
		}

		//Somehow need to unprotect the value when the patcher is destroyed.
		//May be a good idea to make a patcher struct instead of just a function callback?
		//Either way, TODO: fix
		JSValueProtect(JSUtils::GetContext(), jsCallback);

		std::string targetFile = JSUtils::GetString((JSStringRef)jsargv[1]);
		std::function<bool(std::string, std::string&)> patchInvoker = [jsCallback, targetFile](std::string fileName, std::string& fileContent)->bool {
			//Acquire JS context from WebUI
			ultralight::RefPtr<ultralight::JSContext> ctxRef = WebUI::AcquireJSContext();
			JSUtils::SetContext(ctxRef->ctx());
			
			if (!jsCallback) {
				Logger::Print<Logger::WARNING>("Attemped to run a patcher with no callback");
				return false;
			}

			if (JSValueIsUndefined(JSUtils::GetContext(), jsCallback)) {
				Logger::Print<Logger::WARNING>("A patcher was executed but the callback is undefined. Did the callback get GC'd?");
				return false;
			}

			if (!JSValueIsObject(JSUtils::GetContext(), jsCallback)) {
				Logger::Print<Logger::WARNING>("A patcher was executed, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
				return false;
			}

			if (targetFile.size() == 0) {
				Logger::Print<Logger::WARNING>("A javascript patcher was invoked with no target?");
				return false;
			}
			if (fileName.find(targetFile) == std::string::npos) {
				//The patch doesn't target this file
				return false;
			}
			std::string patchResult = "";
			bool success = false;

			JSStringRef jsFileName = JSUtils::CreateString(fileName);
			JSStringRef jsFileContent = JSUtils::CreateString(fileContent);

			const JSValueRef jsArgs[] = {
				(JSValueRef)JSValueMakeString(JSUtils::GetContext(), jsFileName),
				(JSValueRef)JSValueMakeString(JSUtils::GetContext(), jsFileContent)
			};
			JSValueRef result = JSUtils::CallFunction(jsCallback, 0, jsArgs, 2);

			if (!JSValueIsObject(JSUtils::GetContext(), result)) {
				Logger::Print<Logger::WARNING>("JS Patcher callback did NOT return an object! Ignoring patch...");
				return false;
			}

			JSValueRef successful = JSUtils::ReadProperty((JSObjectRef)result, "successful");
			success = JSValueToBoolean(JSUtils::GetContext(), successful);
			if (success) {
				JSStringRef jsPatchedContent = (JSStringRef)JSUtils::ReadProperty((JSObjectRef)result, "data");
				patchResult = JSUtils::GetString(jsPatchedContent);
			}

			fileContent = patchResult;
			return success;
		};
		size_t id = Patchers::RegisterPatcher(patchInvoker);
		patcherId = JSValueMakeNumber(JSUtils::GetContext(), id);
		return patcherId;
	}
	else {
		Logger::Print<Logger::FAILURE>("souped.registerPatcher called with the incorrect number of arguments");
	}
	return patcherId;
}

size_t Patchers::RegisterPatcher(std::function<bool(std::string, std::string&)> patcher) {
	patcherList.push_back(patcher);

	return patcherList.size() - 1;
};

void Patchers::DestroyPatcher(size_t idx) {
	patcherList.erase(patcherList.begin() + idx);
};

void Patchers::PatchData(std::string filename, std::string& data) {
	for (auto patcher : patcherList) {
		//In case the patch fails, we want to fall back to the last successfull version of the data
		std::string prePatch = data;
		if (!patcher(filename, data)) {
			data = prePatch;
		}
	}
}