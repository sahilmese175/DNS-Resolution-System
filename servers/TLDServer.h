#ifndef TLDSERVER_H
#define TLDSERVER_H

#include "DNSServer.h"

class TLDServer : public DNSServer
{
public:

    TLDServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;
};

#endif