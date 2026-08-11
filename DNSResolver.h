#ifndef DNSRESOLVER_H
#define DNSRESOLVER_H

#include "servers/RootServer.h"
#include "servers/TLDServer.h"
#include "servers/AuthoritativeServer.h"

class DNSResolver
{
private:
    RootServer rootServer;
    TLDServer tldServer;
    AuthoritativeServer authoritativeServer;

    string extractTLD(string domain);

public:
    DNSResolver();

    DNSRecord resolve(string domain);
};

#endif