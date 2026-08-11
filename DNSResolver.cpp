#include "DNSResolver.h"
#include <iostream>
#include <sstream>

using namespace std;

DNSResolver::DNSResolver()
{
}

string DNSResolver::extractTLD(string domain)
{
    size_t position = domain.rfind('.');

    if (position == string::npos)
    {
        return "";
    }

    return domain.substr(position + 1);
}

DNSRecord DNSResolver::resolve(string domain)
{
    cout << endl;
    cout << "==========================================" << endl;
    cout << "         DNS RESOLUTION STARTED" << endl;
    cout << "==========================================" << endl;

    cout << "Domain: " << domain << endl;

    // STEP 1: Find TLD
    string tld = extractTLD(domain);

    if (tld.empty())
    {
        cout << "Invalid domain." << endl;
        return DNSRecord();
    }

    cout << endl;
    cout << "[1] Querying Root Server..." << endl;

    string tldServerName =
        rootServer.findTLDServer(tld);

    if (tldServerName.empty())
    {
        cout << "TLD not supported: ."
             << tld << endl;

        return DNSRecord();
    }

    cout << "    Root Server Response: "
         << tldServerName << endl;


    // STEP 2: Query TLD Server
    cout << endl;
    cout << "[2] Querying TLD Server..." << endl;

    string authoritativeServerName =
        tldServer.findAuthoritativeServer(domain);

    if (authoritativeServerName.empty())
    {
        cout << "Authoritative server not found."
             << endl;

        return DNSRecord();
    }

    cout << "    TLD Server Response: "
         << authoritativeServerName << endl;


    // STEP 3: Query Authoritative Server
    cout << endl;
    cout << "[3] Querying Authoritative Server..."
         << endl;

    DNSRecord record =
        authoritativeServer.resolve(domain);

    if (record.getDomain().empty())
    {
        cout << "DNS record not found." << endl;

        return DNSRecord();
    }

    cout << endl;
    cout << "==========================================" << endl;
    cout << "          DNS RESOLUTION SUCCESS" << endl;
    cout << "==========================================" << endl;

    record.display();

    return record;
}