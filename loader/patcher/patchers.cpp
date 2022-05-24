#include "patchers.h"
#include <logger.h>

std::vector<Patchers::Patcher*> patcherList;

jsfunction(Patchers::registerPatcher) {
	JSUtils::JsValue patcherId = -1;
	if (jsargc == 4) {
		JSUtils::JsValue targetBundle = jsargv[1];
		JSUtils::JsValue targetFile = jsargv[2];
		JSUtils::JsValue callback = jsargv[3];
		
		if (!callback.IsValid()) {
			Logger::Debug("Attemped to register a patcher, but the callback is invalid. Was a function passed?");
			return JS_INVALID_REFERENCE;
		}

		ModRegistry::Mod* currentMod = ModRegistry::GetImmediateMod();
		JsPatcher* jsPatcher = new JsPatcher(targetBundle, targetFile, currentMod, callback);
		
		int id = Patchers::RegisterPatcher(jsPatcher);
		patcherId = id;
		Logger::Debug("Registered patcher with id {}", id);
		return patcherId;
	}
	else {
		Logger::Debug("souped.registerPatcher called with the incorrect number of arguments");
	}
	return patcherId;
}

size_t Patchers::RegisterPatcher(Patcher* patcher) {
	patcherList.push_back(patcher);

	return patcherList.size() - 1;
};

void Patchers::DestroyPatcher(size_t idx) {
	patcherList.erase(patcherList.begin() + idx);
};

void Patchers::PatchData(std::string bundleName, std::string fileName, std::string& data) {
	for (auto patcher : patcherList) {
		if (fileName.find(patcher->selector) != std::string::npos) {
			if (patcher->bundleName != "*") {
				if (bundleName.find(patcher->bundleName) == std::string::npos) {
					continue;
				}
			}
			//In case the patch fails, we want to fall back to the last successfull version of the data
			std::string prePatch = data;
			if (!patcher->DoPatchwork(bundleName, fileName, data)) {
				data = prePatch;
			}
		}
	}
}

Patchers::JsPatcher::JsPatcher(std::string bundleName, std::string selector, ModRegistry::Mod* source, JSUtils::JsValue jsCallback) : Patcher(bundleName, selector)
{
	this->source = source;
	this->jsCallback = jsCallback;
	JsErrorCode refErr = JsAddRef(this->jsCallback.internalRef, nullptr);
	if (refErr != JsNoError)
		CATCHERROR(refErr);
}

Patchers::JsPatcher::~JsPatcher() {};

bool Patchers::Patcher::DoPatchwork(std::string, std::string, std::string&)
{
	return true;
}

bool Patchers::JsPatcher::DoPatchwork(std::string bundleName, std::string fileName, std::string& fileContent)
{
	if (!this->jsCallback.IsValid()) {
		Logger::Debug("A patcher was executed, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
		return false;
	}

	if (this->selector.size() == 0) {
		Logger::Debug("A javascript patcher was invoked with no target selector?");
		return false;
	}
	if (this->bundleName != "*") {
		if (bundleName.find(this->bundleName) == std::string::npos) {
			//Not the bundle we're looking for
			return false;
		}
	}
	if (fileName.find(this->selector) == std::string::npos) {
		//The patch doesn't target this file
		return false;
	}

	//Call the function on the ui thread
	Logger::Debug("Invoking patcher callback");
	JSUtils::JsValue result = this->jsCallback(bundleName, fileName, fileContent);
	Logger::Debug("Patcher completed");
	
	if (result.HasProperty("successful")) {
		bool success = result.GetProperty("successful");
		if (success) {
			if (result.HasProperty("data")) {
				fileContent = result.GetProperty("data").cpp_str();
				JsRelease(result.internalRef, nullptr);
				return true;
			}
			else {
				JsRelease(result.internalRef, nullptr);
				JSUtils::ThrowException("Patcher result missing 'data' property");
				return false;
			}
		}
		else {
			JsRelease(result.internalRef, nullptr);
			return false;
		}
	}
	JsRelease(result.internalRef, nullptr);
	JSUtils::ThrowException("Patchers must return an object with the fields 'successful' and 'data'");
	return false;
}
