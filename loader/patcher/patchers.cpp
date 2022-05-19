#include <patchers.h>
#include <logger.h>

//success(filename, data)
std::vector<Patchers::Patcher*> patcherList;

jsfunction(Patchers::registerPatcher) {
	JSUtils::JsValue patcherId = 0;
	if (jsargc == 3) {
		JSUtils::JsValue callback = jsargv[1];
		JSUtils::JsValue targetFile = jsargv[2];
		
		if (!callback.IsValid()) {
			Logger::Debug("Attemped to register a patcher, but the callback is invalid. Was a function passed?");
			return JS_INVALID_REFERENCE;
		}

		JsPatcher* jsPatcher = new JsPatcher(targetFile, callback);
		
		int id = Patchers::RegisterPatcher(jsPatcher);
		patcherId = id;
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

void Patchers::PatchData(std::string filename, std::string& data) {
	for (auto patcher : patcherList) {
		if (filename.find(patcher->selector) != std::string::npos) {
			//In case the patch fails, we want to fall back to the last successfull version of the data
			std::string prePatch = data;
			if (!patcher->DoPatchwork(filename, data)) {
				data = prePatch;
			}
		}
	}
}

Patchers::JsPatcher::JsPatcher(std::string selector, JSUtils::JsValue jsCallback) : Patcher(selector)
{
	this->jsCallback = jsCallback;
	JsErrorCode refErr = JsAddRef(this->jsCallback.internalRef, nullptr);
	if (refErr != JsNoError)
		CATCHERROR(refErr);
}

Patchers::JsPatcher::~JsPatcher() {};

bool Patchers::Patcher::DoPatchwork(std::string, std::string&)
{
	return true;
}

bool Patchers::JsPatcher::DoPatchwork(std::string fileName, std::string& fileContent)
{
	if (!this->jsCallback.IsValid()) {
		Logger::Debug("A patcher was executed, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
		return false;
	}

	if (this->selector.size() == 0) {
		Logger::Debug("A javascript patcher was invoked with no target selector?");
		return false;
	}
	if (fileName.find(this->selector) == std::string::npos) {
		//The patch doesn't target this file
		return false;
	}

	//Call the function on the ui thread
	Logger::Debug("Invoking patcher callback");
	JSUtils::JsValue result = this->jsCallback(fileName, fileContent);
	Logger::Debug("Patcher completed");
	
	if (result.HasProperty("successful")) {
		bool success = result.GetProperty("successful");
		if (success) {
			if (result.HasProperty("data")) {
				fileContent = result.GetProperty("data").cpp_str();
				return true;
			}
			else {
				JSUtils::ThrowException("Patcher result missing 'data' property");
				return false;
			}
		}
		else {
			return false;
		}
	}
	JSUtils::ThrowException("Patchers must return an object with the fields 'successful' and 'data'");
	return false;
}
