#include "deployment_unit.h"

std::string DeploymentUnit::cacheFilePath = "deployment_units_cache.json";

DeploymentUnit::DeploymentUnit(const Json::Value& metadata, const std::string& uuid)
    : uuid(uuid),
      executionEnvRef(metadata["ExecutionEnvRef"].asString()),
      description(metadata["Description"].asString()),
      vendor(metadata["Vendor"].asString()),
      version(metadata["Version"].asInt()) {}

bool DeploymentUnit::install(const std::string& tarballPath) {
    // Store the installation date
    installationDate = std::time(nullptr);

    // Create a temporary directory for extraction
    std::string tempDir = "tmp_du_install";

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

    // Check the installation type
    std::string type = metadata["type"].asString();
    if (type == "squashfs") {

        // Create the destination directory
        std::string destinationDir = "/run/" + uuid;
        std::filesystem::create_directories(destinationDir);

        // Copy the SquashFS rootfs file to the destination
        std::filesystem::copy(tempDir + "/rootfs.squashfs", destinationDir + "/root.squashfs");

    } else if (type == "ipk") {
        // Handle IPK installation
        std::string rootfsPath = "/run/" + uuid + "/rootfs";
        std::filesystem::create_directories(rootfsPath);

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
        std::cerr << "Unknown installation type: " << type << std::endl;
        return false;
    }

    // Clean up the temporary directory
    std::filesystem::remove_all(tempDir);

    return true;
}


bool DeploymentUnit::remove() {
    // Remove the squashfs file
    std::string squashfsPath = "/run/" + uuid + "/root.squashfs";
    if (std::filesystem::exists(squashfsPath)) {
        std::filesystem::remove(squashfsPath);
    } else {
        std::cerr << "Squashfs file not found" << std::endl;
        return false;
    }

    // TODO : Remove any other related files or directories
    // ...

    return true;
}

std::vector<std::string> DeploymentUnit::list() {
    // TODO: Implement list method
}
