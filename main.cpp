#include <iomanip>
#include <iostream>
#include <sstream>
#include "DNSResolver.h"
#include "WebServer.h"

namespace {

void resolveDomain(DNSResolver& resolver) {
    std::cout << "Enter domain [type] (e.g. www.google.com  or  google.com MX): ";
    std::string line, domain, typeStr;
    std::getline(std::cin, line);
    std::istringstream in(line);
    in >> domain >> typeStr;
    if (domain.empty()) {
        std::cout << "No domain entered.\n";
        return;
    }

    uint16_t type = TYPE_A;
    if (!typeStr.empty() && !DNSRecord::stringToType(typeStr, type)) {
        std::cout << "Unsupported type. Use: A, AAAA, CNAME, MX, NS, TXT\n";
        return;
    }

    ResolveResult r = resolver.resolve(domain, type);
    if (!r.success) {
        std::cout << "\nResolution failed: " << r.error << "\n";
        return;
    }
    std::cout << "\nResult:\n";
    for (auto& rec : r.records) rec.display();
    std::cout << "\nResolved in " << std::fixed << std::setprecision(1) << r.ms << " ms (cache " << (r.fromCache ? "HIT" : "MISS") << ")\n";
}

void startWebUI(DNSResolver& resolver) {
    try {
        WebServer(resolver, 8080).run();
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

}  // namespace

int main() {
    DNSResolver resolver;
    while (true) {
        std::cout << "\n===== DNS Resolution System =====\n"
                     "1. Resolve a domain\n"
                     "2. Show DNS cache\n"
                     "3. Statistics and query log\n"
                     "4. Clear cache\n"
                     "5. Start web UI\n"
                     "0. Exit\n"
                     "Choice: ";
        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (line == "1")      resolveDomain(resolver);
        else if (line == "2") resolver.showCache();
        else if (line == "3") resolver.showStats();
        else if (line == "4") { resolver.clearCache(); std::cout << "Cache cleared.\n"; }
        else if (line == "5") startWebUI(resolver);
        else if (line == "0") break;
        else                  std::cout << "Invalid choice. Enter 0-5.\n";
    }
    return 0;
}
