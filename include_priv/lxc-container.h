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

typedef lxcd::vector<lxcd::string> IPAddressList;

static lxcd::string getLxcPath() {
	const char* lxcPath = lxc_get_global_config_item("lxc.lxcpath");
	if(lxcPath) {
		return lxcd::string(lxcPath) + "/";
	}
	fprintf(stderr, "Error: Unable to get lxc.lxcpath, set to default /srv/lxc \n");
	return "/srv/lxc";
}

class DeploymentUnitHelper;

class LxcContainer {
public:
LxcContainer(const lxcd::string name, lxcd::string mTemplate = "busyboxv2", const Method action = Method::IDLE);
~LxcContainer();

bool isRunning() const {
	if (container != nullptr) {
		return container->is_running(container);
	}
	return false;
}

int run();

int create();

int prepare();

int start();

int stop();

int reconfigure();

int destroy();

int setupOverlayFs();
int createOverlayImage(const lxcd::string& overlayImgPath);
bool mountOverlayFs(const lxcd::string& overlayImgPath, const lxcd::string& overlayDirPath);
void makeDirectory(const lxcd::string& dirPath);

IPAddressList getIPAddresses();
void setName(const lxcd::string &newName);
void setTemplate(const lxcd::string &newTemplate);
void setStorageSpace(const int newStorage_space);
void setMemory(const lxcd::string &newMemory);
void setCpuset(const lxcd::string &newCpuset);
void setCpupercent(const lxcd::string &newCpupercent);
void setContainer(lxc_container* newContainer);

void setAction(Method newAction);

private:

bool useOverlay;
Method m_action;
lxcd::string m_name;
lxcd::string mTemplate;
int storageSpace;
lxcd::string memory;
lxcd::string cpuset;
lxcd::string cpupercent;
struct lxc_container* container = nullptr;
};

#endif // LXC-CONTAINER_H
