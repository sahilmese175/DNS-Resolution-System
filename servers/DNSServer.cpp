#include "servers/DNSServer.h"
#include "Platform.h"
#include <random>
#include <stdexcept>

namespace {

const int TIMEOUT_MS = 3000;

void initNetwork() {
#ifdef _WIN32
    static struct Winsock {
        Winsock() { WSADATA d; WSAStartup(MAKEWORD(2, 2), &d); }
        ~Winsock() { WSACleanup(); }
    } instance;
#endif
}

struct Socket {
    sock_t s;
    explicit Socket(sock_t x) : s(x) {}
    ~Socket() { if (s != INVALID_SOCK) CLOSESOCK(s); }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
};

void setTimeouts(sock_t s) {
#ifdef _WIN32
    DWORD tv = TIMEOUT_MS;
#else
    timeval tv{TIMEOUT_MS / 1000, (TIMEOUT_MS % 1000) * 1000};
#endif
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof tv);
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof tv);
}

sockaddr_in makeAddr(const std::string& ip) {
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(53);
    unsigned long addr = inet_addr(ip.c_str());
    if (addr == INADDR_NONE) throw std::runtime_error("invalid server address " + ip);
    a.sin_addr.s_addr = addr;
    return a;
}

std::vector<uint8_t> exchangeUDP(const std::string& ip, const std::vector<uint8_t>& q) {
    initNetwork();
    Socket sock(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (sock.s == INVALID_SOCK) throw std::runtime_error("cannot create socket");
    setTimeouts(sock.s);
    sockaddr_in a = makeAddr(ip);
    if (sendto(sock.s, reinterpret_cast<const char*>(q.data()), static_cast<int>(q.size()), 0,
               reinterpret_cast<sockaddr*>(&a), sizeof a) < 0)
        throw std::runtime_error("send failed");
    std::vector<uint8_t> buf(4096);
    int n = static_cast<int>(recvfrom(sock.s, reinterpret_cast<char*>(buf.data()),
                                      static_cast<int>(buf.size()), 0, nullptr, nullptr));
    if (n <= 0) throw std::runtime_error("no reply (timeout)");
    buf.resize(n);
    return buf;
}

void recvAll(sock_t s, uint8_t* dst, size_t len) {
    size_t got = 0;
    while (got < len) {
        int n = static_cast<int>(recv(s, reinterpret_cast<char*>(dst + got), static_cast<int>(len - got), 0));
        if (n <= 0) throw std::runtime_error("TCP read failed");
        got += n;
    }
}

std::vector<uint8_t> exchangeTCP(const std::string& ip, const std::vector<uint8_t>& q) {
    initNetwork();
    Socket sock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (sock.s == INVALID_SOCK) throw std::runtime_error("cannot create socket");
    setTimeouts(sock.s);
    sockaddr_in a = makeAddr(ip);
    if (connect(sock.s, reinterpret_cast<sockaddr*>(&a), sizeof a) != 0)
        throw std::runtime_error("TCP connect failed");

    std::vector<uint8_t> framed{static_cast<uint8_t>(q.size() >> 8), static_cast<uint8_t>(q.size() & 0xFF)};
    framed.insert(framed.end(), q.begin(), q.end());
    if (send(sock.s, reinterpret_cast<const char*>(framed.data()), static_cast<int>(framed.size()), 0) < 0)
        throw std::runtime_error("TCP send failed");

    uint8_t lenBytes[2];
    recvAll(sock.s, lenBytes, 2);
    std::vector<uint8_t> buf((lenBytes[0] << 8) | lenBytes[1]);
    recvAll(sock.s, buf.data(), buf.size());
    return buf;
}

}  // namespace

DNSServer::DNSServer(std::string name, std::string ip) : name_(std::move(name)), ip_(std::move(ip)) {}

std::string DNSServer::describe() const {
    return role() + " " + name_ + " (" + ip_ + ")";
}

DNSResponse DNSServer::query(const std::string& domain, uint16_t type) const {
    static std::mt19937 rng{std::random_device{}()};
    uint16_t id = static_cast<uint16_t>(rng() & 0xFFFF);
    std::vector<uint8_t> q = DNSPacket::buildQuery(id, domain, type);

    DNSResponse r = DNSPacket::parse(exchangeUDP(ip_, q));
    if (r.truncated) r = DNSPacket::parse(exchangeTCP(ip_, q));
    if (r.id != id) throw std::runtime_error("response ID mismatch");
    return r;
}
