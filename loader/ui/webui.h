#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include <fmt/core.h>

namespace WebUI {
	using namespace ultralight;

	struct WebUIListener : public ViewListener {
		void OnAddConsoleMessage(View* caller,
								MessageSource source,
								MessageLevel level,
								const String& message,
								uint32_t line_number,
								uint32_t column_number,
								const String& source_id) override;
	};

	void Init();
	void InitPlatform();
	bool IsLoaded();
	void CreateRenderer();
	void CreateView(std::string file);
	void RunJS(std::string code);
	template<typename... T>
	void ShowNotif(fmt::string_view fomt, T&&... args)
	{
		std::string fmtMsg = fmt::vformat(fomt, fmt::make_format_args(args...));
		std::string code = fmt::format("notif('{}')", fmtMsg);
		RunJS(code);
	}
	void SetRect(int x, int y, int w, int h);
	void CopyBitmapToTexture(RefPtr<Bitmap> bitmap);
	void UpdateLogic();
	void RenderOneFrame();
	void DrawTexture(uint32_t texId, float x, float y, float w, float h, float angle);

	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};