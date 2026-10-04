#include "DNSResolver.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include "servers/RootServer.h"

using Clock = std::chrono::steady_clock;

void DNSResolver::say(int depth, const std::string& msg) const {
    std::cout << std::string(depth * 4, ' ') << msg << std::endl;
}

bool DNSResolver::normalizeDomain(std::string& d) {
    std::transform(d.begin(), d.end(), d.begin(), ::tolower);
    size_t p = d.find("://");
    if (p != std::string::npos) d.erase(0, p + 3);
    p = d.find_first_of("/?#");
    if (p != std::string::npos) d.erase(p);
    while (!d.empty() && d.back() == '.') d.pop_back();
    if (d.empty() || d.size() > 253) return false;

    size_t start = 0;
    while (true) {
        size_t dot = d.find('.', start);
        size_t end = (dot == std::string::npos) ? d.size() : dot;
        if (end == start || end - start > 63) return false;
        for (size_t i = start; i < end; ++i) {
            unsigned char c = d[i];
            if (!std::isalnum(c) && c != '-' && c != '_') return false;
        }
        if (dot == std::string::npos) break;
        start = dot + 1;
    }
    return true;
}

ResolveResult DNSResolver::resolve(std::string domain, uint16_t type) {
    ResolveResult res;
    auto t0 = Clock::now();
    res.domain = domain;
    if (!normalizeDomain(domain)) {
        res.error = "Invalid domain name";
        return res;
    }
    res.domain = domain;
    hops_.clear();
    say(0, "\nResolving " + domain + " (" + DNSRecord::typeToString(type) + ")");
    try {
        res.records = lookup(domain, type, 0, res.fromCache);
        res.success = true;
    } catch (const std::exception& e) {
        res.error = e.what();
    }
    res.hops = hops_;
    res.ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    log_.push_back({domain, DNSRecord::typeToString(type),
                    !res.success ? "FAILED" : (res.fromCache ? "HIT" : "MISS"), res.ms});
    return res;
}

std::vector<DNSRecord> DNSResolver::lookup(const std::string& domain, uint16_t type, int depth, bool& fromCache) {
    if (depth > MAX_DEPTH) throw std::runtime_error("resolution depth exceeded (possible loop)");

    say(depth, "[0] Checking DNS cache for " + domain + " (" + DNSRecord::typeToString(type) + ")...");
    std::vector<DNSRecord> out;
    CacheStatus st = cache_.lookup(domain, type, out);
    if (st == CacheStatus::HIT) {
        say(depth, "    Cache Status: HIT");
        fromCache = true;
        return out;
    }
    say(depth, st == CacheStatus::EXPIRED ? "    Cache Status: MISS (entry expired)" : "    Cache Status: MISS");
    fromCache = false;

    std::vector<DNSRecord> answers = iterate(domain, type, depth);

    bool hasWanted = false;
    std::string cnameTarget;
    for (auto& r : answers) {
        if (r.getType() == type) hasWanted = true;
        if (r.getType() == TYPE_CNAME) cnameTarget = r.getValue();
    }
    if (!hasWanted && type != TYPE_CNAME) {
        if (cnameTarget.empty())
            throw std::runtime_error("no " + DNSRecord::typeToString(type) + " record found for this domain");
        say(depth, "    CNAME -> " + cnameTarget + ", following alias...");
        bool unused;
        std::vector<DNSRecord> more = lookup(cnameTarget, type, depth + 1, unused);
        answers.insert(answers.end(), more.begin(), more.end());
    }

    cache_.insert(domain, type, answers);
    say(depth, "    Record stored in DNS cache.");
    return answers;
}

std::vector<DNSRecord> DNSResolver::iterate(const std::string& domain, uint16_t type, int depth) {
    std::vector<std::unique_ptr<DNSServer>> servers = RootServer::all();

    for (int hop = 1; hop <= MAX_HOPS; ++hop) {
        DNSResponse resp;
        const DNSServer* used = nullptr;
        std::string lastError;
        int tries = 0;

        for (auto& s : servers) {
            if (tries++ >= 4) break;
            try {
                resp = s->query(domain, type);
                if (resp.rcode == 2 || resp.rcode == 5) throw std::runtime_error("server refused / failed");
                used = s.get();
                break;
            } catch (const std::exception& e) {
                lastError = e.what();
                say(depth, "    " + s->describe() + " failed: " + lastError + ", trying next");
            }
        }
        if (!used) throw std::runtime_error("no server responded (" + lastError + ")");

        say(depth, "[" + toStr(hop) + "] Querying " + used->describe());
        hops_.push_back({depth, used->role(), used->getName(), used->getIP()});
        if (resp.rcode == 3) throw std::runtime_error("domain does not exist (NXDOMAIN)");
        if (resp.rcode != 0) throw std::runtime_error("server returned error code " + toStr(resp.rcode));
        if (!resp.answers.empty()) return resp.answers;

        // No answer: the server sent a referral (NS records) to the next stage
        std::vector<std::string> nsNames;
        for (auto& r : resp.authority)
            if (r.getType() == TYPE_NS) nsNames.push_back(r.getValue());
        if (nsNames.empty())
            throw std::runtime_error("no " + DNSRecord::typeToString(type) + " record found for this domain");

        std::vector<std::unique_ptr<DNSServer>> next;
        for (auto& ns : nsNames)
            for (auto& r : resp.additional)
                if (r.getType() == TYPE_A && r.getName() == ns)
                    next.push_back(used->nextStage(ns, r.getValue()));

        if (next.empty()) {   // no glue records: resolve a name server's address ourselves
            for (size_t i = 0; i < nsNames.size() && i < 3 && next.empty(); ++i) {
                try {
                    bool unused;
                    for (auto& r : lookup(nsNames[i], TYPE_A, depth + 1, unused))
                        if (r.getType() == TYPE_A) next.push_back(used->nextStage(nsNames[i], r.getValue()));
                } catch (const std::exception&) {}
            }
        }
        if (next.empty()) throw std::runtime_error("could not find addresses of the next name servers");

        say(depth, "    Referral -> " + next.front()->role() + " (" + toStr(next.size()) + " servers)");
        servers = std::move(next);
    }
    throw std::runtime_error("too many referrals");
}

void DNSResolver::showStats() const {
    std::cout << "\nCache entries : " << cache_.size()
              << "\nCache hits    : " << cache_.hits()
              << "\nCache misses  : " << cache_.misses()
              << "\nHit ratio     : " << std::fixed << std::setprecision(1) << cache_.hitRatio() << "%\n";

    if (log_.empty()) return;
    std::cout << "\n" << std::left << std::setw(32) << "Query log" << std::setw(8) << "Type"
              << std::setw(10) << "Status" << "Time\n" << std::string(62, '-') << "\n";
    for (auto& q : log_)
        std::cout << std::left << std::setw(32) << q.domain << std::setw(8) << q.type
                  << std::setw(10) << q.status << std::fixed << std::setprecision(1) << q.ms << " ms\n";
}
