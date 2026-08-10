#include "DNSRecord.h"
#include <iostream>

using namespace std;

DNSRecord::DNSRecord()
{
    domain = "";
    type = "";
    value = "";
    ttl = 0;
}

DNSRecord::DNSRecord(string domain, string type, string value, int ttl)
{
    this->domain = domain;
    this->type = type;
    this->value = value;
    this->ttl = ttl;
}

string DNSRecord::getDomain() const
{
    return domain;
}

string DNSRecord::getType() const
{
    return type;
}

string DNSRecord::getValue() const
{
    return value;
}

int DNSRecord::getTTL() const
{
    return ttl;
}

void DNSRecord::display() const
{
    cout << "Domain : " << domain << endl;
    cout << "Type   : " << type << endl;
    cout << "Value  : " << value << endl;
    cout << "TTL    : " << ttl << " seconds" << endl;
}