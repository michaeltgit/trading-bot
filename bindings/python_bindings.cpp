#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "trading/config.hpp"
#include "trading/execution_engine.hpp"
#include "trading/execution_report.hpp"
#include "trading/limit_order_book.hpp"
#include "trading/market_data_connector.hpp"
#include "trading/new_order.hpp"
#include "trading/risk_manager.hpp"
#include "trading/symbol_worker.hpp"

namespace py = pybind11;
using namespace trading;

PYBIND11_MODULE(trading_core, m)
{
    py::class_<Config>(m, "Config")
        .def(py::init<>())
        .def("load", &Config::loadFromFile)
        .def("get_string", &Config::getString)
        .def("get_int", &Config::getInt)
        .def("get_double", &Config::getDouble);

    py::enum_<Side>(m, "Side")
        .value("Bid", Side::Bid)
        .value("Ask", Side::Ask);

    py::class_<OrderBookEntry>(m, "OrderBookEntry")
        .def(py::init<double, double>())
        .def_readonly("price", &OrderBookEntry::price)
        .def_readonly("size", &OrderBookEntry::size);

    py::class_<LimitOrderBook>(m, "LimitOrderBook")
        .def(py::init<const std::string &>())
        .def("handle_market_data_update", &LimitOrderBook::handleMarketDataUpdate)
        .def("top_of_book", [](const LimitOrderBook &orderBook, const std::string &, bool isBid)
        {
            return orderBook.topOfBook(isBid ? Side::Bid : Side::Ask);
        })
        .def("depth", &LimitOrderBook::depth)
        .def("reset_top_levels", [](LimitOrderBook &orderBook, const std::vector<OrderBookEntry> &bids, const std::vector<OrderBookEntry> &asks)
        {
            orderBook.resetTopLevels(bids, asks);
        })
        .def("on_update", &LimitOrderBook::onUpdate);

    py::class_<NewOrder>(m, "NewOrder")
        .def(py::init<std::string, std::string, bool, double, double>())
        .def_readonly("orderId", &NewOrder::orderId)
        .def_readonly("symbol", &NewOrder::symbol)
        .def_readonly("isBuy", &NewOrder::isBuy)
        .def_readonly("price", &NewOrder::price)
        .def_readonly("size", &NewOrder::size);

    py::class_<ExecutionReport>(m, "ExecutionReport")
        .def_readonly("orderId", &ExecutionReport::orderId)
        .def_readonly("symbol", &ExecutionReport::symbol)
        .def_readonly("isBuy", &ExecutionReport::isBuy)
        .def_readonly("execPrice", &ExecutionReport::execPrice)
        .def_readonly("execSize", &ExecutionReport::execSize)
        .def_readonly("isFill", &ExecutionReport::isFill);

    py::class_<ExecutionEngine>(m, "ExecutionEngine")
        .def(py::init<>())
        .def("set_callback", &ExecutionEngine::setCallback)
        .def("send_order", &ExecutionEngine::sendOrder)
        .def("connect", &ExecutionEngine::connect)
        .def("disconnect", &ExecutionEngine::disconnect)
        .def("cancel_order", &ExecutionEngine::cancelOrder)
        .def("set_order_book", &ExecutionEngine::setOrderBook);

    py::class_<RiskManager>(m, "RiskManager")
        .def(py::init<int64_t>())
        .def("set_reject_callback", &RiskManager::setRejectCallback)
        .def("approve_order", &RiskManager::approveOrder)
        .def("on_execution_report", &RiskManager::onExecutionReport);

    py::class_<MarketDataUpdate>(m, "MarketDataUpdate")
        .def_readonly("symbol", &MarketDataUpdate::symbol)
        .def_readonly("price", &MarketDataUpdate::price)
        .def_readonly("size", &MarketDataUpdate::size)
        .def_readonly("isBid", &MarketDataUpdate::isBid);

    py::class_<MarketDataConnector>(m, "MarketDataConnector")
        .def(py::init<std::string>())
        .def("connect", &MarketDataConnector::connect)
        .def("disconnect", &MarketDataConnector::disconnect)
        .def("subscribe", &MarketDataConnector::subscribe)
        .def("poll", &MarketDataConnector::poll)
        .def("set_callback", &MarketDataConnector::setCallback)
        .def("get_order_book", &MarketDataConnector::getOrderBook, py::return_value_policy::reference);

    py::class_<SymbolWorker>(m, "SymbolWorker")
        .def(py::init<std::string, int>())
        .def("start", &SymbolWorker::start)
        .def("stop", &SymbolWorker::stop)
        .def("join", &SymbolWorker::join)
        .def("get_order_book", [](SymbolWorker &self) -> LimitOrderBook & {
            return self.getOrderBook();
        }, py::return_value_policy::reference)
        .def("send_order", [](SymbolWorker &self, const NewOrder &order) {
            self.sendOrder(order);
        })
        .def("get_symbol", &SymbolWorker::getSymbol)
        .def("set_callback", &SymbolWorker::setCallback)
        .def("set_data_callback", [](SymbolWorker &self, std::function<void()> cb) {
            self.setDataCallback(cb);
        });
}