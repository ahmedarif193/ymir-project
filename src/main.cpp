/* Feel free to use this example code in any way
   you see fit (Public Domain) */



#include "httphandler.h"
#include "lxc-container.h"
#include "lxcqueue.h"

#include <json-c/json.h>

#define SEND_RESPONSE(connection, output) \
    do { \
    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(output.length(), (void*) output.c_str(), MHD_RESPMEM_MUST_COPY); \
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response); \
    MHD_destroy_response(mhd_response); \
    json_object_put(root);\
    return ret; \
    } while (0)

int supported_API_versions(struct MHD_Connection *connection, const lxcd::map<lxcd::string, lxcd::string>& params, const lxcd::string& request_body) {
    // Create a new JSON object
    json_object* root = json_object_new_object();
    json_object* metadata = json_object_new_array();
    json_object_array_add(metadata, json_object_new_string("/1.0"));
    json_object_object_add(root, "metadata", metadata);
    json_object_object_add(root, "status", json_object_new_string("Success"));
    json_object_object_add(root, "status_code", json_object_new_int(200));
    json_object_object_add(root, "type", json_object_new_string("sync"));

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);

    return 0;
}



int handle_echo(struct MHD_Connection *connection
                , const lxcd::map<lxcd::string, lxcd::string>& params
                , const lxcd::string &request_body) {
                    json_object* root = json_object_new_object();
    SEND_RESPONSE(connection, request_body);
}

int get_Server_environment(struct MHD_Connection *connection,
                           const lxcd::map<lxcd::string, lxcd::string>& params,
                           const lxcd::string &request_body) {

    // Create a json-c object
    json_object* root = json_object_new_object();
    json_object* metadata = json_object_new_object();
    json_object* api_extensions = json_object_new_array();
    json_object_array_add(api_extensions, json_object_new_string("etag"));
    json_object_array_add(api_extensions, json_object_new_string("patch"));
    json_object_array_add(api_extensions, json_object_new_string("network"));
    json_object_array_add(api_extensions, json_object_new_string("storage"));
    json_object_object_add(metadata, "api_extensions", api_extensions);
    json_object_object_add(metadata, "api_status", json_object_new_string("stable"));
    json_object_object_add(metadata, "api_version", json_object_new_string("1.0"));
    json_object_object_add(metadata, "auth", json_object_new_string("untrusted"));

    // Get system information using Linux syscalls
    struct utsname sys_info;
    if (uname(&sys_info) == -1) {
        fprintf(stderr, "Failed to get system information\n");
        return 1;
    }
    json_object* config = json_object_new_object();
    json_object_object_add(config, "core.https_address", json_object_new_string(":8443"));
    json_object_object_add(config, "core.trust_password", json_object_new_boolean(true));

    json_object* environment = json_object_new_object();
    json_object* addresses = json_object_new_array();
    json_object_array_add(addresses, json_object_new_string(":8443"));
    json_object_object_add(environment, "addresses", addresses);
    json_object* architectures = json_object_new_array();
    json_object_array_add(architectures, json_object_new_string(sys_info.machine));
    json_object_object_add(environment, "architectures", architectures);
    json_object_object_add(environment, "certificate", json_object_new_string("X509 PEM certificate"));
    json_object_object_add(environment, "certificate_fingerprint", json_object_new_string("fd200419b271f1dc2a5591b693cc5774b7f234e1ff8c6b78ad703b6888fe2b69"));
    json_object_object_add(environment, "driver", json_object_new_string("lxc"));
    json_object_object_add(environment, "driver_version", json_object_new_string("4.0.7 | 5.2.0"));
    json_object_object_add(environment, "firewall", json_object_new_string("nftables"));
    json_object_object_add(environment, "kernel", json_object_new_string(sys_info.sysname));
    json_object_object_add(environment, "kernel_architecture", json_object_new_string(sys_info.machine));
    json_object* kernel_features = json_object_new_object();
    json_object_object_add(kernel_features, "netnsid_getifaddrs", json_object_new_string("true"));
    json_object_object_add(kernel_features, "seccomp_listener", json_object_new_string("true"));
    json_object_object_add(environment, "kernel_features", kernel_features);
    json_object_object_add(environment, "kernel_version", json_object_new_string(sys_info.release));
    json_object* lxc_features = json_object_new_object();
    json_object_object_add(lxc_features, "cgroup2", json_object_new_string("true"));
    json_object_object_add(lxc_features, "devpts_fd", json_object_new_string("true"));
    json_object_object_add(lxc_features, "pidfd", json_object_new_string("true"));
    json_object_object_add(environment, "lxc_features", lxc_features);

    json_object_object_add(lxc_features, "os_name", json_object_new_string("Ubuntu"));
    json_object_object_add(lxc_features, "os_version", json_object_new_string("22.04"));

    json_object_object_add(lxc_features, "project", json_object_new_string("default"));
    json_object_object_add(lxc_features, "server", json_object_new_string("lxcd"));
    json_object_object_add(lxc_features, "server_clustered", json_object_new_boolean(false));
    json_object_object_add(lxc_features, "server_name", json_object_new_string(sys_info.nodename));

    json_object_object_add(lxc_features, "server_pid", json_object_new_int(getpid()));
    json_object_object_add(lxc_features, "server_version", json_object_new_string("1.0"));
    json_object_object_add(environment, "storage", json_object_new_string("dir | overlay"));
    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}

int execenv_create(struct MHD_Connection *connection,
                   const lxcd::map<lxcd::string, lxcd::string>& params,
                   const lxcd::string& request_body) {
    printf("execenv_create\n");
    json_object* rootrequest = json_tokener_parse(request_body.c_str());
    json_object* root = json_object_new_object();

    json_object* metadata = json_object_new_array();
    json_object_array_add(metadata, json_object_new_string("/1.0"));
    json_object_object_add(root, "metadata", metadata);

    json_object_object_add(root, "type", json_object_new_string("sync"));

    if (rootrequest == NULL) {
        fprintf(stderr, "Failed to parse JSON file.\n");
        json_object_object_add(root, "status", json_object_new_string("Failed to parse JSON file."));
        json_object_object_add(root, "status_code", json_object_new_int(500));
        lxcd::string output = json_object_to_json_string(root);
        SEND_RESPONSE(connection, output);
        return -1;
    }

    const char* name = json_object_get_string(json_object_object_get(rootrequest, "name"));
    const char* type = json_object_get_string(json_object_object_get(rootrequest, "type"));
    printf("Name: %s\n", name);
    printf("Type: %s\n", type);

    LxcContainer container(name);
    container.setAction(Method::ENABLE);
    container.setTemplate(type);
    container.run();

    json_object_object_add(root, "status", json_object_new_string("Success"));
    json_object_object_add(root, "status_code", json_object_new_int(200));
    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);

    return 0;
}

int execenv_ls(struct MHD_Connection *connection, const lxcd::map<lxcd::string, lxcd::string>& params, const lxcd::string &request_body) {
    json_object *root = json_object_new_object();

    int rc;
    char **names;
    struct lxc_container **cret;

    rc = list_all_containers(NULL, &names, &cret);
    if (rc == -1) {
        lxcd::string output = json_object_to_json_string(root);
        SEND_RESPONSE(connection, output);
    }

    json_object *containers_array = json_object_new_array();
    for (int i = 0; i < rc; i++) {
        struct lxc_container *c = cret[i];
        json_object *container = json_object_new_object();
        json_object_object_add(container, "id", json_object_new_int(i));
        json_object_object_add(container, "name", json_object_new_string(names[i]));
        json_object_object_add(container, "type", json_object_new_string("busybox"));

        json_object_object_add(container, "status", json_object_new_string(c->state(c)));

        json_object_object_add(container, "pid", json_object_new_int((int)c->init_pid(c)));

        //json_object_object_add(container, "lxc.net.0.link", json_object_new_string(c->get_running_config_item(c,"lxc.net.0.link")));

        json_object_array_add(containers_array, container);
        free(names[i]);
        lxc_container_put(c);
    }
    json_object_object_add(root, "containers", containers_array);
    json_object_object_add(root, "numberOfContainers", json_object_new_int(rc));

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}


int execenv_rm(struct MHD_Connection *connection, const lxcd::map<lxcd::string, lxcd::string>& params, const lxcd::string &request_body) {
    json_object *root = json_object_new_object();

    json_object *rootrequest = json_tokener_parse(request_body.c_str());

    lxcd::string name = json_object_get_string(json_object_object_get(rootrequest, "name"));
    LxcContainer container(name);
    container.setAction(Method::DESTROY);
    container.run();

    json_object_object_add(root, "metadata", json_object_new_array());
    json_object_array_add(json_object_object_get(root, "metadata"), json_object_new_string("/1.0"));
    json_object_object_add(root, "type", json_object_new_string("sync"));
    json_object_object_add(root, "status", json_object_new_string("Success"));
    json_object_object_add(root, "status_code", json_object_new_int(200));

    json_object_put(rootrequest);
    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}

int execenv_update(struct MHD_Connection *connection
                   , const lxcd::map<lxcd::string, lxcd::string>& params
                   , const lxcd::string &request_body){

    json_object *root = json_object_new_object();

    json_object *metadata = json_object_new_object();
    json_object_array_add(metadata, json_object_new_string("etag"));
    json_object_array_add(metadata, json_object_new_string("patch"));
    json_object_object_add(root, "metadata", metadata);

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}

int deployementunit_create(struct MHD_Connection *connection
                   , const lxcd::map<lxcd::string, lxcd::string>& params
                   , const lxcd::string &request_body){
    json_object *root = json_object_new_object();

    json_object *metadata = json_object_new_object();
    json_object_array_add(metadata, json_object_new_string("etag"));
    json_object_array_add(metadata, json_object_new_string("patch"));
    json_object_object_add(root, "metadata", metadata);

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}

int deployementunit_delete(struct MHD_Connection *connection
                   , const lxcd::map<lxcd::string, lxcd::string>& params
                   , const lxcd::string &request_body){
    json_object *root = json_object_new_object();

    json_object *metadata = json_object_new_object();
    json_object_array_add(metadata, json_object_new_string("etag"));
    json_object_array_add(metadata, json_object_new_string("patch"));
    json_object_object_add(root, "metadata", metadata);

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}

int deployementunit_ls(struct MHD_Connection *connection
                   , const lxcd::map<lxcd::string, lxcd::string>& params
                   , const lxcd::string &request_body){
    json_object *root = json_object_new_object();

    json_object *metadata = json_object_new_object();
    json_object_array_add(metadata, json_object_new_string("etag"));
    json_object_array_add(metadata, json_object_new_string("patch"));
    json_object_object_add(root, "metadata", metadata);

    lxcd::string output = json_object_to_json_string(root);
    SEND_RESPONSE(connection, output);
}


int main() {
    // Create a RestApiListener instance on port 8080
    RestApiListener rest_api(8080);

    // handle those rpcs in this link https://linuxcontainers.org/lxd/docs/latest/api/#/server/api_get
    // Register handlers for some example endpoints
    rest_api.register_handler("/", "GET", supported_API_versions);
    rest_api.register_handler("/2.0/", "GET", get_Server_environment);

    rest_api.register_handler("/2.0/execenv", "POST", execenv_create);
    rest_api.register_handler("/2.0/execenv", "GET", execenv_ls);
    rest_api.register_handler("/2.0/execenv", "DELETE", execenv_rm);
    rest_api.register_handler("/2.0/execenv", "UPDATE", execenv_update);

    rest_api.register_handler("/2.0/deployementunit", "POST", deployementunit_create);
    rest_api.register_handler("/2.0/deployementunit", "GET", deployementunit_ls);
    rest_api.register_handler("/2.0/deployementunit", "DELETE", deployementunit_delete);

    rest_api.register_handler("/echo/{value1}", "GET", handle_echo);

    // Start the listener
    rest_api.start();

    // Wait for user input to stop the listener
    printf("Press Enter to stop the REST API...\n");
    std::cin.get();

    // Stop the listener
    rest_api.stop();

    return 0;
}
