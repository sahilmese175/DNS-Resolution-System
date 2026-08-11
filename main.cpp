#include <iostream>

#include "DNSResolver.h"

using namespace std;

int main()
{
    cout << "==========================================" << endl;
    cout << "          DNS RESOLUTION SYSTEM" << endl;
    cout << "==========================================" << endl;

    DNSResolver resolver;

    string domain;

    cout << "\nEnter domain name: ";
    cin >> domain;

    DNSRecord result =
        resolver.resolve(domain);

    return 0;
}