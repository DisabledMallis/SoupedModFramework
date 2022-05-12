#include "BattleMenuPatcher.h"
#include <nlohmann/json.hpp>
#include <logger.h>

BattleMenuPatcher::BattleMenuPatcher() : Patchers::Patcher("battle_screen.scene") {};

nlohmann::ordered_json null(nullptr);
nlohmann::ordered_json& findChild(nlohmann::ordered_json& parent, std::string key, nlohmann::ordered_json value) {
	using namespace nlohmann;
	if (parent.contains(key)) {
		if (parent[key] == value) {
			return parent;
		}
	}
	if (parent.contains("entity_children")) {
		ordered_json& children = parent["entity_children"];
		for (ordered_json& child : children) {
			ordered_json& offspring = findChild(child, key, value);
			if (!offspring.is_null()) {
				return offspring;
			}
		}
	}
	return null;
}

bool BattleMenuPatcher::DoPatchwork(std::string fileName, std::string& fileData)
{
	using namespace nlohmann;
	try {
		ordered_json file = json::parse(fileData);
		ordered_json& root = file["root"];

		ordered_json& regularLayout = findChild(root, "entity_name", "regular_layout");

		ordered_json& rankedButton = findChild(regularLayout, "entity_name", "ranked_button_root");
		rankedButton = json(nullptr);

		ordered_json& casualButton = findChild(regularLayout, "entity_name", "casual_button_root");
		casualButton = json(nullptr);

		fileData = file.dump();
		Logger::Print<Logger::SUCCESS>("Patched battle screen properly!");
		return true;
	}
	catch (std::exception& ex) {
		Logger::Print<Logger::FAILURE>("Error patching main menu: {}", ex.what());
	}
	return false;
}