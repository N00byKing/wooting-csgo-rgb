#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT 1
#endif //CPPHTTPLIB_THREAD_POOL_COUNT

#include <httplib.h>
#include "handler.hpp"

int main() {

    httplib::Server svr;

    svr.Post(".*",
             [](const httplib::Request &req, httplib::Response &res) {
                 KeyboardHandler::wooting_handle_event(req.body);
             });

    svr.Get("/stop", [&](const httplib::Request &req, httplib::Response &res) {
        KeyboardHandler::wooting_exit();
        svr.stop();
    });

    svr.listen("127.0.0.1", 3000);
}
