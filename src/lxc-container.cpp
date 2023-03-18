#include "lxc-container.h"
#include <sys/stat.h>
LxcContainer::LxcContainer(const std::string name, const char* m_template, const Method action) {

    this->m_action = action;
    this->m_name = name;
    this->m_template = m_template;
    this->storage_space = "20";
    this->memory = "20";
    this->cpuset = "0-1";
    this->cpupercent = "50";
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

int LxcContainer::run() {
    int ret = 0;
    if (m_action == Method::ENABLE) {
        ret|= create();
        ret|= start();
    } else if (m_action == Method::DISABLE) {
        ret|= stop();
    } else if (m_action == Method::RECONFIGURE) {
        ret|= reconfigure();
        ret|= stop();
        ret|= start();
    } else if (m_action == Method::RESET) {
        ret|= stop();
        //TODO : just clear delta overlayfs if used
        ret|= destroy();
        ret|= create();
        ret|= start();
    } else if (m_action == Method::DESTROY) {
        ret|= stop();
        ret|= destroy();
    } else {
        ;
    }
    return ret;
}

constexpr char* lxc_default_folder = "/var/lib/lxc/";

int LxcContainer::create() {
    if (!container) {
        std::cerr<<"Failed to setup lxc_container struct"<<std::endl;
        return -1;
    }

    if (container->is_defined(container)) {
        std::cerr<<"Container already exists"<<std::endl;
        return -1;
    }

    // // Set up the overlay storage
    // std::string overlay_dir = "/tmp/overlay/dir";
    // std::string overlay_lower = "/tmp/lower/dir";
    // std::string overlay_upper = "/tmp/upper/dir";
    // std::string overlay_workdir = "/tmp/work/dir";
    // if (mkdir(overlay_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    //     throw std::runtime_error("Failed to create overlay directory");
    // }
    // if (mkdir(overlay_lower.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    //     throw std::runtime_error("Failed to create overlay lower directory");
    // }
    // if (mkdir(overlay_upper.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    //     throw std::runtime_error("Failed to create overlay upper directory");
    // }
    // if (mkdir(overlay_workdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    //     throw std::runtime_error("Failed to create overlay work directory");
    // }

    // // Set up the squashfs storage
    // std::string squashfs_file = "/tmp/squashfs/file";
    // std::string squashfs_mount = "/tmp/squashfs/mount";
    // if (mkdir(squashfs_mount.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    //     throw std::runtime_error("Failed to create squashfs mount directory");
    // }
    // if (system(("mksquashfs " + overlay_upper + " " + squashfs_file).c_str()) == -1) {
    //     throw std::runtime_error("Failed to create squashfs file");
    // }

    // Create the container
    std::cout<<"---------Create the container : "<<container->name;
    if (!container->createl(container, "busybox", nullptr, nullptr, LXC_CREATE_QUIET, nullptr)) {
        std::cout<<"---------createl : "<<container->configfile;
        return -1;

    }

    if(0){
        int retcode = 0;
        std::string container_path = lxc_default_folder + this->m_name ;
        std::string container_overlay_dir = container_path +"/overlay";
        std::string container_delta_dir = container_overlay_dir +"/delta";
        std::string command = "dd if=/dev/zero of=" + container_path + "/overlay.img bs=1M count=" + this->storage_space;
        this->exec(command,retcode);
        if(retcode){
            std::cerr<<"error dd"<<std::endl;
            return -1;

        }
        command = "mkfs.ext4 " + container_path + "/overlay.img";
        this->exec(command,retcode);
        if(retcode){
            std::cerr<<"error mkfs.ext4"<<std::endl;
            return -1;

        }

        command = "mkdir -p " + container_overlay_dir;
        this->exec(command,retcode);
        if(retcode){
            std::cerr<<"Failed to create delta overlay directory"<<std::endl;
            return -1;
        }

        command = "mount -t ext4 -o loop " + container_path + "/overlay.img " + container_overlay_dir;
        this->exec(command,retcode);
        if(retcode){
            std::cerr<<"error mount"<<std::endl;
            return -1;
        }

        command = "mkdir -p " + container_delta_dir;
        this->exec(command,retcode);
        if(retcode){
            std::cerr<<"Failed to create delta overlay directory"<<std::endl;
            return -1;
        }

    }

    // // Set up the overlay and squashfs as the container's storage
    // if (!container->set_config_item(container, "lxc.rootfs.path", overlay_upper.c_str())) {
    //     throw std::runtime_error("Failed to set container rootfs path");
    // }
    // if (!container->set_config_item(container, "lxc.rootfs.backend", "overlayfs")) {
    //     throw std::runtime_error("Failed to set container rootfs backend");
    // }
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
    int retcode = 0;
    std::string containerpath = lxc_default_folder + std::string(container->name) ;
    std::string command = "umount " + containerpath + "/overlay.img";
    this->exec(command,retcode);
    if(!retcode){
        std::cerr<<"error dd"<<std::endl;
        return -1;
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

void LxcContainer::setStorage_space(const std::string &newStorage_space)
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
