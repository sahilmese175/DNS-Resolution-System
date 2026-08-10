#include "AuthoritativeServer.h"
#include <iostream>

using namespace std;

AuthoritativeServer::AuthoritativeServer()
    : DNSServer("Authoritative Server")
{
}

DNSRecord AuthoritativeServer::resolve(string domain)
{
    cout << "[Authoritative Server] Received query for: "
         << domain << endl;

    return DNSRecord(
        domain,
        "A",
        "93.184.216.34",
        300
    );
}

void AuthoritativeServer::displayInfo() const
{
    cout << "Server Type: " << serverName << endl;
}