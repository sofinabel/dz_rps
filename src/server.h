#pragma once

#include "protocol.h"

namespace Server {

bool start(uint16_t port, const std::string &riddleFilePath, bool testMode = false);

}

