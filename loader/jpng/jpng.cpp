#include "jpng.h"

#include <imgutils.h>
#include <logger.h>
#include <memory>
#include <algorithm>
#include <CL/cl.h>

JPNG JPNG::FromJPNG(std::vector<uint8_t> imageData)
{
	JPNG result;
	uint8_t* decryptBuffer = imageData.data();

	uint8_t* pImgData = decryptBuffer + 0x8;
	uint64_t imgDataSize = imageData.size();
	uint32_t* pSizeInfo = (uint32_t*)((pImgData + (imgDataSize * 1)) - 0x18);
	uint32_t pngOff = *pSizeInfo;

	uint8_t* pPng = pngOff + pImgData;
	uint64_t pngDataSize = (imgDataSize - pngOff) - 0x18;
	result.pngData = std::vector<uint8_t>(pPng, pPng+pngDataSize);

	uint8_t* pJfif = pImgData;
	uint64_t jfifSize = pngOff;
	result.jpegData = std::vector<uint8_t>(pJfif, pJfif+jfifSize);

	return result;
}

JPNG JPNG::FromPNG(std::vector<uint8_t> imageData)
{
	JPNG result;



	return result;
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

static ULONG_PTR gdiplusToken;
static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
static bool gdiStarted = false;
void StartGDI() {
	if (!gdiStarted) {
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		gdiStarted = true;
	}
}

cl_device_id parallelProcessor;
cl_context context;
cl_command_queue queue;
bool SetupCL() {
	cl_platform_id platforms[64];
	unsigned int platformCount;
	cl_int platformResult = clGetPlatformIDs(64, platforms, &platformCount);
	if (platformResult != CL_SUCCESS) {
		return false;
	}
	for (int i = 0; i < platformCount; ++i) {
		cl_device_id devices[64];
		unsigned int deviceCount;
		cl_int deviceResult = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 64, devices, &deviceCount);
		if (deviceResult == CL_SUCCESS) {
			for (int j = 0; j < deviceCount; ++j) {
				char vendorName[256];
				size_t vendorNameLength;
				cl_int deviceInfoResult = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, 256, vendorName, &vendorNameLength);
				if (deviceInfoResult != CL_SUCCESS) {
					std::string sVendorName = vendorName;
					if (sVendorName.find("NVIDIA") != std::string::npos || sVendorName.find("AMD") != std::string::npos) {
						parallelProcessor = devices[j];
						break;
					}
				}
			}
		}
	}

	cl_int contextResult;
	context = clCreateContext(nullptr, 1, &parallelProcessor, nullptr, nullptr, &contextResult);
	if (contextResult != CL_SUCCESS) {
		return false;
	}

	cl_int commandQueueResult;
	queue = clCreateCommandQueue(context, parallelProcessor, 0, &commandQueueResult);
	if (commandQueueResult != CL_SUCCESS) {
		return false;
	}

	return true;
}

std::vector<uint8_t> JPNG::ToPNG() {

	SetupCL();


	std::vector<uint8_t> resultBuffer;
	StartGDI();

	std::shared_ptr<Gdiplus::Image> jfifImg = ImgUtils::BytesToImg(this->jpegData);
	std::shared_ptr<Gdiplus::Bitmap> jfifBmp = ImgUtils::ImgToBmp(jfifImg);

	if (!jfifBmp) {
		Logger::Print<Logger::FAILURE>("Failed to convert jfif to bitmap");
		return resultBuffer;
	}

	std::shared_ptr<Gdiplus::Image> pngImg = ImgUtils::BytesToImg(this->pngData);
	std::shared_ptr<Gdiplus::Bitmap> pngBmp = ImgUtils::ImgToBmp(pngImg);

	if (!pngBmp) {
		Logger::Print<Logger::FAILURE>("Failed to convert jfif to bitmap");
		return resultBuffer;
	}

	uint64_t pngWidth = pngImg->GetWidth();
	uint64_t pngHeight = pngImg->GetHeight();

	uint64_t jfifWidth = jfifImg->GetWidth();
	uint64_t jfifHeight = jfifImg->GetHeight();

	if (pngWidth != jfifWidth || pngHeight != jfifHeight) {
		Logger::Print<Logger::FAILURE>("Image data size mismatch! pngSize and jfifSize are expected to match!");
		return resultBuffer;
	}

	std::shared_ptr<Gdiplus::Bitmap> resultImg = ImgUtils::CreateBmp(pngWidth, pngHeight);
	for (uint64_t x = 0; x < pngWidth; x++) {
		for (uint64_t y = 0; y < pngHeight; y++) {
			Gdiplus::Color cOpacity;
			pngBmp->GetPixel(x, y, &cOpacity);
			if (cOpacity.GetR() != cOpacity.GetG() && cOpacity.GetG() != cOpacity.GetB() && cOpacity.GetB() != cOpacity.GetR()) {
				//These values are expected to match
				Logger::Print<Logger::FAILURE>("Image's opacity layer has an invalid color!");
				return resultBuffer;
			}
			uint8_t opacity = cOpacity.GetR();

			Gdiplus::Color color;
			jfifBmp->GetPixel(x, y, &color);
			Gdiplus::ARGB col_argb = Gdiplus::Color::MakeARGB(opacity, color.GetR(), color.GetG(), color.GetB());
			color.SetValue(col_argb);
			resultImg->SetPixel(x, y, color);
		}
	}

	return ImgUtils::ImgToBytes(resultImg, L"image/png");
};