#ifndef ROOTSERVER_H
#define ROOTSERVER_H

#include "DNSServer.h"

class RootServer : public DNSServer
{
public:

    RootServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;
};

#endif