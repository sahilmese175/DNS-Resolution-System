#include "TLDServer.h"
#include <iostream>

using namespace std;

TLDServer::TLDServer()
    : DNSServer("TLD Server")
{
    authoritativeServers["example.com"] =
        "Example Authoritative Server";

    authoritativeServers["google.com"] =
        "Google Authoritative Server";

    authoritativeServers["github.com"] =
        "GitHub Authoritative Server";
}

string TLDServer::extractDomain(string domain)
{
    size_t firstDot = domain.find('.');

    if (firstDot == string::npos)
    {
        return "";
    }

    size_t secondDot =
        domain.find('.', firstDot + 1);

    if (secondDot == string::npos)
    {
        return domain;
    }

    return domain.substr(firstDot + 1);
}

DNSRecord TLDServer::resolve(string domain)
{
    cout << "[TLD Server] Processing: "
         << domain << endl;

    return DNSRecord();
}

string TLDServer::findAuthoritativeServer(string domain)
{
    string baseDomain = extractDomain(domain);

    cout << "    TLD Server looking for: "
         << baseDomain << endl;

    if (authoritativeServers.find(baseDomain)
        != authoritativeServers.end())
    {
        return authoritativeServers[baseDomain];
    }

    return "";
}

void TLDServer::displayInfo() const
{
    cout << "Server Type: "
         << serverName << endl;
}