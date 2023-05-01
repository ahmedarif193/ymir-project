

#ifndef VECTOR_H
#define VECTOR_H

namespace lxcd {
#if __WORDSIZE == 64
typedef unsigned long long size_t;
typedef long long int ptrdiff_t;
#else
typedef unsigned long size_t;
typedef int ptrdiff_t;
#endif


template<typename T>
class vector {
public:

class iterator {
public:
iterator(T* ptr) : ptr_(ptr) {
}
iterator operator++() {
    ptr_++; return *this;
}
iterator operator++(int) {
    iterator temp = *this; ptr_++; return temp;
}
iterator operator--() {
    ptr_--; return *this;
}
iterator operator--(int) {
    iterator temp = *this; ptr_--; return temp;
}
bool operator==(const iterator& other) const {
    return ptr_ == other.ptr_;
}
bool operator!=(const iterator& other) const {
    return ptr_ != other.ptr_;
}
ptrdiff_t operator-(const iterator& other) const {
    return ptr_ - other.ptr_;
}
iterator operator-(size_t offset) const {
    return iterator(ptr_ - offset);
}
T& operator*() {
    return *ptr_;
}
T* operator->() {
    return ptr_;
}

private:
T* ptr_;
};

vector(T* arr, size_t size) : size_(size), capacity_(size * 2) {
    data_ = new T[capacity_];
    for(size_t i = 0; i < size; i++) {
        data_[i] = arr[i];
    }
}

T&& move(T& obj) {
    return static_cast<T &&>(obj);
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
iterator begin() {
    return iterator(data_);
}

// Implementation of the end() method
iterator end() {
    return iterator(data_ + size_);
}

iterator erase(iterator pos);

iterator erase(iterator first, iterator last);

void clear();

void remove_empty() {
    for(iterator it = begin(); it != end();) {
        if(it->empty()) {
            it = erase(it);
        } else {
            ++it;
        }
    }
}


private:
T* data_;
lxcd::size_t size_;
lxcd::size_t capacity_;
};

template<typename T>
class InitializerList {
public:
InitializerList() {
}

template<typename First, typename ... Args>
InitializerList(First first, Args... args) {
    data_.push_back(first);
    insert(args ...);
}

const T* begin() const noexcept {
    return data_.data();
}

const T* end() const noexcept {
    return data_.data() + data_.size();
}

size_t size() const noexcept {
    return data_.size();
}

private:
vector<T> data_;

void insert() {
}

template<typename First, typename ... Args>
void insert(First first, Args... args) {
    data_.push_back(first);
    insert(args ...);
}
};

#include "impl_vector.hpp"

}
#endif
