#pragma once

#include <ChakraCore.h>
#include <string>
#include <functional>
#include <filesystem>
#include <map>
#include <vector>

#define jsfunction(funcName) JsValueRef CALLBACK funcName(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, short argumentCount, void *callbackState)
#define jsargc argumentCount
#define jsargv arguments
#define CATCHERROR(error) JSUtils::HandleException(error, __FUNCTION__, __FILE__, __LINE__)

namespace JSUtils {
	/* Runtime functions */
	void SetupRuntime();
	void OnInitialize(std::function<void()>);
	void DestroyRuntime();

	/* Context functions */
	JsContextRef MakeContext();
	JsContextRef GetDefaultContext();
	JsContextRef GetCurrentContext();
	void SetCurrentContext(JsContextRef);

	/*Interop*/
	class JsValue;
	JsValue& GetGlobalObject();
	/*
	* JSUtils::RunCode
	* Returns: The code's result
	* Parameter 1: Script name (file path)
	* Parameter 2: Script code
	*/
	JsValue RunCode(std::string, std::string);
	JsValue RunFile(std::filesystem::path);
	JsErrorCode JsCallSafely(JsValueRef func, JsValueRef* argv, unsigned short argc, JsValueRef* result);
	bool HandleException(JsErrorCode, std::string, std::string, int);
	void ThrowException(std::string);

	class JsValue {
	public:
		JsValueRef internalRef;
		JsValue();
		JsValue(int);
		JsValue(JsValueRef);
		JsValue(std::string);
		JsValue(std::string, bool);
		JsValue(JsNativeFunction);
		~JsValue();
		virtual bool IsValid();
		bool HasProperty(std::string);
		JsValue& GetProperty(std::string);
		void SetProperty(std::string, JsValue&);
		std::string cpp_str();
		operator bool();
		operator int();
		operator double();
		operator JsValueRef();
		operator std::string();
		void operator=(bool);
		void operator=(int);
		void operator=(double);
		void operator=(JsValueRef);
		void operator=(JsNativeFunction);
		void operator=(std::string);
		template<typename... T>
		JsValue operator()(T... argv) {
			constexpr size_t argc = sizeof...(argv);
			JsValueRef args[argc+1];
			args[0] = GetGlobalObject().internalRef;
			int i = 0;
			([&](auto& arg)
				{
					args[i+1] = JsValue(arg).internalRef;
					JsAddRef(args[i + 1], nullptr);
					i++;
				} (argv), ...);
			JsValue resultVal;
			JsErrorCode jsErrC = JSUtils::JsCallSafely(this->internalRef, args, argc+1, &resultVal.internalRef);
			for (JsValueRef arg : args) {
				JsRelease(arg, nullptr);
			}
			JsAddRef(resultVal.internalRef, nullptr);
			return resultVal;
		}
	};
};