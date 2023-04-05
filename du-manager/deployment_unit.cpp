#include "deployment_unit.h"


std::string DeploymentUnit::cacheFilePath = "deployment_units_cache.json";


DeploymentUnit::DeploymentUnit(const std::string &uuid)
{
    this->uuid =uuid;
}

// Helper function for libcurl
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string getLxcPath() {
    const char* lxcPath = lxc_get_global_config_item("lxc.lxcpath");
    if (lxcPath) {
        return std::string(lxcPath);
    } else {
        std::cerr << "Error: Unable to get lxc.lxcpath" << std::endl;
        return "";
    }
}

bool isContainerUsingOverlayFS(const std::string& containerName) {
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
        std::string rootfsBackend(lxcRootfsBackend);
        if (rootfsBackend.find("overlay") != std::string::npos) {
            lxc_container_put(container);
            return true;
        }
    }

    lxc_container_put(container);
    return false;
}

int create_directories(const std::string& path) {
    std::string tmp = path;
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

bool copy_file(const std::string& src, const std::string& dst) {
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

bool remove_directory(const std::string &dir) {
    int flags = FTW_DEPTH | FTW_PHYS;
    int rv = nftw(dir.c_str(), remove_entry, 64, flags);
    if (rv) {
        perror("Failed to remove directory");
        return false;
    }
    return true;
}

bool DeploymentUnit::install(const std::string& tarballPath, const std::string& executionEnvRef) {
    // Store the installation date
    installationDate = std::time(nullptr);

    // Create a temporary directory for extraction
    std::string tempDir = "/tmp/tr157-du";

    // Store the installation date
    installationDate = std::time(nullptr);

    // Check if the tarballPath is a URL
    std::string downloadedFilePath;
    if (tarballPath.find("http://") == 0 || tarballPath.find("https://") == 0) {
        CURL* curl;
        CURLcode res;
        std::string fileContents;

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
            std::string fileName = "downloaded_tarball.tar.gz";
            std::ofstream outputFile(fileName);
            outputFile << fileContents;
            outputFile.close();
            downloadedFilePath = fileName;
        }
        curl_global_cleanup();
    }

    // Use the downloaded file path if it exists, otherwise use the original tarballPath
    std::string pathToUse = downloadedFilePath.empty() ? tarballPath : downloadedFilePath;
    create_directories(tempDir);
    // Extract the tar.gz file to the temporary directory
    std::string command = "tar -C " + tempDir + " -xzf " + pathToUse;
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

    std::string lxcPath = getLxcPath();
    if (lxcPath.empty()) {
        std::cout << "fatal : LXC path no found." << std::endl;
        return false;
    }

    this->executionEnvRef = executionEnvRef;
    this->description = metadata["Description"].asString();
    this->vendor = metadata["Vendor"].asString();
    this->version = metadata["Version"].asInt();
    this->type = metadata["Type"].asString();
    this->rootfsPath = lxcPath+"/"+this->executionEnvRef+"/deploymentunits/" + uuid;

    std::cout << "executionEnvRef "<< this->executionEnvRef<< std::endl;
    std::cout << "description "<< this->description<< std::endl;
    std::cout << "vendor "<< this->vendor<< std::endl;
    std::cout << "type "<< this->type<< std::endl;

    // Check the installation type


    create_directories(rootfsPath);
    std::cout << "installing tarball sources to :"<< rootfsPath<< std::endl;
    if (this->type == "squashfs") {

        std::cout << tempDir << "/rootfs.squashfs >>>>>>>>>>>"<< rootfsPath<<".squashfs"<< std::endl;

        // Copy the SquashFS rootfs file to the destination
        copy_file(tempDir + "/rootfs.squashfs", rootfsPath + ".squashfs");
    } else if (this->type == "ipk") {

        // Iterate through all the IPK files in the temporary directory
        for (const auto& entry : std::filesystem::directory_iterator(tempDir)) {
            if (entry.path().extension() == ".ipk") {
                // Install the IPK file to the rootfs directory
                std::string command = "opkg install --force-depends --force-overwrite --force-postinstall --force-space -d " + rootfsPath + " " + entry.path().string();
                int result = std::system(command.c_str());
                if (result != 0) {
                    std::cerr << "Failed to install IPK: " << entry.path().string() << std::endl;
                    return false;
                }
            }
        }

        // Store the names of installed IPK packages
        ipkPackages.clear();
        for (const auto& entry : std::filesystem::directory_iterator(tempDir)) {
            if (entry.path().extension() == ".ipk") {
                ipkPackages.push_back(entry.path().stem().string());
            }
        }
    } else {
        std::cerr << "Unknown installation type: " << this->type << std::endl;
        return false;
    }

    // Clean up the temporary directory
    remove_directory(tempDir);

    return true;
}

bool DeploymentUnit::remove() {
    remove_directory(this->rootfsPath);

    if (this->type == "squashfs") {
        // Remove the squashfs file mount folder
        //remove_directory(this->rootfsPath);
    }

    // TODO : Remove any other related files or directories
    // ...

    return true;
}
