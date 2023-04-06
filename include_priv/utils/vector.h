

#ifndef VECTOR_H
#define VECTOR_H
#include <cstring>
#include <iostream>

namespace lxcd {
#if __WORDSIZE == 64
typedef unsigned long long size_t;
typedef long long int ptrdiff_t;
#else
typedef unsigned long size_t;
typedef int ptrdiff_t;
#endif

template <typename T>
class initializer_list {
public:
    using value_type = T;
    using reference = const T&;
    using const_reference = const T&;
    using size_type = size_t;

    using iterator = const T*;
    using const_iterator = const T*;

    initializer_list() noexcept : ptr(nullptr), len(0) {}
    initializer_list(const T* p, size_t n) noexcept : ptr(p), len(n) {}

    size_t size() const noexcept { return len; }
    const T* begin() const noexcept { return ptr; }
    const T* end() const noexcept { return ptr + len; }

private:
    const T* ptr;
    size_t len;
};

template<typename T>
class vector {
public:

    class iterator {
    public:
        iterator(T* ptr) : ptr_(ptr) {}
        iterator operator++() { ptr_++; return *this; }
        iterator operator++(int) { iterator temp = *this; ptr_++; return temp; }
        iterator operator--() { ptr_--; return *this; }
        iterator operator--(int) { iterator temp = *this; ptr_--; return temp; }
        bool operator==(const iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const iterator& other) const { return ptr_ != other.ptr_; }
        ptrdiff_t operator-(const iterator& other) const { return ptr_ - other.ptr_; }
        T& operator*() { return *ptr_; }
        T* operator->() { return ptr_; }
    private:
        T* ptr_;
    };

    //TODO : fix lxcd::vector<int> vec = {1, 2, 3, 4, 5};
    vector(initializer_list<T> ilist) : data_(nullptr), size_(0), capacity_(0) {
        for (auto it = ilist.begin(); it != ilist.end(); ++it) {
            push_back(*it);
        }
    }

    vector(T* arr, size_t size) : size_(size), capacity_(size * 2) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < size; i++) {
            data_[i] = arr[i];
        }
    }

    vector();
    vector(lxcd::size_t size);
    vector(lxcd::size_t size, const T& value);
    vector(const vector<T>& other);
    ~vector();
    vector<T>& operator=(const vector<T>& other);
    T& operator[](lxcd::size_t index);
    const T& operator[](lxcd::size_t index) const;

    friend typename vector<T>::iterator operator+(typename vector<T>::iterator it, int offset) {
        return it + offset;
    }

    void push_back(const T& value);
    void pop_back();
    lxcd::size_t size() const;
    lxcd::size_t capacity() const;
    bool empty() const;
    void reserve(lxcd::size_t capacity);
    void resize(lxcd::size_t size);
    void insert(iterator pos, const T& value);

    // Implementation of the begin() method
    iterator begin() { return iterator(data_); }

    // Implementation of the end() method
    iterator end() { return iterator(data_ + size_); }

private:
    T* data_;
    lxcd::size_t size_;
    lxcd::size_t capacity_;
};

template<typename T>
vector<T>::vector() : data_(nullptr), size_(0), capacity_(0) {}

template<typename T>
vector<T>::vector(lxcd::size_t size) : data_(new T[size]), size_(size), capacity_(size) {}

template<typename T>
vector<T>::vector(lxcd::size_t size, const T& value) : data_(new T[size]), size_(size), capacity_(size) {
    for (lxcd::size_t i = 0; i < size_; ++i) {
        data_[i] = value;
    }
}

template<typename T>
vector<T>::vector(const vector<T>& other) : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
    for (lxcd::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}

template<typename T>
vector<T>::~vector() {
    delete[] data_;
}

template<typename T>
vector<T>& vector<T>::operator=(const vector<T>& other) {
    if (this != &other) {
        delete[] data_;
        data_ = new T[other.capacity_];
        size_ = other.size_;
        capacity_ = other.capacity_;
        for (lxcd::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    return *this;
}

template<typename T>
T& vector<T>::operator[](lxcd::size_t index) {
    return data_[index];
}

template<typename T>
const T& vector<T>::operator[](lxcd::size_t index) const {
    return data_[index];
}

template<typename T>
void vector<T>::push_back(const T& value) {
    if (size_ == capacity_) {
        reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    data_[size_++] = value;
}

template<typename T>
void vector<T>::pop_back() {
    if (size_ > 0) {
        --size_;
    }
}

template<typename T>
lxcd::size_t vector<T>::size() const {
    return size_;
}

template<typename T>
lxcd::size_t vector<T>::capacity() const {
    return capacity_;
}

template<typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}

template<typename T>
void vector<T>::reserve(lxcd::size_t capacity) {
    if (capacity > capacity_) {
        T* new_data = new T[capacity];
        for (lxcd::size_t i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = capacity;
    }
}

template<typename T>
void vector<T>::resize(lxcd::size_t size) {
    if (size > size_) {
        reserve(size);
        for (lxcd::size_t i = size_; i < size; ++i) {
            data_[i] = T();
        }
    }
    size_ = size;
}
template <typename T>
void vector<T>::insert(iterator pos, const T& value) {

    size_t index = pos - begin();
    if (size_ == capacity_) {
        reserve(capacity_ * 2);
    }
    memmove(data_ + index + 1, data_ + index, (size_ - index) * sizeof(T));
    data_[index] = value;
    size_++;
}
}
#endif
