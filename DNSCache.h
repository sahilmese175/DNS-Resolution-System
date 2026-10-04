#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include "models/DNSRecord.h"

enum class CacheStatus { HIT, MISS, EXPIRED };

class DNSCache {
    struct Entry {
        std::string domain;
        uint16_t type;
        std::vector<DNSRecord> records;
        std::chrono::steady_clock::time_point cachedAt;
        uint32_t ttl;   // validity = smallest TTL among the records
    };

    std::unordered_map<std::string, Entry> store_;
    size_t hits_ = 0, misses_ = 0;

    static std::string key(const std::string& domain, uint16_t type);
    static uint32_t elapsed(const Entry& e);

public:
    // On HIT, 'out' receives the records with their remaining TTL
    CacheStatus lookup(const std::string& domain, uint16_t type, std::vector<DNSRecord>& out);
    void insert(const std::string& domain, uint16_t type, const std::vector<DNSRecord>& records);
    void clear();
    void display();

    size_t size() const { return store_.size(); }
    size_t hits() const { return hits_; }
    size_t misses() const { return misses_; }
    double hitRatio() const;
};
