#pragma once

#include <stdint.h>
#include <filesystem>
#include <vector>

class JPNG {
	std::vector<uint8_t> jpegData;
	std::vector<uint8_t> pngData;
public:
	JPNG(std::vector<uint8_t> imageData);
	std::vector<uint8_t> ToPNG();
};