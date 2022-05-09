#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>

namespace WebUI {
	using namespace ultralight;

	void Init();
	void InitPlatform();
	void CreateRenderer();
	void CreateView();
	void CopyBitmapToTexture(RefPtr<Bitmap> bitmap);
	void UpdateLogic();
	void RenderOneFrame();
};