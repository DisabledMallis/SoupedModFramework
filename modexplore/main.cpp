#include <modfs.h>
#include <logger.h>

int main(const char* argv, int argc) {
	std::vector<std::filesystem::path> modPaths;

	std::filesystem::path cd = std::filesystem::current_path();
	std::filesystem::path modsDir = cd / "mods";
	if (!std::filesystem::exists(modsDir)) {
		Logger::Print<Logger::FAILURE>("No 'mods' directory to explore");
		return 1;
	}
	for (auto& mod : std::filesystem::directory_iterator(modsDir)) {
		if (!mod.is_directory()) {
			modPaths.push_back(mod);
		}
	}

	for (auto& modPath : modPaths) {
		Logger::Print("Found: {}", modPath.filename().string());
		ModFS::Mod mod = ModFS::OpenArchive(modPath);
		for (auto& author : mod.meta.authors) {
			Logger::Print("Author: {}", author);
		}
		Logger::Print("Description: {}", mod.meta.description);
		Logger::Print("Name: {}", mod.meta.name);
		for (auto& script : mod.meta.scripts) {
			Logger::Print("Script: {}", script);
		}
		Logger::Print("version: {}", mod.meta.version);
	}

	std::cin.get();
	return 0;
}