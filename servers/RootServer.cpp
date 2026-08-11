#include "RootServer.h"
#include <iostream>

using namespace std;

RootServer::RootServer()
    : DNSServer("Root Server")
{
    tldServers["com"] = "COM TLD Server";
    tldServers["org"] = "ORG TLD Server";
    tldServers["net"] = "NET TLD Server";
    tldServers["in"]  = "IN TLD Server";
}

DNSRecord RootServer::resolve(string domain)
{
    cout << "[Root Server] Processing: "
         << domain << endl;

    return DNSRecord();
}

string RootServer::findTLDServer(string tld)
{
    if (tldServers.find(tld) != tldServers.end())
    {
        return tldServers[tld];
    }

    return "";
}

void RootServer::displayInfo() const
{
    cout << "Server Type: " << serverName << endl;
}