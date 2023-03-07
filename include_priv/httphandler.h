#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H
#include <iostream>
#include <string>
#include <functional>
#include <unordered_map>
#include <microhttpd.h>
#include <json/json.h>


//syscalls
#include <unistd.h>
#include <sys/utsname.h>
class RestApiListener {
    enum TYPE{
        GET=0,
        POST=1,
        UPDATE=2,
        DELETE=3,
    };

public:
    RestApiListener(int port);

    void register_handler(const std::string &path, const std::string &http_method, std::function<int(struct MHD_Connection*, const std::string&)> handler);

    void start();

    void stop();

private:
    static int dispatch_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);

    int port_;
    bool is_running_;
    struct MHD_Daemon *daemon_;

    std::unordered_map<std::string, std::unordered_map<std::string, std::function<int(struct MHD_Connection*, const std::string&)>>> handlers_;
};


#endif // HTTPHANDLER_H
