#include <imgutils.h>

#include <Shlwapi.h>
#include <logger.h>

std::shared_ptr<Gdiplus::Image> ImgUtils::BytesToImg(std::vector<uint8_t> bytes) {
	ImgUtils::StreamPtr<IStream> bytesStream = SHCreateMemStream(bytes.data(), bytes.size());
	std::shared_ptr<Gdiplus::Image> image = std::shared_ptr<Gdiplus::Image>(Gdiplus::Image::FromStream(bytesStream));
	return image;
}

std::shared_ptr<Gdiplus::Bitmap> ImgUtils::ImgToBmp(std::shared_ptr<Gdiplus::Image> image) {
	std::shared_ptr<Gdiplus::Bitmap> bmp = nullptr;
	try {
		int wd = image->GetWidth();
		int hgt = image->GetHeight();
		auto format = image->GetPixelFormat();
		bmp = std::shared_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(wd, hgt, format));
		auto g = std::unique_ptr<Gdiplus::Graphics>(Gdiplus::Graphics::FromImage(bmp.get()));
		g->Clear(Gdiplus::Color::Transparent);
		g->DrawImage(image.get(), 0, 0, wd, hgt);
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

std::shared_ptr<Gdiplus::Bitmap> ImgUtils::CreateBmp(int width, int height) {
	std::shared_ptr<Gdiplus::Bitmap> result = std::shared_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));
	return result;
}

//Helpful function from ms docs :)
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
std::vector<uint8_t> ImgUtils::ImgToBytes(std::shared_ptr<Gdiplus::Bitmap> image, std::wstring format) {
	return ImgToBytes((std::shared_ptr<Gdiplus::Image>)image, format);
}
std::vector<uint8_t> ImgUtils::ImgToBytes(std::shared_ptr<Gdiplus::Image> image, std::wstring format) {
	std::vector<uint8_t> resultBuffer;
	// Get the CLSID of the PNG encoder.
	CLSID encoderClsid;
	GetEncoderClsid(L"image/png", &encoderClsid);

	IStream* resultStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &resultStream);
	if (!SUCCEEDED(hr)) {
		Logger::Print<Logger::FAILURE>("Failed to create result stream for image");
		return resultBuffer;
	}
	image->Save(resultStream, &encoderClsid, NULL);

	ULARGE_INTEGER liSize;
	IStream_Size(resultStream, &liSize);
	DWORD len = liSize.LowPart;
	IStream_Reset(resultStream);
	resultBuffer.resize(len);
	IStream_Read(resultStream, resultBuffer.data(), len);

	return resultBuffer;
}