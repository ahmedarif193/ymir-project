#include "lxc-container.h"

LxcContainer::LxcContainer(const char *name, const Method action) {
    this->m_action = action;
    this->m_name = name;

}

LxcContainer::~LxcContainer() {}

void LxcContainer::run() {
    if (m_action == Method::ENABLE) {
        create();
        start();
    } else if (m_action == Method::DISABLE) {
        stop();

    } else if (m_action == Method::RECONFIGURE) {
        reconfigure();
        stop();
        start();
    } else if (m_action == Method::RESET) {
        destroy();
        create();
        start();
    } else if (m_action == Method::DESTROY) {
        destroy();
    } else {
        throw std::runtime_error("Invalid action");
    }

    if (m_callback != nullptr) {
        m_callback(m_data);
    }
}

void LxcContainer::create() {
    container = lxc_container_new(m_name.c_str(), nullptr);
    if (!container) {
        throw std::runtime_error("Failed to setup lxc_container struct");
    }

    if (container->is_defined(container)) {
        throw std::runtime_error("Container already exists");
    }

    if (!container->createl(container, "busybox", nullptr, nullptr, LXC_CREATE_QUIET, nullptr)) {
        throw std::runtime_error("Failed to create container rootfs");
    }
}

void LxcContainer::start() {
    container = lxc_container_new(m_name.c_str(), nullptr);
    if (!container) {
        throw std::runtime_error("Failed to setup lxc_container struct");
    }

    if (!container->start(container, 0, nullptr)) {
        throw std::runtime_error("Failed to start the container");
    }
}

void LxcContainer::stop() {
    container = lxc_container_new(m_name.c_str(), nullptr);
    if (!container) {
        throw std::runtime_error("Failed to setup lxc_container struct");
    }

    if (!container->shutdown(container, 30)) {
        if (!container->stop(container)) {
            throw std::runtime_error("Failed to kill the container.");
        }
    }
}

void LxcContainer::reconfigure() {
}

void LxcContainer::destroy() {
    container = lxc_container_new(m_name.c_str(), nullptr);
    if (!container) {
        throw std::runtime_error("Failed to setup lxc_container struct");
    }

    if (!container->destroy(container)) {
        throw std::runtime_error("Failed to destroy the container.");
    }
}
