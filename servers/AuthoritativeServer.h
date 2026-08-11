#ifndef AUTHORITATIVESERVER_H
#define AUTHORITATIVESERVER_H

#include "DNSServer.h"
#include <map>

class AuthoritativeServer : public DNSServer
{
private:
    map<string, DNSRecord> records;

public:
    AuthoritativeServer();

    DNSRecord resolve(string domain) override;

    void displayInfo() const override;

    bool hasRecord(string domain) const;
};

#endif