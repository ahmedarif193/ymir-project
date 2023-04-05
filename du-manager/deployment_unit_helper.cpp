#include "deployment_unit_helper.h"

#define CACHE_PATH "/run/tr157.cache"

DeploymentUnitHelper::DeploymentUnitHelper() {
    loadCache();
}

bool contains_space(const char *str) {
    if (!str) {
        return false;
    }

    for (const char *p = str; *p != '\0'; ++p) {
        if (*p == ' ') {
            return true;
        }
    }

    return false;
}

std::shared_ptr<DeploymentUnit> DeploymentUnitHelper::getDeploymentUnit(const std::string& uuid) {
    // TODO: Implement getDeploymentUnit method
    for (const auto& duPair : deploymentUnits) {
        const std::shared_ptr<DeploymentUnit>& _du = duPair.second;
        if (_du->uuid == uuid) {
            // The current DeploymentUnit has the target version
            std::cerr << "Found DeploymentUnit with UUID: " << _du->uuid<<", with name :  " << _du->name<< std::endl;
            return _du;
        }
    }
    return nullptr;
}

struct lxc_container* DeploymentUnitHelper::getContainer(const std::string& c){
    struct lxc_container* container = lxc_container_new("Container-4", "/var/lib/lxc/");
    if (!container) {
        std::cerr << "Error: Unable to create container object" << std::endl;
        return nullptr;
    }


    if (!container->load_config(container, nullptr)) {
        std::cerr << "Error: Unable to load container configuration" << std::endl;
        lxc_container_put(container);
        return nullptr;
    }
    return container;
}

bool DeploymentUnitHelper::addDeploymentUnit(const std::string& executionEnvRef, const std::string& tarballPath, const std::string& uuid) {

    struct lxc_container *container;

    // Initialize the container
    container = lxc_container_new(executionEnvRef.c_str(), NULL);
    if (!container) {
        fprintf(stderr, "Failed to initialize the container\n");
        return false;
    }

    if (!container->is_defined(container)) {
        std::cerr << "Error: Container is not defined" << std::endl;
        lxc_container_put(container);
        return false;
    }

    // Create a new DeploymentUnit instance
    std::shared_ptr<DeploymentUnit> du = std::make_shared<DeploymentUnit>(uuid);

    // prepare the DeploymentUnit
    if (!du->prepare(tarballPath, executionEnvRef)) {
        std::cerr << "Failed to install DeploymentUnit" << std::endl;
        lxc_container_put(container);
        return false;
    }

    for (const auto& duPair : deploymentUnits) {
        const std::shared_ptr<DeploymentUnit>& _du = duPair.second;
        if (_du->name == du->name) {
            std::cout << "Found DeploymentUnit with UUID: " << _du->uuid<<", with name :  " << _du->name<< std::endl;
            if(_du->version>=du->version){
                std::cerr << "Error : to upgrade the du you need to increment the version code atleast by 1, the current version is"<<du->version;
                std::cerr <<"the installed version is"<<_du->version<<" abord." << std::endl;
                lxc_container_put(container);
                return false;
            }else{
                if(removeDeploymentUnit(_du->uuid)){
                    std::cout <<"the old package us succesufly removed, continue the installation with the new one."<<std::endl;
                }else{
                    std::cerr <<"Fatal : can't uninstall the old package"<<std::endl;
                    lxc_container_put(container);
                    return false;
                }
            }
            break;
        }
    }

    //name must not contains spaces 
    if(contains_space(du->name.c_str())){
        printf("the name of the du tarball must not contains a space, abord.\n");
        lxc_container_put(container);
        return false;
    }

    //validate using regex
    //name version if > .... files etc..
    // Install the DeploymentUnit
    if (!du->install()) {
        std::cerr << "Failed to install DeploymentUnit" << std::endl;
        lxc_container_put(container);
        return false;
    }

    // Add the DeploymentUnit to the internal context
    deploymentUnits[uuid] = du;

    // Save the cache
    if (!saveCache()) {
        std::cerr << "Failed to save cache" << std::endl;
        lxc_container_put(container);
        return false;
    }

    // Get the lxc.rootfs.mount configuration item and store it in a std::string
    char lxcRootfsMount[MAXPARAMLEN];
    container->get_config_item(container, "lxc.rootfs.path", lxcRootfsMount, MAXPARAMLEN);
    if (strlen(lxcRootfsMount)) {
        std::string rootfsBackend(lxcRootfsMount);
        std::cerr << "rootfsBackend-b---- "<<rootfsBackend << std::endl;

        std::istringstream iss(rootfsBackend);
        std::vector<std::string> tokens;
        std::string token;

        while (std::getline(iss, token, ':')) {
            tokens.push_back(token);
        }

        if (!tokens.empty()) {
            tokens.insert(tokens.end() - 1, du->rootfsPath);

            std::ostringstream oss;
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (i > 0) {
                    oss << ":";
                }
                oss << tokens[i];
            }

            rootfsBackend = oss.str();

            rootfsBackend.insert(0,"overlay:");
            std::cerr << "rootfsBackend-a2---- "<<rootfsBackend << std::endl;

            // Check if the container is already running
            if (container->is_running(container)) {
                fprintf(stderr, "Container is already running\n");
            }

            container->clear_config(container);           

            if (!container->load_config(container, NULL)) {
                fprintf(stderr, "Failed to load config for container \n");
                lxc_container_put(container);
                return false;
            }

            container->clear_config_item(container, "lxc.rootfs.path");
            // Set the configuration item
            if (!container->set_config_item(container, "lxc.rootfs.path", rootfsBackend.c_str())) {
                fprintf(stderr, "Failed to set the configuration item\n");
                lxc_container_put(container);
                return false;
            }
            // Save the configuration to the file
            if (!container->save_config(container, NULL)) {
                fprintf(stderr, "Failed to save the configuration to the file\n");
                lxc_container_put(container);
                return false;
            }


            // Start the container
            if (!container->start(container, 0, NULL)) {
                fprintf(stderr, "-----------------------------------Failed to start the container\n");
                lxc_container_put(container);
                return false;
            }else{
                fprintf(stderr, "-----------------------------------ok\n");
            }
        }
    }

    lxc_container_put(container);

    return true;
}


bool DeploymentUnitHelper::removeDeploymentUnit(const std::string& uuid) {

    // Find the DeploymentUnit in the internal context
    auto it = deploymentUnits.find(uuid);
    if (it != deploymentUnits.end()) {

        struct lxc_container* container = this->getContainer(it->second->executionEnvRef);
        if(container == nullptr) return false;

        char lxcRootfsMount[MAXPARAMLEN];
        container->get_config_item(container, "lxc.rootfs.mount", lxcRootfsMount, MAXPARAMLEN);

        if (strlen(lxcRootfsMount)) {
            std::string rootfsBackend(lxcRootfsMount);
            std::string duRootfsPath = it->second->rootfsPath;

            // Find the position of the path to remove in the string
            std::size_t pathPos = rootfsBackend.find(duRootfsPath);
            if (pathPos != std::string::npos) {
                // Remove the path and the preceding colon
                if (pathPos > 0) {
                    pathPos--; // Move to the colon before the path
                }
                rootfsBackend.erase(pathPos, duRootfsPath.length() + 1);

                // Update the container configuration without the removed path
                container->set_config_item(container, "lxc.rootfs.mount", rootfsBackend.c_str());
            } else {
                std::cerr << "Failed to find the path to remove in lxc.rootfs.mount" << std::endl;
            }
        }
        lxc_container_put(container);

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
        return true;
    }

    // Parse the cache JSON
    Json::Value cache;
    cacheFile >> cache;

    // Load DeploymentUnits from cache
    for (const auto& duData : cache) {
        std::shared_ptr<DeploymentUnit> du = std::make_shared<DeploymentUnit>(duData["UUID"].asString());
        du->executionEnvRef = duData["ExecutionEnvRef"].asString();
        du->description = duData["Description"].asString();
        du->vendor = duData["Vendor"].asString();
        du->version = duData["Version"].asInt();
        du->name = duData["Name"].asString();
        du->type = duData["Type"].asString();
        du->rootfsPath = duData["RootfsPath"].asString();

        // Load IPK package names
        for (const auto& ipkPackageName : duData["ipkPackages"]) {
            du->ipkPackages.push_back(ipkPackageName.asString());
        }

        // Load installation date
        du->installationDate = duData["InstallationDate"].asUInt64();

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
        duData["Name"] = entry.second->name;
        duData["Type"] = entry.second->type;
        duData["RootfsPath"] = entry.second->rootfsPath;

        // Serialize IPK package names
        for (const auto& ipkPackageName : entry.second->ipkPackages) {
            duData["ipkPackages"].append(ipkPackageName);
        }

        // Serialize installation date
        duData["InstallationDate"] = static_cast<Json::UInt64>(entry.second->installationDate);


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
