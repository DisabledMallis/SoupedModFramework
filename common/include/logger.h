#pragma once
#include <cstdarg>
#include <iostream>
#include <fmt/core.h>
#include <fmt/color.h>
#include "config.h"
#ifdef _WIN32
#include <Windows.h>
#endif

#define LOGGER_LBORDER "["
#define LOGGER_RBORDER "] "
#define LOGGER_PREFIX "Souped"
#define LOGGER_PREFIX_SIZE sizeof(LOGGER_LBORDER LOGGER_RBORDER LOGGER_PREFIX)-1
#define LOGGER_PADDING std::string(LOGGER_PREFIX_SIZE, ' ')

namespace Logger {
	enum LogLevel {
#ifdef _WIN32
		DEFAULT = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		SUCCESS = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		WARNING = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		FAILURE = FOREGROUND_INTENSITY | FOREGROUND_RED,
		DEBUG = FOREGROUND_RED | FOREGROUND_GREEN
#else
		DEFAULT = fmt::color::aqua,
		SUCCESS = fmt::color::light_green,
		WARNING = fmt::color::yellow,
		FAILURE = fmt::color::red
#endif
	};

	template <LogLevel level, typename... T>
	void Print(fmt::string_view fomt, T&&... args)
	{
		fmt::print(LOGGER_LBORDER);
#ifdef _WIN32
		HANDLE conHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(conHandle, (WORD)level);
		fmt::print(LOGGER_PREFIX);
		SetConsoleTextAttribute(conHandle, 7);
#else
		fmt::print(fg(level), LOGGER_PREFIX);
#endif
		fmt::print(LOGGER_RBORDER);
		fmt::vprint(fomt, fmt::make_format_args(args...));
		fmt::print("\n");
	}

	template <typename... T>
	void Print(fmt::string_view fomt, T&&... args) {
		Print<LogLevel::DEFAULT>(fomt, args...);
	}

	template<typename... T>
	void Debug(fmt::string_view fomt, T&&... args) {
		if (Config::GetConfig()->DebugMode() == false) {
			return;
		}
		Print<LogLevel::DEBUG>(fomt, args...);
	}
}