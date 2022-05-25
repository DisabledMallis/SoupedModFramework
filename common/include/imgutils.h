#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <gdiplusimaging.h>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace ImgUtils {

	template<typename T>
	class StreamPtr {
		T* innerPtr;
	public:
		StreamPtr() {
			this->innerPtr == nullptr;
		}
		StreamPtr(T* innerPtr) {
			this->innerPtr = innerPtr;
		}
		~StreamPtr() {
			if(this->innerPtr != nullptr)
				this->innerPtr->Release();
		}
		operator bool() {
			return innerPtr == nullptr;
		}
		T* operator ->() {
			return innerPtr;
		}
		operator T* () {
			return innerPtr;
		}
	};

	std::shared_ptr<Gdiplus::Image> BytesToImg(std::vector<uint8_t>);
	std::shared_ptr<Gdiplus::Bitmap> ImgToBmp(std::shared_ptr<Gdiplus::Image>);
	std::shared_ptr<Gdiplus::Bitmap> CreateBmp(int width, int height);
	std::vector<uint8_t> ImgToBytes(std::shared_ptr<Gdiplus::Bitmap>, std::wstring format);
	std::vector<uint8_t> ImgToBytes(std::shared_ptr<Gdiplus::Image>, std::wstring format);
}