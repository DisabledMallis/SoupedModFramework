#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <ziputils.h>

struct archive {};
namespace ModFS {
	struct ModMeta {
		std::string name;
		std::string description;
		std::string version;
		std::vector<std::string> authors;
		std::vector<std::string> scripts;
	};
	struct Mod {
		ZipUtils::ArchiveWrapper* innerArchive;
		ModMeta meta;
		Mod(std::filesystem::path);
		std::string ReadEntry(std::string);
	};
	Mod OpenArchive(std::filesystem::path);
};