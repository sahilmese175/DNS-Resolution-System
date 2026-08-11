#include "DNSResolver.h"
#include <iostream>

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
    cout << "          DNS RESOLUTION STARTED" << endl;
    cout << "==========================================" << endl;

    cout << "Domain: " << domain << endl;


    // ------------------------------------------
    // STEP 0: CACHE
    // ------------------------------------------

    cout << endl;
    cout << "[0] Checking DNS Cache..." << endl;

    if (cache.contains(domain))
    {
        cout << "    Cache Status: HIT" << endl;

        DNSRecord record = cache.get(domain);

        cout << endl;
        cout << "==========================================" << endl;
        cout << "              CACHE HIT" << endl;
        cout << "==========================================" << endl;

        record.display();

        cout << "==========================================" << endl;

        return record;
    }

    cout << "    Cache Status: MISS" << endl;


    // ------------------------------------------
    // STEP 1: ROOT SERVER
    // ------------------------------------------

    string tld = extractTLD(domain);

    if (tld.empty())
    {
        cout << endl;
        cout << "Invalid domain name." << endl;

        return DNSRecord();
    }

    cout << endl;
    cout << "[1] Querying Root Server..." << endl;

    string tldServerName =
        rootServer.findTLDServer(tld);

    if (tldServerName.empty())
    {
        cout << "    TLD not supported: ."
             << tld << endl;

        return DNSRecord();
    }

    cout << "    Root Server Response: "
         << tldServerName << endl;


    // ------------------------------------------
    // STEP 2: TLD SERVER
    // ------------------------------------------

    cout << endl;
    cout << "[2] Querying TLD Server..." << endl;

    string authoritativeServerName =
        tldServer.findAuthoritativeServer(domain);

    if (authoritativeServerName.empty())
    {
        cout << "    Authoritative server not found."
             << endl;

        return DNSRecord();
    }

    cout << "    TLD Server Response: "
         << authoritativeServerName << endl;


    // ------------------------------------------
    // STEP 3: AUTHORITATIVE SERVER
    // ------------------------------------------

    cout << endl;
    cout << "[3] Querying Authoritative Server..."
         << endl;

    DNSRecord record =
        authoritativeServer.resolve(domain);


    if (record.getDomain().empty())
    {
        cout << "    DNS record not found." << endl;

        return DNSRecord();
    }


    // ------------------------------------------
    // STEP 4: CACHE INSERTION
    // ------------------------------------------

    cache.insert(record);

    cout << endl;
    cout << "Record stored in DNS cache." << endl;


    // ------------------------------------------
    // SUCCESS
    // ------------------------------------------

    cout << endl;
    cout << "==========================================" << endl;
    cout << "          DNS RESOLUTION SUCCESS" << endl;
    cout << "==========================================" << endl;

    record.display();

    cout << "==========================================" << endl;

    return record;
}


void DNSResolver::displayCache() const
{
    cout << endl;
    cout << "==========================================" << endl;
    cout << "              DNS CACHE" << endl;
    cout << "==========================================" << endl;

    cout << "Cached Records: "
         << cache.size() << endl;

    cout << "------------------------------------------"
         << endl;

    cache.display();

    cout << "==========================================" << endl;
}