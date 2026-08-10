#ifndef DNSRECORD_H
#define DNSRECORD_H

#include <string>
using namespace std;

class DNSRecord
{
private:
    string domain;
    string type;
    string value;
    int ttl;

public:

    DNSRecord();

    DNSRecord(string domain, string type, string value, int ttl);

    string getDomain() const;
    string getType() const;
    string getValue() const;
    int getTTL() const;

    void display() const;
};

#endif