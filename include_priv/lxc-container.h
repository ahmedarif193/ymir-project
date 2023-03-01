#ifndef LXCCONTAINER_H
#define LXCCONTAINER_H
#include <iostream>
#include <lxc/lxccontainer.h>
#include "common-task.h"

class LxcContainer : public Task {
public:
    LxcContainer(const char* name, const Method action);
    ~LxcContainer();

    void run() override;

    void create();

    void start();

    void stop();

    void reconfigure();

    void destroy();

    Method m_action;
    std::string m_name;

private:
    struct lxc_container *container = nullptr;
};

#endif // LXC-CONTAINER_H
