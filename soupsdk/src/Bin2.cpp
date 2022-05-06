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

bool Soup::Bin2::ReadFile(const std::string& file_path, std::vector<uint8_t>* out_buffer) {
	std::ifstream file_ifstream(file_path, std::ios::binary);

	if (!file_ifstream)
		return false;

	out_buffer->assign((std::istreambuf_iterator<char>(file_ifstream)), std::istreambuf_iterator<char>());
	file_ifstream.close();

	return true;
}

bool Soup::Bin2::WriteFile(const std::string& desired_file_path, uint8_t* address, size_t size) {
	std::ofstream file_ofstream(desired_file_path.c_str(), std::ios_base::out | std::ios_base::binary);

	if (!file_ofstream.write(reinterpret_cast<char*>(address), size)) {
		file_ofstream.close();
		return false;
	}

	file_ofstream.close();
	return true;
}

void Soup::Bin2::DecryptFile(std::filesystem::path p, bool show_failures) {
	// read file
	std::vector<uint8_t> file;
	if (!Soup::Bin2::ReadFile(p.string(), &file)) {
		fprintf(stderr, "err: failed to read file %s\n", p.string().c_str());
		return;
	}

	// check if its a valid file
	char file_header[] = "%BIN_2.0";
	if (file.size() < 8 || memcmp(file_header, file.data(), sizeof(file_header) - 1) != 0) {
		if (show_failures) {
			fprintf(stderr, "err: file is not encrypted\n");
			return;
		}

		return;
	}

	// decrypt
	Key key;
	Soup::Bin2::DeriveKey(&key, file.size());
	Soup::Bin2::ExecuteCrypto(&key, file.data(), file.size());

	// save
	Soup::Bin2::WriteFile(p.string(), file.data(), file.size() - 8);

	// log
	printf("decrypted %s\n", p.string().c_str());
}