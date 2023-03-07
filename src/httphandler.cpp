#include "httphandler.h"

RestApiListener::RestApiListener(int port) : port_(port), is_running_(false) {}

void RestApiListener::register_handler(const std::string &path, const std::string &http_method, std::function<int (MHD_Connection *, const std::string &)> handler) {
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
    auto *rest_api_listener = reinterpret_cast<RestApiListener*>(cls);

    std::string request_body;
    if (*upload_data_size > 0) {
        request_body = std::string(upload_data, *upload_data_size);
        *upload_data_size = 0;
    }

    std::string http_method = method ? method : "";
    std::string path = url ? url : "";

    // Find the handler function for the path and HTTP method
    auto path_handlers_it = rest_api_listener->handlers_.find(path);
    if (path_handlers_it == rest_api_listener->handlers_.end()) {
        // Path not found
        return MHD_NO;
    }

    auto method_handlers_it = path_handlers_it->second.find(http_method);
    if (method_handlers_it == path_handlers_it->second.end()) {
        // Method not supported for this path
        return MHD_NO;
    }

    std::function<int(struct MHD_Connection*, const std::string&)> handler = method_handlers_it->second;
    return handler(connection, request_body);
}
