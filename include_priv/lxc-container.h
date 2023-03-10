#ifndef LXCCONTAINER_H
#define LXCCONTAINER_H
#include <iostream>
#include <lxc/lxccontainer.h>
#include "common-task.h"

class LxcContainer : public Task {
public:
    LxcContainer(const std::string name, const char* m_template = "busybox", const Method action = Method::IDLE);
    ~LxcContainer();

    int run() override;

    void create();

    void start();

    void stop();

    void reconfigure();

    void destroy();


    void setName(const std::string &newName);
    void setTemplate(const std::string &newTemplate);
    void setStorage_space(const std::string &newStorage_space);
    void setMemory(const std::string &newMemory);
    void setCpuset(const std::string &newCpuset);
    void setCpupercent(const std::string &newCpupercent);
    void setContainer(lxc_container *newContainer);

    void setAction(Method newAction);

private:

    Method m_action;
    std::string m_name;
    std::string m_template;
    std::string storage_space;
    std::string memory;
    std::string cpuset;
    std::string cpupercent;

    struct lxc_container *container = nullptr;
};

#endif // LXC-CONTAINER_H
