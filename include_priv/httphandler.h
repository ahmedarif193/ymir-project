#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H
#include <iostream>
#include <string>
#include <regex>

#include <functional>
#include <unordered_map>
#include <microhttpd.h>
#include <json/json.h>


//syscalls
#include <unistd.h>
#include <sys/utsname.h>


typedef std::function<int(MHD_Connection*
                          , const std::unordered_map<std::string, std::string>
                          , std::string )> handler_t;

class RestApiListener {
    enum TYPE{
        GET=0,
        POST=1,
        UPDATE=2,
        DELETE=3,
    };

public:
    RestApiListener(int port);

    void register_handler(const std::string &path, const std::string &http_method, handler_t handler);

    void start();

    void stop();

private:
    static int dispatch_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);

    int port_;
    bool is_running_;
    struct MHD_Daemon *daemon_;

    std::unordered_map<std::string, std::unordered_map<std::string, handler_t>> handlers_;
};

#endif // HTTPHANDLER_H
