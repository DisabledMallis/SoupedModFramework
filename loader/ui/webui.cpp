#include "webui.h"
#include "gl/GPUDriverGL.h"

using namespace ultralight;

void WebUI::Init()
{
	Config config;

	///
	/// We need to tell config where our resources are so it can 
	/// load our bundled SSL certificates to make HTTPS requests.
	///
	config.resource_path_prefix = "./assets/resources/";

	///
	/// Pass our configuration to the Platform singleton so that
	/// the library can use it.
	///
	Platform::instance().set_config(config);
}

void WebUI::InitPlatform()
{
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
	Platform::instance().set_gpu_driver(new GPUDriverGL(new GPUContextGL(false, false)));
}

static RefPtr<Renderer> renderer;
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

static RefPtr<View> view;
void WebUI::CreateView()
{
	ViewConfig config;
	config.enable_images = true;
	config.enable_javascript = true;
	config.is_transparent = true;
	config.is_accelerated = true;

	///
	/// Create an HTML view, 500 by 500 pixels large.
	///
	view = renderer->CreateView(500, 500, config, nullptr);

	///
	/// Load a raw string of HTML.
	///
	view->LoadHTML("<h1>Hello World!</h1>");

	///
	/// Notify the View it has input focus (updates appearance).
	///
	view->Focus();
}

void WebUI::CopyBitmapToTexture(RefPtr<Bitmap> bitmap)
{
	///
	/// Lock the Bitmap to retrieve the raw pixels.
	/// The format is BGRA, 8-bpp, premultiplied alpha.
	///
	void* pixels = bitmap->LockPixels();

	///
	/// Get the bitmap dimensions.
	///
	uint32_t width = bitmap->width();
	uint32_t height = bitmap->height();
	uint32_t stride = bitmap->row_bytes();

	///
	/// Psuedo-code to upload our pixels to a GPU texture.
	///
	CopyPixelsToTexture(pixels, width, height, stride);

	///
	/// Unlock the Bitmap when we are done.
	///
	bitmap->UnlockPixels();
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

	///
	/// Psuedo-code to loop through all active Views.
	///
	for (auto view : view_list) {
		///
		/// Get the Surface as a BitmapSurface (the default implementation).
		///
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
	}
}
