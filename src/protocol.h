#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>  // sockaddr_in

namespace Protocol {
    /*типы сообщений прикладного уровня*/
    enum MessageType : uint16_t {
        MSG_REQUEST  = 1,   // клиент просит загадку
        MSG_RIDDLE   = 2,   // сервер высылает загадку
        MSG_GUESS    = 3,   // клиент отправляет вариант ответа
        MSG_HINT     = 4,   // клиент просит подсказку
        MSG_REPLY    = 5,   // сервер отвечает на ход
        MSG_GIVEUP   = 6,   // клиент сдался
        MSG_ACK      = 7    // подтверждение приёма
    };

    /*заголовок пакета*/
    struct Header {
        uint16_t msg_type;   //тип сообщения
        uint16_t seq_num;    //порядковый номер пакета
        uint16_t flag_test;  //0-релиз, 1-тестовый режим

    };

    /*пакет = заголовок + данные*/
    struct Packet {
        Header hdr;               //заголовок
        std::string payload;      //данные

    };

    /*сериализация/десереализация*/
    std::vector<uint8_t> serialize(const Packet &pkt);              //пакет -> байты
    Packet            deserialize(const uint8_t *buf, size_t len);  //байты -> пакет

    // Отправка и прием "сырых" UDP-байтов
    ssize_t sendRaw(int sockfd, const sockaddr_in &dst, const uint8_t *buf, size_t len);
    ssize_t recvRaw(int sockfd, uint8_t *buf, size_t maxlen, sockaddr_in &src);

    bool sendReliable(int sockfd,
        const sockaddr_in &dst,
        const Packet &pkt,
        int maxRetries = 5,
        int timeoutMs  = 500);
    // Надежный прием (и автоматическая отправка ACK)
    bool recvReliable(int sockfd,
            Packet &outPkt,
            sockaddr_in &src,
            int timeoutMs = 0);

    
}