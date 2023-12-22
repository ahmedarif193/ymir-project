#include "lxc-container.h"

#define EXITIFFAIL(A) do { \
    int ret = (A); \
    if(ret) { \
    printf("Failed at %s with error code %d\n", #A, ret); \
    return ret; \
    } \
    } while(0)


LxcContainer::LxcContainer(const lxcd::string name, lxcd::string m_template, const Method action) {
    this->m_action = action;
    this->m_name = name;
    this->mTemplate = m_template;
    this->storageSpace = 200;
    this->memory = "20";
    this->cpuset = "0-1";
    this->cpupercent = "50";
    this->useOverlay = true;

    container = lxc_container_new(m_name.c_str(), NULL);
    if(!container) {
        printf("Failed to setup lxc_container struct\n");
        return;
    }
}

LxcContainer::~LxcContainer() {
    printf("LxcContainer::~LxcContainer()\n");
    lxc_container_put(container);
}

int LxcContainer::run() {

    int ret = 0;
    if(m_action == Method::ENABLE) {
        EXITIFFAIL(create());
        EXITIFFAIL(prepare());
        EXITIFFAIL(start());
    } else if(m_action == Method::DISABLE) {
        EXITIFFAIL(stop());
    } else if(m_action == Method::RECONFIGURE) {
        EXITIFFAIL(reconfigure());
        EXITIFFAIL(stop());
        EXITIFFAIL(start());
    } else if(m_action == Method::RESET) {
        EXITIFFAIL(stop());
        //TODO : just clear delta overlayfs if used
        EXITIFFAIL(destroy());
        EXITIFFAIL(create());
        EXITIFFAIL(prepare());
        EXITIFFAIL(start());
    } else if(m_action == Method::DESTROY) {
        EXITIFFAIL(stop());
        EXITIFFAIL(destroy());
    } else {
    }
    return ret;
}

int LxcContainer::create() {
    int retCode = 0;
    const char** createArgs = nullptr;
    const char* templateName = mTemplate.c_str();
    this->useOverlay = this->storageSpace > 0 ? true : false;
    const char* fsType = this->useOverlay ? "overlayfs" : nullptr;

    if (container->is_defined(container)) {
        printf("Container already exists.\n");
        goto cleanup;
    }
    printf("action LxcContainer::create()\n");

    if (this->useOverlay) {
        lxcd::vector<lxcd::string> mTemplateParts = mTemplate.split(':');
        lxcd::string mTemplateName = mTemplateParts[0];
        if (mTemplateParts.size() > 1) {
            lxcd::vector<lxcd::string> mTemplateArgParts = mTemplateParts[1].split(';');
            if (mTemplateArgParts.size() < 3) {
                printf("Insufficient template arguments provided.\n");
                return 1;
            }

            lxcd::string distro = mTemplateArgParts[0];
            lxcd::string release = mTemplateArgParts[1];
            lxcd::string arch = mTemplateArgParts[2];
            printf("Distro: %s, Release: %s, Arch: %s\n", distro.c_str(), release.c_str(), arch.c_str());

            const char* overlayArgs[] = {
                "-d", distro.c_str(), "-r", release.c_str(), "-a", arch.c_str()
            };
            createArgs = overlayArgs;
            if (!container->createl(container, mTemplateName, fsType, nullptr, LXC_CLONE_ALLOW_RUNNING,
                                    "-d", distro.c_str(), "-r", release.c_str(), "-a", arch.c_str(), nullptr)) {
                printf("Failed to create container rootfs\n");
            }
        }else{
            if (!container->createl(container, mTemplateName, fsType, nullptr, LXC_CLONE_ALLOW_RUNNING, nullptr)) {
                 printf("Failed to create container with template %s.\n", mTemplateName.c_str());
                 return 1;
             }
        }
        retCode = setupOverlayFs();
    }
    else{
        if (!container->createl(container, mTemplate, fsType, nullptr, LXC_CLONE_ALLOW_RUNNING, nullptr)) {
            printf("Failed to create container with template %s.\n", templateName);
            retCode = 1;
            goto cleanup;
        }
    }

cleanup:
    return retCode;
}


int LxcContainer::setupOverlayFs() {
    // Common setup for paths
    auto lxcPath = getLxcPath();
    lxcd::string containerPath = lxcPath + container->name;
    lxcd::string containerOverlayImg = containerPath + "/overlay.img";
    lxcd::string containerOverlayDir = containerPath + "/overlay";
    lxcd::string containerDeltaDir = containerOverlayDir + "/delta";

    // Create and format overlay image if it does not exist
    createOverlayImage(containerOverlayImg);

    if(!lxcd::isMountExist(containerOverlayDir)){
        printf("delta folder for container \"%s\" is not mounted, mounting ..\n", container->name);
        if (mountOverlayFs(containerOverlayImg, containerOverlayDir)) {
            printf("delta folder for container \"%s\" is successfully mounted\n", container->name);
            // If mount is successful, create the folder delta inside the new overlay folder
            makeDirectory(containerDeltaDir);
        }
    }
    return 0;
}


int LxcContainer::createOverlayImage(const lxcd::string& overlayImgPath) {
    // Check if image file already exists
    struct stat buffer;
    if (stat(overlayImgPath.c_str(), &buffer) != 0) {
        // Create and format the overlay image
        int fd = open(overlayImgPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        if (ftruncate(fd, this->storageSpace * 1024 * 1024) == -1) {
            perror("ftruncate");
            close(fd);
            return -1;
        }

        if (close(fd) == -1) {
            perror("close");
            return -1;
        }

        char command[128];
        snprintf(command, sizeof(command), "mkfs.ext4 -F %s", overlayImgPath.c_str());
        int retcode;
        lxcd::exec(command, retcode);

        if (retcode != 0) {
            printf("Failed to format the overlay image as ext4.\n");
            return -1;
        }

        printf("Created and formatted ext4 image file: %s\n", overlayImgPath.c_str());
    }
    return 0;
}

bool LxcContainer::mountOverlayFs(const lxcd::string& overlayImgPath, const lxcd::string& overlayDirPath) {
    int loopCtlFd = -1, loopFd = -1, imageFd = -1, loopNumber =-1;
    bool mountSuccess = false;

    // Check if already a mount point
    lxcd::string mountpointCheckCmd = "mountpoint -q " + overlayDirPath;
    if (system(mountpointCheckCmd.c_str()) == 0) {
        printf("%s is already a mount point\n", overlayDirPath.c_str());
        return true;
    }

    loopCtlFd = open("/dev/loop-control", O_RDWR);
    if (loopCtlFd < 0) {
        perror("Failed to open loop control device");
        goto cleanup;
    }

    loopNumber = ioctl(loopCtlFd, LOOP_CTL_GET_FREE);
    if (loopNumber < 0) {
        perror("Failed to get free loop device");
        goto cleanup;
    }

    char loopDevice[64];
    snprintf(loopDevice, sizeof(loopDevice), "/dev/loop%d", loopNumber);

    loopFd = open(loopDevice, O_RDWR);
    if (loopFd < 0) {
        perror("Failed to open loop device");
        goto cleanup;
    }

    imageFd = open(overlayImgPath.c_str(), O_RDWR);
    if (imageFd < 0) {
        perror("Failed to open image file :");
        goto cleanup;
    }

    if (ioctl(loopFd, LOOP_SET_FD, imageFd) < 0) {
        perror("Failed to set loop device backing file");
        goto cleanup;
    }
    printf("mounting %s => %s\n", overlayImgPath.c_str(), overlayDirPath.c_str());

    if (mount(loopDevice, overlayDirPath.c_str(), "ext4", MS_MGC_VAL | MS_NOSUID | MS_NODEV, "") == -1) {
        perror("Failed to mount image");
        goto cleanup;
    }

    printf("Image mounted successfully at %s\n", overlayDirPath.c_str());
    mountSuccess = true;

cleanup:
    if (imageFd >= 0) {
        close(imageFd);
    }
    if (loopFd >= 0) {
        if (ioctl(loopFd, LOOP_CLR_FD, 0) < 0) {
            perror("Failed to clear loop device");
        }
        close(loopFd);
    }
    if (loopCtlFd >= 0) {
        close(loopCtlFd);
    }
    return mountSuccess;
}

void LxcContainer::makeDirectory(const lxcd::string& dirPath) {
    struct stat st;

    // Check if directory already exists
    if (stat(dirPath.c_str(), &st) == 0) {
        // Check if the path is a directory
        if (S_ISDIR(st.st_mode)) {
            printf("Directory already exists: %s\n", dirPath.c_str());
            return;
        } else {
            printf("File exists and is not a directory: %s\n", dirPath.c_str());
            exit(EXIT_FAILURE);
        }
    }

    // Create the directory if it does not exist
    if (mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    printf("Directory created: %s\n", dirPath.c_str());
}

int LxcContainer::prepare(){
    //TODO I handle only overlay case, to be fixed
    int ret = 0;
    if(this->useOverlay && this->storageSpace > 0) {
        ret = setupOverlayFs();
        DeploymentUnitHelper helper;
        helper.updateFullRootPath(container);
    }
    return ret;
}

int LxcContainer::start() {
    if (!container) {
        printf("container not found!\n");
        return -1;
    }

    // Check if the container is already running
    if (container->is_running(container)) {
        printf("Wanted to start but The container is already running.\n");
        return 0; // Or you might want to return 0 or another value indicating that no action was needed
    }

    if (!container->start(container, 0, nullptr)) {
        printf("Failed to start the container\n");
        return -1;
    }

    return 0;
}
int LxcContainer::stop() {
    if(!container) {
        printf("container no found !\n");
        return -1;
    }
    //TODO fix using lxc api
    lxcd::string container_cmd = lxcd::string("lxc-stop -k -n ") + container->name;
    system(container_cmd.c_str());
    //if(!container->shutdown(container, 0)) {
    //    return container->stop(container) ? EXIT_SUCCESS : EXIT_FAILURE;
    //}
    return 0;
}

int LxcContainer::reconfigure() {
    return 0;
}

int LxcContainer::destroy() {
    if(!container) {
        printf("container no found !\n");
        return -1;
    }
    int ret = 0;
    auto lxcPath = getLxcPath();
    lxcd::string overlay_path = lxcPath + lxcd::string(container->name) + "/overlay";
    // Unmount the file system
    //todo use utils/linux.h
    ret = umount(overlay_path.c_str());
    if(ret == -1) {
        perror("umount failed");
        exit(EXIT_FAILURE);
    }
    printf("Unmounted %s\n", overlay_path.c_str());
    lxcd::string container_cmd = lxcd::string("lxc-destroy -f -n ") + container->name;
    system(container_cmd.c_str());
    // if(!container->destroy(container)) {
    //     printf("Failed to remove the container\n");
    //     return -1;
    // }
    return 0;
}

IPAddressList LxcContainer::getIPAddresses() {
    lxcd::vector<lxcd::string> ips;
    if (!container) {
        printf("Container not found!\n");
        return ips; // Return an empty vector if the container is not found.
    }

    char **addresses = container->get_ips(container, nullptr, "inet", 0);
    if (addresses) {
        for (int i = 0; addresses[i] != nullptr; ++i) {
            ips.push_back(lxcd::string(addresses[i]));
        }
    }
    return ips;
}

void LxcContainer::setName(const lxcd::string &newName) {
    m_name = newName;
}

void LxcContainer::setTemplate(const lxcd::string &newTemplate) {
    mTemplate = newTemplate;
}

void LxcContainer::setStorageSpace(const int newStorage_space) {
    storageSpace = newStorage_space;
}

void LxcContainer::setMemory(const lxcd::string &newMemory) {
    memory = newMemory;
}

void LxcContainer::setCpuset(const lxcd::string &newCpuset) {
    cpuset = newCpuset;
}

void LxcContainer::setCpupercent(const lxcd::string &newCpupercent) {
    cpupercent = newCpupercent;
}

void LxcContainer::setContainer(lxc_container* newContainer) {
    container = newContainer;
}

void LxcContainer::setAction(Method newAction) {
    m_action = newAction;
}
