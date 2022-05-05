#pragma once

#include <memory>
#include <string>

namespace Soup {
	template<typename T>
	class BasicString {
		static constexpr size_t _BUF_SIZE = 16 / sizeof(T) < 1 ? 1 : 16 / sizeof(T);
		union {
			T* ptr;
			T buf[_BUF_SIZE];
		} box;
		size_t size = 0;
		size_t res = 0;
	public:
		BasicString(const T* const ptr) {
			this->assign(ptr);
		}
		BasicString(BasicString&& right) {
			this->assign(right);
		}
		constexpr BasicString<T>& assign(BasicString<T>&& right) {
			*this = std::move(right);
			return *this;
		}
		constexpr BasicString<T>& assign(const T* const ptr, size_t len) {
			if (len >= _BUF_SIZE) {
				this->box.ptr = ptr;
			}
			else {
				memcpy(this->box.buf, ptr, len);
			}
			this->size = len;
			this->res = _BUF_SIZE;
		}
		constexpr const T* const ptr() {
			if (this->size >= _BUF_SIZE) {
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
			return std::basic_string<T>(this->ptr(), this->size);
		}
	};

	using String = BasicString<char>;
	static_assert(sizeof(String) == 0x20, "String is misaligned!");
};