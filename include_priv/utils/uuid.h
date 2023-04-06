#include <ctime>
#include "string.h"

namespace lxcd {

int rand() {
    static unsigned int seed = time(nullptr);
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

class UUIDGenerator {
public:
    UUIDGenerator() {
    }
    const static string generate() {
        // Generate a random UUID
        string data_;
        static const char* hexDigits = "0123456789abcdef";
        data_.resize(36);
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                data_[i] = '-';
            } else if (i == 14) {
                data_[i] = '4';
            } else {
                int randValue = rand() % 16;
                data_[i] = hexDigits[randValue];
            }
        }
        data_[8] = data_[13] = data_[18] = data_[23] = '-';
        return data_;
    }
};
}
