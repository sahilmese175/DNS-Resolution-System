#ifndef DNSSERVER_H
#define DNSSERVER_H

#include <string>
#include "../models/DNSRecord.h"

using namespace std;

class DNSServer
{
protected:
    string serverName;

public:

    DNSServer(string name);

    virtual DNSRecord resolve(string domain) = 0;

    virtual void displayInfo() const = 0;

    virtual ~DNSServer();
};

#endif