#include "utils/vector.h"

using namespace lxcd;

template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator pos) {
        if (pos == end()) {
            return end();
        }

        iterator i = pos;
        ++i;
        for (iterator j = i; j != end(); ++j, ++pos) {
            *pos = std::move(*j);
        }
        pop_back();

        return i;
    }

template<typename T>
typename vector<T>::iterator vector<T>::erase(typename vector<T>::iterator first, typename vector<T>::iterator last) {
        if (first == last) {
            return last;
        }

        iterator i = first;
        for (iterator j = last; j != end(); ++j, ++i) {
            *i = std::move(*j);
        }

        size_t n = last - first;
        for (size_t k = 0; k < n; ++k) {
            pop_back();
        }

        return first;
    }

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