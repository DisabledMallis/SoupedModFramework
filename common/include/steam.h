#pragma once

#include <filesystem>

namespace Steam {
	std::filesystem::path GetSteamDir();
	std::filesystem::path GetUserdataDir();
	std::string GetUserID();
};