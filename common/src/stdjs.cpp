#include <stdjs.h>

using namespace StdJs;

jsfunction(StdJs::info) {
	return print<Logger::DEFAULT>(callee, isConstructCall, jsargv, jsargc, callbackState);
}
jsfunction(StdJs::log) {
	return print<Logger::DEFAULT>(callee, isConstructCall, jsargv, jsargc, callbackState);
}
jsfunction(StdJs::warn) {
	return print<Logger::WARNING>(callee, isConstructCall, jsargv, jsargc, callbackState);
}
jsfunction(StdJs::error) {
	return print<Logger::FAILURE>(callee, isConstructCall, jsargv, jsargc, callbackState);
}