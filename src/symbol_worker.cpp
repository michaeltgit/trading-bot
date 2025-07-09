#include "trading/symbol_worker.hpp"

#include <spdlog/spdlog.h>

namespace trading {

SymbolWorker::SymbolWorker(std::string symbol, int maxPosition)
    : symbol_(std::move(symbol)),
      mdc_(symbol_),
      riskManager_(maxPosition) {
    engine_.setOrderBook(&mdc_.getOrderBook());

    engine_.setCallback([this](const ExecutionReport& rpt) {
        riskManager_.onExecutionReport(rpt);
        if (callback_) {
            callback_(rpt);
        }
    });

    mdc_.setCallback([this](const MarketDataUpdate&) {
        notifyData();
        if (dataCb_) {
            dataCb_();
        }
    });
}

SymbolWorker::~SymbolWorker() {
    stop();
}

void SymbolWorker::start() {
    running_ = true;
    thread_ = std::thread(&SymbolWorker::run, this);
}

void SymbolWorker::stop() {
    running_ = false;
    notifyData();
    mdc_.disconnect();
    engine_.disconnect();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void SymbolWorker::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void SymbolWorker::notifyData() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        dataAvailable_ = true;
    }
    cv_.notify_one();
}

void SymbolWorker::run() {
    spdlog::info("[{}] Starting symbol thread", symbol_);
    mdc_.connect();
    engine_.connect("sim");

    while (running_) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !running_ || dataAvailable_; });

        if (!running_) break;

        dataAvailable_ = false;
        lock.unlock();

        mdc_.poll();
    }

    spdlog::info("[{}] Exiting symbol thread", symbol_);
}

LimitOrderBook& SymbolWorker::getOrderBook() {
    return mdc_.getOrderBook();
}

void SymbolWorker::sendOrder(const NewOrder& order) {
    engine_.sendOrder(order);
}

std::string SymbolWorker::getSymbol() const {
    return symbol_;
}

void SymbolWorker::setCallback(std::function<void(const ExecutionReport&)> cb) {
    callback_ = std::move(cb);
}

void SymbolWorker::setDataCallback(std::function<void()> cb) {
    dataCb_ = std::move(cb);
}

} // namespace trading