#pragma once
#include <vector>
#include "servers/DNSServer.h"

// Stage 1: the 13 real DNS root servers; they refer queries to the TLD servers
class RootServer : public DNSServer {
public:
    using DNSServer::DNSServer;
    std::string role() const override { return "Root Server"; }
    std::unique_ptr<DNSServer> nextStage(const std::string& name, const std::string& ip) const override;

    static std::vector<std::unique_ptr<DNSServer>> all();
};
