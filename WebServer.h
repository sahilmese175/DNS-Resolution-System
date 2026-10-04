#pragma once
#include <string>
#include "DNSResolver.h"

// Minimal local HTTP server: serves web/index.html and a small JSON API
// that the browser UI uses to talk to the C++ DNSResolver.
class WebServer {
    DNSResolver& resolver_;
    int port_;

    std::string handle(const std::string& method, const std::string& path, const std::string& query);

public:
    WebServer(DNSResolver& resolver, int port) : resolver_(resolver), port_(port) {}
    void run();   // blocks until the program is closed (Ctrl+C)
};
