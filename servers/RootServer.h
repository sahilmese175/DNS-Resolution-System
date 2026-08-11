#ifndef ROOTSERVER_H
#define ROOTSERVER_H

#include "DNSServer.h"
#include <map>

class RootServer : public DNSServer
{
private:
    map<string, string> tldServers;

public:
    RootServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;

    string findTLDServer(string tld);
};

#endif