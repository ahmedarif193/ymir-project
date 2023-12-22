

#ifndef VECTOR_H
#define VECTOR_H

extern "C" {
void* malloc(unsigned long);
void free(void*);
}

inline void* operator new[](unsigned long size) {
	void* ptr = malloc(size);
	if (!ptr) {
		// Handle allocation failure. In standard C++, std::bad_alloc would be thrown.
		// Here, you might terminate or return nullptr, depending on your application's needs.
	}
	return ptr;
}

inline void operator delete[](void* ptr) noexcept {
	free(ptr);
}

// Optional: Sized deallocation (C++14 and later)
inline void operator delete[](void* ptr, unsigned long) noexcept {
	free(ptr);
}

// Optional: Overload for the non-array versions as well
inline void* operator new(unsigned long size) {
	return operator new[](size);
}

inline void operator delete(void* ptr) noexcept {
	operator delete[](ptr);
}

inline void operator delete(void* ptr, unsigned long) noexcept {
	free(ptr);
}

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

class const_iterator {
public:
const_iterator(const T* ptr) : ptr_(ptr) {
}
const_iterator operator++() {
	ptr_++; return *this;
}
const_iterator operator++(int) {
	const_iterator temp = *this; ptr_++; return temp;
}
const_iterator operator--() {
	ptr_--; return *this;
}
const_iterator operator--(int) {
	const_iterator temp = *this; ptr_--; return temp;
}
bool operator==(const const_iterator& other) const {
	return ptr_ == other.ptr_;
}
bool operator!=(const const_iterator& other) const {
	return ptr_ != other.ptr_;
}
ptrdiff_t operator-(const const_iterator& other) const {
	return ptr_ - other.ptr_;
}
const_iterator operator-(size_t offset) const {
	return const_iterator(ptr_ - offset);
}
const T& operator*() const {
	return *ptr_;
}
const T* operator->() const {
	return ptr_;
}

private:
const T* ptr_;
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

iterator begin() {
	return iterator(data_);
}

iterator end() {
	return iterator(data_ + size_);
}

const_iterator begin() const {
	return const_iterator(data_);
}

const_iterator end() const {
	return const_iterator(data_ + size_);
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
template<typename ... Args>
InitializerList(Args... args) {
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
template<typename First, typename ... Args>
void insert(const First& first, const Args& ... args) {
	data_.push_back(first);
	insert(args ...);
}
};

#include "impl_vector.hpp"

}
#endif
