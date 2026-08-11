#include "AuthoritativeServer.h"
#include <iostream>

using namespace std;

AuthoritativeServer::AuthoritativeServer()
    : DNSServer("Authoritative Server")
{
    records["www.example.com"] =
        DNSRecord(
            "www.example.com",
            "A",
            "93.184.216.34",
            300
        );

    records["www.google.com"] =
        DNSRecord(
            "www.google.com",
            "A",
            "142.250.190.78",
            300
        );

    records["www.github.com"] =
        DNSRecord(
            "www.github.com",
            "A",
            "140.82.121.4",
            600
        );
}

DNSRecord AuthoritativeServer::resolve(string domain)
{
    cout << "[Authoritative Server] Processing: "
         << domain << endl;

    if (records.find(domain) != records.end())
    {
        return records[domain];
    }

    return DNSRecord();
}

bool AuthoritativeServer::hasRecord(string domain) const
{
    return records.find(domain) != records.end();
}

void AuthoritativeServer::displayInfo() const
{
    cout << "Server Type: " << serverName << endl;
}