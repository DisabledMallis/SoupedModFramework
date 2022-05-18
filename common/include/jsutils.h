#pragma once

#include <ChakraCore.h>
#include <string>
#include <functional>
#include <filesystem>
#include <map>

#define jsfunction(funcName) JsValueRef CALLBACK funcName(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, short argumentCount, void *callbackState)
#define jsargc (argumentCount-1)
#define jsargv arguments

namespace JSUtils {
	/* Runtime functions */
	void SetupRuntime();
	void OnInitialize(std::function<void()>);
	void DestroyRuntime();

	/* Context functions */
	JsContextRef MakeContext();
	JsContextRef GetDefaultContext();
	JsContextRef GetCurrentContext();
	void SetCurrentContext(JsContextRef ctx);

	/*Interop*/
	class JsValue;
	JsValue& GetGlobalObject();

	class JsValue {
	public:
		JsValueRef internalRef;
		JsValue();
		JsValue(int);
		JsValue(JsValueRef valRef);
		JsValue(std::string text);
		JsValue(std::string objName, bool isObject);
		JsValue(JsNativeFunction);
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
			JsValueRef args[argc];
			int i = 0;
			([&](auto& arg)
				{
					args[i] = JsValue(arg).internalRef;
					i++;
				} (argv), ...);
			JsValue resultVal;
			JsErrorCode jsErrC = JsCallFunction(this->internalRef, args, argc, &resultVal.internalRef);
			//TODO: Error handling
			return resultVal;
		}
	};

	JsValue RunCode(std::string name, std::string code);
	JsValue RunFile(std::filesystem::path file);
};