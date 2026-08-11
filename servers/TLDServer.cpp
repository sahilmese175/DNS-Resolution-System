#include "TLDServer.h"
#include <iostream>

using namespace std;


// ==================================================
// CONSTRUCTOR
// ==================================================

TLDServer::TLDServer()
    : DNSServer("TLD Server")
{
    // Supported domains
    authoritativeServers["example.com"] =
        "Example Authoritative Server";

    authoritativeServers["google.com"] =
        "Google Authoritative Server";

    authoritativeServers["github.com"] =
        "GitHub Authoritative Server";
}


// ==================================================
// EXTRACT BASE DOMAIN
// ==================================================
//
// www.example.com
//      ↓
// example.com
//
// ==================================================

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


// ==================================================
// RESOLVE
// ==================================================

DNSRecord TLDServer::resolve(string domain)
{
    cout << "[TLD Server] Processing: "
         << domain << endl;

    return DNSRecord();
}


// ==================================================
// FIND AUTHORITATIVE SERVER
// ==================================================

string TLDServer::findAuthoritativeServer(string domain)
{
    string baseDomain =
        extractDomain(domain);

    cout << "    TLD Server looking for: "
         << baseDomain << endl;

    auto it =
        authoritativeServers.find(baseDomain);

    if (it != authoritativeServers.end())
    {
        return it->second;
    }

    return "";
}


// ==================================================
// DISPLAY SERVER INFORMATION
// ==================================================

void TLDServer::displayInfo() const
{
    cout << "Server Type: "
         << serverName << endl;
}