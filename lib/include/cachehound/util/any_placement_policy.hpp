#ifndef CACHEHOUND_UTIL_ANY_PLACEMENT_POLICY_HPP
#define CACHEHOUND_UTIL_ANY_PLACEMENT_POLICY_HPP

#include <cstddef>
#include <concepts>
#include <memory>

#include "../concepts/placement_policy.hpp"

namespace cachehound {

class any_placement_policy {
    struct base_class {
        virtual ~base_class() = default;
        virtual std::uint8_t index_bits() const noexcept = 0;
        virtual std::size_t operator()(std::uintptr_t) const noexcept = 0;

        virtual std::unique_ptr<base_class> copy() const = 0;
    };

    template<placement_policy Policy>
        requires (std::copy_constructible<Policy> and std::move_constructible<Policy>)
    class impl_class : public base_class {
        Policy policy_;

    public:
        impl_class(const Policy& policy) : policy_(policy) {}
        impl_class(Policy&& policy) : policy_(std::move(policy)) {}

        std::uint8_t index_bits() const noexcept override {
            return policy_.index_bits();
        }

        std::size_t operator()(std::uintptr_t address) const noexcept override {
            return policy_(address);
        }

        std::unique_ptr<base_class> copy() const override {
            return std::make_unique<impl_class>(policy_);
        }
    };

    std::unique_ptr<base_class> base_;

public:
    any_placement_policy(const any_placement_policy& other)
        : base_(other.base_->copy()) {}

    any_placement_policy(any_placement_policy&&) = default;

    template<placement_policy Policy>
        requires (not std::same_as<Policy, any_placement_policy>)
    any_placement_policy(Policy policy)
        : base_(std::make_unique<impl_class<Policy>>(std::move(policy))) {}

    std::uint8_t index_bits() const noexcept {
        return base_->index_bits();
    }

    std::size_t operator()(std::uintptr_t address) const noexcept {
        return base_->operator()(address);
    }
};
static_assert(placement_policy<any_placement_policy>);

}

#endif /* CACHEHOUND_UTIL_ANY_PLACEMENT_POLICY_HPP */
