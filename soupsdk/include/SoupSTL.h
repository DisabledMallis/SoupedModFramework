#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace Soup {
	template<typename T>
	struct BasicString {
	private:
		static constexpr size_t _BUF_SIZE = 16 / sizeof(T) < 1 ? 1 : 16 / sizeof(T);
	public:
		union {
			T* ptr;
			T buf[_BUF_SIZE];
		} box = 0;
		static_assert(sizeof(box) == _BUF_SIZE, "BasicString::box is the wrong size!");
		size_t length = 0;
		size_t res = 0;

	public:
		template<size_t _LEN> BasicString(const T(& ptr)[_LEN]) {
			this->assign(ptr, _LEN);
		}
		BasicString(const T* ptr, size_t len) {
			this->assign(ptr, len);
		}
		BasicString(BasicString&& right) {
			this->assign(right);
		}
		constexpr BasicString<T>& assign(BasicString<T>& right) {
			this->assign(right.c_str(), right.length());
		}
		constexpr BasicString<T>& assign(const T* ptr, size_t len) {
			if (this->res >= _BUF_SIZE) {
				if (this->box.ptr != nullptr) {
					free(this->box.ptr);
				}
			}

			this->length = len;

			if (len >= _BUF_SIZE) {
				size_t allocSize = (sizeof(T) * len) + 1; //Add a byte for the null terminator
				T* boxPtr = (T*)malloc(allocSize);
				if (!boxPtr) {
					throw std::exception("Failed to allocate ptr for string buffer");
				}
				memcpy(boxPtr, ptr, len);
				this->box.ptr = boxPtr;
				this->res = allocSize;
			}
			else {
				memcpy(this->box.buf, ptr, len);
				this->res = _BUF_SIZE - 1; //All strings are null terminated, so we need to leave a byte for that
			}
			return *this;
		}
		constexpr const T* const ptr() {
			if (this->count() > _BUF_SIZE) {
				return this->box.ptr;
			}
			else {
				return this->box.buf;
			}
		}
		constexpr const T* const c_str() {
			return this->ptr();
		}
		constexpr const std::basic_string<T> cpp_str() {
			return std::basic_string<T>(this->ptr(), this->count());
		}
		constexpr const size_t count() {
			return this->length;
		}
		constexpr const size_t size() {
			return this->length * sizeof(T);
		}
	};
	using String = BasicString<char>;
	static_assert(sizeof(String) == 0x20, "String is the wrong size!");
	static_assert(offsetof(String, box) == 0x0, "String::box is misaligned!");
	static_assert(offsetof(String, length) == 0x10, "String::length is misaligned!");
	static_assert(offsetof(String, res) == 0x18, "String::res is misaligned!");

	template<typename T>
	struct BasicStringView {
		const T* viewBegin;
		const T* viewEnd;

	public:
		BasicStringView() {};

		const std::basic_string_view<T> cpp_sv() {
			return basic_string_view(this->viewBegin, this->viewEnd);
		}
	};
};