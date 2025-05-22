#pragma once

#include <fstream>
#include <sstream>

struct Riddle {
    std::string question;
    std::string answer;
    std::vector<std::string> hints;
};


std::vector<Riddle> loadRiddles(const std::string &filePath) {
    std::vector<Riddle> riddles;
    std::ifstream file(filePath);
    std::string line;

    // Пропускаем строку заголовка
    if (!std::getline(file, line)) return riddles;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Riddle r;
        std::string cell;

        // читаем по ';'
        if (!std::getline(ss, cell, ';')) continue;
        r.question = cell;
        if (!std::getline(ss, cell, ';')) continue;
        r.answer = cell;
        if (!std::getline(ss, cell, ';')) continue;
        r.hints.push_back(cell);
        if (!std::getline(ss, cell, ';')) continue;
        r.hints.push_back(cell);

        riddles.push_back(std::move(r));
    }
    return riddles;
}
