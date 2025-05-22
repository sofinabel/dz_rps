#include "server.h"
#include "riddle.h"
#include "protocol.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <random>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace Server {

bool start(uint16_t port, const std::string &riddleFilePath, bool testMode) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        close(sockfd);
        return false;
    }

    auto riddles = loadRiddles(riddleFilePath);
    if (riddles.empty()) {
        std::cerr << "Нет загадок!\n";
        return false;
    }

    std::cout << "Ожидание игрока...\n";
    Protocol::Packet pkt;
    sockaddr_in clientAddr{};
    if (!Protocol::recvReliable(sockfd, pkt, clientAddr)) return false;

    if (pkt.hdr.msg_type != Protocol::MSG_REQUEST) {
        std::cerr << "Неверный тип первого сообщения\n";
        return false;
    }

    std::string playerName = pkt.payload;
    std::cout << "Игрок: " << playerName << std::endl;

    // Выбор случайной загадки
    std::mt19937 rng(std::random_device{}());
    const Riddle &r = riddles[rng() % riddles.size()];

    Protocol::Packet riddlePkt;
    riddlePkt.hdr.msg_type = Protocol::MSG_RIDDLE;
    riddlePkt.hdr.seq_num = 1;
    riddlePkt.hdr.flag_test = testMode ? 1 : 0;
    riddlePkt.payload = r.question;
    Protocol::sendReliable(sockfd, clientAddr, riddlePkt);

    int attempts = 0;
    int hintUsed = 0;
    const int maxAttempts = testMode ? 100 : 3;

    while (true) {
        Protocol::Packet inPkt;
        if (!Protocol::recvReliable(sockfd, inPkt, clientAddr)) continue;

        Protocol::Packet replyPkt;
        replyPkt.hdr.seq_num = inPkt.hdr.seq_num;
        replyPkt.hdr.msg_type = Protocol::MSG_REPLY;
        replyPkt.hdr.flag_test = testMode ? 1 : 0;

        if (inPkt.hdr.msg_type == Protocol::MSG_GUESS) {
            ++attempts;
            if (inPkt.payload == r.answer) {
                replyPkt.payload = "верно";
                Protocol::sendReliable(sockfd, clientAddr, replyPkt);
                break;
            } else if (attempts >= maxAttempts) {
                replyPkt.payload = "вы проиграли. Ответ: " + r.answer;
                Protocol::sendReliable(sockfd, clientAddr, replyPkt);
                break;
            } else {
                if (testMode) {
                    replyPkt.payload = "нет, осталось бесконечно много попыток";
                } else {
                replyPkt.payload = "нет, осталось " + std::to_string(maxAttempts - attempts) + " попыток";
                }
                Protocol::sendReliable(sockfd, clientAddr, replyPkt);
            }
        } else if (inPkt.hdr.msg_type == Protocol::MSG_HINT) {
            if (hintUsed < r.hints.size()) {
                replyPkt.payload = r.hints[hintUsed++];
            } else {
                replyPkt.payload = "больше нет подсказок";
            }
            Protocol::sendReliable(sockfd, clientAddr, replyPkt);
        } else if (inPkt.hdr.msg_type == Protocol::MSG_GIVEUP) {
            replyPkt.payload = "ответ — " + r.answer;
            Protocol::sendReliable(sockfd, clientAddr, replyPkt);
            break;
        }
    }

    close(sockfd);
    std::cout << "Игра завершена.\n";
    return true;
}

} // namespace Server
