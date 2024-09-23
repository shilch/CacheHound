#ifndef CACHEHOUND_POLICIES_REPLACEMENT_FIFO_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_FIFO_REPLACEMENT_POLICY_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <vector>

#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

class fifo_replacement_policy {
    std::vector<std::size_t> order_;

    void hit(decltype(order_)::iterator it) noexcept {
        std::rotate(order_.begin(), it, it + 1);
    }

public:
    fifo_replacement_policy(std::size_t ways) : order_(ways) {
        assert(ways > 0);
        std::iota(order_.begin(), order_.end(), 0);
    }

    [[nodiscard]] std::size_t ways() const noexcept {
        return order_.size();
    }

    void hit(std::size_t way) noexcept { /* NOOP */ }

    std::size_t miss() noexcept {
        hit(order_.end() - 1);
        return order_.front();
    }
};

static_assert(replacement_policy<fifo_replacement_policy>);

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_FIFO_REPLACEMENT_POLICY_HPP */
