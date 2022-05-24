#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <ziputils.h>

struct archive {};
namespace ModFS {
	struct ModMeta {
		std::vector<std::string> authors;
		std::string description;
		std::string modid;
		std::string name;
		std::vector<std::string> scripts;
		std::string version;
	};
	struct Mod {
		bool isArchive = false;
		ZipUtils::ArchiveWrapper* innerArchive = nullptr;
		std::filesystem::path innerPath = "";
		ModMeta meta;
		Mod() {};
		Mod(std::filesystem::path);
		std::string ReadEntry(std::string);
	};
	Mod OpenArchive(std::filesystem::path);
	std::vector<Mod> LoadAllMods(std::filesystem::path);
};