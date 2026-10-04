#pragma once
#include <memory>
#include <string>
#include "DNSPacket.h"

// Abstract base class for every stage of the DNS hierarchy
class DNSServer {
protected:
    std::string name_;
    std::string ip_;

public:
    DNSServer(std::string name, std::string ip);
    virtual ~DNSServer() = default;

    virtual std::string role() const = 0;
    // Builds the server object for the next stage from a referral
    virtual std::unique_ptr<DNSServer> nextStage(const std::string& name, const std::string& ip) const = 0;

    const std::string& getName() const { return name_; }
    const std::string& getIP() const { return ip_; }
    std::string describe() const;

    // Sends a real DNS query (UDP, TCP fallback). Throws std::runtime_error on failure.
    DNSResponse query(const std::string& domain, uint16_t type) const;
};
