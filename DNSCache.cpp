#include "DNSCache.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

using Clock = std::chrono::steady_clock;

std::string DNSCache::key(const std::string& domain, uint16_t type) {
    return domain + "|" + toStr(type);
}

uint32_t DNSCache::elapsed(const Entry& e) {
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - e.cachedAt).count());
}

CacheStatus DNSCache::lookup(const std::string& domain, uint16_t type, std::vector<DNSRecord>& out) {
    auto it = store_.find(key(domain, type));
    if (it == store_.end()) {
        ++misses_;
        return CacheStatus::MISS;
    }
    uint32_t age = elapsed(it->second);
    if (age >= it->second.ttl) {            // TTL expired -> remove entry
        store_.erase(it);
        ++misses_;
        return CacheStatus::EXPIRED;
    }
    out = it->second.records;
    for (auto& r : out) r.setTTL(r.getTTL() > age ? r.getTTL() - age : 0);
    ++hits_;
    return CacheStatus::HIT;
}

void DNSCache::insert(const std::string& domain, uint16_t type, const std::vector<DNSRecord>& records) {
    if (records.empty()) return;
    uint32_t ttl = records.front().getTTL();
    for (auto& r : records) ttl = std::min(ttl, r.getTTL());
    if (ttl == 0) return;                   // TTL 0 means "do not cache"
    store_[key(domain, type)] = Entry{domain, type, records, Clock::now(), ttl};
}

void DNSCache::clear() {
    store_.clear();
}

void DNSCache::display() {
    for (auto it = store_.begin(); it != store_.end();)   // drop expired entries first
        it = (elapsed(it->second) >= it->second.ttl) ? store_.erase(it) : std::next(it);

    if (store_.empty()) {
        std::cout << "Cache is empty.\n";
        return;
    }
    std::cout << "\n" << std::left << std::setw(32) << "Domain" << std::setw(8) << "Type"
              << std::setw(10) << "Records" << "Remaining TTL\n"
              << std::string(62, '-') << "\n";
    for (auto& kv : store_) {
        const Entry& e = kv.second;
        std::cout << std::left << std::setw(32) << e.domain
                  << std::setw(8) << DNSRecord::typeToString(e.type)
                  << std::setw(10) << e.records.size()
                  << (e.ttl - elapsed(e)) << "s\n";
    }
}

double DNSCache::hitRatio() const {
    size_t total = hits_ + misses_;
    return total ? 100.0 * hits_ / total : 0.0;
}
