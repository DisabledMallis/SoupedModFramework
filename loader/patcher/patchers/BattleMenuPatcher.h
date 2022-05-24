#pragma once

#include "../patchers.h"

struct BattleMenuPatcher : public Patchers::Patcher {
	BattleMenuPatcher();

	bool DoPatchwork(std::string, std::string, std::string&) override;
};