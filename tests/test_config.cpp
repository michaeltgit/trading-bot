#include "trading/config.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>

int main() {
    using namespace trading;

    const char *path = "tmp.cfg";

    {
        std::ofstream out(path);
        if (!out) {
            std::cerr << "[ERROR] Failed to create test config file\n";
            return 1;
        }

        out << "foo=bar\n";
        out << "num=42\n";
        out << "pi=3.14\n";
    }

    Config cfg;
    assert(cfg.loadFromFile(path) && "[FAIL] Could not load config file");

    assert(cfg.getString("foo") == "bar" && "[FAIL] String lookup incorrect");
    assert(cfg.getInt("num") == 42 && "[FAIL] Int lookup incorrect");
    assert(std::abs(cfg.getDouble("pi") - 3.14) < 1e-6 && "[FAIL] Double lookup incorrect");

    bool threw = false;
    try {
        cfg.getString("missing");
    } catch (...) {
        threw = true;
    }

    assert(threw && "[FAIL] Missing key did not throw");

    std::remove(path);
    spdlog::info("[PASS] Config tests succeeded.");
    return 0;
}