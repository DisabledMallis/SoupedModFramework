#include "ui.h"

#include <mutex>
#include <deque>
#include <imgui.h>
#include <fmt/core.h>

#define NOTIF_WIDTH 350
#define NOTIF_HEIGHT 150
#define ANIM_SPEED 300;

struct Notif {
	bool entering = false;
	float animInX = NOTIF_WIDTH;
	bool entered = false;
	bool exiting = false;
	float timeLeft = 5;
	std::string title = "No title";
	std::string text = "No text";

	Notif(std::string title, std::string text, bool entering = true, float timeOnScreen = 5, bool entered = false) {
		this->entering = entering;
		this->entered = entered;
		this->timeLeft = timeOnScreen;
		this->title = title;
		this->text = text;
	}
};

std::deque<Notif> notifs;
std::mutex notifMutex;

void UI::Notify(std::string title, std::string text)
{
	notifMutex.lock();
	notifs.emplace_back(title, text);
	notifMutex.unlock();
}

float animOutY = 0;

float easeInExpo(float x) {
	return x == 0 ? 0 : pow(2, 10 * x - 10);
}
float easeOutExpo(float x) {
	return x == 1 ? 1 : 1 - pow(2, -10 * x);
}

void UI::Render()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushFont(io.Fonts->Fonts[0]);
	notifMutex.lock();
	for (int i = notifs.size()-1; i >= 0; i--)
	{
		if (notifs[i].entering)
			notifs[i].animInX -= io.DeltaTime * ANIM_SPEED;
		if (notifs[i].animInX <= 0) {
			notifs[i].entering = false;
			notifs[i].entered = true;
			notifs[i].animInX = 0;
		}
		if (notifs[i].exiting)
			animOutY += io.DeltaTime * ANIM_SPEED;
		if (notifs[i].entered)
			notifs[i].timeLeft -= io.DeltaTime;
		ImGui::SetNextWindowSize(ImVec2(NOTIF_WIDTH, NOTIF_HEIGHT));
		ImGui::SetNextWindowPos(ImVec2(((io.DisplaySize.x-5) - NOTIF_WIDTH) + (easeInExpo(notifs[i].animInX/NOTIF_WIDTH) * NOTIF_WIDTH), (io.DisplaySize.y-10) - (NOTIF_HEIGHT + (i * (NOTIF_HEIGHT+5)) - ((easeOutExpo(animOutY/NOTIF_HEIGHT)*NOTIF_HEIGHT))-5)));
		std::string winTitle = fmt::format("Notif###{}", i);
		ImGui::Begin(winTitle.c_str(), 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		ImGui::PushFont(io.Fonts->Fonts[1]);
		ImGui::Text(notifs[i].title.c_str());
		ImGui::PopFont();
		ImGui::TextWrapped(notifs[i].text.c_str());
		ImGui::End();
		if (notifs[i].timeLeft <= 0) {
			notifs[i].exiting = true;
		}
		if (animOutY >= NOTIF_HEIGHT) {
			notifs.pop_front();
			animOutY = 0;
		}
	}
	notifMutex.unlock();
	ImGui::PopFont();
}

jsfunction(UI::JsNotify) {
	if (jsargc == 3) {
		JSUtils::JsValue jsTitle = jsargv[1];
		JSUtils::JsValue jsText = jsargv[2];
		UI::Notify(jsTitle, jsText);
	}
	return JS_INVALID_REFERENCE;
}