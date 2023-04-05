#include <iostream>
#include <string>
#include "deployment_unit_helper.h"

void printUsage() {
    std::cout << "Usage: du-manager --ls|--install|--remove [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --ls                           List installed DeploymentUnits" << std::endl;
    std::cout << "  --install -e <container> -d <path> [-u <uuid>]   Install a DeploymentUnit" << std::endl;
    std::cout << "  --remove <uuid>                Remove a DeploymentUnit by UUID" << std::endl;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string action = argv[1];
    DeploymentUnitHelper helper;

    if (action == "--ls") {
        helper.listDeploymentUnits();
    } else if (action == "--install") {
        std::string container, path, uuid;

        for (int i = 2; i < argc; i++) {
            if (std::string(argv[i]) == "-e" && i + 1 < argc) {
                container = argv[++i];
            } else if (std::string(argv[i]) == "-d" && i + 1 < argc) {
                path = argv[++i];
            } else if (std::string(argv[i]) == "-u" && i + 1 < argc) {
                uuid = argv[++i];
            } else {
                std::cout << "Unknown or incomplete option: " << argv[i] << std::endl;
                printUsage();
                return 1;
            }
        }
        std::cout << "---1"<< std::endl;

        if (container.empty() || path.empty()) {
            std::cout << "Error: Both -e <container> and -d <path> options are required for --install." << std::endl;
            printUsage();
            return 1;
        }
        // Generate a UUID for the DeploymentUnit
        if (uuid.empty()){
            uuid_t _uuid;
            uuid_generate(_uuid);
            char uuidStr[37];
            uuid_unparse(_uuid, uuidStr);
            uuid = uuidStr;
        }
        std::cout << "---2"<< std::endl;

        if (!helper.addDeploymentUnit(container, path, uuid)){
            std::cout << "Error: Failed to install the DeploymentUnit." << std::endl;
            return 1;
        }
    } else if (action == "--remove") {
        if (argc != 3) {
            std::cout << "Error: UUID is required for --remove." << std::endl;
            printUsage();
            return 1;
        }

        std::string uuidToRemove = argv[2];
        if (helper.removeDeploymentUnit(uuidToRemove)) {
            std::cout << "Successfully removed DeploymentUnit with UUID: " << uuidToRemove << std::endl;
        } else {
            std::cout << "Error: Failed to remove the DeploymentUnit with UUID: " << uuidToRemove << std::endl;
            return 1;
        }
    } else {
        std::cout << "Unknown action: " << action << std::endl;
        printUsage();
        return 1;
    }

    return 0;
}
