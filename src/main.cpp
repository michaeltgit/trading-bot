#include "trading/config.hpp"
#include "trading/symbol_worker.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace trading;

std::vector<std::string> parseSymbolList(const std::string& csv) {
    std::vector<std::string> result;
    std::stringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}

int main() {
    Config cfg;
    const char* configPath = "../config.cfg";

    if (!cfg.loadFromFile(configPath)) {
        std::cerr << "Failed to load config file: " << configPath << "\n";
        return 1;
    }

    std::vector<std::string> symbols = parseSymbolList(cfg.getString("symbols"));
    int maxPos = cfg.getInt("risk.maxPosition");

    std::vector<std::unique_ptr<SymbolWorker>> workers;
    for (const auto& sym : symbols) {
        spdlog::info("Starting SymbolWorker for {}", sym);
        auto worker = std::make_unique<SymbolWorker>(sym, maxPos);
        worker->start();
        workers.push_back(std::move(worker));
    }

    spdlog::info("All symbol threads started.");

    for (auto& worker : workers) {
        worker->join();
    }

    return 0;
}