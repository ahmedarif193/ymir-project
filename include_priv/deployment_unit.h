#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <ctime>
#include <sstream>

#include <string.h>

#include <json/json.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include <lxc/lxccontainer.h>


//for  syscalls
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <ftw.h>
#define MAXPARAMLEN 1024

class DeploymentUnit {
public:
    DeploymentUnit(const std::string &uuid);
    bool install(const std::string& tarballPath, const std::string &executionEnvRef);
    bool remove();

    std::string squashfsPath;
    static std::string cacheFilePath;

    // DeploymentUnit parameters
    std::string uuid;
    std::string executionEnvRef;
    std::string description;
    std::string vendor;
    std::string type;
    std::string rootfsPath;
    int version;

    // IPK package names
    std::vector<std::string> ipkPackages;

    //metadata
    std::time_t installationDate;
};
