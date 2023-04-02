#include "deployment_unit.h"

class DeploymentUnitHelper {
public:
    DeploymentUnitHelper();
    std::shared_ptr<DeploymentUnit> getDeploymentUnit(const std::string& container);
    bool addDeploymentUnit(const std::string &container, const std::string& tarballPath, const std::string &uuid);
    bool removeDeploymentUnit(const std::string& uuid);
    void listDeploymentUnits();

private:
    std::map<std::string, std::shared_ptr<DeploymentUnit>> deploymentUnits;
    bool loadCache();
    bool saveCache();
};
