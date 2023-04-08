#include "deployment_unit.h"
#include "utils/map.h"

class DeploymentUnitHelper {
public:
    DeploymentUnitHelper();
    std::shared_ptr<DeploymentUnit> getDeploymentUnit(const lxcd::string& uuid);
    bool addDeploymentUnit(const lxcd::string &container, const lxcd::string& tarballPath, const lxcd::string &uuid);
    bool removeDeploymentUnit(const lxcd::string& uuid);
    bool updateFullRootPath(struct lxc_container *container);
    bool restartContainer(struct lxc_container *container);

    struct lxc_container* getContainer(const lxcd::string& c);

    void listDeploymentUnits();

private:
    lxcd::map<lxcd::string, std::shared_ptr<DeploymentUnit>> deploymentUnits;
    bool loadCache();
    bool saveCache();
};
