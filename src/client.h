/**
 * @file client.h
 * @brief Клиентская часть: подключение к серверу, консольный интерфейс игры
 */

#pragma once

#include <string>
#include <cstdint>

namespace Client {

/**
 * @brief Запускает клиент, устанавливает связь с сервером и ведёт игру
 * @param serverIp IP-адрес сервера
 * @param port Порт сервера
 * @param playerName Имя игрока (отправляется в MSG_REQUEST)
 * @param testMode Безлимитный режим, если true
 * @return true при нормальному завершению игры, false при сетевой ошибке
 */
bool start(const std::string &serverIp,
           uint16_t port,
           const std::string &playerName,
           bool testMode);

}

