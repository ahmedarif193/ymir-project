#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <microhttpd.h>

class RestApiListener {
public:
    RestApiListener(int port);

    void start();

    void stop();

private:
    static int handle_request(void *cls, struct MHD_Connection *connection,
                              const char *url, const char *method, const char *version,
                              const char *upload_data, size_t *upload_data_size, void **con_cls);

    int port_;
    struct MHD_Daemon *daemon_ = nullptr;
};


#endif // HTTPHANDLER_H
