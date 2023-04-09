#ifndef LXCDSTRING_H
#define LXCDSTRING_H
#include "vector.h" // assume the vector class is defined in a separate header file
#include <cstring>
#include <iostream>

namespace lxcd {

class string {
public:
    string();
    
    string(const char* str);
    
    string(const string& other);
    
    ~string();

    char* begin();
    const char* begin() const;
    char* end();
    const char* end() const;

    string& operator=(const string& other);

    // Move assignment operator
    string& operator=(string&& other) noexcept;

    // Assignment operator from C-style string
    string& operator=(const char* str);
    char& operator[](size_t index);
    
    const char& operator[](size_t index) const;
    
    size_t size() const;
    
    const char* c_str() const;
    operator const char*() const;
    bool operator==(const char* other) const;
    bool operator==(const string& other) const;

    bool operator!=(const string& other) const;
    
    string operator+(const string& other) const;

    string& operator+=(const string& other);

    string& operator+=(const char* other);

    string& operator+=(const char other);
    friend string operator+(const char* lhs, const string& rhs);
    
    friend string operator+(const string& rhs, const char* lhs);

    vector<string> split(char delimiter) const;


    // Erase a portion of this string
    string& erase(size_t pos, size_t count = npos);

    // Find the first occurrence of a substring in this string
    size_t find(const string& substr, size_t pos = 0) const;

    bool contains(const char* str) const;

    bool contains(const string& str) const;

    // Check if this string is empty
    bool empty() const;

    // Get the length of this string
    size_t length() const;

    size_t rfind(const char* str, size_t pos = npos) const;

    string substr(size_t pos = 0, size_t len = npos) const;

    string& append(const char* str, size_t len);

    string& append(const string& other);

    string& append(const char* str);

    void insert(size_t pos, const string& str);

    void insert(size_t pos, const char* str);

    void resize(size_t n);
    void reserve(size_t n);

    static const size_t npos = static_cast<size_t>(-1);
private:
    char* data_;
    size_t size_;
    
    string(const char* str, size_t size);
};

inline string operator+(const char* lhs, const string& rhs) {
    return string(lhs) + rhs;
}

inline string operator+(const string& rhs, const char* lhs) {
    return rhs + string(lhs);
}

double stod(const string& str);

int stoi(const string& str);

string to_string(int value);

string to_string(double value);

#include "impl_string.hpp"

}

#endif // LXCDSTRING_H

