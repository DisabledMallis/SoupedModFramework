#pragma once

#include <string>
#include <filesystem>

namespace Dumper {
	void DumpToDisk(std::string fileName, std::filesystem::path bundlePath, std::string content);
};