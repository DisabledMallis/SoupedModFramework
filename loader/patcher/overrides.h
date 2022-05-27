#pragma once

#include <string>
#include <jsutils.h>
#include "../modRegistry.h"

namespace Overrides {
	struct Override {
		//If it should override before or after patches
		bool modifyable = false;
		std::string targetBundle;
		std::string targetFile;
	public:
		Override(std::string targetBundle, std::string targetFile, bool modifyable = false);
		virtual void GetContent(std::string targetBundle, std::string targetFile, std::string& content, bool isPost);
	};

	struct JsOverride : public Override {
		ModRegistry::Mod* source;
		JSUtils::JsValue jsCallback;
	public:
		JsOverride(std::string targetBundle, std::string targetFile, bool modifyable, ModRegistry::Mod* source, JSUtils::JsValue jsCallback) : Override(targetBundle, targetFile, modifyable) {
			this->source = source;
			this->jsCallback = jsCallback;
		};
		void GetContent(std::string targetBundle, std::string targetFile, std::string& content, bool isPost) override;
	};

	size_t RegisterOverride(Override*);
	void RunOverrides(std::string targetBundle, std::string targetFile, std::string& content, bool isPost);

	jsfunction(registerOverride);
};