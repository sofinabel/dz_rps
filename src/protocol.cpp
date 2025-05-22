#include "protocol.h"
#include <arpa/inet.h>   // htons, ntohs
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

namespace Protocol {

std::vector<uint8_t> serialize(const Packet &pkt) {
    Header hdr = pkt.hdr;
    // приводим к сетевому порядку
    hdr.msg_type  = htons(hdr.msg_type);
    hdr.seq_num   = htons(hdr.seq_num);
    hdr.flag_test = htons(hdr.flag_test);

    std::vector<uint8_t> buf(sizeof(Header) + pkt.payload.size());
    std::memcpy(buf.data(), &hdr, sizeof(Header));
    std::memcpy(buf.data() + sizeof(Header), pkt.payload.data(), pkt.payload.size());
    return buf;
}

Packet deserialize(const uint8_t *buf, size_t len) {
    Packet pkt;
    std::memcpy(&pkt.hdr, buf, sizeof(Header));
    // обратно к хост-байтам
    pkt.hdr.msg_type  = ntohs(pkt.hdr.msg_type);
    pkt.hdr.seq_num   = ntohs(pkt.hdr.seq_num);
    pkt.hdr.flag_test = ntohs(pkt.hdr.flag_test);

    size_t payloadLen = len - sizeof(Header);
    pkt.payload.assign(reinterpret_cast<const char*>(buf + sizeof(Header)), payloadLen);
    return pkt;
}

ssize_t sendRaw(int sockfd, const sockaddr_in &dst, const uint8_t *buf, size_t len) {
    return sendto(sockfd, buf, len, 0, (sockaddr*)&dst, sizeof(dst));
}

ssize_t recvRaw(int sockfd, uint8_t *buf, size_t maxlen, sockaddr_in &src) {
    socklen_t addrlen = sizeof(src);
    return recvfrom(sockfd, buf, maxlen, 0, (sockaddr*)&src, &addrlen);
}

bool sendReliable(int sockfd, const sockaddr_in &dst, const Packet &pkt, int maxRetries, int timeoutMs) {
    auto buf = serialize(pkt);
    // настроим таймаут на приём ACK
    timeval tv{ timeoutMs/1000, (timeoutMs%1000)*1000 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        sendRaw(sockfd, dst, buf.data(), buf.size());

        // ждём ACK
        uint8_t ackBuf[sizeof(Header)];
        sockaddr_in src;
        ssize_t len = recvRaw(sockfd, ackBuf, sizeof(ackBuf), src);
        if (len >= (ssize_t)sizeof(Header)) {
            Packet resp = deserialize(ackBuf, len);
            if (resp.hdr.msg_type == MSG_ACK && resp.hdr.seq_num == pkt.hdr.seq_num) {
                return true;
            }
        }
        // иначе таймаут — повторить
    }
    return false;
}

bool recvReliable(int sockfd, Packet &outPkt, sockaddr_in &src, int /*timeoutMs*/) {
    // бесконечно ждём пакет
    uint8_t buf[1500];
    ssize_t len;
    while (true) {
        len = recvRaw(sockfd, buf, sizeof(buf), src);
        if (len < (ssize_t)sizeof(Header)) continue;

        Packet pkt = deserialize(buf, len);
        // отправляем ACK всегда
        Packet ack;
        ack.hdr.msg_type  = MSG_ACK;
        ack.hdr.seq_num   = pkt.hdr.seq_num;
        ack.hdr.flag_test = 0;
        auto ackBuf = serialize(ack);
        sendRaw(sockfd, src, ackBuf.data(), ackBuf.size());

        // возвращаем первичный пакет вызывающему
        outPkt = pkt;
        return true;
    }
}

} // namespace Protocol
