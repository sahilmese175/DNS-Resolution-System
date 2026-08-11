#ifndef TLDSERVER_H
#define TLDSERVER_H

#include "DNSServer.h"
#include <map>

class TLDServer : public DNSServer
{
private:

    map<string, string> authoritativeServers;

    string extractDomain(string domain);

public:

    TLDServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;

    string findAuthoritativeServer(string domain);
};

#endif