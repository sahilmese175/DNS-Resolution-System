#include "DNSPacket.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace {

using Buf = std::vector<uint8_t>;

[[noreturn]] void bad() { throw std::runtime_error("malformed DNS response"); }

uint16_t get16(const Buf& b, size_t p) {
    if (p + 2 > b.size()) bad();
    return static_cast<uint16_t>((b[p] << 8) | b[p + 1]);
}

uint32_t get32(const Buf& b, size_t p) {
    if (p + 4 > b.size()) bad();
    return (uint32_t(b[p]) << 24) | (uint32_t(b[p + 1]) << 16) | (uint32_t(b[p + 2]) << 8) | b[p + 3];
}

void put16(Buf& b, uint16_t v) {
    b.push_back(static_cast<uint8_t>(v >> 8));
    b.push_back(static_cast<uint8_t>(v & 0xFF));
}

// Reads a (possibly compressed) domain name and advances pos past it
std::string readName(const Buf& b, size_t& pos) {
    std::string name;
    size_t p = pos, endPos = 0;
    bool jumped = false;
    int jumps = 0;
    while (true) {
        if (p >= b.size()) bad();
        uint8_t len = b[p];
        if (len == 0) { ++p; break; }
        if ((len & 0xC0) == 0xC0) {
            if (p + 1 >= b.size()) bad();
            if (!jumped) { endPos = p + 2; jumped = true; }
            if (++jumps > 20) bad();
            p = static_cast<size_t>(((len & 0x3F) << 8) | b[p + 1]);
            continue;
        }
        if (len & 0xC0) bad();
        ++p;
        if (p + len > b.size()) bad();
        if (!name.empty()) name += '.';
        name.append(reinterpret_cast<const char*>(&b[p]), len);
        p += len;
    }
    pos = jumped ? endPos : p;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name;
}

std::string ipv4(const uint8_t* a) {
    return toStr(int(a[0])) + "." + toStr(int(a[1])) + "." + toStr(int(a[2])) + "." + toStr(int(a[3]));
}

std::string ipv6(const uint8_t* a) {
    unsigned g[8];
    for (int i = 0; i < 8; ++i) g[i] = (a[2 * i] << 8) | a[2 * i + 1];
    int bestStart = -1, bestLen = 0;
    for (int i = 0; i < 8;) {                       // longest run of zero groups -> "::"
        if (g[i] != 0) { ++i; continue; }
        int j = i;
        while (j < 8 && g[j] == 0) ++j;
        if (j - i > bestLen) { bestStart = i; bestLen = j - i; }
        i = j;
    }
    if (bestLen < 2) bestStart = -1;
    std::ostringstream o;
    o << std::hex;
    for (int i = 0; i < 8; ++i) {
        if (i == bestStart) { o << "::"; i += bestLen - 1; continue; }
        if (i > 0 && i != bestStart + bestLen) o << ':';
        o << g[i];
    }
    return o.str();
}

void readRecord(const Buf& b, size_t& pos, std::vector<DNSRecord>& dest) {
    std::string name = readName(b, pos);
    uint16_t type = get16(b, pos);
    uint32_t ttl = get32(b, pos + 4);
    uint16_t rdlen = get16(b, pos + 8);
    size_t rd = pos + 10;
    if (rd + rdlen > b.size()) bad();
    pos = rd + rdlen;

    std::string value;
    switch (type) {
        case 41: return;  // EDNS0 OPT pseudo-record, not real data
        case TYPE_A: {
            if (rdlen != 4) bad();
            value = ipv4(&b[rd]);
            break;
        }
        case TYPE_AAAA: {
            if (rdlen != 16) bad();
            value = ipv6(&b[rd]);
            break;
        }
        case TYPE_NS:
        case TYPE_CNAME: {
            size_t p = rd;
            value = readName(b, p);
            break;
        }
        case TYPE_MX: {
            size_t p = rd + 2;
            value = toStr(get16(b, rd)) + " " + readName(b, p);
            break;
        }
        case TYPE_TXT: {
            size_t p = rd;
            while (p < rd + rdlen) {
                size_t len = b[p++];
                if (p + len > rd + rdlen) bad();
                if (!value.empty()) value += ' ';
                value += '"' + std::string(reinterpret_cast<const char*>(&b[p]), len) + '"';
                p += len;
            }
            break;
        }
        case TYPE_SOA: {
            size_t p = rd;
            value = readName(b, p);
            break;
        }
        default:
            value = "(type " + toStr(type) + ")";
    }
    dest.emplace_back(name, type, value, ttl);
}

}  // namespace

std::vector<uint8_t> DNSPacket::buildQuery(uint16_t id, const std::string& domain, uint16_t type) {
    Buf p;
    put16(p, id);
    put16(p, 0x0000);   // standard query, recursion NOT desired (we resolve iteratively)
    put16(p, 1);        // QDCOUNT
    put16(p, 0);        // ANCOUNT
    put16(p, 0);        // NSCOUNT
    put16(p, 1);        // ARCOUNT (EDNS0 OPT)

    size_t start = 0;
    while (start <= domain.size()) {
        size_t dot = domain.find('.', start);
        if (dot == std::string::npos) dot = domain.size();
        p.push_back(static_cast<uint8_t>(dot - start));
        p.insert(p.end(), domain.begin() + start, domain.begin() + dot);
        start = dot + 1;
    }
    p.push_back(0);
    put16(p, type);
    put16(p, 1);        // class IN

    p.push_back(0);     // OPT record: root name
    put16(p, 41);       // type OPT
    put16(p, 1232);     // advertised UDP payload size
    put16(p, 0);        // extended rcode / version
    put16(p, 0);        // flags
    put16(p, 0);        // rdlength
    return p;
}

DNSResponse DNSPacket::parse(const std::vector<uint8_t>& b) {
    if (b.size() < 12) bad();
    DNSResponse r;
    r.id = get16(b, 0);
    uint16_t flags = get16(b, 2);
    r.rcode = flags & 0x0F;
    r.truncated = (flags & 0x0200) != 0;
    if (r.truncated) return r;  // caller retries over TCP

    uint16_t qd = get16(b, 4), an = get16(b, 6), ns = get16(b, 8), ar = get16(b, 10);
    size_t pos = 12;
    for (uint16_t i = 0; i < qd; ++i) { readName(b, pos); pos += 4; }
    for (uint16_t i = 0; i < an; ++i) readRecord(b, pos, r.answers);
    for (uint16_t i = 0; i < ns; ++i) readRecord(b, pos, r.authority);
    for (uint16_t i = 0; i < ar; ++i) readRecord(b, pos, r.additional);
    return r;
}
