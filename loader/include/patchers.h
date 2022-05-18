#pragma once

#include <string>
#include <functional>
#include <jsutils.h>

namespace Patchers {

	struct Patcher {
		std::string selector;

		Patcher(std::string selector) {
			this->selector = selector;
		}

		virtual bool DoPatchwork(std::string, std::string&);
	};

	struct JsPatcher : public Patcher {
		JSUtils::JsValue jsCallback;

		JsPatcher(std::string selector, JSUtils::JsValue jsCallback);
		~JsPatcher();
		bool DoPatchwork(std::string, std::string&) override;
	};

	jsfunction(registerPatcher);
	size_t RegisterPatcher(Patcher* patcher);
	void DestroyPatcher(size_t idx);
	void PatchData(std::string filename, std::string& data);
};