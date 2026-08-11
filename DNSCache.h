#ifndef DNSCACHE_H
#define DNSCACHE_H

#include "models/DNSRecord.h"

#include <unordered_map>
#include <chrono>

using namespace std;

class DNSCache
{
private:

    struct CacheEntry
    {
        DNSRecord record;
        chrono::steady_clock::time_point cachedAt;
    };

    unordered_map<string, CacheEntry> cache;

public:

    bool contains(string domain);

    DNSRecord get(string domain);

    void insert(const DNSRecord& record);

    void remove(string domain);

    void clear();

    void display() const;

    int size() const;
};

#endif