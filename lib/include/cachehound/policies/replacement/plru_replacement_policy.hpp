#ifndef CACHEHOUND_POLICIES_REPLACEMENT_PLRU_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_PLRU_REPLACEMENT_POLICY_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

class plru_replacement_policy {
    std::size_t tree_height_;
    std::vector<bool> tree_bits_{};

public:
    plru_replacement_policy(std::size_t ways) {
        // ways must be a power of two
        assert(ways != 0 && (ways & (ways - 1)) == 0);

        tree_height_ = std::countr_zero(ways);
        tree_bits_.resize(ways - 1);
        std::fill(tree_bits_.begin(), tree_bits_.end(), false);
    }

    [[nodiscard]] std::size_t ways() const noexcept {
        return 1 << tree_height_;
    }

    void hit(std::size_t way) noexcept {
        std::size_t tree_index = 0;
        for(std::size_t level = 0; level < tree_height_; level++) {
            bool rhs = way & (0b1 << (tree_height_ - level - 1));
            assert(tree_index < tree_bits_.size());
            tree_bits_[tree_index] = rhs;
            tree_index = 2 * tree_index + 1 + rhs;
        }
    }

    std::size_t miss() noexcept {
        std::size_t tree_index = 0;
        std::size_t way = 0;
        for(std::size_t level = 0; level < tree_height_; level++) {
            way <<= 1;
            bool rhs = !tree_bits_[tree_index];
            assert(tree_index < tree_bits_.size());
            tree_bits_[tree_index] = rhs;
            way |= rhs;
            tree_index = 2 * tree_index + 1 + rhs;
        }
        return way;
    }
};

static_assert(replacement_policy<plru_replacement_policy>);

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_PLRU_REPLACEMENT_POLICY_HPP */
