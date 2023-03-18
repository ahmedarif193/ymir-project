/* Feel free to use this example code in any way
   you see fit (Public Domain) */



#include "httphandler.h"
#include "lxc-container.h"
#include "lxcqueue.h"

#define SEND_RESPONSE(connection, response_body) \
    do { \
    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(response_body.size(), (void*) response_body.c_str(), MHD_RESPMEM_MUST_COPY); \
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response); \
    MHD_destroy_response(mhd_response); \
    return ret; \
    } while (0)

int supported_API_versions(struct MHD_Connection *connection
                           , const std::unordered_map<std::string, std::string>& params
                           , const std::string &request_body) {
    // Return a response with the message "Hello, world!"
    Json::Value root;
    root["metadata"].append("/1.0");
    root["status"] = "Success";
    root["status_code"] = 200;
    root["type"] = "sync";

    std::cout << root.toStyledString() << std::endl;

    SEND_RESPONSE(connection, root.toStyledString());
}


int handle_echo(struct MHD_Connection *connection
                , const std::unordered_map<std::string, std::string>& params
                , const std::string &request_body) {
    //    for (const auto& kv : params) {
    //        std::cout << kv.first << " = " << kv.second << std::endl;
    //    }
    //    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(request_body.size(), (void*) request_body.c_str(), MHD_RESPMEM_MUST_COPY);
    //    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    //    MHD_destroy_response(mhd_response);
    //    return ret;
    std::cout << params.at("value1") << std::endl;
    SEND_RESPONSE(connection, request_body);
}
int get_Server_environment(struct MHD_Connection *connection
                           , const std::unordered_map<std::string, std::string>& params
                           , const std::string &request_body){
    // Create a Json::Value object
    Json::Value root;

    // Set the metadata field
    root["metadata"]["api_extensions"].append("etag");
    root["metadata"]["api_extensions"].append("patch");
    root["metadata"]["api_extensions"].append("network");
    root["metadata"]["api_extensions"].append("storage");
    root["metadata"]["api_status"] = "stable";
    root["metadata"]["api_version"] = "1.0";
    root["metadata"]["auth"] = "untrusted";
    root["metadata"]["auth_methods"].append("tls");
    root["metadata"]["auth_methods"].append("candid");

    // Get system information using Linux syscalls
    struct utsname sys_info;
    if (uname(&sys_info) == -1) {
        std::cerr << "Failed to get system information" << std::endl;
        return 1;
    }
    // Set the config field
    root["metadata"]["config"]["core.https_address"] = ":8443";
    root["metadata"]["config"]["core.trust_password"] = true;

    // Set the environment field
    root["metadata"]["environment"]["addresses"].append(":8443");
    root["metadata"]["environment"]["architectures"].append(sys_info.machine);
    root["metadata"]["environment"]["certificate"] = "X509 PEM certificate";
    root["metadata"]["environment"]["certificate_fingerprint"] = "fd200419b271f1dc2a5591b693cc5774b7f234e1ff8c6b78ad703b6888fe2b69";
    root["metadata"]["environment"]["driver"] = "lxc";
    root["metadata"]["environment"]["driver_version"] = "4.0.7 | 5.2.0";
    root["metadata"]["environment"]["firewall"] = "nftables";
    root["metadata"]["environment"]["kernel"] = sys_info.sysname;
    root["metadata"]["environment"]["kernel_architecture"] = sys_info.machine;
    root["metadata"]["environment"]["kernel_features"]["netnsid_getifaddrs"] = "true";
    root["metadata"]["environment"]["kernel_features"]["seccomp_listener"] = "true";
    root["metadata"]["environment"]["kernel_version"] = sys_info.release;
    root["metadata"]["environment"]["lxc_features"]["cgroup2"] = "true";
    root["metadata"]["environment"]["lxc_features"]["devpts_fd"] = "true";
    root["metadata"]["environment"]["lxc_features"]["pidfd"] = "true";
    root["metadata"]["environment"]["os_name"] = "Ubuntu";
    root["metadata"]["environment"]["os_version"] = "22.04";
    root["metadata"]["environment"]["project"] = "default";
    root["metadata"]["environment"]["server"] = "lxcd";
    root["metadata"]["environment"]["server_clustered"] = false;
    //    root["metadata"]["environment"]["server_event_mode"] = "full-mesh";
    root["metadata"]["environment"]["server_name"] = sys_info.nodename;
    root["metadata"]["environment"]["server_pid"] = getpid();
    root["metadata"]["environment"]["server_version"] = "1.0";
    root["metadata"]["environment"]["storage"] = "dir | overlay";
    SEND_RESPONSE(connection, root.toStyledString());
}

int execenv_create(struct MHD_Connection *connection
                   , const std::unordered_map<std::string, std::string>& params
                   , const std::string &request_body){
    std::cout << "execenv_create" << std::endl;
    Json::Value rootrequest;
    Json::Value rootreply;
    rootreply["metadata"].append("/1.0");
    rootreply["type"] = "sync";

    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    std::string errors;
    bool parsingSuccessful = reader->parse(request_body.c_str(), request_body.c_str() + request_body.size(), &rootrequest, &errors);
    delete reader;

    if (!parsingSuccessful) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        rootreply["status"] = "Failed to parse JSON: " + errors;
        rootreply["status_code"] = 500;
        SEND_RESPONSE(connection, rootreply.toStyledString());
    }
    std::string name = rootrequest["name"].asString();
    std::string type = rootrequest["type"].asString();
    std::cout << "Name: " << name << std::endl;
    std::cout << "type: " << type << std::endl;

    LxcContainer container(name);
    container.setAction(Method::ENABLE);
    container.setTemplate(type);
    container.run();
    rootreply["status"] = "Success";
    rootreply["status_code"] = 200;
    SEND_RESPONSE(connection, rootreply.toStyledString());
}
int execenv_ls(struct MHD_Connection *connection
               , const std::unordered_map<std::string, std::string>& params
               , const std::string &request_body){
    Json::Value root;

    int rc;
    char **names;
    struct lxc_container **cret;

    rc = list_all_containers(NULL, &names, &cret);
    if (rc == -1){
        SEND_RESPONSE(connection, root.toStyledString());
    }
    std::cout << "rc  = " <<rc<< std::endl;

    for (int i = 0; i < rc; i++) {
        struct lxc_container *c = cret[i];
        Json::Value container;
        container["id"]=i;
        container["name"]=names[i];
        container["type"]="busybox";

        container["status"]=c->state(c);

        container["pid"]=(int)c->init_pid(c);
        std::cout << "-------" <<std::endl;

        //container["lxc.net.0.link"]=c->get_running_config_item(c,"lxc.net.0.link");
        //blobmsg_add_string(&buf, names[i], c->state(c));
        root["containers"].append(container);
        free(names[i]);
        std::cout << "-------" <<std::endl;
        lxc_container_put(c);
    }
    root["numberOfContainers"]=rc;
    SEND_RESPONSE(connection, root.toStyledString());
}

int execenv_rm(struct MHD_Connection *connection
               , const std::unordered_map<std::string, std::string>& params
               , const std::string &request_body){
    Json::Value root;

    // Set the metadata field
    root["metadata"]["api_extensions"].append("etag");
    root["metadata"]["api_extensions"].append("patch");
    SEND_RESPONSE(connection, root.toStyledString());
}
int execenv_update(struct MHD_Connection *connection
                   , const std::unordered_map<std::string, std::string>& params
                   , const std::string &request_body){
    Json::Value root;

    // Set the metadata field
    root["metadata"]["api_extensions"].append("etag");
    root["metadata"]["api_extensions"].append("patch");
    SEND_RESPONSE(connection, root.toStyledString());
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

    rest_api.register_handler("/echo/{value1}", "GET", handle_echo);

    // Start the listener
    rest_api.start();

    // Wait for user input to stop the listener
    std::cout << "Press Enter to stop the REST API..." << std::endl;
    std::cin.get();

    // Stop the listener
    rest_api.stop();

    return 0;
}
