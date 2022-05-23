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

	//TODO: finish kthx
	template <typename T>
	class ArrayList {
		T* firstAddr;
		T* lastAddr;
		void* nextRealign;
	public:
		size_t count() {
			return (((size_t)lastAddr) - ((size_t)firstAddr)) / sizeof(T);
		}
		T* begin() {
			return firstAddr;
		}
		T* end() {
			return lastAddr;
		}
		void clear() {
			lastAddr = firstAddr;
		}
		T* operator[](long long pos) {
			return this->at(pos);
		}
		T* at(long long pos) {
			long long ptrOffset = pos * sizeof(T);
			T* item = (T*)(((long long)firstAddr) + ptrOffset);
			return item;
		}
		void realign() {
			//Double the element count, and multiply that by the size of a pointer
			//Leaves double the room before needing to realloc
			long long b_size = count() * sizeof(T);
			long long newAllocSize = (count() * sizeof(T)) * 2;
			T* newFirst = (T*)malloc(newAllocSize);
			memset(newFirst, 0, newAllocSize);

			//Copy old objects
			T* contentBegin = newFirst + 16;
			long long contentSize = newAllocSize - 16;
			memcpy_s(contentBegin, contentSize, firstAddr, b_size);

			*(size_t*)(contentBegin - sizeof(T*)) = (size_t)newFirst;

			//set new vals
			firstAddr = contentBegin;
			lastAddr = contentBegin + b_size;
			nextRealign = newFirst + newAllocSize;
		}
		void push_back(T val) {
			if (lastAddr + sizeof(T) >= nextRealign) {
				realign();
			}
			*lastAddr = val;
			lastAddr += sizeof(T);
		}
		bool remove(long long index) {
			long long count = this->count();
			if (index >= count) {
				//Index is outside of the size like bruh
				return false;
			}

			//Get a pointer to the instance at the desired index
			T* tAtOff = this->at(index);
			//Get a pointer to the next instance after the one at the index
			T* tNextOff = this->at(index + 1);
			//Copy the memory to align the next offset at the current offset
			memcpy_s(tAtOff, lastAddr - tAtOff, tNextOff, lastAddr - tNextOff);
			//Throw away/free the last instance (its now at the index before, so our lastAddr pops up sizeof(T)
			this->lastAddr = this->at(this->count() - 1);

			//We're done
			return true;
		}
	};
	static_assert(sizeof(ArrayList<void*>) == 24, "nuv::vector is misaligned!");
};