#include "servers/TLDServer.h"
#include "servers/AuthoritativeServer.h"

std::unique_ptr<DNSServer> TLDServer::nextStage(const std::string& name, const std::string& ip) const {
    return std::make_unique<AuthoritativeServer>(name, ip);
}
