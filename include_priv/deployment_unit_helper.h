#include "deployment_unit.h"
#include "utils/map.h"

class DeploymentUnitHelper {
public:
DeploymentUnitHelper();
lxcd::SharedPtr<DeploymentUnit> getDeploymentUnit(const lxcd::string& uuid);
lxcd::pair<lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit> >::Iterator, bool> addDeploymentUnit(const lxcd::string &container, const lxcd::string& tarballPath, const lxcd::string &uuid);
bool removeDeploymentUnit(const lxcd::string& uuid);
bool updateFullRootPath(struct lxc_container* container);
bool restartContainer(struct lxc_container* container);

struct lxc_container* getContainer(const lxcd::string& c);

lxcd::string listDeploymentUnits();

private:
lxcd::map<lxcd::string, lxcd::SharedPtr<DeploymentUnit> > deploymentUnits;
bool loadCache();
bool saveCache();
};


struct LxcdInfo {
    lxcd::string uuid;
    lxcd::string executionEnvRef;
    lxcd::string url;
    lxcd::string user;
    lxcd::string password;
};

bool parse_lxcd_info(const lxcd::string &json_str, LxcdInfo &info);
