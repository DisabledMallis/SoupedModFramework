#pragma once

#include <string>
#include <functional>

namespace Patchers {
	size_t RegisterPatcher(std::function<bool(std::string&)> patcher);
	void DestroyPatcher(size_t idx);
	void PatchData(std::string& data);
};