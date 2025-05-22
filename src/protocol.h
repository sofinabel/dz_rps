/**
 * @file protocol.h
 * @brief Реализация сетевого протокола над UDP с подтверждениями (ACK), порядком и тестовым режимом
 *
 * Модуль описывает формат пакетов:
 *  - заголовок @ref Protocol::Header
 *  - типы сообщений @ref Protocol::MessageType
 *  - сериализацию/десериализацию пакета @ref Protocol::Packet
 *  - надёжную отправку/приём с подтверждениями
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>

/**
 * @enum MessageType
 * @brief Типы сообщений прикладного уровня
 */
namespace Protocol {
    enum MessageType : uint16_t {
        MSG_REQUEST  = 1,   /**< Клиент запрашивает начало игры, payload = имя игрока */
        MSG_RIDDLE   = 2,   /**< Сервер отправляет текст загадки, payload = вопрос */
        MSG_GUESS    = 3,   /**< Клиент отправляет догадку, payload = вариант ответа */
        MSG_HINT     = 4,   /**< Клиент запрашивает подсказку, payload пуст */
        MSG_REPLY    = 5,   /**< Сервер отвечает на ход клиента, payload = «верно», «нет…» или подсказка */
        MSG_GIVEUP   = 6,   /**< Клиент сдаётся, payload пуст */
        MSG_ACK      = 7    /**< Подтверждение (ACK) с тем же seq_num, payload пуст */
    };

    /**
    * @struct Header
    * @brief Заголовок каждого UDP-пакета
    *
    * Содержит тип сообщения, номер последовательности для упорядочивания
    * и флаг тестового режима (без ограничения ходов)
    */
    struct Header {
        uint16_t msg_type;   /**< @ref тип сообщения MessageType */
        uint16_t seq_num;    /**< Порядковый номер пакета */
        uint16_t flag_test;  /**< 1-тестовый режим, 0-обычный */

    };

    /**
    * @struct Packet
    * @brief Полный пакет: заголовок + данные
    */
    struct Packet {
        Header hdr;          /**< Заголовок */
        std::string payload; /**< Текстовая часть */
    };

    /**
    * @brief Сериализует Packet в байтовый буфер
    * @param pkt Пакет для отправки
    * @return Вектор байт ready-to-send
    */    
    std::vector<uint8_t> serialize(const Packet &pkt);              //пакет -> байты
    /**
    * @brief Десериализует байтовый буфер в Packet
    * @param buf Буфер, полученный по сети
    * @param len Длина буфера
    * @return Распакованный @ref Packet
    */
    Packet            deserialize(const uint8_t *buf, size_t len);  //байты -> пакет

    /**
    * @brief Отправляет «сырые» байты по UDP
    * @param sockfd Дескриптор UDP-сокета
    * @param dst Адрес получателя
    * @param buf Буфер с данными
    * @param len Длина буфера
    * @return Количество отправленных байт или -1 если ошибка
    */
    ssize_t sendRaw(int sockfd, const sockaddr_in &dst, const uint8_t *buf, size_t len);
    /**
    * @brief Принимает «сырые» байты по UDP
    * @param sockfd Дескриптор UDP-сокета
    * @param buf Буфер для приёма
    * @param maxlen Размер буфера
    * @param src Выходной параметр - адрес отправителя
    * @return Количество принятых байт или -1 on error
    */
    ssize_t recvRaw(int sockfd, uint8_t *buf, size_t maxlen, sockaddr_in &src);

    /**
    * @brief Отправляет @a pkt и дожидается ACK с тем же seq_num
    *        Повторяет до @a maxRetries раз с таймаутом @a timeoutMs
    * @return true при получении ACK, false при ошибке/таймауте
    */
    bool sendReliable(int sockfd,
        const sockaddr_in &dst,
        const Packet &pkt,
        int maxRetries = 5,
        int timeoutMs  = 500);
    
    /**
    * @brief Блокирующий приём пакета, отправка ACK назад
    * @param sockfd Дескриптор UDP-сокета
    * @param outPkt Выходной параметр — принятый пакет
    * @param src Выходной параметр — адрес отправителя
    * @return true при успехе, false при ошибке
    */
    bool recvReliable(int sockfd,
            Packet &outPkt,
            sockaddr_in &src,
            int timeoutMs = 0);

    
}