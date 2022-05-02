#include "registry.h"

void Registry::ReadValue(HKEY base_key, std::string path, std::string key, char* value_buffer, bool is_text = false) {

    if (value_buffer == nullptr) {
        throw std::exception("value_buffer must be a char[MAX_PATH]!");
        return;
    }

    HKEY hKey;
    LSTATUS status = RegOpenKeyExA(base_key, path.c_str(), 0, KEY_READ, &hKey);
    if (status != ERROR_SUCCESS) {
        throw std::exception("Failed to open registry key!");
        return;
    }

    DWORD dwLen = MAX_PATH;
    DWORD dwType = 0;
    status = RegQueryValueExA(hKey, key.c_str(), nullptr, &dwType, (BYTE*)value_buffer, &dwLen);
    if (status != ERROR_SUCCESS) {
        throw std::exception("Failed to read registry key!");
        return;
    }
    if (is_text && dwLen <= MAX_PATH) {
        value_buffer[dwLen] = 0;
    }

    return;
}

std::string Registry::ReadString(HKEY base_key, std::string path, std::string key) {
    char value_buffer[MAX_PATH];
    ReadValue(base_key, path, key, value_buffer);
    return std::string(value_buffer);
}

int Registry::ReadDWORD(HKEY base_key, std::string path, std::string key) {
    int value_buffer;
    ReadValue(base_key, path, key, (char*)&value_buffer);
    return value_buffer;
}

void Registry::WriteValue(HKEY base_key, std::string path, std::string key, char* value_buffer, bool is_text)
{
    if (value_buffer == nullptr) {
        throw std::exception("value_buffer must be a char[MAX_PATH]!");
        return;
    }

    HKEY hKey;
    LSTATUS status = RegOpenKeyExA(base_key, path.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (status != ERROR_SUCCESS) {
        status = RegCreateKeyA(base_key, path.c_str(), &hKey);
        if (status != ERROR_SUCCESS) {
            throw std::exception("Failed to create a registry key");
            return;
        }
    }

    DWORD dwLen = is_text ? MAX_PATH : sizeof(int);
    DWORD dwType = is_text ? REG_SZ : REG_DWORD;
    status = RegSetValueExA(hKey, key == "(Default)" ? NULL : key.c_str(), 0, dwType, (BYTE*)value_buffer, dwLen);
    if (status != ERROR_SUCCESS) {
        throw std::exception("Failed to write registry key!");
        return;
    }

    return;
}

void Registry::WriteString(HKEY base_key, std::string path, std::string key, std::string value)
{
    return WriteValue(base_key, path, key, (char*)value.c_str(), true);
}

void Registry::WriteDWORD(HKEY base_key, std::string path, std::string key, int value)
{
    return WriteValue(base_key, path, key, (char*)&value, false);
}