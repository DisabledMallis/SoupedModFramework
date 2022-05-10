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
	view->LoadHTML("<body style=\"background-color: #00000000;\"><h1 style=\"color: white;\">Hello World!</h1></body>");

	///
	/// Notify the View it has input focus (updates appearance).
	///
	view->Focus();
}

void WebUI::SetSize(int w, int h)
{
	view->Resize(w, h);
}

void WebUI::CopyBitmapToTexture(RefPtr<Bitmap> bitmap)
{
	if (renderTarget == 0) {
		::Logger::Print("Netx tex id");
		renderTarget = gpuDriver->NextTextureId();
		::Logger::Print("Create new texture");
		gpuDriver->CreateTexture(renderTarget, bitmap);
	}
	::Logger::Print("Update tex");
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
	///
	/// Render all active Views (this updates the Surface for each View).
	///
	renderer->Render();

	::Logger::Print("Get render surface");
	BitmapSurface* surface = (BitmapSurface*)(view->surface());

	///
	/// Check if our Surface is dirty (pixels have changed).
	///
	::Logger::Print("Dirty check");
	if (!surface->dirty_bounds().IsEmpty()) {
		///
		/// Psuedo-code to upload Surface's bitmap to GPU texture.
		///
		::Logger::Print("Copy bitmap");
		CopyBitmapToTexture(surface->bitmap());

		///
		/// Clear the dirty bounds.
		///
		::Logger::Print("Clear dirty");
		surface->ClearDirtyBounds();
	}

	if (renderTarget) {
		::Logger::Print("Drawing texture...");
		DrawTexture(renderTarget, 0, 0, view->width(), view->height(), 0);
		::Logger::Print("Texture drawn");
	}
}
	

void WebUI::DrawTexture(uint32_t texId, float x, float y, float w, float h, float angle)
{
	ImGui::SetNextWindowPos(ImVec2(x, y));
	ImGui::SetNextWindowSize(ImVec2(w, h));
	ImGui::Begin("##webview", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
	
	uint32_t nativeTexture = gpuDriver->GetNativeTextureID(texId);
	ImGui::Image((void*)nativeTexture, ImGui::GetWindowSize());
	ImGui::End();

	int gle = glGetError();
	if (gle != GL_NO_ERROR) {
		using namespace Logger;
		::Logger::Print<::Logger::WARNING>("GL Error: {}", gle);
	}
}
