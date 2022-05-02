#pragma once

#include <string>
#include <Windows.h>

namespace Registry {
	void ReadValue(HKEY base_key, std::string path, std::string key, char* value_buffer, bool is_text);
	std::string ReadString(HKEY base_key, std::string path, std::string key);
	int ReadDWORD(HKEY base_key, std::string path, std::string key);

	void WriteValue(HKEY base_key, std::string path, std::string key, char* value_buffer, bool is_text);
	void WriteString(HKEY base_key, std::string path, std::string key, std::string value);
	void WriteDWORD(HKEY base_key, std::string path, std::string key, int value);
};