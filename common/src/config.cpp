#include <config.h>
#include <fstream>

static Config* config;

nlohmann::json Config::ReadConfig() {
	std::ifstream confStream(CONF_PATH);
	std::string confStr((std::istreambuf_iterator<char>(confStream)), std::istreambuf_iterator<char>());
	confStream.close();
	if (confStr.empty()) {
		ResetConfig();
		return ReadConfig();
	}
	return nlohmann::json::parse(confStr, nullptr, true, true);
}
void Config::SaveConfig(nlohmann::json config) {
	std::ofstream confStream(CONF_PATH, std::ios::trunc);
	confStream << config.dump(1, '\t', false);
	confStream.close();
}
Config::Config() {
	this->data = ReadConfig();
}
Config* Config::GetConfig() {
	if (!config) {
		config = new Config();
	}
	return config;
}
void Config::ResetConfig() {
	nlohmann::json newConf = nlohmann::json::object();
	newConf["debug"] = false;
	SaveConfig(newConf);
}

bool Config::ReadBool(std::string name) {
	if (!this) {
		return false;
	}
	if (this->data.is_null()) {
		return false;
	}
	if (this->data.empty()) {
		return false;
	}
	return this->data[name];
}

bool Config::DebugMode() {
	return this->ReadBool("debug");
}

bool Config::DumpAssets() {
	return this->ReadBool("dump");
}

bool Config::UnlockTowers() {
	return this->ReadBool("unlockTowers");
}

bool Config::UnlockUpgrades() {
	return this->ReadBool("unlockUpgrades");
}