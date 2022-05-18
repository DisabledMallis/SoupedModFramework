#include <patchers.h>
#include <logger.h>

//success(filename, data)
std::vector<Patchers::Patcher*> patcherList;

jsfunction(Patchers::registerPatcher) {
	JSUtils::JsValue patcherId = 0;
	if (jsargc == 2) {
		JSUtils::JsValue callback = jsargv[0];
		JSUtils::JsValue targetFile = jsargv[1];
		
		if (!callback.IsValid()) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher, but the callback is invalid. Was a function passed?");
			return JS_INVALID_REFERENCE;
		}

		JsPatcher* jsPatcher = new JsPatcher(targetFile, callback);
		
		int id = Patchers::RegisterPatcher(jsPatcher);
		patcherId = id;
		return patcherId;
	}
	else {
		Logger::Print<Logger::FAILURE>("souped.registerPatcher called with the incorrect number of arguments");
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
}

Patchers::JsPatcher::~JsPatcher() {};

bool Patchers::Patcher::DoPatchwork(std::string, std::string&)
{
	return true;
}

bool Patchers::JsPatcher::DoPatchwork(std::string fileName, std::string& fileContent)
{
	if (!this->jsCallback) {
		Logger::Print<Logger::WARNING>("Attemped to run a patcher with no callback");
		return false;
	}

	if (!this->jsCallback.IsValid()) {
		Logger::Print<Logger::WARNING>("A patcher was executed, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
		return false;
	}

	if (this->selector.size() == 0) {
		Logger::Print<Logger::WARNING>("A javascript patcher was invoked with no target selector?");
		return false;
	}
	if (fileName.find(this->selector) == std::string::npos) {
		//The patch doesn't target this file
		return false;
	}
	std::string patchResult = "";
	bool success = false;

	//Call the function on the ui thread
	Logger::Print("Invoking patcher callback");
	JSUtils::JsValue result = this->jsCallback(fileName, fileContent);
	Logger::Print("Patcher completed");

	success = result.GetProperty("successful");
	if (success) {
		fileContent = JSUtils::JsValue(result.GetProperty("data"));
	}
	return success;
}
