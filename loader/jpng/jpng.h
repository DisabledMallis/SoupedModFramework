#pragma once

#include <stdint.h>
#include <filesystem>
#include <vector>

class JPNG {
	std::vector<uint8_t> jpegData;
	std::vector<uint8_t> pngData;
public:
	static JPNG FromJPNG(std::vector<uint8_t> imageData);
	static JPNG FromPNG(std::vector<uint8_t> imageData);
	std::vector<uint8_t> ToJPNG();
	std::vector<uint8_t> ToPNG();
};