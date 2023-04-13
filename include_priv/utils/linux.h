#ifndef LINUX_H
#define LINUX_H

#include "string.h"
#include "vector.h"
#include "map.h"
#include <stdio.h>
namespace lxcd {

#if __WORDSIZE == 64
typedef unsigned long long time_t;
#else
typedef unsigned long time_t;
#endif

class RuntimeError {
public:
    // Constructor that takes a const char pointer as error message
    RuntimeError(const char* errMsg) {
        int len = 0;
        while(errMsg[len] != '\0') {
            len++;
        }
        len++;     // For null character

        message = new char[len];
        for(int i = 0; i < len; i++) {
            message[i] = errMsg[i];
        }
    }

    // Copy constructor
    RuntimeError(const RuntimeError& other) {
        int len = 0;
        while(other.message[len] != '\0') {
            len++;
        }
        len++;     // For null character

        message = new char[len];
        for(int i = 0; i < len; i++) {
            message[i] = other.message[i];
        }
    }

    // Destructor
    ~RuntimeError() {
        delete[] message;
    }

    // Accessor for the error message
    const char* what() const {
        return message;
    }

private:
    char* message;
};

inline lxcd::string exec(lxcd::string cmd, int &retcode) {
    printf("execute the cmd %s", cmd.c_str());
    lxcd::string result = "";
    char buffer[128];
    FILE* pipe = popen(cmd.c_str(), "r");
    if(!pipe) {
        throw lxcd::runtimeErrorException("popen() failed!");
    }
    try {
        while(fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch(...) {
        pclose(pipe);
        throw;
    }
    retcode = WEXITSTATUS(pclose(pipe));
    return result;
}

struct mount {
    string device;
    string mountPoint;
    string fsType;
    string options;

    mount() = default;

    mount(const string& device, const string& mountPoint,
          const string& fsType, const string& options)
        : device(device), mountPoint(mountPoint), fsType(fsType), options(options) {
    }
};

inline vector<mount> parseProcMounts() {
    FILE* fp = fopen("/proc/mounts", "r");
    if(!fp) {
        //throw RuntimeError("failed to open /proc/mounts");
    }

    vector<mount> entries;

    while(true) {
        // Read the next line from the file
        char buf[1024];
        if(!fgets(buf, sizeof(buf), fp)) {
            break;
        }

        // Split the line into its components
        vector<string> tokens;
        string line(buf);
        size_t pos = 0;
        while(true) {
            size_t nextPos = line.find(" ", pos);
            if(nextPos == string::npos) {
                tokens.push_back(line.substr(pos));
                break;
            } else {
                tokens.push_back(line.substr(pos, nextPos - pos));
                pos = nextPos + 1;
            }
        }

        // Create a mount object from the tokens
        if(tokens.size() >= 4) {
            mount entry(tokens[0], tokens[1], tokens[2], tokens[3]);
            entries.push_back(entry);
        }
    }

    fclose(fp);

    return entries;
}

inline vector<string> getProcMountPoints() {
    auto mounts = parseProcMounts();
    vector<string> mountPoints;
    for(auto it = mounts.begin(); it != mounts.end(); ++it) {
        mountPoints.push_back((*it).mountPoint);
    }
    return mountPoints;
}

inline bool isMountExist(const string& mountPoint) {
    auto mounts = getProcMountPoints();
    for(auto it = mounts.begin(); it != mounts.end(); ++it) {
        if(*it == mountPoint) {
            fprintf(stderr, "Already mounted, return %s\n", (*it).c_str());
            return true;
        }
    }
    return false;
}
inline void mountSquashfs(const string& path, const string& mountPoint) {
    if(!isMountExist(mountPoint)) {
        // Not mounted, mount it
        string command = "mount -t squashfs " + path + " " + mountPoint;
        system(command.c_str());
    }

}
inline void umountSquashfs(const string& mountPoint) {
    if(isMountExist(mountPoint)) {
        // Not mounted, mount it
        string command = "umount " + mountPoint;
        system(command.c_str());
    }

}
class Process {
public:
    Process(const string& cmd) : cmd_(cmd) {
    }

    int run() {
        return system(cmd_.c_str());
    }

private:
    string cmd_;
};
// class Filesystem {
// public:
//     static bool umount(const string& mountPoint) {
//         vector<string> args;
//         args.push_back("umount");
//         args.push_back(mountPoint);
//         Process p(buildCommand(args));
//         return p.run() == 0;
//     }

// private:
//     static string buildCommand(const vector<string>& args) {
//         string cmd;
//         for (auto it = args.begin(); it != args.end(); ++it) {
//             cmd += *it;
//             cmd += " ";
//         }
//         return cmd;
//     }
// };

} // namespace lxcd

#endif
