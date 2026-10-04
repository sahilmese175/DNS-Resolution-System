#pragma once
#include <cstdint>
#include <sstream>
#include <string>

template <class T>
inline std::string toStr(T v) {
    std::ostringstream o;
    o << v;
    return o.str();
}

enum RecordType : uint16_t {
    TYPE_A = 1, TYPE_NS = 2, TYPE_CNAME = 5, TYPE_SOA = 6,
    TYPE_MX = 15, TYPE_TXT = 16, TYPE_AAAA = 28
};

class DNSRecord {
    std::string name_;
    uint16_t type_;
    std::string value_;
    uint32_t ttl_;

public:
    DNSRecord(std::string name, uint16_t type, std::string value, uint32_t ttl);

    const std::string& getName() const { return name_; }
    uint16_t getType() const { return type_; }
    const std::string& getValue() const { return value_; }
    uint32_t getTTL() const { return ttl_; }
    void setTTL(uint32_t ttl) { ttl_ = ttl; }

    std::string typeName() const { return typeToString(type_); }
    void display() const;

    static std::string typeToString(uint16_t type);
    static bool stringToType(const std::string& s, uint16_t& type);
};
