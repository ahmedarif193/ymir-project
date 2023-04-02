#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "json/json.h"

#include <filesystem>
#include <uuid/uuid.h>
class DeploymentUnit {
public:
    DeploymentUnit(const Json::Value& metadata, const std::string& uuid);
    bool install(const std::string& tarballPath);
    bool remove();
    static std::vector<std::string> list();

    std::string squashfsPath;
    static std::string cacheFilePath;

    // DeploymentUnit parameters
    std::string uuid;
    std::string executionEnvRef;
    std::string description;
    std::string vendor;
    int version;

    // IPK package names
    std::vector<std::string> ipkPackages;

    //metadata
    std::time_t installationDate;
};
