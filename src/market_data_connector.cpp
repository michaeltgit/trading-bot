#include "trading/market_data_connector.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

namespace trading {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace ssl = boost::asio::ssl;
using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class MarketDataConnector::BinanceWSClient {
public:
    BinanceWSClient(MarketDataConnector& parent, const std::string& uri, const std::string& symbol)
        : parent_(parent),
          symbol_(symbol),
          ctx_(ssl::context::tlsv12_client),
          resolver_(ioc_),
          ws_(ioc_, ctx_),
          running_(false),
          uri_(uri) {}

    void start() {
        running_ = true;
        thread_ = std::thread([this]() { run(); });
    }

    void stop() {
        running_ = false;
        boost::system::error_code ec;
        ws_.next_layer().shutdown(ec);
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    void run() {
        try {
            std::smatch m;
            std::regex re(R"(wss://([^/:]+)(?::(\d+))?/ws/(.+))");
            if (!std::regex_match(uri_, m, re)) {
                throw std::runtime_error("Invalid WebSocket URI: " + uri_);
            }

            std::string host = m[1].str();
            std::string port = m[2].matched ? m[2].str() : "443";
            std::string path = "/ws/" + m[3].str();

            std::string snapshot_url = "https://api.binance.us/api/v3/depth?symbol=" + symbol_ + "&limit=1000";
            CURL* curl = curl_easy_init();
            std::string readBuffer;

            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, snapshot_url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "trading-bot/1.0");
                CURLcode res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                if (res != CURLE_OK) {
                    throw std::runtime_error("Snapshot fetch failed for " + symbol_);
                }
            }

            auto snap = json::parse(readBuffer);
            std::vector<OrderBookEntry> bids, asks;
            for (const auto& b : snap["bids"]) {
                bids.push_back({std::stod(b[0].get<std::string>()), std::stod(b[1].get<std::string>())});
            }
            for (const auto& a : snap["asks"]) {
                asks.push_back({std::stod(a[0].get<std::string>()), std::stod(a[1].get<std::string>())});
            }
            parent_.getOrderBook().resetTopLevels(bids, asks);

            ctx_.set_default_verify_paths();
            auto results = resolver_.resolve(host, port);
            boost::asio::connect(ws_.next_layer().next_layer(), results.begin(), results.end());
            ws_.next_layer().handshake(ssl::stream_base::client);

            ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
                req.set(boost::beast::http::field::host, "stream.binance.com");
                req.set(boost::beast::http::field::user_agent, "trading-bot/1.0");
            }));

            ws_.handshake(host, path);

            while (running_) {
                boost::beast::multi_buffer buffer;
                ws_.read(buffer);
                auto text = boost::beast::buffers_to_string(buffer.data());
                auto j = json::parse(text);

                if (j.contains("b")) {
                    for (const auto& b : j["b"]) {
                        double price = std::stod(b[0].get<std::string>());
                        double size = std::stod(b[1].get<std::string>());
                        parent_.getOrderBook().onUpdate(Side::Bid, price, size);
                        parent_.emit({symbol_, price, size, true, "BinanceUS"});
                    }
                }

                if (j.contains("a")) {
                    for (const auto& a : j["a"]) {
                        double price = std::stod(a[0].get<std::string>());
                        double size = std::stod(a[1].get<std::string>());
                        parent_.getOrderBook().onUpdate(Side::Ask, price, size);
                        parent_.emit({symbol_, price, size, false, "BinanceUS"});
                    }
                }
            }
        } catch (std::exception& e) {
            std::cerr << "WebSocket error for " << symbol_ << ": " << e.what() << std::endl;
        }
    }

    MarketDataConnector& parent_;
    std::string symbol_;
    std::string uri_;
    boost::asio::io_context ioc_;
    ssl::context ctx_;
    tcp::resolver resolver_;
    websocket::stream<ssl::stream<tcp::socket>> ws_;
    std::thread thread_;
    std::atomic<bool> running_;
};

MarketDataConnector::MarketDataConnector(std::string symbol)
    : symbol_(std::move(symbol)), orderBook_(symbol_) {}

MarketDataConnector::~MarketDataConnector() {
    disconnect();
}

void MarketDataConnector::connect() {
    if (!wsClient_) {
        std::string streamSymbol = symbol_;
        std::transform(streamSymbol.begin(), streamSymbol.end(), streamSymbol.begin(), ::tolower);
        std::string uri = "wss://stream.binance.us:9443/ws/" + streamSymbol + "@depth@100ms";
        wsClient_ = std::make_shared<BinanceWSClient>(*this, uri, symbol_);
        wsClient_->start();
    }
}

void MarketDataConnector::disconnect() {
    if (wsClient_) {
        wsClient_->stop();
        wsClient_.reset();
    }
}

void MarketDataConnector::subscribe(const std::string&) {}

void MarketDataConnector::poll() {}

void MarketDataConnector::emit(const MarketDataUpdate& u) {
    if (cb_) {
        cb_(u);
    }
}

void MarketDataConnector::setCallback(Callback cb) {
    cb_ = std::move(cb);
}

LimitOrderBook& MarketDataConnector::getOrderBook() {
    return orderBook_;
}

const std::string& MarketDataConnector::getSymbol() const {
    return symbol_;
}

} // namespace trading