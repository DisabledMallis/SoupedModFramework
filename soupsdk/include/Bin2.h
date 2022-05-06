#pragma once

#include <stdint.h>
#include <string>
#include <filesystem>

namespace Soup {
	namespace Bin2 {
		struct Key {
			union {
				uint64_t u64[2];
				uint32_t u32[4];
				uint8_t u8[16];
			};
		};

		void DeriveKey(Key* output, uint32_t file_size);
		void ExecuteCrypto(Key* key, uint8_t* data, uint32_t data_size);
		bool ReadFile(const std::string& file_path, std::vector<uint8_t>* out_buffer);
		bool WriteFile(const std::string& desired_file_path, uint8_t* address, size_t size);
		void DecryptFile(std::filesystem::path p, bool show_failures = false);
	};
};