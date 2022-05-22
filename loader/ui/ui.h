#pragma once

#include <string>
#include <jsutils.h>

namespace UI {
	void Notify(std::string title, std::string text);
	void Render();
	jsfunction(JsNotify);
};