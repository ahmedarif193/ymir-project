#include "httphandler.h"

RestApiListener::RestApiListener(int port) : port_(port) {}

void RestApiListener::start() {
    daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port_, nullptr, nullptr,
                               &handle_request, nullptr, MHD_OPTION_END);
    if (!daemon_) {
        std::cerr << "Failed to start REST API listener on port " << port_ << std::endl;
    } else {
        std::cout << "Listening for REST API requests on port " << port_ << std::endl;
    }
}

void RestApiListener::stop() {
    if (daemon_) {
        MHD_stop_daemon(daemon_);
        daemon_ = nullptr;
        std::cout << "Stopped listening for REST API requests" << std::endl;
    }
}

int RestApiListener::handle_request(void *cls, MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    // Parse the request URL and method
    std::string request_url = url;
    std::string request_method = method;

    // Log the request
    std::cout << "Received " << request_method << " request for " << request_url << std::endl;

    // TODO: Implement the logic to handle the request

    // Send the response
    const char *response = "Hello, world!";
    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(strlen(response),
                                                                        (void *) response,
                                                                        MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}
