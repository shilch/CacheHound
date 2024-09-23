#ifndef CACHEHOUND_SIM_UTIL_ANY_SET_ASSOCIATIVE_POLICY_HPP
#define CACHEHOUND_SIM_UTIL_ANY_SET_ASSOCIATIVE_POLICY_HPP

#include "../concepts/set_associative_policy.hpp"

#include <memory>

namespace cachehound::sim {

class any_set_associative_policy {
    struct base_class {
        virtual ~base_class() = default;
        virtual std::uint8_t index_bits() const noexcept = 0;
        virtual std::size_t ways() const noexcept = 0;
        virtual std::size_t get_set(std::uintptr_t) const noexcept = 0;
        virtual void register_hit(std::size_t, std::size_t) noexcept = 0;
        virtual std::size_t register_miss(std::size_t) noexcept = 0;

        virtual std::unique_ptr<base_class> copy() const = 0;
    };

    template<set_associative_policy Policy>
        requires (std::copy_constructible<Policy> and std::move_constructible<Policy>)
    class impl_class : public base_class {
        Policy policy_;

    public:
        impl_class(const Policy& policy) : policy_(policy) {}
        impl_class(Policy&& policy) : policy_(std::move(policy)) {}

        std::uint8_t index_bits() const noexcept override {
            return policy_.index_bits();
        }

        std::size_t ways() const noexcept override {
            return policy_.ways();
        }

        std::size_t get_set(std::uintptr_t address) const noexcept override {
            return policy_.get_set(address);
        }

        void register_hit(std::size_t set, std::size_t way) noexcept override {
            policy_.register_hit(set, way);
        }

        std::size_t register_miss(std::size_t set) noexcept override {
            return policy_.register_miss(set);
        }

        std::unique_ptr<base_class> copy() const override {
            return std::make_unique<impl_class>(policy_);
        }
    };

    std::unique_ptr<base_class> base_;

public:
    any_set_associative_policy(any_set_associative_policy&&) = default;

    any_set_associative_policy(const any_set_associative_policy& other)
        : base_(other.base_->copy()) {}

    template<set_associative_policy Policy>
        requires (not std::same_as<Policy, any_set_associative_policy>)
    any_set_associative_policy(Policy&& policy)
        : base_(std::make_unique<impl_class<Policy>>(std::move(policy))) {}

    std::uint8_t index_bits() const noexcept {
        return base_->index_bits();
    }

    std::size_t ways() const noexcept {
        return base_->ways();
    }

    std::size_t get_set(std::uintptr_t address) const noexcept {
        return base_->get_set(address);
    }

    void register_hit(std::size_t set, std::size_t way) noexcept {
        base_->register_hit(set, way);
    }

    std::size_t register_miss(std::size_t set) noexcept {
        return base_->register_miss(set);
    }

};
static_assert(set_associative_policy<any_set_associative_policy>);


}

#endif /* CACHEHOUND_SIM_UTIL_ANY_SET_ASSOCIATIVE_POLICY_HPP */
