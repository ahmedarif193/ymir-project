#include <stdio.h>
#include <getopt.h>
#include "utils/uuid.h"
#include "utils/sharedptr.h"
#include "deployment_unit_helper.h"

//#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "lxc-container.h"

void printUsage() {
    printf("Usage: du-manager --ls|--install|--remove [options]\n");
    printf("Options:\n");
    printf("  --ls                           List installed DeploymentUnits\n");
    printf("  --install -e <container> -d <path> [-u <uuid>]   Install a DeploymentUnit\n");
    printf("  --remove -u <uuid>                Remove a DeploymentUnit by UUID\n");
}

// void signal_handler(int sig) {
//     void *array[1024];
//     size_t size;
//     char **strings;
//     size_t i;

//     // Get void*'s for all entries on the stack
//     size = backtrace(array, 1024);

//     // Print out all the frames to stderr
//     fprintf(stderr, "Error: signal %d:\n", sig);
//     strings = backtrace_symbols(array, size);

//     for (i = 0; i < size; i++) {
//         fprintf(stderr, "%s\n", strings[i]);
//     }

//     free(strings);
//     exit(1);
// }

int main(int argc, char* argv[]) {
    int c;
    int option_index = 0;
    static struct option long_options[] = {
    {"ls", no_argument, 0, 'l'},
    {"install", required_argument, 0, 'i'},
    {"remove", required_argument, 0, 'r'},
    {0, 0, 0, 0}};

    if(argc < 2) {
        printUsage();
        return 1;
    }

    char* action = argv[1];
    DeploymentUnitHelper helper;

    while((c = getopt_long(argc, argv, "e:d:u:", long_options, &option_index)) != -1) {
        switch(c) {
        case 'l': {
            helper.ls();
            return 0;
        }
        case 'i': {
            char* container = NULL;
            char* path = NULL;
            lxcd::string uuidStr;

            for(int i = optind - 1; i < argc; i++) {
                if((strcmp(argv[i], "-e") == 0) && (i + 1 < argc)) {
                    container = argv[++i];
                } else if((strcmp(argv[i], "-d") == 0) && (i + 1 < argc)) {
                    path = argv[++i];
                } else if((strcmp(argv[i], "-u") == 0) && (i + 1 < argc)) {
                    uuidStr = argv[++i];
                } else {
                    printf("Unknown or incomplete option: %s\n", argv[i]);
                    printUsage();
                    return 1;
                }
            }

            if((container == NULL) || (path == NULL)) {
                printf("Error: Both -e <container> and -d <path> options are required for --install.\n");
                printUsage();
                return 1;
            }
            if(uuidStr.empty()) {
                uuidStr = lxcd::UUIDGenerator::generate();
            }
            lxcd::SharedPtr<DeploymentUnit> ret = helper.addDeploymentUnit(container, path, uuidStr);
            if(ret->name.empty()) {
                printf("Failed to install the DeploymentUnit : %s in the container : %s, installation aborted.\n", path, container);
                return 1;
            }
            return 0;
        }
        case 'r': {
            if(argc != optind + 1) {
                printf("Error: UUID is required for --remove.\n");
                printUsage();
                return 1;
            }

            char* uuidToRemove = argv[optind];
            if(helper.removeDeploymentUnit(uuidToRemove)) {
                printf("Successfully removed DeploymentUnit with UUID: %s\n", uuidToRemove);
            } else {
                printf("Error: No DeploymentUnit was removed with UUID: %s\n", uuidToRemove);
                return 1;
            }
            return 0;
        }
        default: {
            printf("Unknown action: %s\n", action);
            printUsage();
            return 1;
        }
        }
    }
    return 0;
}
