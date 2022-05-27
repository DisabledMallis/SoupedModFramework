#include "overrides.h"

#include <vector>
#include <logger.h>

static std::vector<Overrides::Override*> fileOverrides;

Overrides::Override::Override(std::string targetBundle, std::string targetFile, bool modifyable)
{
	this->targetBundle = targetBundle;
	this->targetFile = targetFile;
	this->modifyable = modifyable;
}

void Overrides::Override::GetContent(std::string bundleName, std::string fileName, std::string& content, bool isPost)
{
	return;
}

void Overrides::JsOverride::GetContent(std::string bundleName, std::string fileName, std::string& content, bool isPost)
{
	for (auto* fileOverride : fileOverrides) {
		//Check if its modifyable, if not we should check the next
		if (fileOverride->modifyable != isPost)
			continue;
		if (fileOverride->targetBundle != "*")
			if (bundleName.find(fileOverride->targetBundle) == std::string::npos)
				continue;
		if (fileName.find(fileOverride->targetFile) == std::string::npos)
			continue;
		JSUtils::JsValue result = this->jsCallback(bundleName, fileName, content, isPost);
		if (!result.HasProperty("successful") || !result.HasProperty("data")) {
			Logger::Print<Logger::FAILURE>("souped.registerOverride callbacks MUST return an object with the fields 'successful' and 'data'!");
			continue;
		}
		bool success = result.GetProperty("successful");
		if (success != true) {
			continue;
		}
		content = result.GetProperty("data").cpp_str(); //Get content from js
		//We dont return because pre overrides are expected to be patched, so its assumed conflict takes priority
		if (fileOverride->modifyable) {
			return;
		}
	}
}

size_t Overrides::RegisterOverride(Override* ovride) {
	fileOverrides.push_back(ovride);
	return fileOverrides.size() - 1;
}
void Overrides::RunOverrides(std::string targetBundle, std::string targetFile, std::string& content, bool isPost)
{
	for (auto* fileOverride : fileOverrides) {
		fileOverride->GetContent(targetBundle, targetFile, content, isPost);
	}
}

jsfunction(Overrides::registerOverride) {
	using namespace JSUtils;
	if (jsargc == 5) {
		JsValue bundleName = jsargv[1];
		JsValue fileName = jsargv[2];
		JsValue modifyable = jsargv[3];
		JsValue callback = jsargv[4];

		ModRegistry::Mod* currentMod = ModRegistry::GetImmediateMod();
		JsOverride* jsOverride = new JsOverride(bundleName, fileName, modifyable, currentMod, callback);
		size_t id = RegisterOverride(jsOverride);
		Logger::Debug("Registered override with id {}", id);
		return JsValue(id);
	}
	return JsValue(-1);
}