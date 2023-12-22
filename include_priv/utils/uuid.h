#ifndef UUID_H
#define UUID_H

#include <unistd.h>
#include <fcntl.h>

#include "string.h"

namespace lxcd {

class UUIDGenerator {
public:
UUIDGenerator() {
}
const static string generate() {
	// Generate a random UUID
	string data_;
	static const char* hexDigits = "0123456789abcdef";
	data_.resize(36);
	for(int i = 0; i < 36; i++) {
		if((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
			data_[i] = '-';
		} else if(i == 14) {
			data_[i] = '4';
		} else {

			int randomfd = open("/dev/random", O_RDONLY);
			if (randomfd < 0) {
				return "";
			}

			int randomNumber;
			ssize_t result = read(randomfd, &randomNumber, sizeof(randomNumber));
			if (result < 0) {
				close(randomfd);
				return "";
			}

			close(randomfd);

			data_[i] = hexDigits[randomNumber % 16];
		}
	}
	data_[8] = data_[13] = data_[18] = data_[23] = '-';
	return data_;
}
};
}
#endif
