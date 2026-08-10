#include "RootServer.h"
#include <iostream>

using namespace std;

RootServer::RootServer()
    : DNSServer("Root Server")
{
}

DNSRecord RootServer::resolve(string domain)
{
    cout << "[Root Server] Received query for: "
         << domain << endl;

    return DNSRecord();
}

void RootServer::displayInfo() const
{
    cout << "Server Type: " << serverName << endl;
}