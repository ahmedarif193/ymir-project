#ifndef LXCCONTAINER_H
#define LXCCONTAINER_H
#include <iostream>
#include <lxc/lxccontainer.h>
#include "common-task.h"

// Function to execute a shell command and return its output


class LxcContainer : public Task {
public:
    LxcContainer(const std::string name, const char* m_template = "busybox", const Method action = Method::IDLE);
    ~LxcContainer();

    int run() override;

    int create();

    int start();

    int stop();

    int reconfigure();

    int destroy();


    void setName(const std::string &newName);
    void setTemplate(const std::string &newTemplate);
    void setStorage_space(const std::string &newStorage_space);
    void setMemory(const std::string &newMemory);
    void setCpuset(const std::string &newCpuset);
    void setCpupercent(const std::string &newCpupercent);
    void setContainer(lxc_container *newContainer);

    void setAction(Method newAction);

private:
    std::string exec(std::string cmd, int &retcode) {
        std::cout<<"execute the cmd : "<<cmd<<std::endl;
        std::string result = "";
        char buffer[128];
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer, sizeof buffer, pipe) != NULL) {
                result += buffer;
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        retcode = WEXITSTATUS(pclose(pipe));
        return result;
    }
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
