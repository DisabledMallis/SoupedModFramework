#pragma once

#include <string>
#include <functional>
#include <jsutils.h>
#include "../modRegistry.h"

namespace Patchers {

	struct Patcher {
		std::string bundleName;
		std::string selector;

		Patcher(std::string bundleName, std::string selector) {
			this->bundleName = bundleName;
			this->selector = selector;
		}

		virtual bool DoPatchwork(std::string, std::string, std::string&);
	};

	struct JsPatcher : public Patcher {
		ModRegistry::Mod* source;
		JSUtils::JsValue jsCallback;

		JsPatcher(std::string, std::string, ModRegistry::Mod* source, JSUtils::JsValue);
		~JsPatcher();
		bool DoPatchwork(std::string, std::string, std::string&) override;
	};

	jsfunction(registerPatcher);
	size_t RegisterPatcher(Patcher* patcher);
	void DestroyPatcher(size_t idx);
	void PatchData(std::string, std::string, std::string&);
};