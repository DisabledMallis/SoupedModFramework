#include <patchers.h>
#include <logger.h>

//success(filename, data)
std::vector<std::function<bool(std::string, std::string&)>> patcherList;

jsfunction(Patchers::registerPatcher) {
	if (jsargc == 2) {
		JSObjectRef jsCallback = (JSObjectRef)jsargv[0];
		std::string targetFile = JSUtils::GetString((JSStringRef)jsargv[1]);
		unsigned int cbRefC = 0;
		unsigned int tfRefC = 0;
		std::function<bool(std::string, std::string&)> patchInvoker = [jsCallback, targetFile, &cbRefC, &tfRefC](std::string fileName, std::string& fileContent)->bool {
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

			JSStringRef jsFileName = JSUtils::CreateString(fileName);
			JSStringRef jsFileContent = JSUtils::CreateString(fileContent);

			JSValueRef jsArgs[] = {
				(JSValueRef)jsFileName,
				(JSValueRef)jsFileContent
			};
			JSObjectRef result = (JSObjectRef)JSUtils::CallFunction(jsCallback, jsArgs, 2);

			JSValueRef successful = JSUtils::ReadProperty(result, "successful");
			success = JSValueToBoolean(JSUtils::GetContext(), successful);
			if (success) {
				JSStringRef jsPatchedContent = (JSStringRef)JSUtils::ReadProperty(result, "data");
				patchResult = JSUtils::GetString(jsPatchedContent);
			}

			fileContent = patchResult;
			Logger::Print("fileContent: {}", fileContent);
			return success;
		};
		Patchers::RegisterPatcher(patchInvoker);
	}
	else {
		Logger::Print<Logger::FAILURE>("souped.registerPatcher called with the incorrect number of arguments");
	}
	return JSValueMakeUndefined(JSUtils::GetContext());
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