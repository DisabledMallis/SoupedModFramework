#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include <JavaScriptCore/JavaScript.h>
#include <fmt/core.h>
#include <shared_thread.h>
#include <logger.h>

namespace WebUI {
	using namespace ultralight;

	struct WebUIViewListener : public ViewListener {
		void OnAddConsoleMessage(View* caller,
								MessageSource source,
								MessageLevel level,
								const String& message,
								uint32_t line_number,
								uint32_t column_number,
								const String& source_id) override;
	};

	struct WebUILoadListener : public LoadListener {
		void OnBeginLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url) override {
			::Logger::Print<::Logger::WARNING>("Began loading view. FrameID: {}, is_main_frame: {}, url: {}", frame_id, is_main_frame, std::string(url.utf8().data()));
		};
		void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url) override {
			::Logger::Print<::Logger::WARNING>("Finished loading view. FrameID: {}, is_main_frame: {}, url: {}", frame_id, is_main_frame, std::string(url.utf8().data()));
		};
		void OnFailLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url, const String& description, const String& error_domain, int error_code) override {
			::Logger::Print<::Logger::WARNING>("Failed loading view. FrameID: {}, is_main_frame: {}, url: {}, description: {}, error_domain: {}, error_code: {}", frame_id, is_main_frame, std::string(url.utf8().data()), std::string(description.utf8().data()), std::string(error_domain.utf8().data()), error_code);
		};
		void OnWindowObjectReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url) override {
			::Logger::Print<::Logger::WARNING>("Window object ready. FrameID: {}, is_main_frame: {}, url: {}", frame_id, is_main_frame, std::string(url.utf8().data()));
		};
		void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url) override {
			::Logger::Print<::Logger::WARNING>("DOM ready. FrameID: {}, is_main_frame: {}, url: {}", frame_id, is_main_frame, std::string(url.utf8().data()));
		};
	};

	void Init();
	void InitPlatform();
	bool IsLoaded();
	RefPtr<JSContext> AcquireJSContext();
	void CreateRenderer();
	void CreateView(std::string file);
	JSObjectRef GetAPIObject();
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
	shared_thread& GetThread();

	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};