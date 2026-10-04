#pragma once
#include "servers/DNSServer.h"

// Stage 2: knows the authoritative name servers of each domain under its TLD (e.g. .com)
class TLDServer : public DNSServer {
public:
    using DNSServer::DNSServer;
    std::string role() const override { return "TLD Server"; }
    std::unique_ptr<DNSServer> nextStage(const std::string& name, const std::string& ip) const override;
};
