#ifndef SHAREDPTR_H
#define SHAREDPTR_H

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

SharedPtr(std::nullptr_t) {
    count = nullptr;
    data = nullptr;
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
void reset() {
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

private:
int* count;
T* data;
};

template<typename T, typename ... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return SharedPtr<T>(new T(args ...));
}

}
#endif
