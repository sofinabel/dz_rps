#include "client.h"
#include "protocol.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <limits>  // для std::numeric_limits
#include <ios>      // для std::streamsize
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Client {

bool start(const std::string &serverIp, uint16_t port, const std::string &playerName, bool testMode) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    // отправка MSG_REQUEST
    Protocol::Packet requestPkt;
    requestPkt.hdr.msg_type = Protocol::MSG_REQUEST;
    requestPkt.hdr.seq_num = 1;
    requestPkt.hdr.flag_test = testMode ? 1 : 0;
    requestPkt.payload = playerName;
    if (!Protocol::sendReliable(sockfd, serverAddr, requestPkt)) {
        std::cerr << "Не удалось отправить запрос\n";
        close(sockfd);
        return false;
    }

    // ожидание загадки
    Protocol::Packet riddlePkt;
    sockaddr_in replyAddr;
    if (!Protocol::recvReliable(sockfd, riddlePkt, replyAddr)) {
        std::cerr << "Не удалось получить загадку\n";
        close(sockfd);
        return false;
    }

    std::cout << "Загадка: " << riddlePkt.payload << "\n";

    int seq = 2;
    int hintUsed = 0;

    while (true) {
        std::cout << "\n> ";
        std::string cmd;
        std::getline(std::cin, cmd);
        std::cout << cmd << std::endl; // Выводим введенную строку
        
        Protocol::Packet pkt;
        pkt.hdr.seq_num = seq++;
        pkt.hdr.flag_test = testMode ? 1 : 0;

        // Используем точные проверки
        if (cmd.rfind("это ", 0) == 0) { // Начинается с "это "
            pkt.hdr.msg_type = Protocol::MSG_GUESS;
            pkt.payload = cmd.substr(7);  // Обрезаем первые 4 символа
            std::cout <<  pkt.payload << std::endl; // Выводим введенную строку
        }
        else if (cmd == "дай подсказку") { // Точное совпадение
            pkt.hdr.msg_type = Protocol::MSG_HINT;
        }
        else if (cmd == "сдаюсь") { // Точное совпадение
            pkt.hdr.msg_type = Protocol::MSG_GIVEUP;
        
        } else {
            std::cout << "Неизвестная команда\n";
            continue;
        }

        if (!Protocol::sendReliable(sockfd, serverAddr, pkt)) {
            std::cerr << "Ошибка при отправке\n";
            continue;
        }

        Protocol::Packet reply;
        if (!Protocol::recvReliable(sockfd, reply, replyAddr)) {
            std::cerr << "Ошибка при получении\n";
            continue;
        }

        std::cout << "Сервер: " << reply.payload << "\n";

        if (reply.payload == "верно" || reply.payload.rfind("вы проиграли", 0) == 0 ||
            reply.payload.rfind("ответ — ", 0) == 0) {
            break;
        }
    }

    close(sockfd);
    return true;
}

}
