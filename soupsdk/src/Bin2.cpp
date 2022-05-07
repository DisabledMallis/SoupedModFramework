/*
* The proprietary '%BIN2.0' encryption format was reverse engineered by Woob
* Discord: Woob#3637
* GitHub: https://github.com/Vadmeme
* 
* This file is a reimplementation of his work, with his permission
*/
#include <Bin2.h>
#include <fstream>

void Soup::Bin2::DeriveKey(Soup::Bin2::Key* output, uint32_t file_size) {
	uint8_t initial_values_u8[] = { 0x7B, 0x3A, 0x25, 0x36, 0x23, 0x6A, 0x7E, 0x43, 0x68, 0x61, 0x69, 0x3E, 0x60, 0x67, 0x4E, 0x2E };
	auto initial_values = reinterpret_cast<Key*>(initial_values_u8);

	uint64_t generator = (initial_values->u32[0] ^ file_size) % 0x7FFFFFFF;
	if (generator == 0) {
		generator = 1;
	}

	for (int i = 0; i < 4; i++) {
		generator = (static_cast<uint64_t>(0xBC8F) * generator) % 0x7FFFFFFF;
		output->u32[i] = generator;
		generator = (static_cast<uint64_t>(0xBC8F) * generator) % 0x7FFFFFFF;
	}

	output->u64[0] ^= initial_values->u64[0];
	output->u64[1] ^= initial_values->u64[1];
}

void Soup::Bin2::ExecuteCrypto(Soup::Bin2::Key* key, uint8_t* data, uint32_t data_size) {
	auto buffer = data;
	auto buffer_skip_8 = data + 8;
	auto key_size = sizeof(*key);

	for (int index = 0; index < data_size; index++) {
		*buffer++ = (2 * index) ^ key->u8[static_cast<uint8_t>(key_size - index % key_size - 1)] ^ *buffer_skip_8++;
	}
}