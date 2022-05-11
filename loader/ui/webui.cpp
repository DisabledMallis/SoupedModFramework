#include "webui.h"
#include "gl/GPUDriverGL.h"
#include <logger.h>
#include <imgui.h>
#include <Windows.h>
#include <jsutils.h>
#include <patchers.h>

using namespace ultralight;

void WebUI::Init()
{
	if (glfwInit() != GLFW_TRUE) {
		MessageBoxA(0, "SoupedModFramework couldn't initialize GLFW, and as a result it must exit", "GLFW Error", MB_OK);
		exit(0);
	}
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	gladLoadGL();

	Config config;

	///
	/// We need to tell config where our resources are so it can 
	/// load our bundled SSL certificates to make HTTPS requests.
	///
	config.resource_path_prefix = "./assets/resources/";

	config.num_renderer_threads = 1;

	///
	/// Pass our configuration to the Platform singleton so that
	/// the library can use it.
	///
	Platform::instance().set_config(config);
}

static GPUContextGL* gpuContext;
static GPUDriverGL* gpuDriver;
static uint32_t renderTarget = 0;

static RefPtr<Renderer> renderer;
int vx = 0;
int vy = 0;
int vw = 640;
int vh = 480;
static RefPtr<View> view;
static JSObjectRef souped = 0;


void WebUI::InitPlatform()
{
	gpuContext = new GPUContextGL(false, false);
	gpuDriver = (GPUDriverGL*)gpuContext->driver();

	///
	/// Use the OS's native font loader
	///
	Platform::instance().set_font_loader(GetPlatformFontLoader());

	///
	/// Use the OS's native file loader, with a base directory of "."
	/// All file:/// URLs will load relative to this base directory.
	///
	Platform::instance().set_file_system(GetPlatformFileSystem("."));

	///
	/// Use the default logger (writes to a log file)
	///
	Platform::instance().set_logger(GetDefaultLogger("ultralight.log"));

	//Set the GPU driver
	Platform::instance().set_gpu_driver(gpuDriver);
}

bool WebUI::IsLoaded()
{
	if (view) {
		if (view->is_loading()) {
			return false;
		}
		return true;
	}
	return false;
}

RefPtr<JSContext> WebUI::AcquireJSContext()
{
	return view->LockJSContext();
}

void WebUI::CreateRenderer()
{
	///
	/// Create our Renderer (call this only once per application).
	/// 
	/// The Renderer singleton maintains the lifetime of the library
	/// and is required before creating any Views.
	///
	/// You should set up the Platform handlers before this.
	///
	renderer = Renderer::Create();
}

void WebUI::CreateView(std::string file)
{
	ViewConfig config;
	config.enable_images = true;
	config.enable_javascript = true;
	config.is_transparent = true;
	config.is_accelerated = false;

	///
	/// Create an HTML view, 500 by 500 pixels large.
	///
	view = renderer->CreateView(500, 500, config, nullptr);

	//Setup JS Api
	RefPtr<JSContext> refCtx = view->LockJSContext();
	JSContextRef ctx = refCtx->ctx();
	JSUtils::SetContext(ctx);
	JSObjectRef global = JSUtils::GetGlobalObject();
	souped = JSUtils::CreateObject("souped", global);
	JSObjectRef registerPatcher = JSUtils::CreateFunction("registerPatcher", Patchers::registerPatcher, souped);

	///
	/// Load a raw string of HTML.
	///
	view->LoadURL(file.c_str());

	///
	/// Notify the View it has input focus (updates appearance).
	///
	view->Focus();

	//Set listener
	view->set_view_listener(new WebUIListener);

}

JSObjectRef WebUI::GetAPIObject()
{
	if (!souped) {
		::Logger::Print<::Logger::WARNING>("souped JS object was needed, but its currently undefined. Passing undefined instead.");
		return (JSObjectRef)JSUtils::GetUndefined();
	}
	return souped;
}

void WebUI::RunJS(std::string code)
{
	if (!view || view->is_loading()) {
		::Logger::Print<::Logger::WARNING>("Script tried to execute while view was not loaded!");
		return;
	}
	String exception = "";
	view->EvaluateScript(code.c_str(), &exception);
	if (!exception.empty()) {
		::Logger::Print<::Logger::FAILURE>("Error whilst evaluating script: {}", std::string(exception.utf8().data()));
	}
}

void WebUI::SetRect(int x, int y, int w, int h)
{
	vx = x;
	vy = y;
	vw = w;
	vh = h;
}

void WebUI::CopyBitmapToTexture(RefPtr<Bitmap> bitmap)
{
	if (renderTarget == 0) {
		renderTarget = gpuDriver->NextTextureId();
		gpuDriver->CreateTexture(renderTarget, bitmap);
	}
	gpuDriver->UpdateTexture(renderTarget, bitmap);
}

void WebUI::UpdateLogic()
{
	///
	/// Give the library a chance to handle any pending tasks and timers.
	///
	///
	renderer->Update();
}

void WebUI::RenderOneFrame()
{
	view->Resize(vw, vh);
	UpdateLogic();

	///
	/// Render all active Views (this updates the Surface for each View).
	///
	renderer->Render();

	BitmapSurface* surface = (BitmapSurface*)(view->surface());

	///
	/// Psuedo-code to upload Surface's bitmap to GPU texture.
	///
	CopyBitmapToTexture(surface->bitmap());

	if (renderTarget) {
		DrawTexture(renderTarget, vx, vy, vw, vh, 0);
	}
}
	

void WebUI::DrawTexture(uint32_t texId, float x, float y, float w, float h, float angle)
{
	ImGui::SetNextWindowPos(ImVec2(x, y));
	ImGui::SetNextWindowSize(ImVec2(w, h));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##webview", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
	
	uint32_t nativeTexture = gpuDriver->GetNativeTextureID(texId);
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image((void*)nativeTexture, ImGui::GetWindowSize());
	ImGui::End();
	ImGui::PopStyleColor();

	int gle = glGetError();
	if (gle != GL_NO_ERROR) {
		using namespace Logger;
		::Logger::Print<::Logger::WARNING>("GL Error: {}", gle);
	}
}

MouseEvent::Button cur_btn;
LRESULT WebUI::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (uMsg) {
	case WM_KEYDOWN:
		view->FireKeyEvent(KeyEvent(KeyEvent::kType_RawKeyDown, (uintptr_t)wParam, (intptr_t)lParam, false));
		break;
	case WM_KEYUP:
		view->FireKeyEvent(KeyEvent(KeyEvent::kType_KeyUp, (uintptr_t)wParam, (intptr_t)lParam, false));
		break;
	case WM_CHAR:
		view->FireKeyEvent(KeyEvent(KeyEvent::kType_Char, (uintptr_t)wParam, (intptr_t)lParam, false));
		break;
	case WM_MOUSEMOVE: {
		view->FireMouseEvent({MouseEvent::kType_MouseMoved, (int)io.MousePos.x, (int)io.MousePos.y, cur_btn });
		break;
	}
	case WM_LBUTTONDOWN:
		WebUI::ShowNotif("Button down");
	case WM_LBUTTONDBLCLK:
		cur_btn = MouseEvent::kButton_Left;
		view->FireMouseEvent({ MouseEvent::kType_MouseDown, (int)io.MousePos.x, (int)io.MousePos.y, cur_btn });
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		cur_btn = MouseEvent::kButton_Left;
		view->FireMouseEvent({ MouseEvent::kType_MouseDown, (int)io.MousePos.x, (int)io.MousePos.y, cur_btn });
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		cur_btn = MouseEvent::kButton_Left;
		view->FireMouseEvent({ MouseEvent::kType_MouseDown, (int)io.MousePos.x, (int)io.MousePos.y, cur_btn });
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		view->FireMouseEvent({ MouseEvent::kType_MouseUp, (int)io.MousePos.x, (int)io.MousePos.y, cur_btn });
		cur_btn = MouseEvent::kButton_None;
		break;
	case WM_MOUSEWHEEL:
		view->FireScrollEvent({ ScrollEvent::kType_ScrollByPixel, 0, static_cast<int>(io.MouseWheel * 0.8) });
		break;
	}
	WebUI::UpdateLogic();
	return TRUE;
}

inline const char* Stringify(MessageSource source) {
	switch (source) {
	case kMessageSource_XML: return "XML";
	case kMessageSource_JS: return "JS";
	case kMessageSource_Network: return "Network";
	case kMessageSource_ConsoleAPI: return "ConsoleAPI";
	case kMessageSource_Storage: return "Storage";
	case kMessageSource_AppCache: return "AppCache";
	case kMessageSource_Rendering: return "Rendering";
	case kMessageSource_CSS: return "CSS";
	case kMessageSource_Security: return "Security";
	case kMessageSource_ContentBlocker: return "ContentBlocker";
	case kMessageSource_Other: return "Other";
	default: return "";
	}
}

inline const char* Stringify(MessageLevel level) {
	switch (level) {
	case kMessageLevel_Log: return "Log";
	case kMessageLevel_Warning: return "Warning";
	case kMessageLevel_Error: return "Error";
	case kMessageLevel_Debug: return "Debug";
	case kMessageLevel_Info: return "Info";
	default: return "";
	}
}

void WebUI::WebUIListener::OnAddConsoleMessage(View* caller, MessageSource source, MessageLevel level, const String& message, uint32_t line_number, uint32_t column_number, const String& source_id)
{
	using namespace Logger;
	switch (level) {
	case kMessageLevel_Warning:
		Print<WARNING>("[{}] @[{}:{}] {}", std::string(Stringify(level)), line_number, column_number, std::string(message.utf8().data()));
	case kMessageLevel_Error:
		Print<FAILURE>("[{}] @[{}:{}] {}", std::string(Stringify(level)), line_number, column_number, std::string(message.utf8().data()));
	case kMessageLevel_Debug:
	case kMessageLevel_Log:
	case kMessageLevel_Info:
	default:
		Print("[{}] [{}] @[{}:{}] {}", std::string(Stringify(source)), std::string(Stringify(level)), line_number, column_number, std::string(message.utf8().data()));
	}
}
