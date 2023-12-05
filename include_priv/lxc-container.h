#ifndef LXCCONTAINER_H
#define LXCCONTAINER_H
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/loop.h>
#include <lxc/lxccontainer.h>

#include "common-task.h"
#include "utils/string.h"
#include "deployment_unit_helper.h"


class LxcContainer : public Task {
public:
LxcContainer(const lxcd::string name, const char* m_template = "busybox", const Method action = Method::IDLE);
~LxcContainer();

int run() override;

int create();

int prepare();

int start();

int stop();

int reconfigure();

int destroy();


void setName(const lxcd::string &newName);
void setTemplate(const lxcd::string &newTemplate);
void setStorage_space(const int newStorage_space);
void setMemory(const lxcd::string &newMemory);
void setCpuset(const lxcd::string &newCpuset);
void setCpupercent(const lxcd::string &newCpupercent);
void setContainer(lxc_container* newContainer);

void setAction(Method newAction);

private:

bool use_overlay;
Method m_action;
lxcd::string m_name;
lxcd::string m_template;
int storage_space;
lxcd::string memory;
lxcd::string cpuset;
lxcd::string cpupercent;

struct lxc_container* container = nullptr;
};

#endif // LXC-CONTAINER_H
