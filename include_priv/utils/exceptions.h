#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include "string.h" // assume the vector class is defined in a separate header file

namespace lxcd {
class OutOfRangeException {
public:
explicit OutOfRangeException(const string& message) : message_(message) {
}

virtual const char* what() const noexcept {
    return message_.c_str();
}

private:
string message_;
};

}

#endif // EXCEPTIONS_H
