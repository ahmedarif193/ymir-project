#include "deployment_unit_helper.h"
#include "utils/linux.h"

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

std::shared_ptr<DeploymentUnit> DeploymentUnitHelper::getDeploymentUnit(const lxcd::string& uuid) {
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

struct lxc_container* DeploymentUnitHelper::getContainer(const lxcd::string& c){
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

bool DeploymentUnitHelper::addDeploymentUnit(const lxcd::string& executionEnvRef, const lxcd::string& tarballPath, const lxcd::string& uuid) {
    
    struct lxc_container *container = lxc_container_new(executionEnvRef.c_str(), NULL);
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
            if(_du->version > du->version){//_du->version==du->version
                std::cerr << "Error : to upgrade the du you need to increment the version code atleast by 1, the current version is "<<du->version;
                std::cerr <<"the installed version is "<<_du->version<<", abord." << std::endl;
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

    lxcd::mountSquashfs(du->rootfsPath + ".squashfs",du->rootfsPath);

    // Add the DeploymentUnit to the internal context
    deploymentUnits[uuid] = du;

    // Save the cache
    if (!saveCache()) {
        std::cerr << "Failed to save cache" << std::endl;
        lxc_container_put(container);
        return false;
    }

    if(!updateFullRootPath(container)){
        fprintf(stderr, "Can't handle updating lxc.rootfs.path, abord. \n");
        return false;
    };
    
    lxc_container_put(container);

    return true;
}
bool DeploymentUnitHelper::restartContainer(struct lxc_container *container){
    // Check if the container is already running
    if (container->is_running(container)) {
        printf("Container is already running, restarting.\n");
        // Start the container
        if (!container->stop(container)) {
            printf("Failed to stop the container\n");
            lxc_container_put(container);
            return false;
        }else{
            printf("stopped the container\n");
        }
    }

    // Start the container
    if (!container->start(container, 0, NULL)) {
        printf("Failed to start the container, abord.\n");
        lxc_container_put(container);
        return false;
    }else{
        printf("container started\n");
    }
    return true;
}

bool DeploymentUnitHelper::updateFullRootPath(struct lxc_container *container){
    auto lxcPath = getLxcPath();
    auto initialRootfs = lxcPath + "/" + lxcd::string(container->name) + "/rootfs";
    auto initialOverlayDelta = lxcPath + "/" + lxcd::string(container->name) + "/overlay/delta";

    lxcd::string newRootfsBackend("overlay:");
    newRootfsBackend.append(initialRootfs);

    for (const auto& entry : deploymentUnits) {
        if(entry.second->executionEnvRef == container->name) {
            std::cout << entry.second->name << " found "<<std::endl;
            newRootfsBackend.append(":");
            newRootfsBackend.append(entry.second->rootfsPath);
        }
    }
    newRootfsBackend.append(":");
    newRootfsBackend.append(initialOverlayDelta);
    std::cout << "newRootfsBackend "<< newRootfsBackend << std::endl;

    container->clear_config(container);

    if (!container->load_config(container, NULL)) {
        fprintf(stderr, "Failed to load config for container \n");
        lxc_container_put(container);
        return false;
    }
    container->clear_config_item(container, "lxc.rootfs.path");
    // Set the configuration item
    if (!container->set_config_item(container, "lxc.rootfs.path", newRootfsBackend.c_str())) {
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
    
    return restartContainer(container);
}

bool DeploymentUnitHelper::removeDeploymentUnit(const lxcd::string& uuid) {
    for (const auto& entry : deploymentUnits) {
        //std::cerr << "it ---------> "<<*it << std::endl;
        if (uuid == entry.second->uuid) {

            struct lxc_container *container = lxc_container_new(entry.second->executionEnvRef, NULL);
            if (!container) {
                fprintf(stderr, "Failed to initialize the container\n");
                return false;
            }

            if (!container->is_defined(container)) {
                std::cerr << "Error: Container is not defined" << std::endl;
                lxc_container_put(container);
                return false;
            }
            if(!updateFullRootPath(container)){
                fprintf(stderr, "Can't handle updating lxc.rootfs.path, abord. \n");
                return false;
            };
            lxcd::umountSquashfs(entry.second->rootfsPath);
            lxc_container_put(container);
            if (entry.second->remove()) {
                std::cerr << "Failed to remove DeploymentUnit" << std::endl;
                return false;
            }
            deploymentUnits.erase(entry);
            return true;
        }
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
        std::shared_ptr<DeploymentUnit> du = std::make_shared<DeploymentUnit>(duData["UUID"].asString().c_str());
        du->executionEnvRef = duData["ExecutionEnvRef"].asString().c_str();
        du->description = duData["Description"].asString().c_str();
        du->vendor = duData["Vendor"].asString().c_str();
        du->version = duData["Version"].asInt();
        du->name = duData["Name"].asString().c_str();
        du->type = duData["Type"].asString().c_str();
        du->rootfsPath = duData["RootfsPath"].asString().c_str();

        // Load IPK package names
        for (const auto& ipkPackageName : duData["ipkPackages"]) {
            du->ipkPackages.push_back(ipkPackageName.asString().c_str());
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
        duData["UUID"] = entry.second->uuid.c_str();
        duData["ExecutionEnvRef"] = entry.second->executionEnvRef.c_str();
        duData["Description"] = entry.second->description.c_str();
        duData["Vendor"] = entry.second->vendor.c_str();
        duData["Version"] = entry.second->version;
        duData["Name"] = entry.second->name.c_str();
        duData["Type"] = entry.second->type.c_str();
        duData["RootfsPath"] = entry.second->rootfsPath.c_str();

        // Serialize IPK package names
        for (const auto& ipkPackageName : entry.second->ipkPackages) {
            duData["ipkPackages"].append(ipkPackageName.c_str());
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
        duData["UUID"] = du->uuid.c_str();
        duData["Container"] = du->executionEnvRef.c_str();
        duData["InstalledOn"] = timeBuffer;

        if (!du->ipkPackages.empty()) {
            for (const auto& ipkPackageName : du->ipkPackages) {
                duData["IPKPackages"].append(ipkPackageName.c_str());
            }
        }

        output.append(duData);
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  ";
    lxcd::string jsonString = Json::writeString(writer, output).c_str();
    std::cout << "Installed DeploymentUnits:" << std::endl;
    std::cout << jsonString << std::endl;
}
