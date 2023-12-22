#include "utils/string.h"

lxcd::string::string() : data_(new char[1]), size_(0) {
    data_[0] = '\0';
}

lxcd::string::string(const char *str) : data_(new char[strlen(str) + 1]), size_(strlen(str)) {
    strcpy(data_, str);
}

lxcd::string::string(const lxcd::string &other) : data_(new char[other.size_ + 1]), size_(other.size_) {
    strcpy(data_, other.data_);
}

lxcd::string::~string() {
    delete[] data_;
}

char *lxcd::string::begin() { return data_; }

const char *lxcd::string::begin() const { return data_; }

char *lxcd::string::end() { return data_ + size_; }

const char *lxcd::string::end() const { return data_ + size_; }

lxcd::string &lxcd::string::operator=(const lxcd::string &other) {
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

lxcd::string &lxcd::string::operator=(lxcd::string &&other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

lxcd::string &lxcd::string::operator=(const char *str) {
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

char &lxcd::string::operator[](lxcd::size_t index) {
    return data_[index];
}

const char &lxcd::string::operator[](lxcd::size_t index) const {
    return data_[index];
}

lxcd::size_t lxcd::string::size() const {
    return size_;
}

const char *lxcd::string::c_str() const {
    return data_;
}

lxcd::string::operator const char *() const {
    return data_;
}

bool lxcd::string::operator==(const char *other) const {
    return strcmp(data_, other) == 0;
}

bool lxcd::string::operator==(const lxcd::string &other) const {
    return (size_ == other.size_) && (strcmp(data_, other.data_) == 0);
}

bool lxcd::string::operator!=(const lxcd::string &other) const {
    return !(*this == other);
}
bool lxcd::string::operator<(const lxcd::string &other) const {
    return strcmp(data_, other.data_) < 0;
}
lxcd::string lxcd::string::operator+(const lxcd::string &other) const {
    string result;
    result.size_ = size_ + other.size_;
    delete[] result.data_;
    result.data_ = new char[result.size_ + 1];
    strcpy(result.data_, data_);
    strcat(result.data_, other.data_);
    return result;
}

lxcd::string &lxcd::string::operator+=(const lxcd::string &other) {
    size_t newSize = size_ + other.size_;
    char* newData = new char[newSize + 1];
    strcpy(newData, data_);
    strcat(newData, other.data_);
    delete[] data_;
    data_ = newData;
    size_ = newSize;
    return *this;
}

lxcd::string &lxcd::string::operator+=(const char *other) {
    size_t len = strlen(other);
    reserve(size_ + len);
    strcat(data_, other);
    size_ += len;
    return *this;
}

lxcd::string &lxcd::string::operator+=(const char other) {
    const char temp[2] = { other, '\0' };
    return (*this += temp);
}

lxcd::vector<lxcd::string> lxcd::string::split(char delimiter) const {
    lxcd::vector<string> tokens;
    size_t start = 0;
    for (size_t i = 0; i <= size_; ++i) {
        if (data_[i] == delimiter || data_[i] == '\0') {
            tokens.push_back(string(data_ + start, i - start));
            start = i + 1;
        }
    }
    return tokens;
}

lxcd::string &lxcd::string::erase(lxcd::size_t pos, lxcd::size_t count) {
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

lxcd::size_t lxcd::string::find(const lxcd::string &substr, lxcd::size_t pos) const {
    const char* result = strstr(data_ + pos, substr.data_);
    if (result == nullptr) {
        return npos;
    }
    return result - data_;
}
lxcd::size_t lxcd::string::find(const char subchar, lxcd::size_t pos) const {
    const char* result = strchr(data_ + pos, subchar);
    if (result == nullptr) {
        return npos;
    }
    return result - data_;
}

bool lxcd::string::contains(const char *str) const {
    return find(str) != npos;
}

bool lxcd::string::contains(const lxcd::string &str) const {
    return find(str) != npos;
}

bool lxcd::string::empty() const {
    return size_ == 0;
}

lxcd::size_t lxcd::string::length() const {
    return size_;
}

lxcd::size_t lxcd::string::rfind(const char *str, lxcd::size_t pos) const {
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

lxcd::string lxcd::string::substr(lxcd::size_t pos, lxcd::size_t len) const {
    if (pos > size_) {
        pos = size_;
    }
    if (len > size_ - pos) {
        len = size_ - pos;
    }
    return string(&data_[pos], len);
}

lxcd::string &lxcd::string::append(const char *str, lxcd::size_t len) {
    size_t new_size = size_ + len;
    char* new_data = new char[new_size + 1];
    strcpy(new_data, data_);
    strncat(new_data, str, len);
    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    return *this;
}

lxcd::string &lxcd::string::append(const lxcd::string &other) {
    size_t new_size = size_ + other.size_;
    char* new_data = new char[new_size + 1];
    strcpy(new_data, data_);
    strcat(new_data, other.data_);
    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    return *this;
}

lxcd::string &lxcd::string::append(const char *str) {
    return append(string(str));
}

void lxcd::string::insert(lxcd::size_t pos, const lxcd::string &str) {
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

void lxcd::string::insert(lxcd::size_t pos, const char *str) {
    insert(pos, string(str));
}

void lxcd::string::resize(lxcd::size_t n) {
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

void lxcd::string::reserve(lxcd::size_t n) { resize(n);}

lxcd::string::string(const char *str, lxcd::size_t size) : data_(new char[size + 1]), size_(size) {
    strncpy(data_, str, size);
    data_[size] = '\0';
}

double lxcd::string::stod(const lxcd::string &str) {
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

int lxcd::string::stoi(const lxcd::string &str) {
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
        if (!lxcd::string::isdigit(str[i])) {
            break;
        }
        res = res * 10 + (str[i] - '0');
    }

    return sign * res;
}

lxcd::string lxcd::string::to_string(int value) {
    char buffer[64];
    sprintf(buffer, "%d", value);
    return lxcd::string(buffer);
}

lxcd::string lxcd::string::to_string(double value) {
    char buffer[64];
    sprintf(buffer, "%.8g", value);
    return lxcd::string(buffer);
}

lxcd::string& lxcd::string::replace(lxcd::size_t pos, lxcd::size_t count, const char* str) {
    if (pos > size_) {
        //TODO change this to return : throw OutOfRangeException("pos out of range");
        return *this;
    }
    if (pos + count > size_) {
        count = size_ - pos;
    }
    size_t new_size = size_ - count + strlen(str);
    char* new_data = new char[new_size + 1];
    strncpy(new_data, data_, pos);
    strcpy(new_data + pos, str);
    strcpy(new_data + pos + strlen(str), data_ + pos + count);
    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    return *this;
}
