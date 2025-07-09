#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "trading/execution_report.hpp"
#include "trading/new_order.hpp"

namespace trading {

class RiskManager {
public:
    using RejectCallback = std::function<void(const NewOrder&)>;

    explicit RiskManager(double maxPosition);
    ~RiskManager();

    void setRejectCallback(RejectCallback cb);
    bool approveOrder(const NewOrder& order);
    void onExecutionReport(const ExecutionReport& rpt);

    double getPosition(const std::string& symbol) const;

private:
    double maxPosition_;
    std::unordered_map<std::string, double> positions_;
    RejectCallback rejectCb_;
};

} // namespace trading