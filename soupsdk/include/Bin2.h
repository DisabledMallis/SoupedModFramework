#pragma once

#include <stdint.h>
#include <string>
#include <filesystem>
#include <memory.h>
#include <logger.h>

namespace Soup {
	namespace Signatures {
		static constexpr const char* SIG_BIN2_DECRYPTBYTES = "?? 89 ?? ?? ?? ?? 89 ?? ?? ?? ?? 89 ?? ?? ?? 55 41 ?? 41 ?? 48 8B ?? 48 83 ?? ?? 48 8B ?? ?? ?? ?? ?? 48 33 ?? ?? 89 ?? ?? 48 8B ?? 4C";
		static constexpr const char* SIG_BIN2_GENERATECHECKSUM = "48 89 ?? ?? ?? 48 89 ?? ?? ?? 57 48 83 EC ?? 48 8B ?? ?? 48 8B ?? 48 8B ?? 48 8B ?? 48 8B ?? 48 2B ?? 48 3B ?? 73 ?? 48 8D ?? ?? EB ?? 76 ?? 48 8B ?? ?? 48 2B ?? 48 3B ?? 76 ?? 4C 8D ?? ?? ?? 48 8B ?? 48 8B ?? E8 ?? ?? ?? ?? 48 8B ?? ?? ?? 48 8B ?? ?? ?? 48 83 C4 ?? 5F C3 48 2B ?? 33 D2 4C 8B ?? 48 8B ?? E8 ?? ?? ?? ?? 48 8D ?? ?? 48 89 ?? ?? 48 8B ?? ?? ?? 48 8B ?? ?? ?? 48 83 C4 ?? 5F C3 CC CC";
		static constexpr const char* SIG_BIN2_RESIZEBUFFER = "48 89 5c 24 18 48 89 74 24 20 57 41 56 41 57 48 83 ec 40 4c 8b f2 48 8b f1 48 89 4c 24 30 48 ba ff ff ff ff ff ff ff 7f 4c 3b f2 0f 87 03 01 00 00 4c 8b 79 08 4c 2b 39 48 8b 49 10 48 2b 0e 48 8b c1 48 d1 e8 48 2b d0 48 3b ca 76 05 49 8b fe eb 0b 48 8d 3c 08 49 3b fe 49 0f 42 fe 48 89 7c";
	};
	namespace Bin2 {

		enum DecryptionSource {
			INTERNAL, //Internal game's bin2 implementation
			EXTERNAL //External custom made bin2 implementation
		};

		struct Key {
			union {
				uint64_t u64[2];
				uint32_t u32[4];
				uint8_t u8[16];
			};
		};

		void DeriveKey(Key* output, uint32_t file_size);
		void ExecuteCrypto(Key* key, uint8_t* data, uint32_t data_size);
		template<DecryptionSource source, size_t _size>
		void DecryptBytes(uint8_t(&data)[_size]) {
			DecryptBytes<source>(data, _size);
		};
		template<DecryptionSource source>
		void DecryptBytes(uint8_t* data, size_t _size) {
			if (source == INTERNAL) {
				struct binInf {
					uint8_t* data;
					uint8_t* end;
				} binData = { 0, 0 };
				binData.data = data;
				binData.end = (uint8_t*)(data + _size);
				static uint64_t pDecryptFunc = Memory::FindSig(Signatures::SIG_BIN2_DECRYPTBYTES);
				if (!pDecryptFunc) {
					Logger::Print<Logger::FAILURE>("Couldn't find the Bin2 decryption function! Try using the re-implementation");
				}
				((void(*)(binInf*))pDecryptFunc)(&binData);
			}
			if (source == EXTERNAL) {
				Key key;
				DeriveKey(&key, _size);
				ExecuteCrypto(&key, data, _size);
			}
		};
	};
};