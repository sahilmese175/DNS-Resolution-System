#include <iostream>

#include "models/DNSRecord.h"

#include "servers/DNSServer.h"
#include "servers/RootServer.h"
#include "servers/TLDServer.h"
#include "servers/AuthoritativeServer.h"

using namespace std;

int main()
{
    cout << "==========================================" << endl;
    cout << "       DNS RESOLUTION SYSTEM" << endl;
    cout << "==========================================" << endl;

    RootServer root;
    TLDServer tld;
    AuthoritativeServer authoritative;

    cout << "\n--- Server Information ---\n" << endl;

    root.displayInfo();
    tld.displayInfo();
    authoritative.displayInfo();

    cout << "\n--- Testing DNS Resolution ---\n" << endl;

    root.resolve("www.example.com");

    tld.resolve("www.example.com");

    DNSRecord record =
        authoritative.resolve("www.example.com");

    cout << "\n--- DNS Record ---\n" << endl;

    record.display();

    return 0;
}