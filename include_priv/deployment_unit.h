#ifndef DUNIT_H
#define DUNIT_H
#include <json-c/json.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include <lxc-container.h>

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
#define LXCD_PATH "/opt/lxcd/share/"
#define LXCD_DU_INDEX_PATH LXCD_PATH "index.json"

class JsonHelper {
public:
static struct json_object* GetJsonObject(struct json_object* obj, const char* key) {
	struct json_object* value;
	if (json_object_object_get_ex(obj, key, &value) && value != nullptr && json_object_is_type(value, json_type_object)) {
		return value;
	} else {
		printf("Attribute '%s' is not defined or not a JSON object.\n", key);
		return nullptr;
	}
}

static struct json_object* GetJsonArray(struct json_object* obj, const char* key) {
	struct json_object* value;
	if (json_object_object_get_ex(obj, key, &value) && value != nullptr) {
		if (json_object_is_type(value, json_type_array)) {
			return value;
		} else {
			printf("Attribute '%s' is not a JSON array.\n", key);
			return nullptr;
		}
	} else {
		printf("Attribute '%s' is not defined.\n", key);
		return nullptr;
	}
}

static lxcd::string GetString(struct json_object* obj, const char* key, const lxcd::string& defaultValue = "") {
	struct json_object* value;
	if (json_object_object_get_ex(obj, key, &value) && value != nullptr) {
		return lxcd::string(json_object_get_string(value));
	} else {
		printf("Attribute '%s' is not defined.\n", key);
		return defaultValue;
	}
}

static int GetInt(struct json_object* obj, const char* key, int defaultValue = 0) {
	struct json_object* value;
	if (json_object_object_get_ex(obj, key, &value) && value != nullptr) {
		return json_object_get_int(value);
	} else {
		printf("Attribute '%s' is not defined.\n", key);
		return defaultValue;
	}
}

static bool GetBoolean(struct json_object* obj, const char* key, bool defaultValue = false) {
	struct json_object* value;
	if (json_object_object_get_ex(obj, key, &value) && value != nullptr) {
		return json_object_get_boolean(value);
	} else {
		printf("Attribute '%s' is not defined.\n", key);
		return defaultValue;
	}
}
};

class DeploymentUnit {
public:
DeploymentUnit(const lxcd::string &uuid);
bool prepare(const lxcd::string& tarballPath, const lxcd::string &executionEnvRef);
bool install();

bool duRemove();

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
lxcd::time_t installationDate;
struct eu {
	lxcd::string name;
	lxcd::string exec;
	lxcd::string pidfile;
	bool autostart;
};
lxcd::vector<eu> executionunits;
int version;
bool mounted;

// IPK package names
lxcd::vector<lxcd::string> ipkPackages;
};

#endif
