#include <iostream>
#include <string>

#include "DNSResolver.h"

using namespace std;


int main()
{
    cout << "==========================================" << endl;
    cout << "          DNS RESOLUTION SYSTEM" << endl;
    cout << "==========================================" << endl;


    DNSResolver resolver;

    string choice;


    while (true)
    {
        cout << endl;
        cout << "============== MENU =====================" << endl;
        cout << "1. Resolve Domain" << endl;
        cout << "2. View DNS Cache" << endl;
        cout << "3. Exit" << endl;
        cout << "==========================================" << endl;

        cout << "Enter choice: ";
        cin >> choice;


        // ------------------------------------------
        // OPTION 1
        // ------------------------------------------

        if (choice == "1")
        {
            string domain;

            cout << "Enter domain: ";
            cin >> domain;

            resolver.resolve(domain);
        }


        // ------------------------------------------
        // OPTION 2
        // ------------------------------------------

        else if (choice == "2")
        {
            resolver.displayCache();
        }


        // ------------------------------------------
        // OPTION 3
        // ------------------------------------------

        else if (choice == "3")
        {
            cout << endl;
            cout << "Exiting DNS Resolution System..."
                 << endl;

            break;
        }


        // ------------------------------------------
        // INVALID OPTION
        // ------------------------------------------

        else
        {
            cout << endl;
            cout << "Invalid choice. Please enter 1, 2, or 3."
                 << endl;
        }
    }


    return 0;
}