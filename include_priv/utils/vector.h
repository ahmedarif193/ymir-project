

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
        iterator operator-(size_t offset) const { return iterator(ptr_ - offset); }
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

    iterator erase(iterator pos);

    iterator erase(iterator first, iterator last);

private:
    T* data_;
    lxcd::size_t size_;
    lxcd::size_t capacity_;
};


}
#endif
