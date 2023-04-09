#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H
#include <iostream>
#include <regex>

#include <functional>
#include <unordered_map>
#include <microhttpd.h>
#include <json/json.h>

#include "utils/string.h"
#include "utils/map.h"

//syscalls
#include <unistd.h>
#include <sys/utsname.h>


typedef std::function<int(MHD_Connection*
                          , const std::unordered_map<lxcd::string, lxcd::string>
                          , lxcd::string )> handler_t;

class RestApiListener {
    enum TYPE{
        GET=0,
        POST=1,
        UPDATE=2,
        DELETE=3,
    };

public:
    RestApiListener(int port);

    void register_handler(const lxcd::string &path, const lxcd::string &http_method, handler_t handler);

    void start();

    void stop();
    std::unordered_map<lxcd::string, std::unordered_map<lxcd::string, handler_t>> handlers_;

private:
    static int dispatch_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    bool is_match_match_regex(lxcd::string path, lxcd::string request, std::unordered_map<lxcd::string, lxcd::string> &params);

    int port_;
    bool is_running_;
    struct MHD_Daemon *daemon_;

    bool are_paths_equal(lxcd::string path1, lxcd::string path2);

};

#endif // HTTPHANDLER_H
