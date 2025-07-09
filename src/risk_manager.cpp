#include "trading/risk_manager.hpp"
#include "trading/new_order.hpp"
#include "trading/execution_report.hpp"

#include <utility>

namespace trading {

RiskManager::RiskManager(double maxPosition)
    : maxPosition_(maxPosition) {}

RiskManager::~RiskManager() = default;

void RiskManager::setRejectCallback(RejectCallback cb) {
    rejectCb_ = std::move(cb);
}

bool RiskManager::approveOrder(const NewOrder& order) {
    double current = positions_[order.symbol];
    double delta = order.size * (order.isBuy ? 1.0 : -1.0);
    double projected = current + delta;

    if (projected > maxPosition_ || projected < -maxPosition_) {
        if (rejectCb_) {
            rejectCb_(order);
        }
        return false;
    }

    return true;
}

void RiskManager::onExecutionReport(const ExecutionReport& rpt) {
    if (!rpt.isFill) return;

    double delta = rpt.execSize * (rpt.isBuy ? 1.0 : -1.0);
    positions_[rpt.symbol] += delta;
}

double RiskManager::getPosition(const std::string& symbol) const {
    auto it = positions_.find(symbol);
    return (it != positions_.end()) ? it->second : 0.0;
}

} // namespace trading