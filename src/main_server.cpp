#include "server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <riddles_file.csv>\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
    std::string riddlesFile = argv[2];

    if (!Server::start(port, riddlesFile)) {
        std::cerr << "Server failed\n";
        return 1;
    }

    return 0;
}
