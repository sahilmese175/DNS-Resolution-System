#include "TLDServer.h"
#include <iostream>

using namespace std;

TLDServer::TLDServer()
    : DNSServer("TLD Server")
{
}

DNSRecord TLDServer::resolve(string domain)
{
    cout << "[TLD Server] Received query for: "
         << domain << endl;

    return DNSRecord();
}

void TLDServer::displayInfo() const
{
    cout << "Server Type: " << serverName << endl;
}