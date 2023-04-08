#include "deployment_unit.h"


lxcd::string DeploymentUnit::cacheFilePath = "deployment_units_cache.json";
lxcd::string DeploymentUnit::tempDir = "/tmp/tr157-du";


DeploymentUnit::DeploymentUnit(const lxcd::string &uuid)
{
    this->uuid =uuid;
}

// Helper function for libcurl
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((lxcd::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

lxcd::string getLxcPath() {
    const char* lxcPath = lxc_get_global_config_item("lxc.lxcpath");
    if (lxcPath) {
        return lxcd::string(lxcPath);
    } else {
        std::cerr << "Error: Unable to get lxc.lxcpath" << std::endl;
        return "";
    }
}

bool isContainerUsingOverlayFS(const lxcd::string& containerName) {
    struct lxc_container* container = lxc_container_new(containerName.c_str(), nullptr);
    if (!container) {
        std::cerr << "Error: Unable to create container object" << std::endl;
        return false;
    }

    if (!container->is_defined(container)) {
        std::cerr << "Error: Container is not defined" << std::endl;
        lxc_container_put(container);
        return false;
    }

    if (!container->load_config(container, nullptr)) {
        std::cerr << "Error: Unable to load container configuration" << std::endl;
        lxc_container_put(container);
        return false;
    }

    char lxcRootfsBackend[MAXPARAMLEN];
    container->get_config_item(container, "lxc.rootfs.path", lxcRootfsBackend, MAXPARAMLEN);
    if (strlen(lxcRootfsBackend)) {
        lxcd::string rootfsBackend(lxcRootfsBackend);
        if (rootfsBackend.find("overlay") != lxcd::string::npos) {
            lxc_container_put(container);
            return true;
        }
    }

    lxc_container_put(container);
    return false;
}

int create_directories(const lxcd::string& path) {
    lxcd::string tmp = path;
    size_t len = tmp.length();
    mode_t mode = 0777;
    mode_t current_umask = umask(0); // Get current umask
    umask(current_umask); // Reset umask to its original value

    if (tmp[len - 1] == '/')
        tmp.erase(len - 1);
    for (size_t i = 1; i < tmp.length(); ++i) {
        if (tmp[i] == '/') {
            tmp[i] = 0;
            if (mkdir(tmp.c_str(), mode & ~current_umask) != 0 && errno != EEXIST) {
                perror("Failed to create directory");
                return -1;
            }
            tmp[i] = '/';
        }
    }
    if (mkdir(tmp.c_str(), mode & ~current_umask) != 0 && errno != EEXIST) {
        perror("Failed to create directory");
        return -1;
    }

    return 0;
}

bool copy_file(const lxcd::string& src, const lxcd::string& dst) {
    int src_fd = open(src.c_str(), O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source file");
        return false;
    }

    struct stat src_stat;
    if (fstat(src_fd, &src_stat) == -1) {
        perror("Failed to stat source file");
        close(src_fd);
        return false;
    }

    int dst_fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dst_fd == -1) {
        perror("Failed to open destination file");
        close(src_fd);
        return false;
    }

    off_t offset = 0;
    ssize_t sent_bytes;
    while ((sent_bytes = sendfile(dst_fd, src_fd, &offset, src_stat.st_size)) > 0) {
        src_stat.st_size -= sent_bytes;
    }

    close(src_fd);
    close(dst_fd);

    if (sent_bytes == -1) {
        perror("Failed to copy file using sendfile");
        return false;
    }

    return true;
}
int remove_entry(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    int rv = remove(path);
    if (rv) {
        perror("Failed to remove");
    }
    return rv;
}

bool remove_directory(const lxcd::string &dir) {
    int flags = FTW_DEPTH | FTW_PHYS;
    int rv = nftw(dir.c_str(), remove_entry, 64, flags);
    if (rv) {
        perror("Failed to remove directory");
        return false;
    }
    return true;
}

bool DeploymentUnit::prepare(const lxcd::string& tarballPath, const lxcd::string& executionEnvRef) {
        // Store the installation date
    installationDate = std::time(nullptr);

    // Store the installation date
    installationDate = std::time(nullptr);

    // Check if the tarballPath is a URL
    lxcd::string downloadedFilePath;
    if (tarballPath.find("http://") == 0 || tarballPath.find("https://") == 0) {
        CURL* curl;
        CURLcode res;
        lxcd::string fileContents;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, tarballPath.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileContents);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Failed to download the tarball from URL: " << tarballPath << std::endl;
                curl_easy_cleanup(curl);
                return false;
            }
            curl_easy_cleanup(curl);

            // Save the downloaded file to a temporary directory
            lxcd::string fileName = "downloaded_tarball.tar.gz";
            std::ofstream outputFile(fileName);
            outputFile << fileContents;
            outputFile.close();
            downloadedFilePath = fileName;
        }
        curl_global_cleanup();
    }

    // Use the downloaded file path if it exists, otherwise use the original tarballPath
    lxcd::string pathToUse = downloadedFilePath.empty() ? tarballPath : downloadedFilePath;
    create_directories(tempDir);
    // Extract the tar.gz file to the temporary directory
    lxcd::string command = "tar -C " + tempDir + " -xzf " + pathToUse;
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

    lxcd::string lxcPath = getLxcPath();
    if (lxcPath.empty()) {
        std::cout << "fatal : LXC path no found." << std::endl;
        return false;
    }

    this->executionEnvRef = executionEnvRef;
    this->description = metadata["Description"].asString().c_str();
    this->vendor = metadata["Vendor"].asString().c_str();
    this->version = metadata["Version"].asInt();
    this->type = metadata["Type"].asString().c_str();
    this->name = metadata["Name"].asString().c_str();

    this->rootfsPath = lxcPath+"/"+this->executionEnvRef+"/deploymentunits/" + uuid;

    std::cout << "executionEnvRef "<< this->executionEnvRef<< std::endl;
    std::cout << "description "<< this->description<< std::endl;
    std::cout << "vendor "<< this->vendor<< std::endl;
    std::cout << "type "<< this->type<< std::endl;
    std::cout << "name "<< this->name<< std::endl;
    return true;

}
bool DeploymentUnit::install() {

    // Check the installation type


    create_directories(rootfsPath);
    std::cout << "installing tarball sources to :"<< rootfsPath<< std::endl;
    if (this->type == "squashfs") {

        std::cout << tempDir << "/rootfs.squashfs >>>>>>>>>>>"<< rootfsPath<<".squashfs"<< std::endl;

        // Copy the SquashFS rootfs file to the destination
        copy_file(tempDir + "/rootfs.squashfs", rootfsPath + ".squashfs");
    } else if (this->type == "ipk") {

        DIR* dir;
        struct dirent* entry;

        dir = opendir(tempDir.c_str());
        if (dir == nullptr) {
            std::cerr << "Failed to open directory: " << tempDir << std::endl;
        } else {
            // Store the names of installed IPK packages
            ipkPackages.clear();
            while ((entry = readdir(dir)) != nullptr) {
                // Check if the file has an .ipk extension
                lxcd::string fileName(entry->d_name);
                std::size_t extPos = fileName.rfind(".ipk");
                if (extPos != lxcd::string::npos && fileName.size() - extPos == 4) {
                    // Install the IPK file to the rootfs directory
                    lxcd::string filePath = tempDir + "/" + fileName;
                    lxcd::string command = "opkg install --force-depends --force-overwrite --force-postinstall --force-space -d " + rootfsPath + " " + filePath;
                    int result = std::system(command.c_str());
                    if (result != 0) {
                        std::cerr << "Failed to install IPK: " << filePath << std::endl;
                        return false;
                    }
                    std::size_t extPos = fileName.rfind(".ipk");
                    if (extPos != lxcd::string::npos && fileName.size() - extPos == 4) {
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

bool DeploymentUnit::remove() {
    std::cerr << "----------------------2 " <<this->rootfsPath<< std::endl;
    remove_directory(this->rootfsPath);

    if (this->type == "squashfs") {
        // Remove the squashfs file mount folder
        //remove_directory(this->rootfsPath);
    }

    // TODO : Remove any other related files or directories
    // ...

    return true;
}
