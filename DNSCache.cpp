#include "DNSCache.h"
#include <iostream>

using namespace std;


// Check whether a valid record exists in cache
bool DNSCache::contains(string domain)
{
    auto it = cache.find(domain);

    if (it == cache.end())
    {
        return false;
    }

    auto now = chrono::steady_clock::now();

    long long elapsed =
        chrono::duration_cast<chrono::seconds>(
            now - it->second.cachedAt
        ).count();

    int ttl = it->second.record.getTTL();

    if (elapsed >= ttl)
    {
        cache.erase(it);

        cout << "    Cache entry expired." << endl;

        return false;
    }

    return true;
}


// Get record from cache
DNSRecord DNSCache::get(string domain)
{
    auto it = cache.find(domain);

    if (it == cache.end())
    {
        return DNSRecord();
    }

    return it->second.record;
}


// Insert record into cache
void DNSCache::insert(const DNSRecord& record)
{
    CacheEntry entry;

    entry.record = record;
    entry.cachedAt = chrono::steady_clock::now();

    cache[record.getDomain()] = entry;
}


// Remove a record
void DNSCache::remove(string domain)
{
    cache.erase(domain);
}


// Clear complete cache
void DNSCache::clear()
{
    cache.clear();
}


// Display all cached records
void DNSCache::display() const
{
    if (cache.empty())
    {
        cout << "DNS Cache is empty." << endl;
        return;
    }

    for (auto it = cache.begin(); it != cache.end(); ++it)
    {
        const DNSRecord& record = it->second.record;

        cout << "Domain       : "
             << record.getDomain() << endl;

        cout << "Record Type  : "
             << record.getType() << endl;

        cout << "IP Address   : "
             << record.getValue() << endl;

        cout << "TTL          : "
             << record.getTTL()
             << " seconds" << endl;

        cout << "------------------------------------------"
             << endl;
    }
}


// Return number of cached records
int DNSCache::size() const
{
    return static_cast<int>(cache.size());
}