#include "deployment_unit_helper.h"
#include "utils/linux.h"

#define CACHE_PATH "/run/tr157.cache"

#include <cstdio>

bool parse_lxcd_info(const lxcd::string &json_str, LxcdInfo &info) {
    json_object *root_obj = json_tokener_parse(json_str.c_str());

    if (!root_obj) {
        fprintf(stderr, "Error parsing JSON string\n");;
        return false;
    }

    json_object *uuid_obj, *executionEnvRef_obj, *url_obj, *user_obj, *password_obj;

    if (json_object_object_get_ex(root_obj, "uuid", &uuid_obj) &&
            json_object_object_get_ex(root_obj, "executionEnvRef", &executionEnvRef_obj) &&
            json_object_object_get_ex(root_obj, "url", &url_obj) &&
            json_object_object_get_ex(root_obj, "user", &user_obj) &&
            json_object_object_get_ex(root_obj, "password", &password_obj)) {

        info.uuid = json_object_get_string(uuid_obj);
        info.executionEnvRef = json_object_get_string(executionEnvRef_obj);
        info.url = json_object_get_string(url_obj);
        info.user = json_object_get_string(user_obj);
        info.password = json_object_get_string(password_obj);

        json_object_put(root_obj);
        return true;
    }

    fprintf(stderr, "Error retrieving JSON values\n");;
    json_object_put(root_obj);
    return false;
}

DeploymentUnitHelper::DeploymentUnitHelper() {
    loadCache();
}

bool contains_space(const char* str) {
    if(!str) {
        return false;
    }

    for(const char* p = str; *p != '\0'; ++p) {
        if(*p == ' ') {
            return true;
        }
    }

    return false;
}

lxcd::SharedPtr<DeploymentUnit> DeploymentUnitHelper::getDeploymentUnit(const lxcd::string& uuid) {
    // TODO: Implement getDeploymentUnit method
    for(const auto& duPair : deploymentUnits) {
        const lxcd::SharedPtr<DeploymentUnit>& _du = duPair.value;
        if(_du->uuid == uuid) {
            // The current DeploymentUnit has the target version
            fprintf(stderr, "Found DeploymentUnit with UUID: %s, with name: %s\n", _du->uuid.c_str(), _du->name.c_str());

            return _du;
        }
    }
    return nullptr;
}

struct lxc_container* DeploymentUnitHelper::getContainer(const lxcd::string& c) {
    auto lxcPath = getLxcPath();

    struct lxc_container* container = lxc_container_new("Container-4", lxcPath);
    if(!container) {
        fprintf(stderr, "Error: Unable to create container object\n");
        return nullptr;
    }


    if(!container->load_config(container, nullptr)) {
        fprintf(stderr, "Error: Unable to load container configuration\n");
        lxc_container_put(container);
        return nullptr;
    }
    return container;
}

lxcd::pair<lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit>>::Iterator, bool> DeploymentUnitHelper::addDeploymentUnit(const lxcd::string& executionEnvRef, const lxcd::string& tarballPath, const lxcd::string& uuid) {
    lxcd::pair<lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit>>::Iterator, bool> ret;
    struct lxc_container* container = lxc_container_new(executionEnvRef.c_str(), NULL);
    if(!container) {
        fprintf(stderr, "Failed to initialize the container\n");
        ret.value = false;
        return ret;
    }

    if(!container->is_defined(container)) {
        fprintf(stderr, "Error: Container is not defined\n");
        lxc_container_put(container);
        ret.value = false;
        return ret;
    }

    // Create a new DeploymentUnit instance
    lxcd::SharedPtr<DeploymentUnit> du = lxcd::makeShared<DeploymentUnit>(uuid);

    // prepare the DeploymentUnit
    if(!du->prepare(tarballPath, executionEnvRef)) {
        fprintf(stderr, "Failed to download/prepare DeploymentUnit\n");
        lxc_container_put(container);
        ret.value = false;
        return ret;
    }

    for(const auto& duPair : deploymentUnits) {
        const lxcd::SharedPtr<DeploymentUnit>& _du = duPair.value;
        if(_du->name == du->name) {
            printf("Found DeploymentUnit with UUID: %s, with name: %s\n", _du->uuid.c_str(), _du->name.c_str());

            if(_du->version > du->version) {  //_du->version == du->version<=
                printf("1\n");
                printf("Error: to upgrade the du you need to increment the version code at least by 1, the current version is %d, the installed version is %d, abort.\n", du->version, _du->version);
                lxc_container_put(container);
                ret.value = false;
                return ret;
            } else {
                printf("removing the old DU ...\n");
                if(removeDeploymentUnit(_du->uuid)) {
                    printf("3\n");
                    printf("The old package is successfully removed, continue the installation with the new one.\n");
                } else {
                    printf("4\n");
                    printf("Fatal: can't uninstall the old package.\n");
                    lxc_container_put(container);
                    ret.value = false;
                    return ret;
                }
            }
            break;
        }
    }
    //name must not contains spaces
    if(contains_space(du->name.c_str())) {
        printf("the name of the du tarball must not contains a space, abord.\n");
        lxc_container_put(container);
        ret.value = false;
        return ret;
    }

    //validate using regex
    //name version if > .... files etc..
    // Install the DeploymentUnit
    if(!du->install()) {
        fprintf(stderr, "Failed to install DeploymentUnit\n");
        lxc_container_put(container);
        ret.value = false;
        return ret;
    }

    lxcd::mountSquashfs(du->rootfsPath + ".squashfs", du->rootfsPath);

    // Add the DeploymentUnit to the internal context
    ret = deploymentUnits.insert({uuid, du});

    // Save the cache
    if(!saveCache()) {
        fprintf(stderr, "Failed to save cache, rolling back the installation ...\n");
        removeDeploymentUnit(du->uuid);
        lxc_container_put(container);
        ret.value = false;
        return ret;
    }

    if(!updateFullRootPath(container)) {
        fprintf(stderr, "Can't handle updating lxc.rootfs.path, rolling back the installation ...\n");
        removeDeploymentUnit(du->uuid);
        ret.value = false;
        return ret;
    }

    lxc_container_put(container);

    return ret;
}
bool DeploymentUnitHelper::restartContainer(struct lxc_container* container) {
    // Check if the container is already running
    if(container->is_running(container)) {
        printf("Container is already running, restarting.\n");
        // Start the container
        if(!container->stop(container)) {
            printf("Failed to stop the container\n");
            lxc_container_put(container);
            return false;
        } else {
            printf("stopped the container\n");
        }
    }

    // Start the container
    if(!container->start(container, 0, NULL)) {
        printf("Failed to start the container, abord.\n");
        lxc_container_put(container);
        return false;
    } else {
        printf("container started\n");
    }
    return true;
}

bool DeploymentUnitHelper::updateFullRootPath(struct lxc_container* container) {
    auto lxcPath = getLxcPath();
    auto initialRootfs = lxcPath + "/" + lxcd::string(container->name) + "/rootfs";
    auto initialOverlayDelta = lxcPath + "/" + lxcd::string(container->name) + "/overlay/delta";

    lxcd::string newRootfsBackend("overlay:");
    newRootfsBackend.append(initialRootfs);

    for(const auto& entry : deploymentUnits) {
        if(entry.value->executionEnvRef == container->name) {
            fprintf(stdout, "%s found\n", entry.value->name.c_str());
            newRootfsBackend.append(":");
            newRootfsBackend.append(entry.value->rootfsPath);
        }
    }
    newRootfsBackend.append(":");
    newRootfsBackend.append(initialOverlayDelta);
    fprintf(stdout, "newRootfsBackend %s\n", newRootfsBackend.c_str());

    container->clear_config(container);

    if(!container->load_config(container, NULL)) {
        fprintf(stderr, "Failed to load config for container \n");
        lxc_container_put(container);
        return false;
    }
    container->clear_config_item(container, "lxc.rootfs.path");
    // Set the configuration item
    if(!container->set_config_item(container, "lxc.rootfs.path", newRootfsBackend.c_str())) {
        fprintf(stderr, "Failed to set the configuration item\n");
        lxc_container_put(container);
        return false;
    }
    // Save the configuration to the file
    if(!container->save_config(container, NULL)) {
        fprintf(stderr, "Failed to save the configuration to the file\n");
        lxc_container_put(container);
        return false;
    }

    return restartContainer(container);
}

bool DeploymentUnitHelper::removeDeploymentUnit(const lxcd::string& uuid) {
    for(const auto& entry : deploymentUnits) {
        if(uuid == entry.value->uuid) {
            struct lxc_container* container = lxc_container_new(entry.value->executionEnvRef, NULL);
            if(!container) {
                fprintf(stderr, "Failed to initialize the container\n");
                return false;
            }

            if(!container->is_defined(container)) {
                fprintf(stderr, "Error: Container is not defined\n");
                lxc_container_put(container);
                return false;
            }
            lxcd::umountSquashfs(entry.value->rootfsPath);
            if(!entry.value->remove()) {
                fprintf(stderr, "Failed to remove DeploymentUnit\n");
                return false;
            }
            deploymentUnits.erase(entry.value->uuid);

            if(!updateFullRootPath(container)) {
                fprintf(stderr, "Can't handle updating lxc.rootfs.path, abord. \n");
                return false;
            }

            lxc_container_put(container);
            // Save the cache
            if(!saveCache()) {
                fprintf(stderr, "Failed to save cache\n");
                return false;
            }

            return true;
        }
    }

    return true;
}

bool DeploymentUnitHelper::loadCache() {
    // Read cache from file
    FILE* cacheFile = fopen(CACHE_PATH, "r");
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
        fprintf(stderr, "Failed to parse cache file\n");
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

        json_object_array_add(cache, duData);
    }

    // Write cache to file
    int cacheFd = open(CACHE_PATH, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
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

lxcd::string DeploymentUnitHelper::listDeploymentUnits() {
    json_object* root = json_object_new_object();
    json_object* rootdu = json_object_new_array();

    for(const auto& entry : deploymentUnits) {
        lxcd::SharedPtr<DeploymentUnit> du = entry.value;

        lxcd::string installedOn = formatTime(du->installationDate);

        struct json_object* duData = json_object_new_object();
        json_object_object_add(duData, "UUID", json_object_new_string(du->uuid.c_str()));
        json_object_object_add(duData, "Container", json_object_new_string(du->executionEnvRef.c_str()));
        json_object_object_add(duData, "InstalledOn", json_object_new_string(installedOn.c_str()));

        if(!du->ipkPackages.empty()) {
            struct json_object* ipkPackages = json_object_new_array();
            for(const auto& ipkPackageName : du->ipkPackages) {
                json_object_array_add(ipkPackages, json_object_new_string(ipkPackageName.c_str()));
            }
            json_object_object_add(duData, "IPKPackages", ipkPackages);
        }

        json_object_array_add(rootdu, duData);
    }
    json_object_object_add(root, "deploymentUnits", rootdu);
    json_object_object_add(root, "numberOfDeploymentUnits", json_object_new_int(deploymentUnits.size()));
    lxcd::string jsonString = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    printf("Installed DeploymentUnits:\n%s\n", jsonString.c_str());

    json_object_put(root);
    return  jsonString;
}
