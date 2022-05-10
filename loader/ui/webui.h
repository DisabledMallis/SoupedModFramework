#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>

namespace WebUI {
	using namespace ultralight;

	void Init();
	void InitPlatform();
	void CreateRenderer();
	void CreateView();
	void SetSize(int w, int h);
	void CopyBitmapToTexture(RefPtr<Bitmap> bitmap);
	void UpdateLogic();
	void RenderOneFrame();
	void DrawTexture(uint32_t texId, float x, float y, float w, float h, float angle);
};