#include "client.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port> <player_name> [--test]\n";
        return 1;
    }

    std::string serverIp = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));
    std::string playerName = argv[3];
    bool testMode = (argc > 4 && std::string(argv[4]) == "--test");

    if (!Client::start(serverIp, port, playerName, testMode)) {
        std::cerr << "Client failed\n";
        return 1;
    }

    return 0;
}
