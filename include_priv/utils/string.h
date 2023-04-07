#ifndef LXCDSTRING_H
#define LXCDSTRING_H
#include "vector.h" // assume the vector class is defined in a separate header file
#include <cstring>
#include <iostream>

namespace lxcd {

class string {
public:
    string() : data_(new char[1]), size_(0) {
        data_[0] = '\0';
    }
    
    string(const char* str) : data_(new char[strlen(str) + 1]), size_(strlen(str)) {
        strcpy(data_, str);
    }
    
    string(const string& other) : data_(new char[other.size_ + 1]), size_(other.size_) {
        strcpy(data_, other.data_);
    }
    
    ~string() {
        delete[] data_;
    }

    char* begin() { return data_; }
    const char* begin() const { return data_; }
    char* end() { return data_ + size_; }
    const char* end() const { return data_ + size_; }

    string& operator=(const string& other) {
        if (this != &other) {
            delete[] data_;
            data_ = new char[other.size_ + 1];
            size_ = other.size_;
            if (data_ != nullptr) { // check for null pointer
                strcpy(data_, other.data_);
            }
        }
        return *this;
    }

    // Move assignment operator
    string& operator=(string&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // Assignment operator from C-style string
    string& operator=(const char* str) {
        size_t len = strlen(str);
        if (size_ < len) {
            char* newData = new char[len + 1];
            delete[] data_;
            data_ = newData;
            size_ = len;
        }
        strcpy(data_, str);
        return *this;
    }
    char& operator[](size_t index) {
        return data_[index];
    }
    
    const char& operator[](size_t index) const {
        return data_[index];
    }
    
    size_t size() const {
        return size_;
    }
    
    const char* c_str() const {
        return data_;
    }
    operator const char*() const {
        return data_;
    }
    bool operator==(const char* other) const {
        return strcmp(data_, other) == 0;
    }
    bool operator==(const string& other) const {
        return (size_ == other.size_) && (strcmp(data_, other.data_) == 0);
    }

    bool operator!=(const string& other) const {
        return !(*this == other);
    }
    
    string operator+(const string& other) const {
        string result;
        result.size_ = size_ + other.size_;
        delete[] result.data_;
        result.data_ = new char[result.size_ + 1];
        strcpy(result.data_, data_);
        strcat(result.data_, other.data_);
        return result;
    }

    string& operator+=(const string& other) {
        size_t newSize = size_ + other.size_;
        char* newData = new char[newSize + 1];
        strcpy(newData, data_);
        strcat(newData, other.data_);
        delete[] data_;
        data_ = newData;
        size_ = newSize;
        return *this;
    }

    string& operator+=(const char* other) {
        size_t len = strlen(other);
        reserve(size_ + len);
        strcat(data_, other);
        size_ += len;
        return *this;
    }

    string& operator+=(const char other) {
        const char temp[2] = { other, '\0' };
        return (*this += temp);
    }
    friend string operator+(const char* lhs, const string& rhs);
    
    friend string operator+(const string& rhs, const char* lhs);

    vector<string> split(char delimiter) const {
        vector<string> tokens;
        size_t start = 0;
        for (size_t i = 0; i <= size_; ++i) {
            if (data_[i] == delimiter || data_[i] == '\0') {
                tokens.push_back(string(data_ + start, i - start));
                start = i + 1;
            }
        }
        return tokens;
    }


    // Erase a portion of this string
    string& erase(size_t pos, size_t count = npos) {
        if (pos >= size_) {
            return *this;
        }
        if (count == npos || pos + count >= size_) {
            size_ = pos;
            data_[size_] = '\0';
            return *this;
        }
        memmove(data_ + pos, data_ + pos + count, size_ - pos - count + 1);
        size_ -= count;
        return *this;
    }

    // Find the first occurrence of a substring in this string
    size_t find(const string& substr, size_t pos = 0) const {
        const char* result = strstr(data_ + pos, substr.data_);
        if (result == nullptr) {
            return npos;
        }
        return result - data_;
    }

    bool contains(const char* str) const {
        return find(str) != npos;
    }

    bool contains(const string& str) const {
        return find(str) != npos;
    }

    // Check if this string is empty
    bool empty() const {
        return size_ == 0;
    }

    // Get the length of this string
    size_t length() const {
        return size_;
    }

    size_t rfind(const char* str, size_t pos = npos) const {
        if (pos > size_) {
            pos = size_;
        }
        size_t len = strlen(str);
        if (len == 0) {
            return pos;
        }
        for (size_t i = pos; i >= len; i--) {
            if (strncmp(&data_[i - len], str, len) == 0) {
                return i - len;
            }
        }
        return npos;
    }

    string substr(size_t pos = 0, size_t len = npos) const {
        if (pos > size_) {
            pos = size_;
        }
        if (len > size_ - pos) {
            len = size_ - pos;
        }
        return string(&data_[pos], len);
    }

    string& append(const char* str, size_t len) {
        size_t new_size = size_ + len;
        char* new_data = new char[new_size + 1];
        strcpy(new_data, data_);
        strncat(new_data, str, len);
        delete[] data_;
        data_ = new_data;
        size_ = new_size;
        return *this;
    }

    string& append(const string& other) {
        size_t new_size = size_ + other.size_;
        char* new_data = new char[new_size + 1];
        strcpy(new_data, data_);
        strcat(new_data, other.data_);
        delete[] data_;
        data_ = new_data;
        size_ = new_size;
        return *this;
    }

    string& append(const char* str) {
        return append(string(str));
    }

    void insert(size_t pos, const string& str) {
        size_t new_size = size_ + str.size_;
        char* new_data = new char[new_size + 1];
        memcpy(new_data, data_, pos);
        memcpy(new_data + pos, str.data_, str.size_);
        memcpy(new_data + pos + str.size_, data_ + pos, size_ - pos);
        new_data[new_size] = '\0';
        delete[] data_;
        data_ = new_data;
        size_ = new_size;
    }

    void insert(size_t pos, const char* str) {
        insert(pos, string(str));
    }

    void resize(size_t n) {
        if (n <= size_) {
            data_[n] = '\0';
        } else {
            char* newData = new char[n + 1];
            memset(newData + size_, 0, n - size_ + 1);
            strcpy(newData, data_);
            delete[] data_;
            data_ = newData;
            size_ = n;
        }
    }
    void reserve(size_t n) { resize(n);}

    static const size_t npos = static_cast<size_t>(-1);
private:
    char* data_;
    size_t size_;
    
    string(const char* str, size_t size) : data_(new char[size + 1]), size_(size) {
        strncpy(data_, str, size);
        data_[size] = '\0';
    }
};

inline string operator+(const char* lhs, const string& rhs) {
    return string(lhs) + rhs;
}

inline string operator+(const string& rhs, const char* lhs) {
    return rhs + string(lhs);
}

inline double stod(const string& str) {
    double result = 0.0;
    double factor = 1.0;
    bool negative = false;
    bool decimal = false;

    for (const char c : str) {
        if (c == '-') {
            negative = true;
        } else if (c == '.') {
            decimal = true;
        } else {
            int digit = c - '0';
            if (digit >= 0 && digit <= 9) {
                if (decimal) {
                    factor /= 10.0;
                }
                result = result * 10.0 + digit;
            } else {
                break;
            }
        }
    }

    if (negative) {
        result *= -1.0;
    }

    return result * factor;
}

inline int stoi(const string& str) {
    int res = 0;
    int sign = 1;
    size_t i = 0;

    // Check for leading sign
    if (str[i] == '-') {
        sign = -1;
        ++i;
    } else if (str[i] == '+') {
        ++i;
    }

    // Parse digits
    for (; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            break;
        }
        res = res * 10 + (str[i] - '0');
    }

    return sign * res;
}

inline  string to_string(int value) {
    char buffer[20];
    sprintf(buffer, "%d", value);
    return string(buffer);
}

inline  string to_string(double value) {
    char buffer[20];
    sprintf(buffer, "%.8g", value);
    return string(buffer);
}
}

#endif // LXCQUEUE_H

