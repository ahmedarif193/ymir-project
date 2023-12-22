#include "deployment_unit.h"


lxcd::string DeploymentUnit::cacheFilePath = "deployment_units_cache.json";
lxcd::string DeploymentUnit::tempDir = "/tmp/tr157-du";


DeploymentUnit::DeploymentUnit(const lxcd::string &uuid) {
    this->uuid = uuid;
}

// Helper function for libcurl
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((lxcd::string*) userp)->append((char*) contents, size * nmemb);
    return size * nmemb;
}

bool copy_file(const lxcd::string& src, const lxcd::string& dst) {
    int src_fd = open(src.c_str(), O_RDONLY);
    if(src_fd == -1) {
        perror("Failed to open source file");
        return false;
    }

    struct stat src_stat;
    if(fstat(src_fd, &src_stat) == -1) {
        perror("Failed to stat source file");
        close(src_fd);
        return false;
    }

    int dst_fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if(dst_fd == -1) {
        perror("Failed to open destination file");
        close(src_fd);
        return false;
    }

    off_t offset = 0;
    ssize_t sent_bytes;
    while((sent_bytes = sendfile(dst_fd, src_fd, &offset, src_stat.st_size)) > 0) {
        src_stat.st_size -= sent_bytes;
    }

    close(src_fd);
    close(dst_fd);

    if(sent_bytes == -1) {
        perror("Failed to copy file using sendfile");
        return false;
    }

    return true;
}
int remove_entry(const char* path, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    int rv = remove(path);
    if(rv) {
        perror("Failed to remove");
    }
    return rv;
}

bool remove_directory(const lxcd::string &dir) {
    int flags = FTW_DEPTH | FTW_PHYS;
    int rv = nftw(dir.c_str(), remove_entry, 64, flags);
    if(rv) {
        perror("Failed to remove directory");
        return false;
    }
    return true;
}

bool DeploymentUnit::prepare(const lxcd::string& tarballPath, const lxcd::string& executionEnvRef) {
    // Store the installation date
    //CHECK IF STD still exist
    installationDate = time(NULL);

    // Check if the tarballPath is a URL
    lxcd::string downloadedFilePath;
    if((tarballPath.find("http://") == 0) || (tarballPath.find("https://") == 0)) {
        CURL* curl;
        CURLcode res;
        lxcd::string fileContents;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, tarballPath.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileContents);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                printf("Failed to download the tarball from URL: %s\n", tarballPath.c_str());
                curl_easy_cleanup(curl);
                return false;
            }
            curl_easy_cleanup(curl);

            // Save the downloaded file to a temporary directory
            lxcd::string fileName = "downloaded_tarball.tar.gz";
            FILE* outputFile = fopen(fileName.c_str(), "wb");
            if(!outputFile) {
                perror("Failed to open output file");
                return false;
            }
            size_t bytesWritten = fwrite(fileContents.c_str(), 1, fileContents.size(), outputFile);
            if(bytesWritten != fileContents.size()) {
                perror("Failed to write output file");
                fclose(outputFile);
                return false;
            }
            fclose(outputFile);
            downloadedFilePath = fileName;
        }
        curl_global_cleanup();
    }

    // Use the downloaded file path if it exists, otherwise use the original tarballPath
    lxcd::string pathToUse = downloadedFilePath.empty() ? tarballPath : downloadedFilePath;
    create_directories(tempDir);

    // Extract the tar.gz file to the temporary directory
    lxcd::string command = "tar -C " + tempDir + " -xzf " + pathToUse;
    lxcd::Process process(command);
    int result = process.run();
    if(result != 0) {
        printf("Failed to extract tarball\n");
        return false;
    }


    // Read the metadata.json file
    FILE* metadataFile = fopen((tempDir + "/metadata.json").c_str(), "r");
    if(!metadataFile) {
        printf("metadata.json is nto present\n");
        return false;
    }
    fseek(metadataFile, 0, SEEK_END);
    long fileSize = ftell(metadataFile);
    fseek(metadataFile, 0, SEEK_SET);

    char* metadataBuffer = (char*) malloc(fileSize + 1);
    if(!metadataBuffer) {
        printf("Failed to allocate memory for metadata buffer\n");
        fclose(metadataFile);
        return false;
    }

    if(fread(metadataBuffer, 1, fileSize, metadataFile) != fileSize) {
        printf("Failed to read metadata.json\n");
        free(metadataBuffer);
        fclose(metadataFile);
        return false;
    }
    metadataBuffer[fileSize] = '\0';

    json_object* metadata = json_tokener_parse(metadataBuffer);
    if(!metadata) {
        printf("Failed to parse metadata.json\n");
        free(metadataBuffer);
        fclose(metadataFile);
        return false;
    }

    free(metadataBuffer);
    fclose(metadataFile);

    lxcd::string lxcPath = LXCD_PATH;
    if(lxcPath.empty()) {
        printf("fatal : LXC path not found.\n");
        return false;
    }


    this->rootfsPath = "/opt/lxcd/deploymentunits/" + uuid;

    // Load Services
    this->executionEnvRef = executionEnvRef;
    this->description = JsonHelper::GetString(metadata, "Description");
    this->vendor = JsonHelper::GetString(metadata, "Vendor");
    this->version = JsonHelper::GetInt(metadata, "Version");
    this->type = JsonHelper::GetString(metadata, "Type");
    this->name = JsonHelper::GetString(metadata, "Name");


    printf("\n\n-----------------DU global informations : -------\n\n");
    printf("container's name : %s\n", this->executionEnvRef.c_str());
    printf("description : %s\n", this->description.c_str());
    printf("vendor : %s\n", this->vendor.c_str());
    printf("type : %s\n", this->type.c_str());
    printf("tarball name : %s\n", this->name.c_str());
    printf("Assigned UUID : %s\n", uuid.c_str());
    printf("\n\n-------------------------------------------------\n\n");

    // Load Services
    struct json_object* services = JsonHelper::GetJsonArray(metadata, "Service");
    if(services){
        int serviceArrayLength = json_object_array_length(services);
        for(int j = 0; j < serviceArrayLength; j++) {
            struct json_object* serviceData = json_object_array_get_idx(services, j);
            DeploymentUnit::eu serviceUnit;
            serviceUnit.name = JsonHelper::GetString(serviceData, "Name");
            serviceUnit.exec = JsonHelper::GetString(serviceData, "Exec");
            printf("PREPARE serviceUnit.exec %s\n", serviceUnit.exec.c_str());

            serviceUnit.pidfile = JsonHelper::GetString(serviceData, "Pidfile");
            serviceUnit.autostart = JsonHelper::GetBoolean(serviceData, "Autostart");
            this->executionunits.push_back(serviceUnit);
        }
    }
    return true;

}
bool DeploymentUnit::install() {

    create_directories(rootfsPath);

    if(this->type == "squashfs") {
        copy_file(tempDir + "/rootfs.squashfs", rootfsPath + ".squashfs");
    } else if(this->type == "ipk") {

        DIR* dir;
        struct dirent* entry;

        dir = opendir(tempDir.c_str());
        if(dir == nullptr) {
            printf("Failed to open directory: %s\n", tempDir.c_str());
        } else {
            // Store the names of installed IPK packages
            ipkPackages.clear();
            while((entry = readdir(dir)) != nullptr) {
                // Check if the file has an .ipk extension
                lxcd::string fileName(entry->d_name);
                lxcd::size_t extPos = fileName.rfind(".ipk");
                if((extPos != lxcd::string::npos) && (fileName.size() - extPos == 4)) {
                    // Install the IPK file to the rootfs directory
                    lxcd::string filePath = tempDir + "/" + fileName;
                    lxcd::string command = "opkg install --force-depends --force-overwrite --force-postinstall --force-space -d " + rootfsPath + " " + filePath;
                    lxcd::Process process(command);
                    int result = process.run();
                    if(result != 0) {
                        printf("Failed to install IPK: %s\n", filePath.c_str());
                        return false;
                    }
                    lxcd::size_t extPos = fileName.rfind(".ipk");
                    if((extPos != lxcd::string::npos) && (fileName.size() - extPos == 4)) {
                        ipkPackages.push_back(fileName.substr(0, extPos));
                    }
                }

            }
            closedir(dir);
        }
    }
    // Clean up the temporary directory
    remove_directory(tempDir);

    return true;
}
bool DeploymentUnit::duRemove() {
    printf("removing du Filesystem ... %s\n", this->rootfsPath.c_str());
    if(this->type == "squashfs") {
        lxcd::string squashfs = this->rootfsPath + ".squashfs";
        lxcd::umountSquashfs(this->rootfsPath);
        remove_directory(this->rootfsPath);
        remove(squashfs.c_str());
    }

    // TODO : Remove any other related files or directories
    // ...

    return true;
}
