#pragma once

#include <ChakraCore.h>
#include <string>
#include <functional>

#define jsfunction(funcName) JsValueRef CALLBACK funcName(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, short argumentCount, void *callbackState)
#define jsargc argumentCount
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
	JsValue GetGlobalObject();

	class JsValue {
		JsValueRef internalRef;
	public:
		JsValue();
		JsValue(int);
		JsValue(JsValueRef valRef);
		JsValue(std::string, JsValue parentObj = GetGlobalObject());
		JsValue(std::string name, JsNativeFunction func, JsValue parentObj = GetGlobalObject());
		JsValueRef* GetInternalRef();
		virtual bool IsValid();
		operator bool();
		operator int();
		operator double();
		operator JsValueRef();
		void operator=(bool);
		void operator=(int);
		void operator=(double);
		void operator=(JsValueRef);
		void operator=(std::string);
		std::string cpp_str();
		operator std::string();
		JsValue operator[](const char*);
		JsValue operator[](std::string);
		template<typename... T>
		JsValue operator()(T... argv) {
			constexpr size_t argc = sizeof...(argv);
			JsValueRef args[argc];
			int i = 0;
			([&](auto& arg)
				{
					args[i] = JsValue(arg).GetInternalRef();
					i++;
				} (argv), ...);
			JsValue resultVal;
			JsErrorCode jsErrC = JsCallFunction(this->GetInternalRef(), args, argc, resultVal.GetInternalRef());
			//TODO: Error handling
			return resultVal;
		}
	};

	JsValue RunScript(std::string name, std::string code);
};