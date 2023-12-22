#include "deployment_unit_helper.h"
#include "utils/linux.h"

#include <cstdio>

DeploymentUnitHelper::DeploymentUnitHelper() {
    loadCache();
}

lxcd::SharedPtr<DeploymentUnit> DeploymentUnitHelper::getDeploymentUnit(const lxcd::string& name) {
    // TODO: NOT USED YET Implement getDeploymentUnit method
    for(const auto& duPair : deploymentUnits) {
        const lxcd::SharedPtr<DeploymentUnit>& _du = duPair.value;
        printf("seaching for %s,\n",_du->name.c_str());
        if(_du->name == name) {
            // The current DeploymentUnit has the target version
            printf("Found DeploymentUnit with UUID: %s, with name: %s\n", _du->uuid.c_str(), _du->name.c_str());

            return _du;
        }
    }
    lxcd::SharedPtr<DeploymentUnit> du;
    return du;
}

struct lxc_container* DeploymentUnitHelper::getContainer(const lxcd::string& c) {
    auto lxcPath = getLxcPath();

    struct lxc_container* container = lxc_container_new("Container-4", lxcPath);
    if(!container) {
        printf("Error: Unable to create container object\n");
        return nullptr;
    }


    if(!container->load_config(container, nullptr)) {
        printf("Error: Unable to load container configuration\n");
        freeContainer(container);
        return nullptr;
    }
    return container;
}
//TODO : check size
lxcd::SharedPtr<DeploymentUnit> DeploymentUnitHelper::addDeploymentUnit(const lxcd::string& executionEnvRef, const lxcd::string& tarballPath, const lxcd::string& uuid) {
    bool needInstallation = true;
    bool installationSuccess = true;
    // Create a new DeploymentUnit instance
    lxcd::SharedPtr<DeploymentUnit> du = lxcd::makeShared<DeploymentUnit>(uuid);
    lxcd::SharedPtr<DeploymentUnit> existingDu;

    struct lxc_container* container = lxc_container_new(executionEnvRef.c_str(), NULL);
    if(!container) {
        printf("Failed to initialize the container\n");
        goto cleanup;
    }

    if(!container->is_defined(container)) {
        printf("Error: Container is not defined\n");
        goto cleanup;
    }
    // prepare the DeploymentUnit
    if(!du->prepare(tarballPath, executionEnvRef)) {
        printf("Failed to download/prepare DeploymentUnit\n");
        goto cleanup;
    }
    existingDu = getDeploymentUnit(du->name);
    //TODO validate using regex or raw checks in a separate function
    //TODO validate if URL or path maybe ?
    if(du->name.empty()) {
        printf("the name of the du tarball must exist, abord.\n");
        goto cleanup;
    }

    if(du->name.contains(" ")) {
        printf("the name of the du tarball must not contains a space, abord.\n");
        goto cleanup;
    }
    printf("TODO : cache desactivated, handle cache later\n");

//    if (existingDu) {
//        printf("Notice: Similar DeploymentUnit exists in cache.\n");
//        if (du->version != existingDu->version) {
//            printf("Action: installing DeploymentUnit to new version : %d, While keeping the old one : %d.\n", du->version, existingDu->version);
//        } else {
//            printf("Info: DeploymentUnit has same version : %d, updating info only.\n", du->version);
//            needInstallation = false;
//        }
//        du->uuid = existingDu->uuid;
//    }
    if (needInstallation) {
        printf("Action: Installing new DeploymentUnit.\n");
        installationSuccess = du->install();
        if (!installationSuccess) {
            printf("Failed to save cache, rolling back the installation ...\n");
            goto cleanup;
        }
        printf("Action: Install done.\n");
        lxcd::create_directories(du->rootfsPath);
        lxcd::mountSquashfs(du->rootfsPath + ".squashfs", du->rootfsPath);
        // Add the DeploymentUnit to the internal context
        deploymentUnits.insert({uuid, du});
    }
    // Save the cache
    if(!saveCache()) {
        printf("Failed to save cache, rolling back the installation ...\n");
        removeDeploymentUnit(du->uuid);
        goto cleanup;
    }else{
        printf("Action: context synced.\n");
    }
    if(!updateFullRootPath(container)) {
        printf("Error: Can't update lxc.rootfs.path, rolling back the installation ...\n");
        removeDeploymentUnit(du->uuid);
        return du;
    }else{
        printf("Action: Container's config updated.\n");
    }
cleanup:
    freeContainer(container);

    return du;
}
bool DeploymentUnitHelper::restartContainer(struct lxc_container* container) {
    // Check if the container is already running
    if(container->is_running(container)) {
        printf("Container is already running, restarting.\n");
        // Start the container
        if(!container->stop(container)) {
            printf("Failed to stop the container\n");
            freeContainer(container);
            return false;
        } else {
            printf("stopped the container\n");
        }
    }

    // Start the container
    if(!container->start(container, 0, NULL)) {
        printf("Failed to start the container, abord.\n");
        freeContainer(container);
        return false;
    } else {
        printf("container started\n");
    }
    return true;
}
//TODO fix ret in all functions
bool DeploymentUnitHelper::mount(struct lxc_container* container) {
    bool ret = false;
    for(const auto& entry : deploymentUnits) {
        if(entry.value->executionEnvRef == container->name) {
            printf("found DU with name : %s to be mounted in %s\n", entry.value->name.c_str() ,container->name);
            lxcd::mountSquashfs(entry.value->rootfsPath + ".squashfs", entry.value->rootfsPath);
            ret = true;
        }
    }
    return ret;
}
void DeploymentUnitHelper::freeContainer(struct lxc_container* container){
    if(container != NULL){
        lxc_container_put(container);
        container = NULL;
    }
}

bool DeploymentUnitHelper::updateFullRootPath(struct lxc_container* container) {
    bool success = false;

    int length = container->get_config_item(container, "lxc.rootfs.path", NULL, 0);
    if (length < 0) {
        printf("Failed to get the length of the current rootfs path\n");
        return false;
    }

    // Allocate memory for the old rootfs path
    char* oldRootfsPath = new char[length + 1];
    if (!container->get_config_item(container, "lxc.rootfs.path", oldRootfsPath, length + 1)) {
        printf("Failed to get the current rootfs path\n");
        delete[] oldRootfsPath;
        return false;
    }

    auto lxcPath = getLxcPath();
    auto initialRootfs = lxcPath + lxcd::string(container->name) + "/rootfs";
    auto initialOverlayDelta = lxcPath + lxcd::string(container->name) + "/overlay/delta";

    lxcd::string newRootfsBackend("overlay:");
    newRootfsBackend.append(initialRootfs);

    for (const auto& entry : deploymentUnits) {
        if (entry.value->executionEnvRef == container->name) {
            fprintf(stdout, "Installed Du found with name : %s\n", entry.value->name.c_str());
            newRootfsBackend.append(":");
            newRootfsBackend.append(entry.value->rootfsPath);
        }
    }

    newRootfsBackend.append(":");
    newRootfsBackend.append(initialOverlayDelta);

    container->clear_config(container);

    if (!container->load_config(container, NULL)) {
        printf("Failed to load config for container \n");
        goto cleanup;
    }

    container->clear_config_item(container, "lxc.rootfs.path");

    if (!container->set_config_item(container, "lxc.rootfs.path", newRootfsBackend.c_str())) {
        printf("Failed to set the new rootfs path\n");
        goto cleanup;
    }

    if (!container->save_config(container, NULL)) {
        printf("Failed to save the configuration to the file\n");
        goto cleanup;
    }

    success = restartContainer(container);

    if (!success) {
        printf("Failed to restart the container, restoring the old rootfs path %s\n", oldRootfsPath);
        container->set_config_item(container, "lxc.rootfs.path", oldRootfsPath);
        container->save_config(container, NULL);
    }

cleanup:
    delete[] oldRootfsPath;
    if (!success) {
        freeContainer(container);
    }
    return success;
}

bool DeploymentUnitHelper::removeDeploymentUnit(const lxcd::string& uuid) {
    bool ret = false;
    struct lxc_container* container = nullptr;

    for (auto it = deploymentUnits.begin(); it != deploymentUnits.end();) {
        if (uuid != it->value->uuid) {
            ++it;
            continue;
        }

        container = lxc_container_new(it->value->executionEnvRef.c_str(), NULL);
        if (!container) {
            printf("Failed to initialize the container\n");
            goto cleanup;
        }
        if (!container->is_defined(container)) {
            printf("Error: Container is not defined\n");
            goto cleanup;
        }
        if (!updateFullRootPath(container)) {
            printf("Can't handle updating lxc.rootfs.path, continue anyway\n");
        }
        if (!it->value->duRemove()) {
            printf("Failed to remove DeploymentUnit\n");
            goto cleanup;
        }
        deploymentUnits.erase(it++);
        if (!saveCache()) {
            printf("Failed to save cache\n");
            goto cleanup;
        }

        ret = true;
        goto cleanup; // Proceed to cleanup after successful removal
    }

cleanup:
    if (container = NULL) {
        freeContainer(container);
    }
    return ret;
}

bool DeploymentUnitHelper::loadCache() {
    // Read cache from file
    FILE* cacheFile = fopen(LXCD_DU_INDEX_PATH, "r");
    if(!cacheFile) {
        return true;
    }

    // Read cache file into a string
    lxcd::string cacheStr;
    char buffer[1024];
    size_t bytesRead = 0;
    while((bytesRead = fread(buffer, 1, sizeof(buffer), cacheFile)) > 0) {
        cacheStr.append(buffer, bytesRead);
    }

    // Parse the cache JSON
    struct json_object* json = json_tokener_parse(cacheStr.c_str());
    if(!json) {
        printf("Failed to parse cache file\n");
        fclose(cacheFile);
        return false;
    }

    // Load DeploymentUnits from cache
    int arrayLength = json_object_array_length(json);
    for(int i = 0; i < arrayLength; i++) {
        struct json_object* duData = json_object_array_get_idx(json, i);
        lxcd::SharedPtr<DeploymentUnit> du = lxcd::makeShared<DeploymentUnit>(json_object_get_string(json_object_object_get(duData, "UUID")));
        du->executionEnvRef = json_object_get_string(json_object_object_get(duData, "ExecutionEnvRef"));
        du->description = json_object_get_string(json_object_object_get(duData, "Description"));
        du->vendor = json_object_get_string(json_object_object_get(duData, "Vendor"));
        du->version = json_object_get_int(json_object_object_get(duData, "Version"));
        du->name = json_object_get_string(json_object_object_get(duData, "Name"));
        du->type = json_object_get_string(json_object_object_get(duData, "Type"));
        du->rootfsPath = json_object_get_string(json_object_object_get(duData, "RootfsPath"));

        // Load IPK package names
        struct json_object* ipkPackages = json_object_object_get(duData, "ipkPackages");
        int ipkArrayLength = json_object_array_length(ipkPackages);
        for(int j = 0; j < ipkArrayLength; j++) {
            struct json_object* ipkPackageName = json_object_array_get_idx(ipkPackages, j);
            du->ipkPackages.push_back(json_object_get_string(ipkPackageName));
        }

        // Load installation date
        du->installationDate = json_object_get_int64(json_object_object_get(duData, "InstallationDate"));
        // Load Services
        struct json_object* services = json_object_object_get(duData, "Service");
        if(services){
            int serviceArrayLength = json_object_array_length(services);
            for(int j = 0; j < serviceArrayLength; j++) {
                struct json_object* serviceData = json_object_array_get_idx(services, j);
                DeploymentUnit::eu serviceUnit;
                serviceUnit.name = json_object_get_string(json_object_object_get(serviceData, "Name"));
                serviceUnit.exec = json_object_get_string(json_object_object_get(serviceData, "Exec"));
                serviceUnit.pidfile = json_object_get_string(json_object_object_get(serviceData, "Pidfile"));
                serviceUnit.autostart = json_object_get_boolean(json_object_object_get(serviceData, "Autostart"));
                du->executionunits.push_back(serviceUnit);
            }
        }

        deploymentUnits.insert({du->uuid, du});
    }

    json_object_put(json);
    fclose(cacheFile);

    return true;
}


bool DeploymentUnitHelper::saveCache() {
    struct json_object* cache = json_object_new_array();

    // Serialize DeploymentUnits to cache
    for(const auto& entry : deploymentUnits) {
        struct json_object* duData = json_object_new_object();
        json_object_object_add(duData, "UUID", json_object_new_string(entry.value->uuid.c_str()));
        json_object_object_add(duData, "ExecutionEnvRef", json_object_new_string(entry.value->executionEnvRef.c_str()));
        json_object_object_add(duData, "Description", json_object_new_string(entry.value->description.c_str()));
        json_object_object_add(duData, "Vendor", json_object_new_string(entry.value->vendor.c_str()));
        json_object_object_add(duData, "Version", json_object_new_int(entry.value->version));
        json_object_object_add(duData, "Name", json_object_new_string(entry.value->name.c_str()));
        json_object_object_add(duData, "Type", json_object_new_string(entry.value->type.c_str()));
        json_object_object_add(duData, "RootfsPath", json_object_new_string(entry.value->rootfsPath.c_str()));

        // Serialize IPK package names
        struct json_object* ipkPackages = json_object_new_array();
        for(const auto& ipkPackageName : entry.value->ipkPackages) {
            json_object_array_add(ipkPackages, json_object_new_string(ipkPackageName.c_str()));
        }
        json_object_object_add(duData, "ipkPackages", ipkPackages);

        // Serialize installation date
        json_object_object_add(duData, "InstallationDate", json_object_new_int64(entry.value->installationDate));
        // Serialize Services
        struct json_object* services = json_object_new_array();
        if(services){
            for(const auto& serviceUnit : entry.value->executionunits) {
                struct json_object* serviceData = json_object_new_object();
                json_object_object_add(serviceData, "Name", json_object_new_string(serviceUnit.name.c_str()));
                json_object_object_add(serviceData, "Exec", json_object_new_string(serviceUnit.exec.c_str()));
                json_object_object_add(serviceData, "Pidfile", json_object_new_string(serviceUnit.pidfile.c_str()));
                json_object_object_add(serviceData, "Autostart", json_object_new_boolean(serviceUnit.autostart));
                json_object_array_add(services, serviceData);
            }
            json_object_object_add(duData, "Service", services);
            json_object_array_add(cache, duData);
        }
    }

    // Write cache to file
    int cacheFd = open(LXCD_DU_INDEX_PATH, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(cacheFd == -1) {
        perror("Failed to open cache file");
        return false;
    }

    lxcd::string jsonString = json_object_to_json_string_ext(cache, JSON_C_TO_STRING_PRETTY);

    ssize_t bytesWritten = write(cacheFd, jsonString.c_str(), jsonString.length());
    if(bytesWritten == -1) {
        perror("Failed to write cache file");
        close(cacheFd);
        return false;
    }

    close(cacheFd);
    json_object_put(cache);

    return true;
}

//TODO : check if time_t does belong to std
lxcd::string formatTime(time_t timeValue) {
    struct tm localTime;
    localtime_r(&timeValue, &localTime);
    char timeBuffer[256];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return timeBuffer;
}
//TODO : check if time_t does belong to std
void DeploymentUnitHelper::ls() {
    printf("\n%-36s %-20s %-15s %-10s %-20s %-15s\n",
           "UUID", "DU Name", "Container", "Size(MB)", "Installed On", "Type");
    printf("%-36s %-20s %-15s %-10s %-20s %-15s\n\n",
           "------------------------------------", "--------------------", "---------------", "----------", "--------------------", "---------------");

    for (const auto& entry : deploymentUnits) {
        lxcd::SharedPtr<DeploymentUnit> du = entry.value;

        lxcd::string installedOn = formatTime(du->installationDate);
        lxcd::string ipkPackagesStr = "";

        for (const auto& ipkPackageName : du->ipkPackages) {
            ipkPackagesStr += ipkPackageName + ", ";
        }

        // Trim the trailing comma and space
        if (!ipkPackagesStr.empty()) {
            ipkPackagesStr.erase(ipkPackagesStr.size() - 2); // remove last comma and space
        }

        // Convert size from bytes to MB and format to 2 decimal places
        int sizeInMB = lxcd::getFileSize(du->rootfsPath+".squashfs");
        printf("%-36s %-20s %-15s %-10d %-20s %-15s\n",
               du->uuid.c_str(),
               du->name.c_str(),
               du->executionEnvRef.c_str(),
               sizeInMB,
               installedOn.c_str(),
               du->type.c_str());
    }
}


