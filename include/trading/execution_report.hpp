#pragma once

#include <string>

namespace trading {

struct ExecutionReport {
    std::string orderId;
    std::string symbol;
    bool isBuy;
    bool isFill;
    double execPrice;
    double execSize;
};

} // namespace trading