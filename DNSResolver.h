#pragma once
#include <string>
#include <vector>
#include "DNSCache.h"

struct ResolveResult {
    bool success = false;
    bool fromCache = false;
    std::vector<DNSRecord> records;
    std::string error;
    double ms = 0;
};

class DNSResolver {
    struct QueryLog {
        std::string domain, type, status;
        double ms;
    };

    DNSCache cache_;
    std::vector<QueryLog> log_;

    static const int MAX_DEPTH = 8;
    static const int MAX_HOPS = 12;

    void say(int depth, const std::string& msg) const;
    std::vector<DNSRecord> lookup(const std::string& domain, uint16_t type, int depth, bool& fromCache);
    std::vector<DNSRecord> iterate(const std::string& domain, uint16_t type, int depth);

public:
    // Resolves a real domain: cache -> Root -> TLD -> Authoritative
    ResolveResult resolve(std::string domain, uint16_t type);

    void showCache() { cache_.display(); }
    void clearCache() { cache_.clear(); }
    void showStats() const;

    static bool normalizeDomain(std::string& domain);
};
