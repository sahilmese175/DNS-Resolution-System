#include "models/DNSRecord.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

DNSRecord::DNSRecord(std::string name, uint16_t type, std::string value, uint32_t ttl)
    : name_(std::move(name)), type_(type), value_(std::move(value)), ttl_(ttl) {}

void DNSRecord::display() const {
    std::cout << "  " << std::left << std::setw(30) << name_
              << std::setw(7) << typeName()
              << std::setw(42) << value_
              << "TTL " << ttl_ << "s\n";
}

std::string DNSRecord::typeToString(uint16_t type) {
    switch (type) {
        case TYPE_A:     return "A";
        case TYPE_NS:    return "NS";
        case TYPE_CNAME: return "CNAME";
        case TYPE_SOA:   return "SOA";
        case TYPE_MX:    return "MX";
        case TYPE_TXT:   return "TXT";
        case TYPE_AAAA:  return "AAAA";
        default:         return "TYPE" + toStr(type);
    }
}

bool DNSRecord::stringToType(const std::string& s, uint16_t& type) {
    std::string u = s;
    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
    if (u == "A")          type = TYPE_A;
    else if (u == "AAAA")  type = TYPE_AAAA;
    else if (u == "CNAME") type = TYPE_CNAME;
    else if (u == "MX")    type = TYPE_MX;
    else if (u == "NS")    type = TYPE_NS;
    else if (u == "TXT")   type = TYPE_TXT;
    else return false;
    return true;
}
