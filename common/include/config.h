#pragma once
#include "nlohmann/json.hpp"

#define CONF_PATH "./config.json"

class Config {
	nlohmann::json data;
	Config();
	static nlohmann::json ReadConfig();
	static void SaveConfig(nlohmann::json);
public:
	static Config* GetConfig();
	static void ResetConfig();
	bool ReadBool(std::string);
	bool DebugMode();
	bool DumpAssets();
	bool UnlockTowers();
	bool UnlockUpgrades();
};