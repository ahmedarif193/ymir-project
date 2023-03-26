#include "lxc-container.h"
LxcContainer::LxcContainer(const std::string name, const char* m_template, const Method action) {

    this->m_action = action;
    this->m_name = name;
    this->m_template = m_template;
    this->storage_space = 20;
    this->memory = "20";
    this->cpuset = "0-1";
    this->cpupercent = "50";
    this->use_overlay = true;
    std::cout<<"---------LxcContainer::LxcContainer: "<<m_name<<std::endl;

    container = lxc_container_new(m_name.c_str(), NULL);
    if (!container) {
        std::cerr<<"Failed to setup lxc_container struct"<<std::endl;
        return;
    }
    std::cout<<"---------Create the container 2: "<<container->name<<std::endl;

}

LxcContainer::~LxcContainer() {
    lxc_container_put(container);

}
#define EXITIFFAIL(A) {int ret= A; if(ret) return ret;}
int LxcContainer::run() {
    int ret = 0;
    if (m_action == Method::ENABLE) {
        EXITIFFAIL(create());
        EXITIFFAIL(start());
    } else if (m_action == Method::DISABLE) {
        EXITIFFAIL(stop());
    } else if (m_action == Method::RECONFIGURE) {
        EXITIFFAIL(reconfigure());
        EXITIFFAIL(stop());
        EXITIFFAIL(start());
    } else if (m_action == Method::RESET) {
        EXITIFFAIL(stop());
        //TODO : just clear delta overlayfs if used
        EXITIFFAIL(destroy());
        EXITIFFAIL(create());
        EXITIFFAIL(start());
    } else if (m_action == Method::DESTROY) {
        EXITIFFAIL(stop());
        EXITIFFAIL(destroy());
    } else {
        ;
    }
    return ret;
}

int LxcContainer::create() {
    //    const char *containername = "ext2222222";
    //    container = lxc_container_new(containername, NULL);
    //specs.fstype=strdup("overlayfs");

    int retcode = 0;
    if(this->use_overlay){
        if (!container->createl(container, "busybox", "overlayfs", nullptr, LXC_CREATE_QUIET, nullptr)) {
            std::cerr<<"error createlb"<<std::endl;
            return -1;
        }
        int fd;
        const std::string lxc_default_folder = "/var/lib/lxc/";
        std::string container_path = lxc_default_folder + container->name;
        std::string container_overlay_img = container_path +"/overlay.img";
        std::string container_overlay_dir = container_path +"/overlay";
        std::string container_delta_dir = container_overlay_dir +"/delta";
        std::cout<<"--------------000 : "<<container_overlay_img.c_str()<<std::endl;

        // create a file with the desired size
        fd = open(container_overlay_img.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (ftruncate(fd, this->storage_space * 1024 * 1024) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
        if (close(fd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        // format the file as an ext4 file system
        char command[100];
        sprintf(command, "mkfs.ext4 -F %s", container_overlay_img.c_str());
        exec(command,retcode);

        printf("Created ext4 image file: %s\n", container_overlay_img.c_str());

        int loop_ctl_fd = open("/dev/loop-control", O_RDWR);
        if (loop_ctl_fd < 0) {
            perror("Failed to open loop control device");
            return 1;
        }

        // Get a free loop device number
        int loop_number = ioctl(loop_ctl_fd, LOOP_CTL_GET_FREE);
        if (loop_number < 0) {
            perror("Failed to get free loop device");
            close(loop_ctl_fd);
            return 1;
        }

        // Create the loop device file path
        char loop_device[64];
        snprintf(loop_device, sizeof(loop_device), "/dev/loop%d", loop_number);

        // Open the loop device file
        int loop_fd = open(loop_device, O_RDWR);
        if (loop_fd < 0) {
            perror("Failed to open loop device");
            close(loop_ctl_fd);
            return 1;
        }

        // Set the backing file for the loop device
        int image_fd = open(container_overlay_img.c_str(), O_RDWR);
        if (image_fd < 0) {
            perror("Failed to open image file");
            close(loop_fd);
            close(loop_ctl_fd);
            return 1;
        }

        if (ioctl(loop_fd, LOOP_SET_FD, image_fd) < 0) {
            perror("Failed to set loop device backing file");
            close(image_fd);
            close(loop_fd);
            close(loop_ctl_fd);
            return 1;
        }

        // Mount the ext4 file system on the loop device
        if (mount(loop_device, container_overlay_dir.c_str(), "ext4", MS_MGC_VAL|MS_NOSUID|MS_NODEV, "") == -1) {
            perror("Failed to mount image");
            close(image_fd);
            close(loop_fd);
            close(loop_ctl_fd);
            return 1;
        }

        printf("Image mounted successfully at %s\n", container_overlay_dir.c_str());

        close(image_fd);
        close(loop_fd);
        close(loop_ctl_fd);

        int result = mkdir(container_delta_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (result < 0) {
            perror("mkdir");
            return 1;
        }
        //--------------------------------
        printf("Mounted %s to %s\n", container_overlay_img.c_str(), container_overlay_dir.c_str());

        //    if (!container->start(container,0,NULL)) {
        //        std::cerr<<"error createlb"<<std::endl;
        //        return -1;
        //    }

        // Set up the overlay and squashfs as the container's storage

        // if (!container->set_config_item(container, "lxc.rootfs.options", ("lowerdir=" + overlay_lower + ",upperdir=" + overlay_upper + ",workdir=" + overlay_workdir).c_str())) {
        //     throw std::runtime_error("Failed to set container rootfs options");
        // }
        // if (!container->set_config_item(container, "lxc.rootfs.mount", squashfs_mount.c_str())) {
        //     throw std::runtime_error("Failed to set container rootfs mount");
        // }
        // if (!container->set_config_item(container, "lxc.rootfs.backend", "squashfs")) {
        //     throw std::runtime_error("Failed to set container rootfs backend");
        // }
        // if (!container->set_config_item(container, "lxc.rootfs.options", ("loop=" + squashfs_file).c_str())) {
        //     throw std::runtime_error("Failed to set container rootfs options");
        // }
    }else{
        if (!container->createl(container, "busybox", nullptr, nullptr, LXC_CREATE_QUIET, nullptr)) {
            std::cerr<<"error createlb"<<std::endl;
            return -1;
        }
    }
    return 0;
}

int LxcContainer::start() {
    if (!container) {
        std::cerr<<"Failed to setup lxc_container struct"<<std::endl;
        return -1;
    }

    if (!container->start(container, 0, nullptr)) {
        std::cerr<<"Failed to start the container"<<std::endl;
        return -1;
    }
    return 0;
}

int LxcContainer::stop() {
    if (!container) {
        std::cerr<<"Failed to setup lxc_container struct"<<std::endl;
        return -1;
    }

    if (!container->shutdown(container, 30)) {
        if (!container->stop(container)) {
            std::cerr<<"Failed to kill the container."<<std::endl;
            return -1;
        }
    }
    return 0;
}

int LxcContainer::reconfigure() {
    return 0;
}

int LxcContainer::destroy() {
    if (!container) {
        std::cerr<<"Failed to setup lxc_container struct"<<std::endl;
        return -1;
    }
    int ret = 0;
    std::string overlay_path = LXC_DEFAULT_FOLDER + std::string(container->name)+"/overlay" ;
    // Unmount the file system
    ret = umount(overlay_path.c_str());
    if (ret == -1) {
        perror("umount failed");
        exit(EXIT_FAILURE);
    }
    printf("Unmounted %s\n", overlay_path.c_str());
    if (!container->destroy(container)) {
        std::cerr<<"Failed to destroy the container."<<std::endl;
        return -1;
    }
    return 0;
}

void LxcContainer::setName(const std::string &newName)
{
    m_name = newName;
}

void LxcContainer::setTemplate(const std::string &newTemplate)
{
    m_template = newTemplate;
}

void LxcContainer::setStorage_space(const int newStorage_space)
{
    storage_space = newStorage_space;
}

void LxcContainer::setMemory(const std::string &newMemory)
{
    memory = newMemory;
}

void LxcContainer::setCpuset(const std::string &newCpuset)
{
    cpuset = newCpuset;
}

void LxcContainer::setCpupercent(const std::string &newCpupercent)
{
    cpupercent = newCpupercent;
}

void LxcContainer::setContainer(lxc_container *newContainer)
{
    container = newContainer;
}

void LxcContainer::setAction(Method newAction)
{
    m_action = newAction;
}
