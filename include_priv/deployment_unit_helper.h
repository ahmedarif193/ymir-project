#ifndef DUNIT_HELPER_H
#define DUNIT_HELPER_H
#include "utils/map.h"
#include "utils/sharedptr.h"
#include "deployment_unit.h"

class DeploymentUnit;

class DeploymentUnitHelper {
public:
DeploymentUnitHelper();
lxcd::SharedPtr<DeploymentUnit> getDeploymentUnit(const lxcd::string& name);
lxcd::SharedPtr<DeploymentUnit> addDeploymentUnit(const lxcd::string &container, const lxcd::string& tarballPath, const lxcd::string &uuid);
bool removeDeploymentUnit(const lxcd::string& uuid);
bool updateFullRootPath(struct lxc_container* container);
bool restartContainer(struct lxc_container* container);
bool mount(struct lxc_container* container);
void freeContainer(struct lxc_container* container);
struct lxc_container* getContainer(const lxcd::string& c);

void ls();

private:
lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit> > deploymentUnits;
bool loadCache();
bool saveCache();
};
#endif
