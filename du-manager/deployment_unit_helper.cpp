#include "deployment_unit_helper.h"

#define CACHE_PATH "/run/tr157.cache"
DeploymentUnitHelper::DeploymentUnitHelper() {
    loadCache();
}

std::shared_ptr<DeploymentUnit> DeploymentUnitHelper::getDeploymentUnit(const std::string& container) {
    // TODO: Implement getDeploymentUnit method
}


bool DeploymentUnitHelper::addDeploymentUnit(const std::string& container, const std::string& tarballPath, const std::string& uuid) {
    // Create a temporary directory for extraction
    std::string tempDir = "tmp_du_install";
    std::filesystem::create_directories(tempDir);

    // Extract the tar.gz file to the temporary directory
    std::string command = "tar -C " + tempDir + " -xzf " + tarballPath;
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to extract tarball" << std::endl;
        return false;
    }

    // Read the metadata.json file
    std::ifstream metadataFile(tempDir + "/metadata.json");
    if (!metadataFile.is_open()) {
        std::cerr << "Failed to open metadata.json" << std::endl;
        return false;
    }
    Json::Value metadata;
    metadataFile >> metadata;

    // Create a new DeploymentUnit instance
    std::shared_ptr<DeploymentUnit> du = std::make_shared<DeploymentUnit>(metadata, uuid);

    // Install the DeploymentUnit
    if (!du->install(tarballPath)) {
        std::cerr << "Failed to install DeploymentUnit" << std::endl;
        return false;
    }

    // Add the DeploymentUnit to the internal context
    deploymentUnits[uuid] = du;
    // Save the cache
    if (!saveCache()) {
        std::cerr << "Failed to save cache" << std::endl;
        return false;
    }


    return true;
}


bool DeploymentUnitHelper::removeDeploymentUnit(const std::string& uuid) {
    // Load the cache
    if (!loadCache()) {
        std::cerr << "Failed to load cache" << std::endl;
        return false;
    }

    // Find the DeploymentUnit in the internal context
    auto it = deploymentUnits.find(uuid);
    if (it != deploymentUnits.end()) {
        // Remove the DeploymentUnit
        if (!it->second->remove()) {
            std::cerr << "Failed to remove DeploymentUnit" << std::endl;
            return false;
        }

        // Remove the DeploymentUnit from the internal context
        deploymentUnits.erase(it);
    } else {
        std::cerr << "DeploymentUnit not found" << std::endl;
        return false;
    }

    // Save the cache
    if (!saveCache()) {
        std::cerr << "Failed to save cache" << std::endl;
        return false;
    }

    return true;
}


bool DeploymentUnitHelper::loadCache() {
    // Read cache from file
    std::ifstream cacheFile(CACHE_PATH);
    if (!cacheFile.is_open()) {
        std::cerr << "Failed to open cache file" << std::endl;
        return false;
    }

    // Parse the cache JSON
    Json::Value cache;
    cacheFile >> cache;

    // Load DeploymentUnits from cache
    for (const auto& duData : cache) {
        std::shared_ptr<DeploymentUnit> du = std::make_shared<DeploymentUnit>(duData, duData["UUID"].asString());
        du->executionEnvRef = duData["executionEnvRef"].asString();
        du->description = duData["Description"].asString();
        du->vendor = duData["Vendor"].asString();
        du->version = duData["Version"].asInt();

        // Load IPK package names
        for (const auto& ipkPackageName : duData["ipkPackages"]) {
            du->ipkPackages.push_back(ipkPackageName.asString());
        }

        // Load installation date
        du->installationDate = duData["installationDate"].asUInt64();

        deploymentUnits[du->uuid] = du;
    }

    return true;
}

bool DeploymentUnitHelper::saveCache() {
    Json::Value cache;

    // Serialize DeploymentUnits to cache
    for (const auto& entry : deploymentUnits) {
        Json::Value duData;
        duData["UUID"] = entry.second->uuid;
        duData["ExecutionEnvRef"] = entry.second->executionEnvRef;
        duData["Description"] = entry.second->description;
        duData["Vendor"] = entry.second->vendor;
        duData["Version"] = entry.second->version;

        // Serialize IPK package names
        for (const auto& ipkPackageName : entry.second->ipkPackages) {
            duData["ipkPackages"].append(ipkPackageName);
        }

        // Serialize installation date
        duData["installationDate"] = static_cast<Json::UInt64>(entry.second->installationDate);


        cache.append(duData);
    }

    // Write cache to file
    std::ofstream cacheFile(CACHE_PATH);
    if (!cacheFile.is_open()) {
        std::cerr << "Failed to open cache file" << std::endl;
        return false;
    }
    cacheFile << cache;
    cacheFile.close();

    return true;
}

void DeploymentUnitHelper::listDeploymentUnits() {
    Json::Value output(Json::arrayValue);

    for (const auto& entry : deploymentUnits) {
        std::shared_ptr<DeploymentUnit> du = entry.second;

        // Convert the installation date to a human-readable string
        std::time_t installationDate = du->installationDate;
        std::tm* localTime = std::localtime(&installationDate);
        char timeBuffer[256];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

        Json::Value duData;
        duData["UUID"] = du->uuid;
        duData["Container"] = du->executionEnvRef;
        duData["InstalledOn"] = timeBuffer;

        if (!du->ipkPackages.empty()) {
            for (const auto& ipkPackageName : du->ipkPackages) {
                duData["IPKPackages"].append(ipkPackageName);
            }
        }

        output.append(duData);
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  ";
    std::string jsonString = Json::writeString(writer, output);
    std::cout << "Installed DeploymentUnits:" << std::endl;
    std::cout << jsonString << std::endl;
}
