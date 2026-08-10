#ifndef AUTHORITATIVESERVER_H
#define AUTHORITATIVESERVER_H

#include "DNSServer.h"

class AuthoritativeServer : public DNSServer
{
public:

    AuthoritativeServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;
};

#endif