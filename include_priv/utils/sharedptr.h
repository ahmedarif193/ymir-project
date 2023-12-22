#ifndef SHAREDPTR_H
#define SHAREDPTR_H

#include <sys/mman.h>

namespace lxcd {

template<class T>
class SharedPtr {
public:
// Constructors
SharedPtr() : count(nullptr), data(nullptr) {
}
explicit SharedPtr(T* ptr) : count(new int(1)), data(ptr) {
}
SharedPtr(const SharedPtr& other) : count(other.count), data(other.data) {
	if(count) {
		(*count)++;
	}
}

// Destructor
~SharedPtr() {
	reset();
}

// Assignment operators
SharedPtr& operator=(const SharedPtr& other) {
	if(this != &other) {
		reset();
		count = other.count;
		data = other.data;
		if(count) {
			(*count)++;
		}
	}
	return *this;
}

template<typename U>
SharedPtr& operator=(const SharedPtr<U>& other) {
	reset();
	count = other.count;
	data = other.data;
	if(count) {
		(*count)++;
	}
	return *this;
}

// Move constructors and assignment operator
SharedPtr(SharedPtr&& other) noexcept {
	count = other.count;
	data = other.data;
	other.count = nullptr;
	other.data = nullptr;
}

SharedPtr& operator=(SharedPtr&& other) noexcept {
	if(this != &other) {
		reset();
		count = other.count;
		data = other.data;
		other.count = nullptr;
		other.data = nullptr;
	}
	return *this;
}

// Reset the pointer and the reference count
virtual void reset() {
	if(count) {
		(*count)--;
		if(*count == 0) {
			delete count;
			delete data;
		}
		count = nullptr;
		data = nullptr;
	}
}

// Check if the pointer is null
bool isNull() const {
	return data == nullptr;
}

// Get the pointer
T* get() const {
	return data;
}

// Get the reference count
int use_count() const {
	return (count ? *count : 0);
}

// Pointer operators
T& operator*() const {
	return *data;
}

T* operator->() const {
	return data;
}

// Conversion operator
operator bool() const {
	return !isNull();
}

protected:
int* count;
T* data;
};

template<typename T, typename ... Args>
SharedPtr<T> makeShared(Args&& ... args) {
	return SharedPtr<T>(new T(args ...));
}


template<class T>
class MmapSharedPtr : public SharedPtr<T> {
public:
MmapSharedPtr() {
	this->size = sizeof(T);  // Determine size based on type T
	this->count = new int(1);
	this->data = static_cast<T*>(mmap(nullptr, this->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
	if (this->data == MAP_FAILED) {
		delete this->count;
		throw "mmap failed";
	}
}
~MmapSharedPtr() {
	this->reset();
}

MmapSharedPtr(const MmapSharedPtr& other) = delete;
MmapSharedPtr& operator=(const MmapSharedPtr& other) = delete;

void reset() override {
	if (this->count && --(*this->count) == 0) {
		delete this->count;
		if (this->data != MAP_FAILED) {
			munmap(this->data, this->size);
		}
	}
	this->count = nullptr;
	this->data = nullptr;
}

private:
size_t size;
};

}
#endif
