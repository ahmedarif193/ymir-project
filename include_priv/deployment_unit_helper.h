#include "deployment_unit.h"
#include "utils/map.h"

class DeploymentUnitHelper {
public:
DeploymentUnitHelper();
lxcd::SharedPtr<DeploymentUnit> getDeploymentUnit(const lxcd::string& uuid);
bool addDeploymentUnit(const lxcd::string &container, const lxcd::string& tarballPath, const lxcd::string &uuid);
bool removeDeploymentUnit(const lxcd::string& uuid);
bool updateFullRootPath(struct lxc_container* container);
bool restartContainer(struct lxc_container* container);

struct lxc_container* getContainer(const lxcd::string& c);

void listDeploymentUnits();

private:
lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit> > deploymentUnits;
bool loadCache();
bool saveCache();
};
