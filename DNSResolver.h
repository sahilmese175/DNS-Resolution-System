#ifndef DNSRESOLVER_H
#define DNSRESOLVER_H

#include "servers/RootServer.h"
#include "servers/TLDServer.h"
#include "servers/AuthoritativeServer.h"
#include "DNSCache.h"

#include <string>

using namespace std;

class DNSResolver
{
private:

    RootServer rootServer;
    TLDServer tldServer;
    AuthoritativeServer authoritativeServer;

    DNSCache cache;

    string extractTLD(string domain);

public:

    DNSResolver();

    DNSRecord resolve(string domain);

    void displayCache() const;
};

#endif