#pragma once

#include <string>
#include <functional>
#include <jsutils.h>

namespace Patchers {
	jsfunction(registerPatcher);
	size_t RegisterPatcher(std::function<bool(std::string, std::string&)> patcher);
	void DestroyPatcher(size_t idx);
	void PatchData(std::string filename, std::string& data);
};