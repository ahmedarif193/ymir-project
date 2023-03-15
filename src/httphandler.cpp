#include "httphandler.h"

RestApiListener::RestApiListener(int port) : port_(port), is_running_(false) {}

void RestApiListener::register_handler(const std::string &path, const std::string &http_method, handler_t handler) {
    handlers_[path][http_method] = handler;
}

void RestApiListener::start() {
    daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port_, nullptr, nullptr, &RestApiListener::dispatch_handler, this, MHD_OPTION_END);
    if (daemon_) {
        is_running_ = true;
        std::cout << "REST API listening on port " << port_ << std::endl;
    } else {
        std::cerr << "Failed to start REST API" << std::endl;
    }
}

void RestApiListener::stop() {
    if (is_running_) {
        MHD_stop_daemon(daemon_);
        std::cout << "REST API stopped" << std::endl;
        is_running_ = false;
    }
}

int RestApiListener::dispatch_handler(void *cls, MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    std::string mmethod = method;
    std::string murl = url;
    std::string mversion = version;

    static int count= 0;

    static int dummy;
    static char* buffer = new char[500];
    int ret;
    std::cout <<dummy<< count++<<"dispatch_handler" <<*upload_data_size<<murl<<mmethod<<mversion << std::endl;

    if (&dummy != *con_cls) {
        *con_cls = &dummy;
        return MHD_YES;
    }

    if (*upload_data_size != 0) {
        strncpy(buffer, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    std::cout <<"-------------------------------"<<buffer << std::endl;
    auto *rest_api_listener = reinterpret_cast<RestApiListener*>(cls);
    std::unordered_map<std::string, std::string> params;
    std::string request_body = buffer;
    std::string http_method = method ? method : "";
    std::string path = url ? url : "";

    // Find the handler function for the path and HTTP method
    //    auto path_handlers_it = rest_api_listener->handlers_.find(path);
    //    if (path_handlers_it == rest_api_listener->handlers_.end()) {
    //        // static Path not found
    //    }
    bool found = false;
    auto path_handlers_it = rest_api_listener->handlers_.begin();
    for(; path_handlers_it != rest_api_listener->handlers_.end(); ++path_handlers_it)
    {
        std::string k =  path_handlers_it->first;

        if(rest_api_listener->is_match_match_regex(k,path, params)){

            // Print parameter values for debugs
            //            for (const auto& kv : params) {
            //                std::cout << kv.first << " = " << kv.second << std::endl;
            //            }
            found = true;
            break;
        }
        //ignore value
        //Value v = iter->second;
    }
    if(!found)
        return MHD_NO;

    auto method_handlers_it = path_handlers_it->second.find(http_method);
    if (method_handlers_it == path_handlers_it->second.end()) {
        // Method not supported for this path
        return MHD_NO;
    }

    handler_t handler = method_handlers_it->second;
    return handler(connection, params, request_body);
}

bool RestApiListener::is_match_match_regex(std::string path, std::string request, std::unordered_map<std::string, std::string> &params){
    if(are_paths_equal(path,request))
        return true;
    const std::regex param_regex("\\{([^}]+)\\}");

    std::vector<std::string> param_names;
    std::sregex_iterator param_begin(path.begin(), path.end(), param_regex);
    std::sregex_iterator param_end;
    for (std::sregex_iterator i = param_begin; i != param_end; ++i) {
        param_names.push_back(i->str(1));
    }

    // Build regular expression to match path
    std::string path_regex = path;
    std::smatch match;
    for (const auto& param_name : param_names) {
        path_regex.replace(path_regex.find("{" + param_name + "}"), param_name.length() + 2, "([^/]+)");
    }
    path_regex = "^" + path_regex + "$";

    // Match request against path regular expression
    std::regex regex(path_regex);

    if (std::regex_match(request, match, regex)) {
        // Extract parameter values into map
        for (size_t i = 1; i < match.size(); ++i) {
            params[param_names[i - 1]] = match[i].str();
        }
        return true;
    }
    //    std::cout << "Request does not match path" << std::endl;
    return false;
}

bool RestApiListener::are_paths_equal(std::string path1, std::string path2) {
    // Replace double forward slashes with a single forward slash
    std::regex regex("//+");
    std::string path1Normalized = std::regex_replace(path1, regex, "/");
    std::string path2Normalized = std::regex_replace(path2, regex, "/");
    std::cout <<"are_paths_equal "<<path2Normalized<< "  --  "<<path1Normalized << "  --  "<< (path1Normalized == path2Normalized)<< std::endl;

    // Compare the normalized paths
    return path1Normalized == path2Normalized;
}
