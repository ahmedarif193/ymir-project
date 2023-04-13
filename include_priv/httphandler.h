#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <stdio.h>
#include <cstdlib>
#include <fcntl.h>

#include <functional>

#include <microhttpd.h>

#include "utils/string.h"
#include "utils/map.h"
#include "utils/sharedptr.h"

#include <json-c/json.h>

//syscalls
#include <unistd.h>
#include <sys/utsname.h>

#include <sys/stat.h>
#include <pthread.h>

typedef std::function<int (const lxcd::map<lxcd::string, lxcd::string>,
                           const lxcd::string&, lxcd::string&)> handler_t;

class RestApiListener {
enum TYPE {
    GET=0,
    POST=1,
    UPDATE=2,
    DELETE=3,
};

public:
RestApiListener(int port);
~RestApiListener() {
    remove_pipe_socket();
}
void listenUnix(const lxcd::string &pipe_name) {
    pipe_name_ = pipe_name;
    create_pipe_socket();
    pthread_create(&listener_thread_, NULL, &RestApiListener::listen_thread_func, this);
    pthread_detach(listener_thread_);
}

void register_handler(const lxcd::string &path, const lxcd::string &http_method, handler_t handler);

void start();

void stop();
lxcd::map<lxcd::string, lxcd::map<lxcd::string, handler_t> > handlers_;

bool create_pipe_socket();

bool remove_pipe_socket();

private:
static int dispatch_handler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** ptr);
bool is_match_match_regex(const lxcd::string& path, const lxcd::string& request, lxcd::map<lxcd::string, lxcd::string>& params);

int port_;
bool is_running_;
struct MHD_Daemon* daemon_;

bool are_paths_equal(lxcd::string path1, lxcd::string path2);
lxcd::string replace_double_slashes(const lxcd::string& str);

lxcd::string pipe_name_;
pthread_t listener_thread_;
static void* listen_thread_func(void* arg);
};

#endif // HTTPHANDLER_H
