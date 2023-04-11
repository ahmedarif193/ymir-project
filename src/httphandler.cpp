#include "httphandler.h"

RestApiListener::RestApiListener(int port) : port_(port), is_running_(false) {
}

void RestApiListener::register_handler(const lxcd::string &path, const lxcd::string &http_method, handler_t handler) {
    printf("register_handler %s,%s \n", path.c_str(), http_method.c_str());

    handlers_[path][http_method] = handler;
}

void RestApiListener::start() {
    printf("start%d.\n", port_);
    daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port_, nullptr, nullptr, &RestApiListener::dispatch_handler, this, MHD_OPTION_END);
    if(daemon_) {
        is_running_ = true;
        printf("REST API listening on port %d.\n", port_);
    } else {
        fprintf(stderr, "Failed to start REST API\n");
    }
}

void RestApiListener::stop() {
    if(is_running_) {
        MHD_stop_daemon(daemon_);
        printf("REST API stopped\n");
        is_running_ = false;
    }
}

bool RestApiListener::create_pipe_socket() {
    if(mkfifo(pipe_name_.c_str(), 0666) < 0) {
        perror("Failed to create pipe socket");
        return false;
    }
    return true;
}

bool RestApiListener::remove_pipe_socket() {
    if(unlink(pipe_name_.c_str()) < 0) {
        perror("Failed to remove pipe socket");
        return false;
    }
    return true;
}
#include <iostream>
int RestApiListener::dispatch_handler(void* cls, MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls) {
    lxcd::string mmethod = method;
    lxcd::string murl = url;
    lxcd::string mversion = version;

    static int count = 0;

    static int dummy;
    static char* buffer = new char[500];
    int ret;
    printf("dispatch_handler dummy %dcount %d upload_data_size %lu murl %s mmethod %s mversion %s", dummy, count++, *upload_data_size, murl.c_str(), mmethod.c_str(), mversion.c_str());
    //    std::cout <<dummy<< count++<<"dispatch_handler" <<*upload_data_size<<murl<<mmethod<<mversion << std::endl;

    if(&dummy != *con_cls) {
        *con_cls = &dummy;
        return MHD_YES;
    }

    if(*upload_data_size != 0) {
        strncpy(buffer, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }
    printf("start.\n");

    auto* rest_api_listener = reinterpret_cast<RestApiListener*>(cls);
    lxcd::map<lxcd::string, lxcd::string> params;
    lxcd::string request_body = buffer;
    lxcd::string http_method = method ? method : "";
    lxcd::string path = url ? url : "";

    // Find the handler function for the path and HTTP method
    //    auto path_handlers_it = rest_api_listener->handlers_.find(path);
    //    if (path_handlers_it == rest_api_listener->handlers_.end()) {
    //        // static Path not found
    //    }
    bool found = false;
    auto path_handlers_it = rest_api_listener->handlers_.begin();
    for(; path_handlers_it != rest_api_listener->handlers_.end(); ++path_handlers_it) {
        lxcd::string k = path_handlers_it->first;
        printf("k =  path_handlers_it->first %s == %s.\n", k.c_str(), path.c_str());
        if(rest_api_listener->is_match_match_regex(k, path, params)) {
            printf("k =  ---------- found\n");

            for(const auto& kv : params) {
                std::cout << kv.first << " = " << kv.second << std::endl;
            }
            found = true;
            break;
        }
        //ignore value
        //Value v = iter->second;
    }
    printf("start2.\n");

    if(!found) {
        return MHD_NO;
    }
    printf("start3.\n");
    printf("k for test  ---------- found %s\n", http_method.c_str());
    //TODO : fix map's find function
    for(const auto &method : path_handlers_it->second) {
        if(method.first == http_method) {
            printf("k =  ---------- found %s\n", method.first.c_str());
            handler_t handler = method.second;
            lxcd::string reply_body;
            auto ret = handler(params, request_body, reply_body);
            struct MHD_Response* mhd_response = MHD_create_response_from_buffer(reply_body.length(), (void*) reply_body.c_str(), MHD_RESPMEM_MUST_COPY);
            return MHD_queue_response(connection, ret, mhd_response);
        }
    }
    return MHD_YES;
}


bool RestApiListener::is_match_match_regex(lxcd::string path, lxcd::string request, lxcd::map<lxcd::string, lxcd::string> &params) {
    if(are_paths_equal(path, request)) {
        return true;
    }
    const lxcd::string param_regex = "\\{([^}]+)\\}";

    lxcd::vector<lxcd::string> param_names;
    size_t pos = 0;
    while((pos = path.find(param_regex, pos)) != lxcd::string::npos) {
        pos += 1; // Skip the '{' character
        size_t param_end_pos = path.find('}', pos);
        if(param_end_pos == lxcd::string::npos) {
            break; // Invalid path: a parameter is not closed
        }
        size_t param_name_length = param_end_pos - pos;
        param_names.push_back(path.substr(pos, param_name_length));
        pos = param_end_pos + 1; // Skip the '}' character
    }

    // Build regular expression to match path
    lxcd::string path_regex = path;
    for(const auto& param_name : param_names) {
        size_t param_start_pos = path_regex.find("{" + param_name + "}");
        if(param_start_pos == lxcd::string::npos) {
            break; // Invalid path: parameter not found
        }
        path_regex.replace(param_start_pos, param_name.length() + 2, "([^/]+)");
    }
    path_regex = "^" + path_regex + "$";

    // Match request against path regular expression
    bool match = false;
    if(request.size() >= path.size()) {
        lxcd::string request_prefix = request.substr(0, path.size());
        if(request_prefix == path) {
            lxcd::string request_suffix = request.substr(path.size());
            lxcd::vector<lxcd::string> request_params;
            size_t request_pos = 0;
            for(const auto& param_name : param_names) {
                size_t param_end_pos = request_suffix.find('/');
                if(param_end_pos == lxcd::string::npos) {
                    request_params.push_back(request_suffix);
                    break;
                }
                request_params.push_back(request_suffix.substr(0, param_end_pos));
                request_suffix = request_suffix.substr(param_end_pos + 1);
            }
            if(request_params.size() == param_names.size()) {
                // Extract parameter values into map
                for(size_t i = 0; i < param_names.size(); ++i) {
                    params[param_names[i]] = request_params[i];
                }
                match = true;
            }
        }
    }
    return match;
}

lxcd::string RestApiListener::replace_double_slashes(const lxcd::string& str) {
    lxcd::string result;
    bool prev_is_slash = false;
    for(char c : str) {
        if(c == '/') {
            if(!prev_is_slash) {
                result += c;
                prev_is_slash = true;
            }
        } else {
            result += c;
            prev_is_slash = false;
        }
    }
    return result;
}

void* RestApiListener::listen_thread_func(void* arg) {
    RestApiListener* handler = reinterpret_cast<RestApiListener*>(arg);
    int fd = open(handler->pipe_name_.c_str(), O_RDONLY);
    if(fd < 0) {
        perror("Failed to open pipe socket");
        return NULL;
    }

    char buf[1024];
    while(true) {
        size_t len = read(fd, buf, sizeof(buf));
        printf("len %lu buf %s", len, buf);

        if(len <= 0) {
            break;
        }
        buf[len] = '\0';

        json_object* root = json_tokener_parse(buf);
        if(root == NULL) {
            perror("Failed to parse JSON data");
            continue;
        }

        lxcd::SharedPtr<json_object*> path_obj = lxcd::makeShared<json_object*>(json_object_object_get(root, "path"));
        if(!path_obj) {
            perror("Failed to find path in JSON data");
            json_object_put(root);
            continue;
        }
        lxcd::string path_obj_str = json_object_get_string(*path_obj);

        lxcd::SharedPtr<json_object*> method_obj = lxcd::makeShared<json_object*>(json_object_object_get(root, "method"));
        if(!method_obj) {
            perror("Failed to find Method in JSON data");
            json_object_put(root);
            continue;
        }

        lxcd::string method_obj_str = json_object_get_string(*method_obj);
        printf("listen_thread_func method_obj_str %s path_obj_str %s", method_obj_str.c_str(), path_obj_str.c_str());
        dispatch_handler(arg, NULL, path_obj_str, method_obj_str, NULL, buf, &len, NULL);
        //TODO : add handler to fix deleting method
    }

    close(fd);
    return NULL;
}

bool RestApiListener::are_paths_equal(lxcd::string path1, lxcd::string path2) {
    lxcd::string path1Normalized = replace_double_slashes(path1);
    lxcd::string path2Normalized = replace_double_slashes(path2);

    return path1Normalized == path2Normalized;
}
