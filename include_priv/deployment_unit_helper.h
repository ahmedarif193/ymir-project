#include "deployment_unit.h"

class DeploymentUnitHelper {
public:
    DeploymentUnitHelper();
    std::shared_ptr<DeploymentUnit> getDeploymentUnit(const lxcd::string& uuid);
    bool addDeploymentUnit(const lxcd::string &container, const lxcd::string& tarballPath, const lxcd::string &uuid);
    bool removeDeploymentUnit(const lxcd::string& uuid);
    struct lxc_container* getContainer(const lxcd::string& c);

    void listDeploymentUnits();

private:
    std::map<lxcd::string, std::shared_ptr<DeploymentUnit>> deploymentUnits;
    bool loadCache();
    bool saveCache();
};
