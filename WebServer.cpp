#include "WebServer.h"
#include "Platform.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

const char* JSON_TYPE = "application/json; charset=utf-8";
const char* HTML_TYPE = "text/html; charset=utf-8";

std::string jsonEscape(const std::string& s) {
    std::string o;
    for (unsigned char c : s) {
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            default:   o += (c < 0x20) ? ' ' : static_cast<char>(c);
        }
    }
    return o;
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::string urlDecode(const std::string& s) {
    std::string o;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            o += ' ';
        } else if (s[i] == '%' && i + 2 < s.size() && hexValue(s[i + 1]) >= 0 && hexValue(s[i + 2]) >= 0) {
            o += static_cast<char>(hexValue(s[i + 1]) * 16 + hexValue(s[i + 2]));
            i += 2;
        } else {
            o += s[i];
        }
    }
    return o;
}

std::string getParam(const std::string& query, const std::string& name) {
    size_t pos = 0;
    while (pos <= query.size()) {
        size_t amp = query.find('&', pos);
        if (amp == std::string::npos) amp = query.size();
        std::string pair = query.substr(pos, amp - pos);
        size_t eq = pair.find('=');
        if (eq != std::string::npos && pair.substr(0, eq) == name) return urlDecode(pair.substr(eq + 1));
        pos = amp + 1;
    }
    return "";
}

bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

std::string httpResponse(int status, const std::string& type, const std::string& body) {
    std::string text = status == 200 ? "200 OK" : status == 404 ? "404 Not Found" : "405 Method Not Allowed";
    return "HTTP/1.1 " + text + "\r\nContent-Type: " + type + "\r\nContent-Length: " + toStr(body.size()) +
           "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n" + body;
}

void sendAll(sock_t s, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        int n = static_cast<int>(send(s, data.data() + sent, static_cast<int>(data.size() - sent), 0));
        if (n <= 0) return;
        sent += n;
    }
}

std::string errorJson(const std::string& msg) {
    return "{\"ok\":false,\"error\":\"" + jsonEscape(msg) + "\",\"records\":[],\"hops\":[]}";
}

std::string resultJson(const ResolveResult& r, uint16_t type) {
    std::ostringstream o;
    o << "{\"ok\":" << (r.success ? "true" : "false")
      << ",\"domain\":\"" << jsonEscape(r.domain) << "\""
      << ",\"type\":\"" << DNSRecord::typeToString(type) << "\""
      << ",\"cache\":\"" << (r.fromCache ? "HIT" : "MISS") << "\""
      << ",\"ms\":" << std::fixed << std::setprecision(1) << r.ms
      << ",\"error\":\"" << jsonEscape(r.error) << "\",\"records\":[";
    for (size_t i = 0; i < r.records.size(); ++i) {
        const DNSRecord& d = r.records[i];
        o << (i ? "," : "") << "{\"name\":\"" << jsonEscape(d.getName()) << "\",\"type\":\"" << d.typeName()
          << "\",\"value\":\"" << jsonEscape(d.getValue()) << "\",\"ttl\":" << d.getTTL() << "}";
    }
    o << "],\"hops\":[";
    for (size_t i = 0; i < r.hops.size(); ++i) {
        const Hop& h = r.hops[i];
        o << (i ? "," : "") << "{\"depth\":" << h.depth << ",\"role\":\"" << jsonEscape(h.role)
          << "\",\"name\":\"" << jsonEscape(h.name) << "\",\"ip\":\"" << jsonEscape(h.ip) << "\"}";
    }
    o << "]}";
    return o.str();
}

}  // namespace

std::string WebServer::handle(const std::string& method, const std::string& path, const std::string& query) {
    if (path == "/" || path == "/index.html") {
        std::string page;
        if (!readFile("web/index.html", page))
            return httpResponse(404, "text/plain; charset=utf-8",
                                "web/index.html not found. Start dns.exe from the project folder.");
        return httpResponse(200, HTML_TYPE, page);
    }

    if (path == "/api/resolve") {
        std::string domain = getParam(query, "domain");
        std::string typeStr = getParam(query, "type");
        uint16_t type = TYPE_A;
        if (domain.empty()) return httpResponse(200, JSON_TYPE, errorJson("Enter a website name first"));
        if (!typeStr.empty() && !DNSRecord::stringToType(typeStr, type))
            return httpResponse(200, JSON_TYPE, errorJson("Unsupported record type"));
        return httpResponse(200, JSON_TYPE, resultJson(resolver_.resolve(domain, type), type));
    }

    if (path == "/api/cache") {
        std::ostringstream o;
        o << "[";
        std::vector<DNSCache::CacheInfo> list = resolver_.cache().entries();
        for (size_t i = 0; i < list.size(); ++i)
            o << (i ? "," : "") << "{\"domain\":\"" << jsonEscape(list[i].domain) << "\",\"type\":\""
              << DNSRecord::typeToString(list[i].type) << "\",\"records\":" << list[i].records
              << ",\"remaining\":" << list[i].remaining << "}";
        o << "]";
        return httpResponse(200, JSON_TYPE, o.str());
    }

    if (path == "/api/stats") {
        DNSCache& c = resolver_.cache();
        std::ostringstream o;
        o << "{\"entries\":" << c.size() << ",\"hits\":" << c.hits() << ",\"misses\":" << c.misses()
          << ",\"ratio\":" << std::fixed << std::setprecision(1) << c.hitRatio() << ",\"log\":[";
        const std::vector<DNSResolver::QueryLog>& log = resolver_.queryLog();
        for (size_t i = 0; i < log.size(); ++i)
            o << (i ? "," : "") << "{\"domain\":\"" << jsonEscape(log[i].domain) << "\",\"type\":\""
              << log[i].type << "\",\"status\":\"" << log[i].status << "\",\"ms\":"
              << std::fixed << std::setprecision(1) << log[i].ms << "}";
        o << "]}";
        return httpResponse(200, JSON_TYPE, o.str());
    }

    if (path == "/api/clear") {
        if (method != "POST") return httpResponse(405, "text/plain; charset=utf-8", "Use POST");
        resolver_.clearCache();
        return httpResponse(200, JSON_TYPE, "{\"ok\":true}");
    }

    return httpResponse(404, "text/plain; charset=utf-8", "Not found");
}

void WebServer::run() {
    initNetwork();
    Socket listener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listener.s == INVALID_SOCK) throw std::runtime_error("cannot create socket");
#ifndef _WIN32
    int yes = 1;
    setsockopt(listener.s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof yes);
#endif
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(static_cast<unsigned short>(port_));
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // local machine only
    if (bind(listener.s, reinterpret_cast<sockaddr*>(&a), sizeof a) != 0)
        throw std::runtime_error("cannot start web UI: port " + toStr(port_) + " is already in use");
    if (listen(listener.s, 16) != 0) throw std::runtime_error("cannot listen on port " + toStr(port_));

    std::string url = "http://localhost:" + toStr(port_);
    std::cout << "Web UI running at " << url << "  (press Ctrl+C to stop)" << std::endl;
#ifdef _WIN32
    std::system(("start \"\" " + url).c_str());
#endif

    while (true) {
        sock_t c = accept(listener.s, nullptr, nullptr);
        if (c == INVALID_SOCK) continue;
        Socket client(c);
        setTimeouts(c, 500);   // ignore idle browser connections quickly

        std::string req;
        char buf[2048];
        while (req.find("\r\n\r\n") == std::string::npos && req.size() < 8192) {
            int n = static_cast<int>(recv(c, buf, sizeof buf, 0));
            if (n <= 0) break;
            req.append(buf, n);
        }
        size_t eol = req.find("\r\n");
        if (eol == std::string::npos) continue;

        std::istringstream line(req.substr(0, eol));
        std::string method, target;
        line >> method >> target;
        size_t q = target.find('?');
        std::string path = target.substr(0, q);
        std::string query = (q == std::string::npos) ? "" : target.substr(q + 1);
        sendAll(c, handle(method, path, query));
    }
}
