#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <ZipArchive.h>
#include <ZipArchiveEntry.h>
#include <ZipCpp.h>
#include <ZipFile.h>

namespace ModFS {
	struct ModMeta {
		std::string name;
		std::string description;
		std::string version;
		std::vector<std::string> authors;
		std::vector<std::string> scripts;
	};
	struct Mod {
		ZipArchive::Ptr pArchive;
		ModMeta meta;
		Mod(std::filesystem::path);
		std::string ReadEntry(std::string);
	};
	Mod OpenArchive(std::string);
};