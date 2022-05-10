#include "webui.h"
#include "gl/GPUDriverGL.h"
#include <logger.h>
#include <imgui.h>

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

static RefPtr<Renderer> renderer;
int vx = 0;
int vy = 0;
int vw = 640;
int vh = 480;
static RefPtr<View> view;
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

void WebUI::CreateView()
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

	///
	/// Load a raw string of HTML.
	///
	view->LoadURL("file:///assets/souped.html");

	///
	/// Notify the View it has input focus (updates appearance).
	///
	view->Focus();
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

	///
	/// Render all active Views (this updates the Surface for each View).
	///
	renderer->Render();

	BitmapSurface* surface = (BitmapSurface*)(view->surface());

	///
	/// Check if our Surface is dirty (pixels have changed).
	///
	if (!surface->dirty_bounds().IsEmpty()) {
		///
		/// Psuedo-code to upload Surface's bitmap to GPU texture.
		///
		CopyBitmapToTexture(surface->bitmap());

		///
		/// Clear the dirty bounds.
		///
		surface->ClearDirtyBounds();
	}

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
