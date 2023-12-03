#include "httphandler.h"
#include <iostream>

RestApiListener::RestApiListener(int port) : port_(port), is_running_(false) {
}

void RestApiListener::register_handler(const lxcd::string &path, const lxcd::string &http_method, handler_t handler) {
    printf("%lu --------------------------------- -register_handler- %s %s ---------------------------------\n", handlers_.size(), path.c_str(), http_method.c_str());

    bool found = false;
    //auto pathmethos = handlers_[path];
    for(auto &handlermethod : handlers_) {
        printf(" |>>>>>>>>>>>>>>>>>>>>>            %s ==? %s \n", handlermethod.key.c_str(), path.c_str());

        if(handlermethod.key == path) {

            lxcd::map<lxcd::string, handler_t> &path = handlermethod.value;
            printf("equal %lu \n", path.size());
            path.insert({http_method, handler});
            printf("equal2 %lu \n", path.size());
            for(auto handler_obg: path) {
                printf("equal2 %s \n", handler_obg.key.c_str());
            }
            found = true;
            break;
        }
    }

    if(!found) {
        printf("create \n");
        //lxcd::map<lxcd::string, lxcd::map<lxcd::string, handler_t> >
        lxcd::map<lxcd::string, handler_t> newpath;
        newpath.insert({http_method, handler});
        auto pairpath = lxcd::pair{path, newpath};
        handlers_.insert(pairpath);
    }

    //    auto path_handlers_it = handlers_.begin();
    //    for(; path_handlers_it != handlers_.end(); ++path_handlers_it) {
    //        lxcd::string k = path_handlers_it->key;
    //        printf(" >>>>>>>>>>>>>>>>>>>>>            %s == %s \n", k.c_str(), path.c_str());
    //        for(const auto &method : path_handlers_it->value) {
    //            printf("      method:%s      second.size:%lu    \n", method.key.c_str(),  path_handlers_it->value.size());
    //        }
    //    }

    //    printf("register_handler %s,%s \n", path.c_str(), http_method.c_str());
    //    printf("/-------------------------------- *register_handler* ---------------------------------\n");

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

MHD_Result RestApiListener::dispatch_handler(void* cls, MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls) {
    lxcd::string murl = url;
    lxcd::string mversion = version;

    static int count = 0;

    static int dummy;
    static char* buffer = new char[500];
    int ret;
    printf("dispatch_handler dummy %dcount %d upload_data_size %lu murl %s mmethod %s mversion %s\n", dummy, count++, *upload_data_size, murl.c_str(), method, mversion.c_str());
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
        lxcd::string k = path_handlers_it->key;
        printf("k =  path_handlers_it->key %s == %s \n", k.c_str(), path.c_str());
        if(rest_api_listener->is_match_match_regex(k, path, params)) {
            printf("k =  ---------- found\n");

            for(const auto& kv : params) {
                std::cout << kv.key << " = " << kv.value << std::endl;
            }
            found = true;
            break;
        }
        //ignore value
        //Value v = iter->value;
    }

    if(!found) {
        return MHD_NO;
    }

    printf("key for test %s %lu\n", http_method.c_str(), path_handlers_it->value.size());
    //TODO : fix map's find function
    for(const auto &method : path_handlers_it->value) {
        printf("search  %s == %s %lu \n", method.key.c_str(), http_method.c_str(), path_handlers_it->value.size());

        if(method.key == http_method) {
            printf("found  key %s\n", method.key.c_str());
            handler_t handler = method.value;
            lxcd::string reply_body;
            auto ret = handler(params, request_body, reply_body);

            if(connection != NULL) {
                struct MHD_Response* mhd_response = MHD_create_response_from_buffer(reply_body.length(), (void*) reply_body.c_str(), MHD_RESPMEM_MUST_COPY);
                return MHD_queue_response(connection, ret, mhd_response);
            }
            //TODO : queue response to unix socket

        }
    }
    return MHD_NO;
}

bool RestApiListener::is_match_match_regex(const lxcd::string& path, const lxcd::string& request, lxcd::map<lxcd::string, lxcd::string>& params) {
    if(are_paths_equal(path, request)) {
        return true;
    }
    printf("key for test %s %s \n", path.c_str(), request.c_str());
    lxcd::vector<lxcd::string> path_parts = path.split('/');
    lxcd::vector<lxcd::string> request_parts = request.split('/');
    //    path_parts.remove_empty();
    //    request_parts.remove_empty();

    if(path_parts.size() != request_parts.size()) {
        return false;
    }
    printf("key for test %lu %lu \n", path_parts.size(), request_parts.size());

    for(size_t i = 0; i < path_parts.size(); ++i) {


        if(path_parts[i].empty() && request_parts[i].empty()) {
            continue;
        } else {
            return false;
        }

        if((path_parts[i][0] == '{') && (path_parts[i][path_parts[i].size() - 1] == '}')) {
            printf("if (path_parts[i][0] == \n");
            params[path_parts[i].substr(1, path_parts[i].size() - 2)] = request_parts[i];
        } else if(path_parts[i] != request_parts[i]) {
            printf("else if (path_parts[i] != request_parts[i]\n");
            return false;
        }
    }

    return true;
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
