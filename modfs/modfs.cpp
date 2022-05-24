#include <modfs.h>
#include <nlohmann/json.hpp>
#include <logger.h>
#include <fstream>

ModFS::Mod::Mod(std::filesystem::path pathOnDisk) {
	if (std::filesystem::is_directory(pathOnDisk)) {
		this->isArchive = false;
		this->innerPath = pathOnDisk;
	}
	else {
		this->isArchive = true;
		this->innerArchive = ZipUtils::ArchiveWrapper::OpenArchive(pathOnDisk);
	}
	this->meta = ModFS::ModMeta();
	std::string modMeta = this->ReadEntry("meta.json");
	nlohmann::json metaJson = nlohmann::json::parse(modMeta);
	if (metaJson.contains("authors")) {
		this->meta.authors = metaJson["authors"];
	}
	if (metaJson.contains("description")) {
		this->meta.description = metaJson["description"];
	}
	if (metaJson.contains("name")) {
		this->meta.name = metaJson["name"];
	}
	else {
		Logger::Print<Logger::FAILURE>("Mod {} is missing the field '{}' in the meta.json", pathOnDisk.filename().string(), "name");
	}
	if (metaJson.contains("modid")) {
		this->meta.name = metaJson["modid"];
	}
	else {
		Logger::Print<Logger::FAILURE>("Mod {} is missing the field '{}' in the meta.json", pathOnDisk.filename().string(), "modid");
	}
	if (metaJson.contains("scripts")) {
		this->meta.scripts = metaJson["scripts"];
	}
	if (metaJson.contains("version")) {
		this->meta.version = metaJson["version"];
	}
	else {
		Logger::Print<Logger::FAILURE>("Mod {} is missing the field '{}' in the meta.json", pathOnDisk.filename().string(), "version");
	}
}

std::string ModFS::Mod::ReadEntry(std::string entry) {
	if (this->isArchive) {
		return this->innerArchive->ReadEntry(entry);
	}
	else {
		std::ifstream stream(this->innerPath / entry);
		std::string entryStr((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		return entryStr;
	}
}

ModFS::Mod ModFS::OpenArchive(std::filesystem::path pathOnDisk) {
	return ModFS::Mod(pathOnDisk);
}

std::vector<ModFS::Mod> ModFS::LoadAllMods(std::filesystem::path cd)
{
	std::vector<std::filesystem::path> modPaths;
	std::filesystem::path modsDir = cd / "mods";
	if (!std::filesystem::exists(modsDir)) {
		Logger::Print<Logger::FAILURE>("No 'mods' directory to explore");
		return std::vector<ModFS::Mod>();
	}
	for (auto& mod : std::filesystem::directory_iterator(modsDir)) {
		if (!mod.is_directory()) {
			if (mod.path().extension() == ".smf") {
				modPaths.push_back(mod);
			}
		}
		if (mod.is_directory()) {
			std::filesystem::path metaFile = mod.path() / "meta.json";
			if (std::filesystem::exists(metaFile)) {
				modPaths.push_back(mod);
			}
		}
	}

	std::vector<ModFS::Mod> result;
	for (auto& modPath : modPaths) {
		try {
			result.push_back(ModFS::OpenArchive(modPath));
		}
		catch (std::exception& ex) {
			Logger::Print<Logger::FAILURE>("Failed to load mod {}: {}", modPath.filename().string(), std::string(ex.what()));
		}
	}
	return result;
}
