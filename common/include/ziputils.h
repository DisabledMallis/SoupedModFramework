#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ZipUtils {
	class ArchiveWrapper {
		std::filesystem::path pathOnDisk;
		ArchiveWrapper(std::filesystem::path pArchive);
	public:
		~ArchiveWrapper();
		static ArchiveWrapper* OpenArchive(std::filesystem::path);
		std::string ReadEntry(std::string path);
	};
};