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

	struct JSPatcher : public Patcher {
		JSObjectRef jsCallback;

		JSPatcher(std::string selector, JSObjectRef jsCallback);
		~JSPatcher();
		bool DoPatchwork(std::string, std::string&) override;
	};

	jsfunction(registerPatcher);
	size_t RegisterPatcher(Patcher* patcher);
	void DestroyPatcher(size_t idx);
	void PatchData(std::string filename, std::string& data);
};