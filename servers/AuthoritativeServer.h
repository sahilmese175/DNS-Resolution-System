#pragma once
#include "servers/DNSServer.h"

// Stage 3: holds the actual DNS records of a domain
class AuthoritativeServer : public DNSServer {
public:
    using DNSServer::DNSServer;
    std::string role() const override { return "Authoritative Server"; }
    // Deeper delegations (e.g. co.uk, subdomain zones) lead to another authoritative server
    std::unique_ptr<DNSServer> nextStage(const std::string& name, const std::string& ip) const override;
};
