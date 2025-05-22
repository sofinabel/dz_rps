#pragma once

#include <string>
#include <cstdint>

namespace Client {

bool start(const std::string &serverIp,
           uint16_t port,
           const std::string &playerName,
           bool testMode);

}

