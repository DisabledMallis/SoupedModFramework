#pragma once
#include <jsutils.h>
#include <logger.h>

namespace StdJs {
	template<Logger::LogLevel level>
	jsfunction(print) {
		if (jsargc == 1) {
			JSUtils::JsValue consoleObj = jsargv[0];
			JSUtils::JsValue message = jsargv[1];
			Logger::Print<level>("{}", message.cpp_str());
		}
		else {
			Logger::Print <Logger::FAILURE>("console out func called with incorrect number of arguments");
		}
		return JS_INVALID_REFERENCE;
	}
	jsfunction(info);
	jsfunction(log);
	jsfunction(warn);
	jsfunction(error);
};