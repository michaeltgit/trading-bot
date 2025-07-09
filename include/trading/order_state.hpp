#pragma once

namespace trading {

enum class OrderState {
    PendingNew,
    Filled,
    Canceled
};

} // namespace trading