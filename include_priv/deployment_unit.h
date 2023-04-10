
#include <json-c/json.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include <lxc/lxccontainer.h>

#include "utils/string.h"
#include "utils/sharedptr.h"
#include "utils/linux.h"


//for  syscalls
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <ftw.h>

#define MAXPARAMLEN 1024

class DeploymentUnit {
public:
    DeploymentUnit(const lxcd::string &uuid);
    bool prepare(const lxcd::string& tarballPath, const lxcd::string &executionEnvRef);
    bool install();

    bool remove();

    //TODO :  to be used
    lxcd::string squashfsPath;
    static lxcd::string cacheFilePath;
    static lxcd::string tempDir;

    // DeploymentUnit parameters
    lxcd::string uuid;
    lxcd::string executionEnvRef;
    lxcd::string description;
    lxcd::string vendor;
    lxcd::string type;
    lxcd::string name;
    lxcd::string rootfsPath;
    int version;
    bool mounted;


    // IPK package names
    lxcd::vector<lxcd::string> ipkPackages;

    //metadata
    lxcd::time_t installationDate;
};

lxcd::string getLxcPath();
