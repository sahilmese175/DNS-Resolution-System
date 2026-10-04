#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "models/DNSRecord.h"

struct DNSResponse {
    uint16_t id = 0;
    uint8_t rcode = 0;          // 0 = OK, 3 = NXDOMAIN
    bool truncated = false;
    std::vector<DNSRecord> answers, authority, additional;
};

// Builds and parses real DNS wire-format messages (RFC 1035)
class DNSPacket {
public:
    static std::vector<uint8_t> buildQuery(uint16_t id, const std::string& domain, uint16_t type);
    static DNSResponse parse(const std::vector<uint8_t>& buf);
};
