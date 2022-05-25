#include "jpng.h"

#include <imgutils.h>
#include <logger.h>
#include <memory>
#include <algorithm>
#define __CL_ENABLE_EXCEPTIONS
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

	size_t pngWidth = pngImg->GetWidth();
	size_t pngHeight = pngImg->GetHeight();

	size_t jfifWidth = jfifImg->GetWidth();
	size_t jfifHeight = jfifImg->GetHeight();

	if (pngWidth != jfifWidth || pngHeight != jfifHeight) {
		Logger::Print<Logger::FAILURE>("Image data size mismatch! pngSize and jfifSize are expected to match!");
		return resultBuffer;
	}

	auto* jfifBmpData = new Gdiplus::BitmapData;
	Gdiplus::Rect jfifRect(0, 0, jfifWidth, jfifHeight);
	jfifBmp->LockBits(&jfifRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, jfifBmpData);
	void* jfifBytes = jfifBmpData->Scan0;

	auto* pngBmpData = new Gdiplus::BitmapData;
	Gdiplus::Rect pngRect(0, 0, pngWidth, pngHeight);
	pngBmp->LockBits(&pngRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, pngBmpData);
	void* pngBytes = pngBmpData->Scan0;

	SetupCL();
	const cl_image_format imageFormat = {
		CL_ARGB,
		CL_UNSIGNED_INT8,
	};

	cl_int jfifCopy_error;
	cl_mem jfifPixels = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&imageFormat,
		jfifWidth,
		jfifHeight,
		1,
		jfifBytes,
		&jfifCopy_error);
	if (jfifCopy_error != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Couldn't make OpenCL buffer for jfif data");
		return resultBuffer;
	}

	cl_int pngCopy_error;
	cl_mem pngPixels = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&imageFormat,
		pngWidth,
		pngHeight,
		1,
		pngBytes,
		&pngCopy_error);
	if (pngCopy_error != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Couldn't make OpenCL buffer for png data");
		return resultBuffer;
	};

	cl_int resultBuff_error;
	cl_mem resultPixels = clCreateImage2D(context,
		CL_MEM_READ_WRITE,
		&imageFormat,
		pngWidth,
		pngHeight,
		1,
		nullptr,
		&resultBuff_error);

	const char* convertToPngSrc =
		#include "jpngToPng.cl"
		;

	cl_int prog_error;
	cl_program program = clCreateProgramWithSource(context, 1, &convertToPngSrc, NULL, &prog_error);
	if (prog_error != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Failed to create OpenCL program with source");
		return resultBuffer;
	}
	cl_int build_errror = clBuildProgram(program, 1, &parallelProcessor, NULL, NULL, NULL);
	if (build_errror != CL_SUCCESS) {
		size_t len = 0;
		cl_int logRes = clGetProgramBuildInfo(program, parallelProcessor, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
		if (logRes != CL_SUCCESS) {
			Logger::Print<Logger::FAILURE>("Failed to size OpenCL build log!");
		}
		char* buffer = (char*)_malloca(len);
		logRes = clGetProgramBuildInfo(program, parallelProcessor, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
		if (logRes != CL_SUCCESS) {
			Logger::Print<Logger::FAILURE>("Failed to read OpenCL build log!");
		}
		std::string buildLogs(buffer, len);
		Logger::Print<Logger::FAILURE>("Failed to build OpenCL program: {}", buildLogs);
		return resultBuffer;
	}

	cl_int kernel_error;
	cl_kernel kernel = clCreateKernel(program, "jpngToPng", &kernel_error);
	if (kernel_error != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Failed to create cl_kernel");
		return resultBuffer;
	}

	cl_int argResult;
	argResult = clSetKernelArg(kernel, 0, sizeof(cl_mem), pngPixels);
	argResult |= clSetKernelArg(kernel, 1, sizeof(cl_mem), jfifPixels);
	argResult |= clSetKernelArg(kernel, 2, sizeof(cl_mem), resultPixels);
	if (argResult != CL_SUCCESS)
	{
		Logger::Print<Logger::FAILURE>("Error: Failed to set kernel arguments! {}", argResult);
		return resultBuffer;
	}

	cl_int execErr = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &pngWidth, &pngHeight, 0, NULL, NULL);
	if (execErr != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Error: Failed to execute kernel");
		return resultBuffer;
	}

	void* hostResult = _malloca(pngWidth * pngHeight * sizeof(uint32_t));
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { pngWidth, pngHeight, 1 };
	cl_int readResult = clEnqueueReadImage(queue, resultPixels, CL_TRUE, origin, region, 0, 0, hostResult, 0, NULL, NULL);
	if (readResult != CL_SUCCESS) {
		Logger::Print<Logger::FAILURE>("Error: Failed to read kernel result");
		return resultBuffer;
	}

	std::shared_ptr<Gdiplus::Bitmap> resultBmp = std::shared_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(pngWidth, pngHeight, 1, PixelFormat32bppARGB, (BYTE*)hostResult));
	/*
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
	}*/

	return ImgUtils::ImgToBytes(resultBmp, L"image/png");
};