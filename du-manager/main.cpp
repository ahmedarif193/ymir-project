#include <stdio.h>
#include "utils/uuid.h"

#include "deployment_unit_helper.h"

void printUsage() {
    printf("Usage: du-manager --ls|--install|--remove [options]\n");
    printf("Options:\n");
    printf("  --ls                           List installed DeploymentUnits\n");
    printf("  --install -e <container> -d <path> [-u <uuid>]   Install a DeploymentUnit\n");
    printf("  --remove <uuid>                Remove a DeploymentUnit by UUID\n");
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printUsage();
        return 1;
    }

    char* action = argv[1];
    DeploymentUnitHelper helper;

    if (strcmp(action, "--ls") == 0) {
        helper.listDeploymentUnits();
    } else if (strcmp(action, "--install") == 0) {
        char* container = NULL;
        char* path = NULL;
        lxcd::string uuidStr;
        uuid_t _uuid;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
                container = argv[++i];
            } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
                path = argv[++i];
            } else if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
                uuidStr = argv[++i];
            } else {
                printf("Unknown or incomplete option: %s\n", argv[i]);
                printUsage();
                return 1;
            }
        }

        if (container == NULL || path == NULL) {
            printf("Error: Both -e <container> and -d <path> options are required for --install.\n");
            printUsage();
            return 1;
        }

        uuidStr = lxcd::UUIDGenerator::generate();

        if (!helper.addDeploymentUnit(container, path, uuidStr)){
            printf("Error: Failed to install the DeploymentUnit.\n");
            return 1;
        }
    } else if (strcmp(action, "--remove") == 0) {
        if (argc != 3) {
            printf("Error: UUID is required for --remove.\n");
            printUsage();
            return 1;
        }

        char* uuidToRemove = argv[2];
        if (helper.removeDeploymentUnit(uuidToRemove)) {
            printf("Successfully removed DeploymentUnit with UUID: %s\n", uuidToRemove);
        } else {
            printf("Error: Failed to remove the DeploymentUnit with UUID: %s\n", uuidToRemove);
            return 1;
        }
    } else {
        printf("Unknown action: %s\n", action);
        printUsage();
        return 1;
    }

    return 0;
}
