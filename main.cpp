#include <httplib.h>

#include "handler.hpp"

int main() {

    httplib::Server svr;

    svr.Post(".*",
             [](const httplib::Request &req, httplib::Response &res) {
                 wooting_handle_event(req.body);
             });

    svr.Get("/stop", [&](const httplib::Request &req, httplib::Response &res) {
        wooting_handle_event("exit");
        svr.stop();
    });

    svr.listen("127.0.0.1", 3000);
}
