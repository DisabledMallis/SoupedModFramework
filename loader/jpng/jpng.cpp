#include "jpng.h"

#include <Windows.h>
#include <Unknwn.h>
#include <objidl.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <gdiplusimaging.h>
#include <logger.h>
#include <Shlwapi.h>

JPNG::JPNG(std::vector<uint8_t> imageData)
{
	uint8_t* decryptBuffer = imageData.data();

	uint8_t* pImgData = decryptBuffer + 0x8;
	uint64_t imgDataSize = imageData.size();
	uint32_t* pSizeInfo = (uint32_t*)((pImgData + (imgDataSize * 1)) - 0x18);
	uint32_t pngOff = *pSizeInfo;

	uint8_t* pPng = pngOff + pImgData;
	uint64_t pngDataSize = (imgDataSize - pngOff) - 0x18;
	this->pngData = std::vector<uint8_t>(pPng, pPng+pngDataSize);

	uint8_t* pJfif = pImgData;
	uint64_t jfifSize = pngOff;
	this->jpegData = std::vector<uint8_t>(pJfif, pJfif+jfifSize);
}

#pragma pack(push, 1)
struct COLOR4 {
	char a;
	char b;
	char g;
	char r;
};
#pragma pack(pop)
static_assert(sizeof(COLOR4) == 4, "COLOR4 is not the correct size");
static_assert(offsetof(COLOR4, a) == 0, "COLOR4::a is at the wrong location");
static_assert(offsetof(COLOR4, b) == 1, "COLOR4::b is at the wrong location");
static_assert(offsetof(COLOR4, g) == 2, "COLOR4::g is at the wrong location");
static_assert(offsetof(COLOR4, r) == 3, "COLOR4::r is at the wrong location");

Gdiplus::Bitmap* GdiplusImageToBitmap(Gdiplus::Image* img, Gdiplus::Color bkgd = Gdiplus::Color::Transparent)
{
	Gdiplus::Bitmap* bmp = nullptr;
	try {
		int wd = img->GetWidth();
		int hgt = img->GetHeight();
		auto format = img->GetPixelFormat();
		bmp = new Gdiplus::Bitmap(wd, hgt, format);
		auto g = std::unique_ptr<Gdiplus::Graphics>(Gdiplus::Graphics::FromImage(bmp));
		g->Clear(bkgd);
		g->DrawImage(img, 0, 0, wd, hgt);
	}
	catch (std::exception& ex) {
		Logger::Debug("Error converting image to bitmap: {}", std::string(ex.what()));
	}
	if (bmp) {
		if (bmp->GetLastStatus() != Gdiplus::Status::Ok) {
			Logger::Debug("Image::GetLastStatus was NOT ok: {}", std::to_string(bmp->GetLastStatus()));
		}
	}
	return bmp;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

std::vector<uint8_t> JPNG::ToPNG() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	IStream* jfifStream = SHCreateMemStream(this->jpegData.data(), this->jpegData.size());
	IStream* pngStream = SHCreateMemStream(this->pngData.data(), this->pngData.size());
	Gdiplus::Image* jfifFromStream = Gdiplus::Image::FromStream(jfifStream);
	if (!jfifFromStream) {
		Logger::Print<Logger::FAILURE>("Failed to read jfif from stream");
		return std::vector<uint8_t>();
	}
	Gdiplus::Image* pngFromStream = Gdiplus::Image::FromStream(pngStream);
	if (!pngFromStream) {
		Logger::Print<Logger::FAILURE>("Failed to read png from stream");
		return std::vector<uint8_t>();
	}
	Gdiplus::Bitmap* jfifImg = GdiplusImageToBitmap(jfifFromStream);
	if (!jfifImg) {
		Logger::Print<Logger::FAILURE>("Failed to convert jfif to bitmap");
		return std::vector<uint8_t>();
	}
	Gdiplus::Bitmap* pngImg = GdiplusImageToBitmap(pngFromStream);
	if (!pngImg) {
		Logger::Print<Logger::FAILURE>("Failed to convert png to bitmap");
		return std::vector<uint8_t>();
	}

	uint64_t pngWidth = pngImg->GetWidth();
	uint64_t pngHeight = pngImg->GetHeight();

	uint64_t jfifWidth = jfifImg->GetWidth();
	uint64_t jfifHeight = jfifImg->GetHeight();

	if (pngWidth != jfifWidth || pngHeight != jfifHeight) {
		Logger::Print<Logger::FAILURE>("Image data size mismatch! pngSize and jfifSize are expected to match!");
		return std::vector<uint8_t>();
	}

	Gdiplus::Bitmap* resultImg = new Gdiplus::Bitmap(pngWidth, pngHeight, PixelFormat32bppARGB);
	for (uint64_t x = 0; x < pngWidth; x++) {
		for (uint64_t y = 0; y < pngHeight; y++) {
			Gdiplus::Color cOpacity;
			pngImg->GetPixel(x, y, &cOpacity);
			if (cOpacity.GetR() != cOpacity.GetG() && cOpacity.GetG() != cOpacity.GetB() && cOpacity.GetB() != cOpacity.GetR()) {
				//These values are expected to match
				Logger::Print<Logger::FAILURE>("Image's opacity layer has an invalid color!");
				return std::vector<uint8_t>();
			}
			uint8_t opacity = cOpacity.GetR();

			Gdiplus::Color color;
			jfifImg->GetPixel(x, y, &color);
			Gdiplus::ARGB col_argb = Gdiplus::Color::MakeARGB(opacity, color.GetR(), color.GetG(), color.GetB());
			color.SetValue(col_argb);
			resultImg->SetPixel(x, y, color);
		}
	}

	// Get the CLSID of the PNG encoder.
	CLSID encoderClsid;
	GetEncoderClsid(L"image/png", &encoderClsid);

	IStream* resultStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &resultStream);
	if (!SUCCEEDED(hr)) {
		Logger::Print<Logger::FAILURE>("Failed to create result stream for image");
		return std::vector<uint8_t>();
	}
	resultImg->Save(resultStream, &encoderClsid, NULL);

	jfifStream->Release();
	pngStream->Release();

	std::vector<uint8_t> resultBuffer;
	ULARGE_INTEGER liSize;
	IStream_Size(resultStream, &liSize);
	DWORD len = liSize.LowPart;
	IStream_Reset(resultStream);
	resultBuffer.resize(len);
	IStream_Read(resultStream, resultBuffer.data(), len);
	resultStream->Release();

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return resultBuffer;
};